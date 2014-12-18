<?php

/**
 * @file
 * Common functions used for secure file share.
 */

define('SFS_HASH_ALGO', 'sha512');
define('SFS_HASH_COUNT', 15);

/**
 * Function to generate a record to store into the "sfs_files" table.
 */
function _sfs_generate_record($sfs_file, $sfs_values) {
  $record = array();
  $record['sfs_id'] = $sfs_file->fid;
  $record['internal_users'] = serialize($sfs_values['internal_users']);
  if (isset($sfs_values['sfs_password'])) {
    $record['password'] = _password_crypt(SFS_HASH_ALGO, $sfs_values['sfs_password'], _password_generate_salt(SFS_HASH_COUNT));
  }
  $sfs_expiration = '';
  if (isset($sfs_values['expiration_period'])) {
    if ($sfs_values['expiration_period'] == 'date') {
      $sfs_expiration = strtotime($sfs_values['expiration_date']);
    }
    if ($sfs_values['expiration_period'] == 'days') {
      $sfs_expiration = strtotime("+" . $sfs_values['expiration_days'] . "day");
    }
    if ($sfs_values['expiration_period'] == 'hours') {
      $sfs_expiration = strtotime("+" . $sfs_values['expiration_hours'] . "hours");
    }
  }
  $record['expiration'] = $sfs_expiration;
  // Expiration status by default set to 1.
  $record['status'] = 1;
  return $record;
}

/**
 * Function to create a record into the "sfs_files" table.
 */
function _sfs_create_record($sfs_record) {
  if (drupal_write_record('sfs_files', $sfs_record)) {
    drupal_set_message(t("Uploaded files are successfully shared with the user's."));
  }
  else {
    drupal_set_message(t("Problem in sharing the files with the user's. please try again."), 'error');
  }
}

/**
 * Function to delete the directory and its files.
 */
function _sfs_delete_directory($path) {
  return is_file($path) ? @unlink($path) : array_map(__FUNCTION__, glob($path . '/*')) == @rmdir($path);
}

/**
 * Function to notify users about the files shared with them.
 */
function _sfs_notify_file_share($record) {
  global $user;
  $language = language_default();
  $users = unserialize($record['internal_users']);
  $results = db_select('users', 'u')->condition('uid', $users, 'IN')->fields('u', array('mail'))->execute();
  $user_mails = array();
  foreach ($results as $row) {
    $user_mails[] = $row->mail;
  }
  $to = implode(', ', $user_mails);
  $from = $user->mail;
  $params['record'] = $record;
  $message = drupal_mail('sfs', 'notice', $to, $language, $params, $from, FALSE);
  if ($message['send']) {
    drupal_set_message(t("Email notification about the shared file's sent successfully."));
  }
  else {
    drupal_set_message(t('Problem in sending email. Please contact administrator'), 'error');
  }
}

/**
 * Function to replace the tokens add to the email.
 */
function _sfs_replace_email_tokens($email, $params) {
  global $user;
  $record = $params['record'];
  $replacement_keys = array('[sfs:file-shared-by]',
    '[sfs:file-shared-on]',
    '[sfs:file-url]',
    '[sfs:file-password]',
  );
  $replacement_values = array($user->name,
    date('m/d/Y H:i'),
    $record['url'],
    $record['password'],
  );
  $new_email = str_replace($replacement_keys, $replacement_values, $email);
  return $new_email;
}

/**
 * Function to create download link for the file. (Remove if not used)
 */
function _sfs_create_download_link($fid) {
  $file = file_load($fid);
  $key = base64_encode(serialize($file));
  return 'sfs/' . $key . '/download';
}

function _sfs_generate_encrypt_password($password, $salt) {
  include_once DRUPAL_ROOT . '/includes/password.inc';
  //module_load_include('inc', 'sfs', 'sfs.common');
  if (empty($salt)) {
    return _password_crypt(SFS_HASH_ALGO, $password, _password_generate_salt(SFS_HASH_COUNT));
  }
  else {
    return _password_crypt(SFS_HASH_ALGO, $password, $salt);
  }
}

/**
 * Function to create table with the list of replacement patterns.
 *
 * Available for using them in email.
 */
function _sfs_get_replacement_patterns() {
  $header = array('Name', 'Replacement Pattern', 'Description');

  $rows[] = array(t('File Shared By'),
    '[sfs:file-shared-by]',
    t('This contains the name of the user, the file is shared by.'),
  );
  $rows[] = array(t('File Shared On'),
    '[sfs:file-shared-on]',
    t("This contains the date on which the file is shared with user's."),
  );
  $rows[] = array(t('File URL'),
    '[sfs:file-url]',
    t('This contains the url to download the file.'),
  );
  $rows[] = array(t('File Password'),
    '[sfs:file-password]',
    t('This contains the password to download the file.'),
  );
  return theme('table', array('header' => $header, 'rows' => $rows));
}

/**
 * Function to check if the user is a valid internal user to download file.
 */
function _sfs_is_valid_internal_user($fid) {
  global $user;
  $internal_users = unserialize(db_query("SELECT internal_users FROM {sfs_files} WHERE sfs_id = :fid", array(':fid' => $fid))->fetchField());
  $file_owner = db_query("SELECT uid FROM {file_managed} WHERE fid = :fid", array(':fid' => $fid))->fetchField();
  if (is_array($internal_users) && in_array($user->uid, $internal_users) || $user->uid == $file_owner) {
    return TRUE;
  }
  return FALSE;
}

/**
 * Function to check if the expiration set for the file download is still valid.
 */
function _sfs_is_expiration_set($fid) {
  $expiration = db_query("SELECT expiration FROM {sfs_files} WHERE sfs_id = :fid", array(':fid' => $fid))->fetchField();
  if (!empty($expiration)) {
    return $expiration;
  }
  return FALSE;
}

/**
 * Function to check if the file status is active.
 */
function _sfs_is_file_status_active($fid) {
  $status = db_query("SELECT status FROM {sfs_files} WHERE sfs_id = :fid", array(':fid' => $fid))->fetchField();
  if ($status) {
    return TRUE;
  }
  return FALSE;
}

/**
 * Function to check if the password is set for the file.
 */
function _sfs_is_password_set($fid) {
  $password = db_query("SELECT password FROM {sfs_files} WHERE sfs_id = :fid", array(':fid' => $fid))->fetchField();
  if (!empty($password)) {
    return TRUE;
  }
  return FALSE;
}
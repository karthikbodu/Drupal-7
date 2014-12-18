<?php

/**
 * @file
 * Functions used for downloading secure files.
 */

/**
 * Function to download the files.
 */
function sfs_download_files($file) {
  $password = db_query("SELECT password FROM {sfs_files} WHERE sfs_id = :fid", array(':fid' => $file->fid))->fetchField();
  $sfs_path = variable_get('file_public_path', conf_path() . '/files/') . variable_get('sfs_file_path', '');
  if (empty($password)) {
    drupal_add_http_header('Pragma', 'public');
    drupal_add_http_header('Expires', '0');
    drupal_add_http_header('Cache-Control', 'must-revalidate, post-check=0, pre-check=0');
    drupal_add_http_header('Content-Type', $file->filemime);
    drupal_add_http_header('Content-Disposition', 'attachment; filename=' . $file->filename . ';');
    drupal_add_http_header('Content-Transfer-Encoding', 'binary');
    drupal_add_http_header('Content-Length', $file->filesize);
    readfile($sfs_path . '/' . $file->filename);
    drupal_exit();
  }
  else {
	$key = base64_encode(serialize($file));
    drupal_goto('sfs/' . $key . '/secure_download');
  }
}

/**
 * Implements form to verify the password set for the file.
 */
function sfs_verify_password_form($form, &$form_state, $record) {
  $form = array();
  $form['verify_password'] = array(
    '#type' => 'password',
    '#description' => t('Enter the shared password to download the files.'),
  );
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Download'),
  );
  $form['actions']['cancel'] = array(
    '#markup' => l(t('Cancel'), 'admin/config/sfs/dashboard'),
  );
  $form['sfs_file'] = array(
    '#type' => 'hidden',
    '#value' => $record->fid,
  );
  return $form;
}

/**
 * Implements submit handler to validate the password enterd by the user.
 */
function sfs_verify_password_form_submit(&$form, &$form_state) {
  module_load_include('inc', 'sfs', 'sfs.common');
  $values = $form_state['values'];
  $file_id = $values['sfs_file'];
  $password = db_query("SELECT password FROM {sfs_upass} WHERE fid = :fid", array(':fid' => $file_id))->fetchField();
  if (empty($password)) {
    $password = db_query("SELECT password FROM {sfs_files} WHERE sfs_id = :fid", array(':fid' => $file_id))->fetchField();
  }
  $verify_password = _sfs_generate_encrypt_password($values['verify_password'], $password);
  if ($verify_password && $password == $verify_password) {
    $file = file_load($file_id);
    drupal_add_http_header('Pragma', 'public');
    drupal_add_http_header('Expires', '0');
    drupal_add_http_header('Cache-Control', 'must-revalidate, post-check=0, pre-check=0');
    drupal_add_http_header('Content-Type', $file->filemime);
    drupal_add_http_header('Content-Disposition', 'attachment; filename=' . $file->filename . ';');
    drupal_add_http_header('Content-Transfer-Encoding', 'binary');
    drupal_add_http_header('Content-Length', filesize($sfs_path . '/' . $file->filename));
    readfile($sfs_path . '/' . $file->filename);
    drupal_exit();
  }
  else {
    drupal_set_message(t('Entered password did not match. Please enter the correct password.'), 'error');
  }
}
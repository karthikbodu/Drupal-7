<?php

/**
 * @file
 * Administrative pages for listing secure file's shared.
 */

/**
 * Function to generate a table with the list of.
 *
 * Files shared by the logged in user.
 */
function sfs_list_files_sharedby() {
  global $user;

  $header = array(t('File Name'),
    t('File Size(MB)'),
    t('Shared To'),
    t('Valid Till'),
    //t('Status'),
	t('Operations'),
  );

  $query = db_select('sfs_files', 'sf')->extend('PagerDefault');
  $query->join('file_managed', 'fm', 'fm.fid = sf.sfs_id');

  $query->condition('uid', $user->uid, '=');
  $query->fields('fm', array('fid', 'uid', 'filename', 'filesize'));
  $query->fields('sf', array('internal_users', 'expiration', 'status', 'password'));
  $query->orderBy('timestamp', 'DESC');
  $query->limit(10);
  $results = $query->execute();
  $rows = array();
  foreach ($results as $row) {
	// Retrieving user names.
    $user_ids = unserialize($row->internal_users);
    $users = array();
	foreach ($user_ids as $user_id) {
      $users[$user_id] = db_query("SELECT name FROM {users} WHERE uid = :uid", array(':uid' => $user_id))->fetchField();
    }
    $user_names = implode(', ', $users);
    // Retrieving file download expiration.
    if (empty($row->expiration)) {
      $expiration = 'N/A';
    }
    else {
      $expiration = date('m/d/Y H:i:s', $row->expiration);
    }
    // Retrieving file status.
    $status = ($row->status == 0 ? 'Active' : 'Inactive');
	//$file_link = _sfs_create_download_link($row->fid, $row->password);
	$file = file_load($row->fid);
    $key = base64_encode(serialize($file));
    $rows[] = array($row->filename,
      round(($row->filesize) / (1024 * 1024), 2),
      $user_names,
      $expiration,
      l(t('Download'), 'sfs/' . $key . '/download') . ' | ' . l($status, 'sfs/' . $key . '/status') . " | " . l(t('Delete'), 'sfs/' . $key . '/delete') . ' | ' . l(t('Set Expiration'), 'sfs/' . $key . '/expiration') . ' | ' . l(t('Share'), 'sfs/' . $key . '/share') . ' | ' . l(t('Change Password'), 'sfs/' . $key . '/password'),
    );
  }
  $output = theme('table', array('header' => $header, 'rows' => $rows));
  $output .= theme('pager');

  return $output;
}

/**
 * Function to generate a table with the list of.
 *
 * Files shared with the logged in user.
 */
function sfs_list_files_sharedto() {
  global $user;

  $header = array(t('File Name'),
    t('File Size(MB)'),
    t('Shared By'),
    t('Valid Till'),
	t('Status'),
    t('Operations'),
  );

  $query = db_select('sfs_files', 'sf')->extend('PagerDefault');
  $query->join('file_managed', 'fm', 'fm.fid = sf.sfs_id');
  $query->condition('sf.status', 1, '=');
  $query->fields('fm', array('fid', 'uid', 'filename', 'filesize'));
  $query->fields('sf', array('internal_users', 'expiration', 'status', 'password'));
  $query->orderBy('timestamp', 'DESC');
  $query->limit(10);

  $results = $query->execute();

  // Build the table fields.
  $rows = array();
  foreach ($results as $row) {
    // Retrieving user names.
    $user_ids = unserialize($row->internal_users);
    if (in_array($user->uid, $user_ids)) {
      /* foreach ($user_ids as $user_id) {
        $users[$user_id] = db_query("SELECT name FROM {users} WHERE uid = :uid", array(':uid' => $user_id))->fetchField();
      }
      $user_names = implode(', ', $users); */
	  $shared_by = db_select('users', 'u')
	              ->fields('u',array('name'))
				  ->condition('uid', $row->uid)
				  ->execute()
				  ->fetchField();
      // Retrieving file download expiration.
      if (empty($row->expiration)) {
        $expiration = 'N/A';
      }
      else {
        $expiration = date('m/d/Y H:i:s', $row->expiration);
      }
      // Retrieving file status.
      $status = ($row->status == 1 ? 'Active' : 'Deactive');
     //$file_link = _sfs_create_download_link($row->fid, $row->password);
	  // delete link
	  $file = file_load($row->fid);
      $key = base64_encode(serialize($file));
      $rows[] = array($row->filename,
        round(($row->filesize) / (1024 * 1024), 2),
        $shared_by,
        $expiration,
		$status,
        //l($status, 'sfs/toggle/status'),
		l(t('Download'), 'sfs/' . $key . '/download') .  "|" . l(t('Reset Password'), 'sfs/' . $key . '/reset-password'),
		//l($status, 'sfs/toggle/status') . "|" . l(t('Delete'), 'sfs/' . $key . '/delete'),
      );
	}
  }
  $output = theme('table', array('header' => $header, 'rows' => $rows));

  $output .= theme('pager');

  return $output;
}

<?php

/**
 * @file
 * Operation pages for secure file share.
 */

function sfs_file_delete_confirm($form, &$form_state, $file) {
  $form['fid'] = array('#type' => 'value', '#value' => $file->fid);
  return confirm_form($form,
    t('Are you sure you want to delete file %title?', array('%title' => $file->filename)),
	'admin/config/sfs/dashboard/shared-by',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

function sfs_file_delete_confirm_submit($form, &$form_state) {
  if($form_state['values']['confirm']) {
    db_delete('sfs_files')
      ->condition('sfs_id', $form_state['values']['fid'])
      ->execute();
	db_delete('file_managed}')
      ->condition('fid', $form_state['values']['fid'])
      ->execute();
  }
  $form_state['redirect'] = 'admin/config/sfs/dashboard/shared-by';
}

function sfs_file_status_confirm($form, &$form_state, $file) {
  $form['fid'] = array('#type' => 'value', '#value' => $file->fid);
  $status = db_select('sfs_files', 'sf')
	              ->fields('sf',array('status'))
				  ->condition('sfs_id', $file->fid)
				  ->execute()
				  ->fetchField();
  $new_status = ($status == 0 ? 'Active' : 'Inactive');
  $form['new_status'] = array('#type' => 'value', '#value' => $status == 0 ? 1 : 0);
  return confirm_form($form,
    t('Are you sure you want to update file %title status to %status?', array('%title' => $file->filename, '%status' => $new_status)),
	'admin/config/sfs/dashboard/shared-by',
    t('This action cannot be undone.'),
    t('Update'),
    t('Cancel')
  );
}

function sfs_file_status_confirm_submit($form, &$form_state) {
  if($form_state['values']['confirm']) {
    db_update('sfs_files')
       ->fields(array(
         'status' => $form_state['values']['new_status'],
       ))
       ->condition('sfs_id', $form_state['values']['fid'])
       ->execute();
  }
  $form_state['redirect'] = 'admin/config/sfs/dashboard/shared-by';
}

/**
 * Function to create a form to update the expiration period of files.
 */
function sfs_file_update_expiration_form($form, &$form_state, $file) {
  global $user;
  dsm($file);
  
  $expiration = db_select('sfs_files', 'sf')
	              ->fields('sf',array('expiration'))
				  ->condition('sfs_id', $file->fid)
				  ->execute()
				  ->fetchField();
  $current_time = time();
  $expiration_date = date('m/d/Y H:i:s', $expiration);
  if (empty($expiration)) {
    $expiration_message = t('<b>Note : <font color="green">No expiration set for this file.</font></b></br>');
  }
  elseif ($current_time > $expiration) {
    $expiration_message = t('<b>Note : <font color="red">File already expired on %time</font></b></br>', array('%time' => $expiration_date));
  }
  else {
	$expiration_message = t('<b>Note : <font color="red">File expires on %time</font></b></br>', array('%time' => $expiration_date));
  }
  $form = array();
  $form['fid'] = array('#type' => 'value', '#value' => $file->fid);
  $form['fname'] = array('#type' => 'value', '#value' => $file->filename);
  $form['expiration'] = array(
    '#title' => t('Expiration Settings'),
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  $form['expiration']['notice'] = array(
   '#markup' => $expiration_message,
  );
  $form['expiration']['expiration_period'] = array(
    '#title' => t('Expiration Period'),
    '#type' => 'radios',
    '#options' => array(
      'date' => t('Date'),
      'days' => t('Days'),
      'hours' => t('Hours'),
    ),
  );
  $form['expiration']['expiration_date'] = array(
    '#title' => t('Expiration Date'),
    '#type' => 'date_popup',
    '#states' => array(
      // Show the expiration date if the
      // expiration period is selected by date.
      'visible' => array(
	    ':input[name="expiration_period"]' => array('value' => 'date'),
      ),
    ),
  );
  $form['expiration']['expiration_days'] = array(
    '#title' => t('Expiration Days'),
    '#type' => 'textfield',
    '#size' => 20,
    '#states' => array(
      // Show the expiration date if the
      // expiration period is selected by days.
      'visible' => array(
	    ':input[name="expiration_period"]' => array('value' => 'days'),
      ),
    ),
  );
  $form['expiration']['expiration_hours'] = array(
    '#title' => t('Expiration Hours'),
    '#type' => 'textfield',
    '#size' => 20,
    '#states' => array(
       // Show the expiration date if the
       // expiration period is selected by hours.
       'visible' => array(
	     ':input[name="expiration_period"]' => array('value' => 'hours'),
       ),
    ),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
  );
  return $form;
}

/**
 * Function to update the expiration period of the file.
 */
function sfs_file_update_expiration_form_submit(&$form, &$form_state) {
  $values = $form_state['values'];
  if (isset($values['expiration_period'])) {
    if ($values['expiration_period'] == 'date') {
      $sfs_expiration = strtotime($values['expiration_date']);
    }
    if ($values['expiration_period'] == 'days') {
      $sfs_expiration = strtotime("+" . $values['expiration_days'] . "day");
    }
    if ($values['expiration_period'] == 'hours') {
      $sfs_expiration = strtotime("+" . $values['expiration_hours'] . "hours");
    }
  }
  db_update('sfs_files')
       ->fields(array(
         'expiration' => $sfs_expiration,
       ))
       ->condition('sfs_id', $values['fid'])
       ->execute();
  drupal_set_message(t('Expiration period is updateds for the file %file_name', array('%file_name' => $values['fname'])));
  $form_state['redirect'] = 'admin/config/sfs/dashboard/shared-by';
}

/**
 * Function to create a form to update the users of the files shared.
 */
function sfs_file_update_users_form($form, &$form_state, $file) {
  global $user;
  $form = array();
  $form['fid'] = array('#type' => 'value', '#value' => $file->fid);
  $form['fname'] = array('#type' => 'value', '#value' => $file->filename);
  $form['select_users'] = array(
      '#title' => t("User's Selection"),
      '#type' => 'fieldset',
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
    );

    $users = entity_load('user');
    $user_names = array();
    foreach ($users as $user_id => $user_rec) {
      if ($user_id != $user->uid && $user_id != 0 && $user_id != 1) {
        $user_names[$user_id] = $user_rec->name;
      }
    }
	$existing_users = db_select('sfs_files', 'sf')
	              ->fields('sf',array('internal_users'))
				  ->condition('sfs_id', $file->fid)
				  ->execute()
				  ->fetchField();
	$user_ids = unserialize($existing_users);
    $form['select_users']['internal_users'] = array(
      '#type' => 'select',
      '#multiple' => TRUE,
      '#required' => TRUE,
      '#attributes' => array('class' => array('chosen-widget')),
      '#title' => t("Add Internal User's"),
      '#description' => t('Add users available in the system to share the uploaded files.'),
      '#options' => $user_names,
	  '#default_value' => $user_ids,
    );
	$form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
  );
  return $form;
}

/**
 * Function to update the expiration period of the files.
 */
function sfs_file_update_users_form_submit(&$form, &$form_state) {
  $values = $form_state['values'];
  $users = serialize($values['internal_users']);
  dsm($users);
  db_update('sfs_files')
       ->fields(array(
         'internal_users' => $users,
       ))
       ->condition('sfs_id', $values['fid'])
       ->execute();
  drupal_set_message(t('Users are updated for the file %file_name', array('%file_name' => $values['fname'])));
  $form_state['redirect'] = 'admin/config/sfs/dashboard/shared-by';
}

/**
 * Function to create a form to update the password of the files.
 */
function sfs_file_update_password_form($form, &$form_state, $file) {
  global $user;
  $form = array();
  $form['fid'] = array('#type' => 'value', '#value' => $file->fid);
  $form['fname'] = array('#type' => 'value', '#value' => $file->filename);
  $form['password'] = array(
        '#title' => t('Password Settings'),
        '#type' => 'fieldset',
        
      );

      $form['password']['sfs_password'] = array(
        '#type' => 'password_confirm',
		'#required' => TRUE,
        '#description' => t('Choose a password to share the file securely. Enter the same password in both fields'),
      );
$form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
  );
  return $form; 
}

/**
 * Function to update the password of the file.
 */
function sfs_file_update_password_form_submit(&$form, &$form_state) {
  module_load_include('inc', 'sfs', 'sfs.common');
  $values = $form_state['values'];
  if (isset($values['sfs_password'])) {
    $password = _sfs_generate_encrypt_password($values['sfs_password'], '');
  }
  db_update('sfs_files')
       ->fields(array(
         'password' => $password,
       ))
       ->condition('sfs_id', $values['fid'])
       ->execute();
  drupal_set_message(t('Password is updated for the file %file_name', array('%file_name' => $values['fname'])));
  $form_state['redirect'] = 'admin/config/sfs/dashboard/shared-by';
}

/**
 * Function to create a form to reset the password of the files by the user.
 */
function sfs_file_reset_password_form($form, &$form_state, $file) {
  global $user;
  $form = array();
  $form['fid'] = array('#type' => 'value', '#value' => $file->fid);
  $form['fname'] = array('#type' => 'value', '#value' => $file->filename);
  $form['password'] = array(
        '#title' => t('Password Settings'),
        '#type' => 'fieldset',
        
      );

      $form['password']['sfs_password'] = array(
        '#type' => 'password_confirm',
		'#required' => TRUE,
        '#description' => t('Choose a password to share the file securely. Enter the same password in both fields'),
      );
$form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
  );
  return $form; 
}

/**
 * Function to update the password of the file.
 */
function sfs_file_reset_password_form_submit(&$form, &$form_state) {
  global $user;
  module_load_include('inc', 'sfs', 'sfs.common');
  $values = $form_state['values'];
  $record['fid'] = $form_state['values']['fid'];
  $record['uid'] = $user->uid;
  if (isset($values['sfs_password'])) {
    $record['password'] = _sfs_generate_encrypt_password($values['sfs_password'], '');
  }
  drupal_write_record('sfs_upass', $record);
  drupal_set_message(t('Password is updated for the file %file_name', array('%file_name' => $values['fname'])));
  $form_state['redirect'] = 'admin/config/sfs/dashboard/shared-by';
}

<?php

/**
 * @file
 * Configuration pages for secure file share.
 */

/**
 * Function to create a form to share the uploaded files with the users.
 */
function sfs_form($form, &$form_state) {
  global $user;
  $form = array();
  $sfs_size = variable_get('sfs_size', '');
  $sfs_types = variable_get('sfs_types', '');
  if (!empty($sfs_size) && !empty($sfs_types)) {
    $form['upload_settings'] = array(
      '#title' => t('Upload Settings'),
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    );

    $sfs_valid_size = array(1024 * 1024 * ($sfs_size));
    $sfs_valid_types = array(str_replace(',', ' ', $sfs_types));
    $form['upload_settings']['secure_files'] = array(
      '#title' => t('Upload files'),
      '#type' => 'plupload',
      '#required' => TRUE,
      '#upload_validators' => array(
        'file_validate_extensions' => $sfs_valid_types,
        'file_validate_size' => $sfs_valid_size,
      ),
    );
    $form['select_users'] = array(
      '#title' => t("User's Selection"),
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    );

    $users = entity_load('user');
    $user_names = array();
    foreach ($users as $user_id => $user_rec) {
      if ($user_id != $user->uid && $user_id != 0 && $user_id != 1) {
        $user_names[$user_id] = $user_rec->name;
      }
    }
    $form['select_users']['internal_users'] = array(
      '#type' => 'select',
      '#multiple' => TRUE,
      '#required' => TRUE,
      '#attributes' => array('class' => array('chosen-widget')),
      '#title' => t("Add Internal User's"),
      '#description' => t('Add users available in the system to share the uploaded files.'),
      '#options' => $user_names,
    );
	if (variable_get('sfs_external_sharing', '')) {
      $form['select_users']['external_users'] = array(
        '#type' => 'textarea',
        '#title' => t("Add External User's"),
        '#description' => t('Add external users email id\'s separated by comma, to share the uploaded files.'),
      );
	}
    if (variable_get('sfs_password_protection', '')) {
      $form['password'] = array(
        '#title' => t('Password Settings'),
        '#type' => 'fieldset',
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      );

      $form['password']['sfs_password'] = array(
        '#type' => 'password_confirm',
		'#required' => TRUE,
        '#description' => t('Choose a password to share the file securely. Enter the same password in both fields'),
      );
    }
    if (variable_get('sfs_expiration', '')) {
      $form['expiration'] = array(
        '#title' => t('Expiration Settings'),
        '#type' => 'fieldset',
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
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
    }
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Share'),
    );
  }
  else {
    $form['config_message'] = array(
      '#markup' => t('Please !configure the secure file share settings to start sharing the files.', array('!configure' => l(t('configure'), 'admin/config/sfs/settings'))),
    );
  }
  return $form;
}

/**
 * Function to submit the values to the database.
 *
 * Sending out email notifications to the users about the file.
 */
function sfs_form_submit(&$form, &$form_state) {
  module_load_include('inc', 'sfs', 'sfs.common');
  $values = $form_state['values'];
  // Creating the directory for storing the shared files.
  $sfs_path = variable_get('file_public_path', conf_path() . '/files/') . variable_get('sfs_file_path', '');
  file_prepare_directory($sfs_path, FILE_CREATE_DIRECTORY);
  if (!empty($values['secure_files'])) {
    $zip_folder = $sfs_path . '/' . time();
    file_prepare_directory($zip_folder, FILE_CREATE_DIRECTORY);
    $zip = new ZipArchive();
    $zip->open($zip_folder . '.zip', ZipArchive::CREATE);
    foreach ($values['secure_files'] as $sfs_file) {
      $temp_file = $sfs_file['tmppath'];
      $file_name = $sfs_file['name'];
      $file_path = file_unmanaged_move($temp_file, $zip_folder . '/' . $file_name, FILE_EXISTS_RENAME);
      $zip->addFile($file_path, $file_name);
    }
    $zip->close();
    $sfs_file = plupload_file_uri_to_object($zip_folder . '.zip');
    $sfs_file_saved = file_save($sfs_file);
    $sfs_record = _sfs_generate_record($sfs_file_saved, $values);
    _sfs_create_record($sfs_record);
    _sfs_delete_directory($zip_folder);
    $key = base64_encode(serialize($sfs_file_saved));
    if (isset($values['sfs_password']) && !empty($values['sfs_password'])) {
	  $sfs_record['password'] = $values['sfs_password'];
      $sfs_record['url'] = l(t('Click here'), 'sfs/' . $key . '/secure_download');
      _sfs_notify_file_share($sfs_record);
    }
    else {
	  $sfs_record['password'] = '';
      $sfs_record['url'] = l(t('Click here'), 'sfs/' . $key . '/download');
      _sfs_notify_file_share($sfs_record);
    }
  }
}

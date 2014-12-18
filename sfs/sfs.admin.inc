<?php

/**
 * @file
 * Administrative pages for secure file share.
 */

/**
 * Administrative page to configure secure file share settings.
 */
function sfs_settings_form($form, &$form_state) {
  module_load_include('inc', 'sfs', 'sfs.common');
  $form = array();
  $form['sfs_size'] = array(
    '#title' => t('File Size'),
    '#type' => 'textfield',
    '#size' => 20,
    '#required' => TRUE,
    '#default_value' => variable_get('sfs_size', ''),
    '#description' => t("Choose a size limit for the upload file's in <b>MB</b>."),
  );
  $form['sfs_file_path'] = array(
    '#title' => t("File Path"),
    '#type' => 'textfield',
    '#required' => TRUE,
    '#default_value' => variable_get('sfs_file_path', ''),
    '#description' => t("Enter the file path to store the uploaded files. Provided path will be assumed inside files directory."),
  );
  $form['sfs_types'] = array(
    '#title' => t("File Type's"),
    '#type' => 'textfield',
    '#required' => TRUE,
    '#default_value' => variable_get('sfs_types', ''),
    '#description' => t("Choose allowed file extensions for the upload file's as comma separated values."),
  );
  $form['sfs_external_sharing'] = array(
    '#title' => t('External Sharing'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('sfs_external_sharing', ''),
    '#description' => t("Check this box if you would like to share the file's with external users by adding their email id's."),
  );
  $form['sfs_password_protection'] = array(
    '#title' => t('Password Protection'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('sfs_password_protection', ''),
    '#description' => t("Check this box if you would like to enable password protection for the upload file's."),
  );
  $form['sfs_expiration'] = array(
    '#title' => t('Expiration'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('sfs_expiration', ''),
    '#description' => t("Check this box if you would like to set expiration time for the upload file's."),
  );
  $form['sfs_visibiity'] = array(
    '#title' => t('Visibility'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('sfs_visibiity', ''),
    '#description' => t("Check this box if you would like to make file available for all the users in the system, with no access to download."),
  );
  $form['email_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t("Email Setting's"),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['email_settings']['sfs_mail_subject'] = array(
    '#type' => 'textfield',
    '#title' => t('Subject'),
    '#required' => TRUE,
    '#default_value' => variable_get('sfs_mail_subject', ''),
  );
  $form['email_settings']['sfs_mail_body'] = array(
    '#type' => 'textarea',
    '#title' => t('Body'),
    '#required' => TRUE,
    '#rows' => 10,
    '#default_value' => variable_get('sfs_mail_body', ''),
  );
  $form['email_settings']['replacement_help'] = array(
    '#type' => 'fieldset',
    '#title' => t('Available tokens for replacement'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#weight' => 1,
  );
  $form['email_settings']['replacement_help']['sfs_replacements'] = array(
    '#markup' => _sfs_get_replacement_patterns(),
  );
  return system_settings_form($form);
}



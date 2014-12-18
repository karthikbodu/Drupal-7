<?php

/**
* @file
* Views definitions for user profile views module.
*/
 
/**
* Implements hook_views_data().
*/
function upv_views_data() {
  watchdog("Hello", "hewww");
  $data = array();
  //$data['upv']['table']['group']  = t('UPV');
  $data['upv']['vuid'] = array(
    'title' => t('User Id of the visited user.'),
    'help' => t('Description User Id of the visited user.'),
    'field' => array(
      //'handler' => 'mymodule_handler_handlername',
	  'handler' => 'views_handler_field',
	  'click sortable' => TRUE,
    ),
  );
  return $data;
}
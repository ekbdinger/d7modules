<?php

/**
 * Menu callback. Displays the administration settings.
 */
function slidebox_admin_settings() {
  $form = array();

  $form['slidebox_content_types'] = array(
    '#type' => 'select',
    '#options' => _node_types_build()->names,
    '#multiple' => TRUE,
    '#title' => t('Content types'),
    '#description' => t('Select the content types on which Slidebox should appear'),
    '#default_value' => variable_get('slidebox_content_types', array())
  );

  $form['display'] = array(
    '#type' => 'fieldset',
    '#title' => t('Slidebox Display Options'),
    '#collapsible' => FALSE
  );

  $form['display']['slidebox_show_time'] = array(
    '#title' => t('Slidebox Show Time'),
    '#description' => t('Number of miliseconds during which Slidebox should slide out'),
    '#type' => 'textfield',
    '#default_value' => variable_get('slidebox_show_time', 300)
  );

  $form['display']['slidebox_hide_time'] = array(
    '#title' => t('Slidebox Hide Time'),
    '#description' => t('Number of miliseconds during which Slidebox should slide back in'),
    '#type' => 'textfield',
    '#default_value' => variable_get('slidebox_hide_time', 100)
  );

  return system_settings_form($form);
}


<?php

/**
 * @file
 * Describe the file
 *
 * @author Mathew Winstone <mwinstone@coldfrontlabs.ca>
 */

/**
 * Defines theme_views_view_flexslider().
 */
function theme_views_view_flexslider($vars) {
  $items = array();

  $view = $vars['view'];
  $img_field_name = $vars['img_field_name'];
  foreach ($vars['rows'] as $row) {
    $lang = $row->_field_data[$view->base_field]['entity']->language;
    // omit rows without image field.
    if (!isset($row->_field_data[$view->base_field]['entity']->{$img_field_name})) {
      continue;
    }
    $item = $row->_field_data[$view->base_field]['entity']->{$img_field_name}[$lang][0];
    $items[] = $item;
  }

  return theme('flexslider_container', array(
    'items' => $items,
    'settings' => $vars['options'],
  ));
}


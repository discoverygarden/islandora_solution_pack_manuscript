<?php

/**
 * @file
 * Form functions for associating transforms.
 */

/**
 * Form for editing transform association.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param AbstractObject $object
 *   The object to build the form for.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_manuscript_tei_transform_form(array $form, array &$form_state, AbstractObject $object) {
  module_load_include('inc', 'islandora_manuscript', 'includes/utilities');
  $current_transform = islandora_manuscript_get_transform_pid($object);
  $transforms = islandora_manuscript_get_transforms();

  $transform_options = array();
  foreach ($transforms as $pid => &$transform) {
    $transform_options[$pid] = array(
      'transform' => l(
        isset($transform['label']) ? $transform['label'] : $pid,
        "islandora/object/$pid"
      ),
      'description' => $transform['description'],
    );
  }
  $header = array(
    'transform' => t('Transform'),
    'description' => t('Description'),
  );
  $form['transform_table'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $transform_options,
    '#default_value' => $current_transform,
    '#empty' => t('No transforms ingested.'),
    '#multiple' => FALSE,
  );

  $form['object'] = array(
    '#type' => 'value',
    '#value' => $object->id,
  );

  $form['set_transform'] = array(
    '#type' => 'submit',
    '#value' => t('Set Transform'),
  );
  $form['remove_transform'] = array(
    '#type' => 'submit',
    '#value' => t('Remove Transform'),
  );
  return $form;
}

/**
 * Submit handler for editing transform association.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_manuscript_tei_transform_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_manuscript', 'includes/utilities');
  $values = &$form_state['values'];
  $object = islandora_object_load($values['object']);
  if ($form_state['clicked_button']['#parents'] == array('remove_transform')) {
    islandora_manuscript_remove_transform($object);
  }
  elseif ($form_state['clicked_button']['#parents'] == array('set_transform')) {
    islandora_manuscript_set_transform($object, $values['transform_table']);
  }
}

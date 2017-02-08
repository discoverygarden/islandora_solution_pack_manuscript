<?php

/**
 * @file
 * Handles the display/submission of the admin settings form for this module.
 */

/**
 * Defines the admin settings form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_manuscript_admin_settings_form(array $form, array &$form_state) {
  $get_default_value = function ($name, $default) use (&$form_state) {
    return isset($form_state['values'][$name]) ? $form_state['values'][$name] : variable_get($name, $default);
  };
  $form = array(
    'local_derivative_settings' => array(
      '#type' => 'fieldset',
      '#title' => t('Create Page/Manuscript Derivatives Locally'),
      '#description' => t('These options allow you to create derivatives
        automatically when ingesting pages or issues, using this server. If you
        intend to only use microservices to generate derivatives, you should
        not have any of these settings enabled.'),
      'islandora_manuscript_ingest_derivatives' => array(
        '#type' => 'checkboxes',
        '#element_validate' => array('islandora_manuscript_admin_settings_form_ingest_derivatives_validate'),
        '#options' => array(
          'pdf' => t('PDF datastream. <strong>Requires</strong> <em>ImageMagick</em>'),
          'image' => t('Image datastreams (TN, JPEG, JP2). <strong>Requires</strong> <em>Large Image Solution Pack</em>'),
          'ocr' => t('OCR datastreams (OCR, HOCR). <strong>Requires</strong> <em>Islandora OCR module</em>'),
        ),
        '#default_value' => $get_default_value('islandora_manuscript_ingest_derivatives', drupal_map_assoc(
          array('pdf', 'image', 'ocr'))),
      ),
    ),
    // Metadata display.
    'metadata_display_fieldset' => array(
      '#type' => 'fieldset',
      '#title' => t('Metadata display'),
      'islandora_manuscript_metadata_display' => array(
        '#type' => 'checkbox',
        '#title' => t('Display object metadata'),
        '#description' => t('Display object metadata below the collection display.'),
        '#default_value' => variable_get('islandora_manuscript_metadata_display', FALSE),
      ),
    ),
  );

  // Solr field containing the parent book PID.
  $form['islandora_manuscript_parent_manuscript_solr_field'] = array(
    '#type' => 'textfield',
    '#title' => t('Parent Solr Field'),
    '#description' => t("Solr field containing the parent Manuscript's PID."),
    '#default_value' => variable_get('islandora_manuscript_parent_manuscript_solr_field', 'RELS_EXT_isMemberOf_uri_ms'),
    '#size' => 30,
  );
  $form['islandora_manuscript_box_identifier_solr_field'] = array(
    '#type' => 'textfield',
    '#title' => t('Manuscript Box Solr Field'),
    '#description' => t("Solr field containing the box identifier, on manuscript objects."),
    '#default_value' => variable_get('islandora_manuscript_box_identifier_solr_field', 'mods_relatedItem_host_part_detail_box_number_ms'),
    '#size' => 30,
  );
  $form['islandora_manuscript_folder_identifier_solr_field'] = array(
    '#type' => 'textfield',
    '#title' => t('Manuscript Folder Solr Field'),
    '#description' => t("Solr field containing the folder identifier, on manuscript objects."),
    '#default_value' => variable_get('islandora_manuscript_folder_identifier_solr_field', 'mods_relatedItem_host_part_detail_folder_number_ms'),
    '#size' => 30,
  );
  $form['islandora_manuscript_component_identifier_solr_field'] = array(
    '#type' => 'textfield',
    '#title' => t('Component ID Solr Field'),
    '#description' => t("Solr field containing the identifier, on manuscript objects."),
    '#default_value' => variable_get('islandora_manuscript_component_identifier_solr_field', 'dereffed_ead_component_id_ms'),
    '#size' => 30,
  );

  module_load_include('inc', 'islandora', 'includes/solution_packs');
  $form += islandora_viewers_form('islandora_manuscript_viewers', NULL, 'islandora:manuscriptCModel');
  $form['manuscript_viewers'] = $form['viewers'];
  $form['manuscript_viewers']['#title'] = t('Manuscript Viewers');
  unset($form['viewers']);
  $form += islandora_viewers_form('islandora_manuscript_page_viewers', 'image/jp2');
  $form['page_viewers'] = $form['viewers'];
  $form['page_viewers']['#title'] = t('Page Viewers');
  unset($form['viewers']);

  return system_settings_form($form);
}

/**
 * Check if the required resources are enabled.
 *
 * @param array $element
 *   The element to check.
 * @param array $form_state
 *   The Drupal form state.
 * @param array $form
 *   The Drupal form definition.
 */
function islandora_manuscript_admin_settings_form_ingest_derivatives_validate(array $element, array &$form_state, array $form) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  // form_error() doesn't seem to work here, it prevents the submit but
  // also prevents the value from being set. So we are using
  // drupal_set_message().
  if (array_key_exists('pdf', $element['#value']) && !islandora_paged_content_can_create_pdf()) {
    $element['#value']['pdf'] = 0;
    form_set_value($element, $element['#value'], $form_state);
    drupal_set_message(t('ImageMagick must be enabled to derive the PDF datastream.'), 'error');
  }
  if (array_key_exists('image', $element['#value']) && !islandora_paged_content_can_create_images()) {
    $element['#value']['image'] = 0;
    form_set_value($element, $element['#value'], $form_state);
    drupal_set_message(t('The Large Image Solution Pack must be enabled to derive image datastreams.'), 'error');
  }
  if (array_key_exists('ocr', $element['#value']) && !(module_exists('islandora_ocr'))) {
    $element['#value']['ocr'] = 0;
    form_set_value($element, $element['#value'], $form_state);
    drupal_set_message(t('The Islandora OCR module must be enabled to derive OCR datastreams.'), 'error');
  }
}

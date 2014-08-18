<?php

/**
 * @file
 * Handles the uploading of a TEI file. TEI schema's are often heavily
 * customized so we don't perform any validation as we do with the EAD.
 */

/**
 * Defines a file upload form for uploading the TEI document.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 *
 * @return array
 *   The drupal form definition.
 */
function islandora_manuscript_tei_upload_form(array $form, array &$form_state) {
  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  $extensions = array('xml');
  return array(
    'file' => array(
      '#title' => t('TEI (Text Encoding Initiative)'),
      '#type' => 'managed_file',
      '#required' => FALSE,
      '#description' => filter_xss(t('Select xml file to upload.<br/>Files must be less than <b>!size MB.</b><br/>Allowed file types: <b>!ext.</b>', array('!size' => $upload_size, '!ext' => $extensions[0])), array('br', 'b')),
      '#default_value' => isset($form_state['values']['file']) ? $form_state['values']['file'] : NULL,
      '#upload_location' => 'temporary://',
      '#upload_validators' => array(
        'file_validate_extensions' => $extensions,
        // Assume its specified in MB.
        'file_validate_size' => array($upload_size * 1024 * 1024),
      ),
    ),
  );
}

/**
 * Validation to see if an XML file is well-formed without schema checking.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_manuscript_tei_upload_form_validate(array $form, array &$form_state) {
  if ($form_state['values']['file']) {
    $file = file_load($form_state['values']['file']);
    libxml_use_internal_errors(TRUE);
    // Using drupal_realpath because loading URI's seems to screw up on files
    // with spaces otherwise.
    simplexml_load_file(drupal_realpath($file->uri));
    $errors = count(libxml_get_errors());
    libxml_use_internal_errors(FALSE);
    if ($errors) {
      form_error($form['file'], t('The uploaded file @file is not well-formed XML.', array('@file' => $file->filename)));
    }
  }
}

/**
 * Submit handler, adds uploaded file to ingest object.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_manuscript_tei_upload_form_submit(array $form, array &$form_state) {
  $object = islandora_ingest_form_get_object($form_state);
  $file = file_load($form_state['values']['file']);
  if ($file) {
    islandora_manuscript_ingest_tei_datastream($object, $file);
  }
}

/**
 * Ingest the given file as the given objects TEI datastream.
 *
 * @param AbstractObject $object
 *   The object which will ingest the given TEI file.
 * @param object $file
 *   A drupal file containing the TEI content to be ingested as a datastream.
 */
function islandora_manuscript_ingest_tei_datastream(AbstractObject $object, $file) {
  if (empty($object['TEI'])) {
    $ds = $object->constructDatastream('TEI', 'M');
    $object->ingestDatastream($ds);
  }
  else {
    $ds = $object['TEI'];
  }
  $path = drupal_realpath($file->uri);
  $ds->setContentFromFile($path, FALSE);
  $ds->label = $file->filename;
  $ds->mimetype = 'text/xml';
}

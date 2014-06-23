<?php

/**
 * @file
 * Handles the uploading of a XSLT and CSS files.
 */

/**
 * Defines a file upload form for uploading the XSLT and CSS files.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 *
 * @return array
 *   The drupal form definition.
 */
function islandora_manuscript_transform_upload_form(array $form, array &$form_state) {
  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  $xslt_extensions = array('xsl', 'xslt');
  $css_extensions = array('css');
  return array(
    'label' => array(
      '#title' => t('Label'),
      '#type' => 'textfield',
      '#description' => t('This will be used as the display label for this object.'),
      '#required' => TRUE,
    ),
    'description' => array(
      '#title' => t('Description'),
      '#type' => 'textarea',
      '#description' => t('Please describe what this transform does to TEI.'),
      '#required' => TRUE,
    ),
    // There is no standard W3C schema for xslt files, so we don't validate it.
    'xslt' => array(
      '#title' => t('EXtensible Stylesheet Transform (XSLT 1.0 Only)'),
      '#type' => 'managed_file',
      '#required' => TRUE,
      '#description' => filter_xss(t('Select transform file to upload.<br/>Files must be less than <b>!size MB.</b><br/>Allowed file types: <b>!ext.</b>', array('!size' => $upload_size, '!ext' => $xslt_extensions[0])), array('br', 'b')),
      '#default_value' => isset($form_state['values']['xslt']) ? $form_state['values']['xslt'] : NULL,
      '#upload_location' => 'temporary://',
      '#upload_validators' => array(
        'file_validate_extensions' => $xslt_extensions,
        // Assume its specified in MB.
        'file_validate_size' => array($upload_size * 1024 * 1024),
      ),
    ),
    'css' => array(
      '#title' => t('Cascading Style Sheet (CSS)'),
      '#type' => 'managed_file',
      '#required' => FALSE,
      '#description' => filter_xss(t('Select transform file to upload.<br/>Files must be less than <b>!size MB.</b><br/>Allowed file types: <b>!ext.</b>', array('!size' => $upload_size, '!ext' => $css_extensions[0])), array('br', 'b')),
      '#default_value' => isset($form_state['values']['css']) ? $form_state['values']['css'] : NULL,
      '#upload_location' => 'temporary://',
      '#upload_validators' => array(
        'file_validate_extensions' => $css_extensions,
        // Assume its specified in MB.
        'file_validate_size' => array($upload_size * 1024 * 1024),
      ),
    ),
  );
}

/**
 * Submit handler, adds uploaded file(s) to ingest object.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_manuscript_transform_upload_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora', 'includes/dublin_core');
  $values = $form_state['values'];
  $object = islandora_ingest_form_get_object($form_state);
  // Update the object's label / DC datastream.
  $object->label = $values['label'];
  islandora_manuscript_transform_ingest_dc_datastream($object, $values['label'], $values['description']);
  // Set the XSLT datastream.
  $xslt_file = file_load($values['xslt']);
  islandora_manuscript_ingest_xslt_datastream($object, $xslt_file);
  // CSS Datastream is optional.
  $css_file = file_load($values['css']);
  if ($css_file) {
    islandora_manuscript_ingest_css_datastream($object, $css_file);
  }
}

/**
 * Creates a minimal DC datastream from the title and description.
 */
function islandora_manuscript_transform_ingest_dc_datastream(AbstractObject $object, $title, $description) {
  if (empty($object['DC'])) {
    $ds = $object->constructDatastream('DC', 'X');
    $object->ingestDatastream($ds);
  }
  else {
    $ds = $object['DC'];
  }
  $ds->content = <<<EOT
<oai_dc:dc xmlns:oai_dc="http://www.openarchives.org/OAI/2.0/oai_dc/" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.openarchives.org/OAI/2.0/oai_dc/ http://www.openarchives.org/OAI/2.0/oai_dc.xsd">
  <dc:title>{$title}</dc:title>
  <dc:description>{$description}</dc:description>
</oai_dc:dc>
EOT;
  $ds->label = 'Dublin Core Record for this object';
  $ds->mimetype = 'text/xml';
}

/**
 * Ingest the given file as the given objects XSLT datastream.
 *
 * @param AbstractObject $object
 *   The object which will ingest the given XSLT file.
 * @param object $file
 *   A drupal file containing the XSLT content to be ingested as a datastream.
 */
function islandora_manuscript_ingest_xslt_datastream(AbstractObject $object, $file) {
  // Create / Update the EAD datastream with the uploaded file.
  if (empty($object['XSLT'])) {
    $ds = $object->constructDatastream('XSLT', 'M');
    $object->ingestDatastream($ds);
  }
  else {
    $ds = $object['XSLT'];
  }
  $path = drupal_realpath($file->uri);
  $ds->setContentFromFile($path, FALSE);
  $ds->label = $file->filename;
  $ds->mimetype = 'text/xsl';
}

/**
 * Ingest the given file as the given objects CSS datastream.
 *
 * @param AbstractObject $object
 *   The object which will ingest the given CSS file.
 * @param object $file
 *   A drupal file containing the CSS content to be ingested as a datastream.
 */
function islandora_manuscript_ingest_css_datastream(AbstractObject $object, $file) {
  // Create / Update the EAD datastream with the uploaded file.
  if (empty($object['CSS'])) {
    $ds = $object->constructDatastream('CSS', 'M');
    $object->ingestDatastream($ds);
  }
  else {
    $ds = $object['CSS'];
  }
  $path = drupal_realpath($file->uri);
  $ds->setContentFromFile($path, FALSE);
  $ds->label = $file->filename;
  $ds->mimetype = 'text/css';
}

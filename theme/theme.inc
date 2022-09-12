<?php

/**
 * @file
 * Theme hooks.
 */

/**
 * Implements hook_preprocess_theme().
 */
function template_preprocess_islandora_manuscript_manuscript(array &$variables) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  module_load_include('inc', 'islandora_manuscript', 'includes/utilities');
  $object = $variables['object'];
  $params = array(
    'object' => $object,
    'pages' => islandora_paged_content_get_pages($object),
    'page_progression' => islandora_paged_content_get_page_progression($object),
    'transform_object' => islandora_manuscript_get_manuscript_transform_object($object),
  );
  module_load_include('inc', 'islandora', 'includes/solution_packs');
  $viewer = islandora_get_viewer($params, 'islandora_manuscript_viewers');
  if ($viewer) {
    $variables['viewer'] = $viewer;
    if (islandora_get_viewer_id('islandora_manuscript_viewers') == 'islandora_internet_archive_bookreader') {
      // Add in our IA class if applicable.
      // Create the class array here so we don't get an empty attribute.
      $variables['viewer_wrapper_attributes']['class'] = array();
      $variables['viewer_wrapper_attributes']['class'][] = 'ia-bookreader';
    }
  }

  $variables['islandora_manuscript_metadata'] = variable_get('islandora_manuscript_metadata_display', FALSE);
  if ($variables['islandora_manuscript_metadata']) {
    drupal_add_js('misc/form.js');
    drupal_add_js('misc/collapse.js');
    module_load_include('inc', 'islandora', 'includes/metadata');
    $variables['parent_collections'] = islandora_get_parents_from_rels_ext($object);
    $variables['metadata'] = islandora_retrieve_metadata_markup($object);
    $variables['description'] = islandora_retrieve_description_markup($object);
  }
}

/**
 * Implements hook_preprocess_theme().
 */
function template_preprocess_islandora_manuscript_page(array &$variables) {
  module_load_include('inc', 'islandora', 'includes/solution_packs');
  $object = $variables['object'];
  $results = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf');
  $result = reset($results);
  $variables['manuscript_object_id'] = $result ? $result['object']['value'] : FALSE;

  $params = array();
  if (isset($object['JP2']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['JP2'])) {
    // Get token to allow access to XACML protected datastreams.
    // Always use token authentication in case there is a global policy.
    module_load_include('inc', 'islandora', 'includes/authtokens');
    $token = islandora_get_object_token($object->id, 'JP2', 2);
    $jp2_url = url("islandora/object/{$object->id}/datastream/JP2/view", array(
      'absolute' => TRUE,
      'query' => array('token' => $token),
    ));
    $params['token'] = $token;
    $params['pid'] = $object->id;
    $params['dsid'] = 'JP2';
    // Can be removed after 7.x-1.11 is out the door islandora_deprecated.
    $params['jp2_url'] = $jp2_url;
  }

  $viewer = islandora_get_viewer($params, 'islandora_manuscript_page_viewers');
  if ($viewer) {
    $variables['viewer'] = $viewer;
  }
}

/**
 * Prepares variables for islandora_manuscript_ead_display templates.
 *
 * Default template: islandora-manuscript-ead-display.tpl.php
 *
 * @param array $variables
 * @see islandora_manuscript_preprocess_ead_display_variables
 */
function template_preprocess_islandora_manuscript_ead_display(&$variables) {
  module_load_include('inc', 'islandora_manuscript', 'includes/ead_html');
  islandora_manuscript_preprocess_ead_display_variables($variables);

  // Add JS for fieldsets.
  drupal_add_js('misc/form.js');
  drupal_add_js('misc/collapse.js');
  // Add JS for finding aid.
  $mod_path = drupal_get_path('module', 'islandora_manuscript');
  drupal_add_js($mod_path.'/js/findingaid.js');
}

/**
 * Process variables for islandora_manuscript_ead_display templates.
 *
 * Default template: islandora-manuscript-ead-display.tpl.php
 *
 * @param array $variables
 * @see islandora_manuscript_process_ead_display_variables
 */
function template_process_islandora_manuscript_ead_display(&$variables) {
  module_load_include('inc', 'islandora_manuscript', 'includes/ead_html');
  islandora_manuscript_process_ead_display_variables($variables);
}



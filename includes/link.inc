<?php
/**
 * @file
 * Functions associated with linking manuscripts to finding aids.
 */

/**
 * Determine if a given object has a link to a finding aid.
 *
 * @param AbstractObject $object
 *   The object to test.
 *
 * @return AbstractObject|bool
 *   The first available finding aid if there is one; otherwise, FALSE.
 */
function islandora_manuscript_has_finding_aid_link(AbstractObject $object) {
  foreach ($object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf') as $relationship) {
    $parent_object = islandora_object_load($relationship['object']['value']);
    if ($parent_object && in_array('islandora:findingAidCModel', $parent_object->models)) {
      return $parent_object;
    }
  }
  return FALSE;
}

function islandora_manuscript_ajax_callback(&$form, &$form_state) {
  return $form['tree'];
}

function islandora_manuscript_link_to_finding_aid_form($form, &$form_state) {
  form_load_include($form_state, 'inc', 'islandora_manuscript', 'includes/link');
  $wrapper_id = 'islandora-manuscript-ajax-wrapper';
  $object = NULL;
  $form['finding_aid'] = array(
    '#type' => isset($object) ? 'item' : 'textfield',
    '#title' => t('Finding Aid'),
    '#description' => t('The finding aid object to which this manuscript is associated.'),
    '#ajax' => array(
      'wrapper' => $wrapper_id,
      'callback' => 'islandora_manuscript_ajax_callback',
    ),
    //'#autocomplete_path' => ''
  );
  if (isset($form_state['values']['finding_aid'])) {
    $object = islandora_object_load($form_state['values']['finding_aid']);
    if (!$object) {
      form_set_error('finding_aid', 'Could not load object.');
    }
  }

  $module_path = drupal_get_path('module', 'islandora_manuscript');
  $library_path = libraries_get_path('jstree');
  $id = drupal_html_id('islandora_manuscript_linking_tree');
  $form['tree'] = array(
    '#type' => 'markup',
    '#prefix' => "<div id='$wrapper_id'>",
    '#suffix' => '</div>',
    '#markup' => "<div id='$id'></div>",
  );

  if (isset($object)) {
    $form['tree']['#attached'] = array(
      'css' => array(
        "$library_path/dist/themes/default/style.min.css",
      ),
      'js' => array(
        array(
          'type' => 'setting',
          'data' => array(
            'islandora_manuscript' => array(
              'jstree' => array(
                'info' => array(
                  $id => array(
                    'core' => array(
                      'multiple' => FALSE,
                      'data' => islandora_manuscript_finding_aid_get_component_tree_for_jstree($object),
                    ),
                  ),
                ),
              ),
            ),
          ),
        ),
        "$library_path/dist/libs/jquery.js" => array(
          'group' => JS_LIBRARY,
        ),
        "$library_path/dist/jstree.min.js" => array(
          'group' => JS_LIBRARY,
        ),
        "$module_path/js/jstree.js" => array(
          'group' => JS_LIBRARY,
        ),
      ),
    );
  }

  $form['component_path'] = array(
    '#type' => 'hidden',
  );

  // TODO: Use this stuff...
  $form['containers'] = array(
    '#type' => 'fieldset',
    '#title' => t('Containers'),
    'box' => array(
      '#title' => t('Box'),
      '#description' => t('The box in which this manuscript is stored.'),
    ),
    'folder' => array(
      '#title' => t('Folder'),
      '#description' => t('The folder in which this manuscript is stored.'),
    ),
  );

  return $form;
}

function islandora_manuscript_finding_aid_get_component_tree_for_jstree(AbstractObject $object) {
  $ead_doc = new DOMDocument();
  $ead_doc->loadXML($object['EAD']->content);
  $ead_xpath = new DOMXPath($ead_doc);
  $ead_xpath->registerNamespace('ead', 'urn:isbn:1-931666-22-9');

  return islandora_manuscript_finding_aid_get_component_tree_level($ead_xpath->query('//ead:dsc/ead:c | //ead:dsc/ead:c01'), $ead_xpath);
}

function islandora_manuscript_finding_aid_get_component_tree_level(DOMNodeList $list, DOMXPath $xpath) {
  $roots = array();
  foreach ($list as $component) {
    $roots[] = islandora_manuscript_finding_aid_get_component_tree($component, $xpath);
  }
  return $roots;
}

function islandora_manuscript_finding_aid_get_component_tree(DOMElement $element, DOMXPath $xpath) {
  $info = array(
    'id' => $element->getAttribute('id'),
    'text' => format_string('@title (@date)', array(
      '@title' => $xpath->evaluate('string(ead:did/ead:unittitle/text())', $element),
      '@date' => $xpath->evaluate('string(ead:did/ead:unitdate/text())', $element),
    )),
    'children' => islandora_manuscript_finding_aid_get_component_tree_level($xpath->query('ead:c01 | ead:c02 | ead:c03 | ead:c04 | ead:c05 | ead:c', $element), $xpath),
    'path' => $element->getNodePath(),
  );
  return $info;
}

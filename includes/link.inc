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
    '#autocomplete_path' => 'islandora_manuscript/autocomplete/finding_aid',
    '#ajax' => array(
      'event' => 'change',
      'wrapper' => $wrapper_id,
      'callback' => 'islandora_manuscript_ajax_callback',
    ),
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
    '#attached' => array(
      'css' => array(
        "$library_path/dist/themes/default/style.min.css",
      ),
      'js' => array(
        "$library_path/dist/libs/jquery.js" => array(
          'group' => JS_LIBRARY,
        ),
        // XXX: Extra slash to fool Drupal, 'cause we have to load it in the
        // other jquery... blargh.
        "misc//jquery.once.js" => array(
          'group' => JS_LIBRARY,
        ),
        "$library_path/dist/jstree.min.js" => array(
          'group' => JS_LIBRARY,
        ),
        "$library_path/src/misc.js" => array(
          'group' => JS_LIBRARY,
        ),
        "$module_path/js/jstree.js" => array(
          'group' => JS_LIBRARY,
        ),
      ),
    ),
  );

  if (isset($object)) {
    $form['tree']['#attached']['js'][] = array(
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
                'plugins' => array('types', 'conditionalselect'),
                'types' => array(
                  // TODO: Get some icons to differentiate.
                  'default' => array(
                    'select_node' => FALSE,
                  ),
                  'series' => array(
                    'valid_children' => array(
                      'subseries',
                      'file',
                    ),
                  ),
                  'subseries' => array(
                    'valid_children' => array(
                      'subseries',
                      'file',
                    ),
                  ),
                  'file' => array(
                    'valid_children' => array(
                      'container',
                    ),
                  ),
                  'container' => array(
                    'valid_children' => array(
                      'none',
                    ),
                    'select_node' => TRUE,
                  ),
                ),
              ),
            ),
          ),
        ),
      ),
    );
  }

  $form['selected_node'] = array(
    '#type' => 'hidden',
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
  $title = $xpath->evaluate('normalize-space(string(ead:did/ead:unittitle/text()))', $element);
  $date = $xpath->evaluate('normalize-space(string(ead:did/ead:unitdate/text()))', $element);
  $text = empty($date) ?
    $title :
    format_string('@title (@date)', array(
      '@title' => $title,
      '@date' => $date,
    ));
  $info = array(
    'id' => $element->getAttribute('id'),
    'text' => $text,
    'type' => $element->getAttribute('level'),
    'children' => array_merge(
      islandora_manuscript_finding_aid_get_component_tree_level($xpath->query('ead:c01 | ead:c02 | ead:c03 | ead:c04 | ead:c05 | ead:c', $element), $xpath),
      islandora_manuscript_finding_aid_get_containers($element, $xpath)
    ),
    'path' => $element->getNodePath(),
  );
  return $info;
}

/**
 * Get all containers of this component.
 */
function islandora_manuscript_finding_aid_get_containers(DOMElement $element, DOMXPath $xpath) {
  $containers = islandora_manuscript_get_parented_containers($element, $xpath);
  if (empty($containers)) {
    $containers = islandora_manuscript_get_flat_containers($element, $xpath);
  }
  return $containers;
}
function islandora_manuscript_get_parented_containers(DOMElement $element, DOMXPath $xpath) {
  $to_return = array();
  foreach ($xpath->query('ead:did/ead:container[@parent and (starts-with(@type, "Folder") or starts-with(@type, "folder"))]', $element) as $container) {
    $info = array(
      'type' => 'container',
    );
    $parent_id = $container->getAttribute('parent');
    $parent_containers = $xpath->query("//ead:container[@id='$parent_id']");
    if ($parent_containers->length !== 1) {
      // If there's more or less than one, something is going terribly wrong?
      continue;
    }
    $parent_container = $parent_containers->item(0);
    $info['container'] = array(
      $parent_container,
      $container,
    );
    $subs = array(
      '@parent_type' => $parent_container->getAttribute('type'),
      '@parent_text' => $parent_container->nodeValue,
      '@current_type' => $container->getAttribute('type'),
      '@current_text' => $container->nodeValue,
    );
    $info['text'] = array(
      format_string('@parent_type @parent_text', $subs),
      format_string('@current_type @current_text', $subs),
    );
    $to_return[] = islandora_manuscript_finding_aid_process_info($info);
  }
  return $to_return;
};
function islandora_manuscript_get_flat_containers(DOMElement $element, DOMXPath $xpath) {
  $containers = array();
  $container_info = NULL;
  foreach ($xpath->query('ead:did/ead:container', $element) as $container) {
    $raw_type = $container->getAttribute('type');
    $container_type = strtolower($raw_type);
    if (strpos($container_type, 'box') === 0) {
      if (isset($container_info)) {
        $containers[] = islandora_manuscript_finding_aid_process_info($container_info);
      }
      $container_info = array(
        'type' => 'container',
        'text' => array(),
      );
    }
    $container_info['text'][] = format_string('@type @text', array(
      '@type' => $raw_type,
      '@text' => $container->nodeValue,
    ));
    $container_info['container'][] = $container;
  }
  if (isset($container_info)) {
    $containers[] = islandora_manuscript_finding_aid_process_info($container_info);
  }
  return $containers;
}

function islandora_manuscript_finding_aid_process_info($info) {
  $containers = array();
  foreach ($info['container'] as $container) {
    $containers[$container->getAttribute('type')] = array(
      'id' => $container->getAttribute('id'),
      'value' => $container->nodeValue,
      'path' => $container->getNodePath(),
    );
  }
  $info['container'] = $containers;
  $info['text'] = implode(', ', $info['text']);
  return $info;
}

function islandora_manuscript_autocomplete_finding_aid($search_string) {
  $output = array(
    'islandora:62' => 'Alpha',
    'islandora:63' => 'Bravo',
    'islandora:64' => 'Charlie',
  );
  drupal_json_output($output);
}

<?php
/**
 * @file
 * Functions associated with linking manuscripts to finding aids.
 */

define('ISLANDORA_MANUSCRIPT_FAKE_PID', 'fake:pid');

/**
 * Manuscript ingest menu page callback.
 */
function islandora_manuscript_manuscript_ingest(AbstractObject $object) {
  module_load_include('inc', 'islandora', 'includes/ingest.form');
  return drupal_get_form('islandora_ingest_form', islandora_manuscript_get_finding_aid_link_config($object));
}

/**
 * Build out config to pass to islandora_ingest_form.
 *
 * For ingesting a manuscript "into" a finding aid...
 */
function islandora_manuscript_get_finding_aid_link_config(AbstractObject $finding_aid) {
  $tuque = islandora_get_tuque_connection();
  $object = $tuque->repository->constructObject(ISLANDORA_MANUSCRIPT_FAKE_PID);
  $object->label = 'New Object';
  $object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOf', $finding_aid->id);

  return array(
    'models' => array('islandora:manuscriptCModel'),
    'islandora_manuscript' => array(
      'finding_aid' => $finding_aid->id,
    ),
    'objects' => array($object),
  );
}

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

/**
 * AJAX callback to get the jstree.
 */
function islandora_manuscript_ajax_callback(&$form, &$form_state) {
  return $form['tree'];
}

/**
 * Build a form to allow linking of manuscripts to finding aids.
 *
 * Gets a little gross due to handling both stand-alone usage, along side being
 * used in form steps...
 *
 * @param AbstractObject $operand
 *   If being used stand-alone, the object to link to a finding aid.
 */
function islandora_manuscript_link_to_finding_aid_form($form, &$form_state, AbstractObject $operand = NULL) {
  form_load_include($form_state, 'inc', 'islandora_manuscript', 'includes/link');
  form_load_include($form_state, 'inc', 'islandora', 'includes/ingest.form');
  $shared_storage = islandora_ingest_form_get_shared_storage($form_state);
  $step_storage =& islandora_ingest_form_get_step_storage($form_state, 'islandora_manuscript_link_to_finding_aid');
  if (isset($shared_storage['islandora_manuscript']['finding_aid'])) {
    $object = islandora_object_load($shared_storage['islandora_manuscript']['finding_aid']);
  }
  $form_state['islandora_manuscript_id'] = isset($operand) ?
    $operand->id :
    NULL;

  $wrapper_id = 'islandora-manuscript-ajax-wrapper';
  $form['finding_aid'] = array(
    '#type' => isset($object) ? 'item' : 'textfield',
    '#title' => t('Finding Aid'),
    '#description' => t('The finding aid object to which this manuscript is associated.'),
    '#ajax' => array(
      'event' => 'change',
      'wrapper' => $wrapper_id,
      'callback' => 'islandora_manuscript_ajax_callback',
    ),
  );
  if (module_exists('islandora_solr')) {
    $form['finding_aid']['#autocomplete_path'] = 'islandora_manuscript/autocomplete/finding_aid';
  }

  if (isset($object)) {
    $form['finding_aid']['#markup'] = $object->id;
  }
  elseif (isset($form_state['values']['finding_aid'])) {
    $form['finding_aid']['#default_value'] = $form_state['values']['finding_aid'];
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

  if (isset($object) && $object) {
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
  if (!isset($shared_storage['models'])) {
    $form['actions-thing'] = array(
      '#type' => 'actions',
      'submit' => array(
        '#type' => 'submit',
        '#value' => t('Link Manuscript'),
      ),
    );
  }

  return $form;
}

/**
 * Form validation; ensure that only "containers" can be selected.
 */
function islandora_manuscript_link_to_finding_aid_form_validate(&$form, &$form_state) {
  $form_state['islandora_manuscript_decoded_selected_node'] = $selected_node = json_decode($form_state['values']['selected_node']);
  if (isset($selected_node->type) && $selected_node->type != 'container') {
    form_set_error('', t('You must select a "container" entry.'));
    $object = islandora_ingest_form_get_object($form_state);
  }
}

/**
 * Form submission; update/stub MODS with our xlink fun.
 */
function islandora_manuscript_link_to_finding_aid_form_submit(&$form, &$form_state) {
  $shared_storage = islandora_ingest_form_get_shared_storage($form_state);

  if (isset($shared_storage['models'])) {
    $object = islandora_ingest_form_get_object($form_state);
    if ($object->id == ISLANDORA_MANUSCRIPT_FAKE_PID) {
      $finding_aid_object = isset($shared_storage['islandora_manuscript']['finding_aid']) ?
        islandora_object_load($shared_storage['islandora_manuscript']['finding_aid']) :
        NULL;
      if ($finding_aid_object) {
        $object->id = $object->repository->getNextIdentifier(islandora_get_namespace($finding_aid_object->id));
      }
    }
  }
  else {
    $object = islandora_object_load($form_state['islandora_manuscript_id']);
    if (isset($form_state['values']['finding_aid'])) {
      $finding_aid_object = islandora_object_load($form_state['values']['finding_aid']);
      $object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOf', $finding_aid_object->id);
    }
  }

  if (!isset($finding_aid_object)) {
    return;
  }

  $info = array(
    'finding_aid' => $finding_aid_object->id,
    'selected' => $form_state['islandora_manuscript_decoded_selected_node'],
  );

  if (isset($object['MODS'])) {
    islandora_manuscript_link_update_mods($object['MODS'], $info);
  }
  else {
    islandora_manuscript_link_create_mods($object, $info);
  }

  $form_state['redirect'] = "islandora/object/{$object->id}";
}

/**
 * Undo submission; ger rid of our xlink fun.
 */
function islandora_manuscript_link_to_finding_aid_form_undo_submit(&$form, &$form_state) {
  $object = islandora_ingest_form_get_object($form_state);
  if (isset($object['MODS'])) {
    islandora_manuscript_link_cleanup_mods($object['MODS']);
  }
}

/**
 * Update the MODS with the given info, first getting rid of any old elements.
 *
 * @param AbstractDatastream $mods
 *   A MODS datastream.
 * @param object $info
 *   A associative array containing:
 *   - finding_aid: The ID of the finding aid with which to associate.
 *   - selected: A stdClass object containing the "selected node" info as
 *     passed from jstree. We use:
 *     - parent: A string containing the ID of the parent node (should identify
 *       our EAD component.
 *     - original: A stdClass object containiner the original data we passed to
 *       jstree for it to do its thing, of which we use:
 *       - container: An array of stdClass objects on which we use:
 *         - type: A string indicating the type of container (box vs folder,
 *           typically).
 *         - value: A (numerical?) string identifying an item (or range of
 *           items) of the given type.
 */
function islandora_manuscript_link_update_mods(AbstractDatastream $mods, $info) {
  $doc = new DOMDocument();
  $doc->loadXML($mods->content);
  $xpath = new DOMXPath($doc);
  $xpath->registerNamespace('mods', 'http://www.loc.gov/mods/v3');
  $xpath->registerNamespace('xlink', 'http://www.w3.org/1999/xlink');
  $mods_roots = $xpath->query('//mods:mods[1]');
  if ($mods_roots->length === 0) {
    throw new InvalidArgumentException('The given MODS datastream does not appear to have a "mods" element.');
  }

  $mods_root = $mods_roots->item(0);
  islandora_manuscript_nuke_mods_finding_aid_entry($mods_root, $xpath);
  $related_item = $doc->createElementNS('http://www.loc.gov/mods/v3', 'relatedItem');
  $related_item->setAttribute('type', 'host');
  $related_item->setAttributeNS('http://www.w3.org/1999/xlink', 'xlink:role', 'islandora-manuscript-finding-aid');
  $related_item->setAttributeNS('http://www.w3.org/1999/xlink', 'href', "info:fedora/{$info['finding_aid']}/EAD" . (isset($info['selected']) ? "#{$info['selected']->parent}" : ''));
  $part = $doc->createElementNS('http://www.loc.gov/mods/v3', 'part');
  if (isset($info['selected'])) {
    foreach ($info['selected']->original->container as $type => $container) {
      $detail = $doc->createElementNS('http://www.loc.gov/mods/v3', 'detail');
      $detail->setAttribute('type', $type);
      $number = $doc->createElementNS('http://www.loc.gov/mods/v3', 'number');
      $number->nodeValue = $container->value;
      $detail->appendChild($number);
      $part->appendChild($detail);
    }
  }
  $related_item->appendChild($part);
  $mods_root->appendChild($related_item);
  $mods->content = $doc->saveXML();
}

/**
 * Cleanup; get rid of our related item elements from the MODS.
 *
 * @param AbstractDatastream $mods
 *   A MODS datastream from which we shall delete any of our related item
 *   entries.
 */
function islandora_manuscript_link_cleanup_mods(AbstractDatastream $mods) {
  $doc = new DOMDocument();
  $doc->loadXML($mods->content);
  $xpath = new DOMXPath($doc);
  $xpath->registerNamespace('mods', 'http://www.loc.gov/mods/v3');
  $xpath->registerNamespace('xlink', 'http://www.w3.org/1999/xlink');
  $mods_roots = $xpath->query('//mods:mods[1]');
  if ($mods_roots->length === 0) {
    throw new InvalidArgumentException('The given MODS datastream does not appear to have a "mods" element.');
  }
  $mods_root = $mods_roots->item(0);

  islandora_manuscript_nuke_mods_finding_aid_entry($mods_root, $xpath);
  $mods->content = $doc->saveXML();
}

/**
 * Get rid of the related elements, proper.
 *
 * @param DOMElement $mods_root
 *   The root MODS element from which the related item elements will be removed.
 * @param DOMXPath $xpath
 *   A DOMXPath instance which will likely have already been created...
 */
function islandora_manuscript_nuke_mods_finding_aid_entry(DOMElement $mods_root, DOMXPath $xpath) {
  $our_related_items = $xpath->query('mods:relatedItem[@type="host" and @xlink:role="islandora-manuscript-finding-aid"]', $mods_root);
  foreach (iterator_to_array($our_related_items) as $item) {
    $item->parentNode->removeChild($item);
  }
}

/**
 * Create stub MODS containiner our related item entry.
 *
 * @param AbstractObject $object
 *   An object to which we will ingest a MODS datastream.
 * @param object $info
 *   A associative array containing:
 *   - finding_aid: The ID of the finding aid with which to associate.
 *   - selected: A stdClass object containing the "selected node" info as
 *     passed from jstree. We use:
 *     - parent: A string containing the ID of the parent node (should identify
 *       our EAD component.
 *     - original: A stdClass object containiner the original data we passed to
 *       jstree for it to do its thing, of which we use:
 *       - container: An array of stdClass objects on which we use:
 *         - type: A string indicating the type of container (box vs folder,
 *           typically).
 *         - value: A (numerical?) string identifying an item (or range of
 *           items) of the given type.
 */
function islandora_manuscript_link_create_mods(AbstractObject $object, array $info) {
  $datastream = $object->constructDatastream('MODS', 'M');
  $datastream->mimetype = 'text/xml';
  $datastream->label = 'MODS Record';
  $datastream->content = <<<EOXML
<?xml version="1.0" encoding="UTF-8"?>
<mods
  xmlns="http://www.loc.gov/mods/v3"
  xmlns:mods="http://www.loc.gov/mods/v3"
  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xmlns:xlink="http://www.w3.org/1999/xlink"/>
EOXML;
  islandora_manuscript_link_update_mods($datastream, $info);
  $object->ingestDatastream($datastream);
}

/**
 * Start building the required structure for jstree.
 *
 * @param AbstractObject $object
 *   A finding aid object containing an EAD datastream.
 *
 * @return array
 *   An indexed array, as could be passed to jstree core.data. We build out
 *   the hierarchical structure, as opposed to the flat structure with IDs.
 */
function islandora_manuscript_finding_aid_get_component_tree_for_jstree(AbstractObject $object) {
  $ead_doc = new DOMDocument();
  $ead_doc->loadXML($object['EAD']->content);
  $ead_xpath = new DOMXPath($ead_doc);
  $ead_xpath->registerNamespace('ead', 'urn:isbn:1-931666-22-9');

  return islandora_manuscript_finding_aid_get_component_tree_level($ead_xpath->query('//ead:dsc/ead:c | //ead:dsc/ead:c01'), $ead_xpath);
}

/**
 * Build out the list of nodes at this level.
 *
 * @param DOMNodeList $list
 *   A list of nodes to process.
 * @param DOMXPath $xpath
 *   A DOMXPath instance to reuse.
 *
 * @return array
 *   An indexed array of associative arrays, each representing a component
 *   within the EAD, as returned by
 *   islandora_manuscript_finding_aid_get_component_tree().
 *
 * @see islandora_manuscript_finding_aid_get_component_tree()
 */
function islandora_manuscript_finding_aid_get_component_tree_level(DOMNodeList $list, DOMXPath $xpath) {
  $roots = array();
  foreach ($list as $component) {
    $roots[] = islandora_manuscript_finding_aid_get_component_tree($component, $xpath);
  }
  return $roots;
}

/**
 * Build out a given component node for jstree.
 *
 * @param DOMElement $element
 *   An EAD component element.
 * @param DOMXPath $xpath
 *   An instance to reuse.
 *
 * @return array
 *   An associative array containing:
 *   - id: A string containing the ID of the component. May be empty if there
 *     is none.
 *   - text: The text to display for the given node.
 *   - children: An array of child nodes, as returned by
 *     islandora_manuscript_finding_aid_get_component_tree_level().
 *   - path: An xpath which could be used to get back to the given node...
 *     Never actually got used?
 */
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

/**
 * Get containers which are related with parent => id references.
 */
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

/**
 * Get containers which listed side by side.
 */
function islandora_manuscript_get_flat_containers(DOMElement $element, DOMXPath $xpath) {
  $containers = array();
  $container_info = NULL;
  foreach ($xpath->query('ead:did/ead:container', $element) as $container) {
    $raw_type = $container->getAttribute('type');
    $container_type = strtolower($raw_type);
    // Some collections list multiple boxes side by side... We break them apart
    // as separate containers.
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

/**
 * Helper function; process containers and text into appropriate forms.
 */
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

/**
 * Autocomplete callback.
 */
function islandora_manuscript_autocomplete_finding_aid($search_string) {
  $output = array();
  if (module_exists('islandora_solr')) {
    module_load_include('inc', 'islandora_solr', 'includes/utilities');
    $qp = new IslandoraSolrQueryProcessor();
    $qp->buildQuery($search_string);
    $qp->solrParams['fq'][] = format_string('!field:("info:fedora/!pid" OR "!pid")', array(
      '!field' => islandora_solr_lesser_escape(variable_get('islandora_solr_content_model_field', 'RELS_EXT_hasModel_uri_ms')),
      '!pid' => 'islandora:findingAidCModel',
    ));
    $label_field = variable_get('islandora_solr_object_label_field', 'fgs_label_s');
    $qp->solrParams['fl'] = implode(',', array(
      'PID',
      $label_field,
    ));
    $qp->solrParams['defType'] = 'dismax';
    $qp->executeQuery();
    foreach ($qp->islandoraSolrResult['response']['objects'] as $result) {
      $output[$result['PID']] = $result['solr_doc'][$label_field];
    }
  }
  drupal_json_output($output);
}

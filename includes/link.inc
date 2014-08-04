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
  $module_path = drupal_get_path('module', 'islandora_manuscript');
  $shared_storage = islandora_ingest_form_get_shared_storage($form_state);
  $step_storage =& islandora_ingest_form_get_step_storage($form_state, 'islandora_manuscript_link_to_finding_aid');
  if (isset($shared_storage['islandora_manuscript']['finding_aid'])) {
    $object = islandora_object_load($shared_storage['islandora_manuscript']['finding_aid']);
  }
  $form_state['islandora_manuscript_id'] = isset($operand) ?
    $operand->id :
    NULL;

  $form['description'] = array(
    '#type' => 'item',
    '#title' => t('Note'),
    '#title_display' => 'invisible',
    '#description' => t('You may link this manuscript with none or at most one Finding Aid. Note that if this manuscript is already linked to a Finding Aid, changes made here can remove that link.'),
  );
  $wrapper_id = 'islandora-manuscript-ajax-wrapper';
  $form['finding_aid'] = array(
    '#type' => isset($object) ? 'item' : 'textfield',
    '#title' => t('Finding Aid'),
    '#description' => t('The finding aid object to which this manuscript is associated.'),
    '#ajax' => array(
      // XXX: The "autocomplete.drupal" event is actually triggered by our
      // monkey-patched code.
      'event' => 'autocomplete.drupal',
      'wrapper' => $wrapper_id,
      'callback' => 'islandora_manuscript_ajax_callback',
    ),
    '#attached' => array(
      'js' => array(
        // Monkey-patch to deal with autocomplete/ajax interaction.
        "$module_path/js/autocomplete.js",
      ),
    ),
  );
  if (module_exists('islandora_solr')) {
    $form['finding_aid']['#autocomplete_path'] = 'islandora/manuscript/autocomplete/finding_aid';
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
  elseif (isset($step_storage['finding_aid'])) {
    $form['finding_aid']['#default_value'] = $step_storage['finding_aid'];
    $object = islandora_object_load($step_storage['finding_aid']);
    if (!$object) {
      form_set_error('finding_aid', 'Could not load object.');
    }
  }
  elseif ($operand !== NULL) {
    $found = islandora_manuscript_has_finding_aid_link($operand);
    if ($found) {
      $object = $found;
      $form['finding_aid']['#default_value'] = $object->id;
    }
  }

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
    module_load_include('inc', 'islandora_manuscript', 'includes/jstreebuilder');
    $builder = new \Islandora\Manuscript\JSTreeBuilder($object);
    $structure = $builder->getTreeStructure();

    // Get box/folder stuff from $operands MODS (if it exists), and make
    // the appropriate node get selected in $structure.
    if ($operand !== NULL) {
      $info = islandora_manuscript_get_link_info($operand['MODS']);
      if ($info) {
        $found = islandora_manuscript_select_container($structure, $info);
      }
    }
    elseif (isset($step_storage['selected'])) {
      $info = array(
        'id' => '#' . $step_storage['selected']->parent,
        'containers' => array(),
      );
      foreach ($step_storage['selected']->original->container as $type => $values) {
        $info['containers'][$type] = $values->value;
      }
      $found = islandora_manuscript_select_container($structure, $info);
    }

    $form['tree']['#attached']['js'][] = array(
      'type' => 'setting',
      'data' => array(
        'islandora_manuscript' => array(
          'jstree' => array(
            'info' => array(
              $id => array(
                'core' => array(
                  'multiple' => FALSE,
                  'data' => $structure,
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
  $form_state['islandora_manuscript_selected_node_default_value'] = new stdClass();
  if (isset($found) && $found && isset($info) && $info) {
    $default = $form_state['islandora_manuscript_selected_node_default_value'];
    $default->type = 'container';
    $default->parent = substr($info['id'], 1);
    $default->original = new stdClass();
    $default->original->container = array();
    foreach ($info['containers'] as $type => $value) {
      $obj = $default->original->container[$type] = new stdClass();
      $obj->value = $value;
    }
  }
  if (!isset($shared_storage['models'])) {
    $form['actions-thing'] = array(
      '#type' => 'actions',
      'unlink' => array(
        '#type' => 'submit',
        '#value' => t('Unlink'),
        '#validate' => array(
          'islandora_manuscript_will_unlink_orphan_object',
        ),
        '#submit' => array(
          'islandora_manuscript_unlink_from_finding_aids',
        ),
      ),
      'submit' => array(
        '#type' => 'submit',
        '#value' => t('Link Manuscript'),
      ),
      'cancel' => array(
        '#theme' => 'link',
        '#text' => t('Cancel'),
        '#path' => "islandora/object/{$operand->id}/manage/manuscript",
        '#options' => array(
          'attributes' => array(),
          'html' => FALSE,
        ),
      ),
    );
  }

  return $form;
}

/**
 * Form validation callback for "unlink" action.
 *
 * Test if "unlinking" a manuscript from its finding aid will result in it being
 * orphaned...  If it /will/ be orphaned, prevent the unlinking and reccomend
 * that the object gets associated with a collection.
 */
function islandora_manuscript_will_unlink_orphan_object(&$form, &$form_state) {
  $operand = islandora_object_load($form_state['build_info']['args'][0]);
  $finding_aid = islandora_manuscript_has_finding_aid_link($operand);
  module_load_include('inc', 'islandora_basic_collection', 'includes/utilities');
  if ($finding_aid && !islandora_basic_collection_get_other_parents($operand, $finding_aid)) {
    form_set_error('', filter_xss(t('Unlinking this manuscript would cause it to be orphaned. !share or !migrate this object to a collection to avoid orphaning if you wish to unlink it from the finding aid.', array(
      '!share' => l(t('Share'), "islandora/object/{$operand->id}/manage/object/share"),
      '!migrate' => l(t('migrate'), "islandora/object/{$operand->id}/manage/object/migrate"),
    ))));
  }
}

/**
 * Form submission callback for "unlink" action.
 *
 * Remove link to the finding aid.
 */
function islandora_manuscript_unlink_from_finding_aids(&$form, &$form_state) {
  $operand = islandora_object_load($form_state['build_info']['args'][0]);
  islandora_manuscript_clear_all_finding_aid_links($operand);
  islandora_manuscript_link_cleanup_mods($operand['MODS']);
  drupal_set_message(t('Unlinked @pid from any finding aids.', array(
    '@pid' => $operand->id,
  )));
  $form_state['redirect'] = "islandora/object/{$operand->id}";
}

/**
 * Search tree structure for node matching $info, and mark it as selected.
 *
 * @param array $structure
 *   Reference to a numerically indexed array of associative arrays, as
 *   provided by \Islandora\Manuscript\JSTreeBuilder::getLevel().
 * @param array $info
 *   An array as provided by islandora_manuscript_get_link_info().
 *
 * @return bool
 *   TRUE if we managed to find the node to select (and selected it); otherwise,
 *   FALSE.
 */
function islandora_manuscript_select_container(array &$structure, array $info) {
  foreach ($structure as &$node) {
    if ($node['type'] == 'container') {
      // Test if it's the proper one.
      if (array_intersect_key($node['container'], $info['containers']) == $node['container']) {
        $found = TRUE;
        foreach ($node['container'] as $type => $values) {
          if ($info['containers'][$type] != $values['value']) {
            // Found something which /doesn't/ match...
            $found = FALSE;
            break;
          }
        }
        if ($found) {
          $node['state'] = array(
            'selected' => TRUE,
          );
          return TRUE;
        }
        else {
          continue;
        }
      }
    }
    elseif (count($node['children']) > 0) {
      $found = islandora_manuscript_select_container($node['children'], $info);
      if ($found) {
        return $found;
      }
      else {
        continue;
      }
    }
  }
  return FALSE;
}

/**
 * Get link info out of MODS.
 *
 * @param AbstractDatastream $ds
 *   A MODS datastream from which to grab our link info.
 *
 * @return array
 *   An associative array containing:
 *   - id: A string indicating the component in which the container is found.
 *     May be indicate the PID of the finding aid object if we failed to find
 *     a fragment ID in the relevant attribute.
 *   - containers: An associative array mapping the container type
 *     (box/folder/etc) to a locator value inside the container.
 */
function islandora_manuscript_get_link_info(AbstractDatastream $ds) {
  if ($ds->id != 'MODS') {
    // Not MODS, what're you doing!?
    return FALSE;
  }

  $doc = new DOMDocument();
  $doc->loadXML($ds->content);
  $xpath = new DOMXPath($doc);
  $xpath->registerNamespace('mods', 'http://www.loc.gov/mods/v3');
  $xpath->registerNamespace('xlink', 'http://www.w3.org/1999/xlink');
  $finding_aid_refs = $xpath->query('/mods:mods/mods:relatedItem[@type="host" and @xlink:role="islandora-manuscript-finding-aid"]');
  if ($finding_aid_refs->length !== 1) {
    // No/too many entries?
    return FALSE;
  }

  $info = array(
    'containers' => array(),
  );
  $ref = $finding_aid_refs->item(0);
  $href = $ref->getAttributeNS('http://www.w3.org/1999/xlink', 'href');
  $info['id'] = substr($href, strrpos($href, '#'));
  foreach ($xpath->query('mods:part/mods:detail[@type and mods:number]', $ref) as $detail) {
    $info['containers'][$detail->getAttribute('type')] = $xpath->evaluate('normalize-space(mods:number)', $detail);
  }
  return $info;
}

/**
 * Form validation; ensure that only "containers" can be selected.
 */
function islandora_manuscript_link_to_finding_aid_form_validate(&$form, &$form_state) {
  $form_state['islandora_manuscript_decoded_selected_node'] = $selected_node = ((isset($form_state['values']['selected_node']) && !empty($form_state['values']['selected_node'])) ?
    json_decode($form_state['values']['selected_node']) :
    $form_state['islandora_manuscript_selected_node_default_value']);

  if (isset($selected_node->type) && $selected_node->type != 'container') {
    form_set_error('', t('You must select a "container" entry.'));
  }
  if (isset($form_state['values']['finding_aid']) && trim($form_state['values']['finding_aid']) != '') {
    $finding_aid = islandora_object_load($form_state['values']['finding_aid']);
    if (!$finding_aid || !in_array('islandora:findingAidCModel', $finding_aid->models)) {
      form_set_error('finding_aid', t('If a value is provided, it must identify a valid finding aid object.'));
    }
  }
}

/**
 * Form submission; update/stub MODS with our xlink fun.
 */
function islandora_manuscript_link_to_finding_aid_form_submit(&$form, &$form_state) {
  $shared_storage = islandora_ingest_form_get_shared_storage($form_state);
  $step_storage =& islandora_ingest_form_get_step_storage($form_state, 'islandora_manuscript_link_to_finding_aid');

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
    $step_storage['finding_aid'] = drupal_array_get_nested_value($form_state['values'], array('finding_aid'));
    $step_storage['selected'] = $form_state['islandora_manuscript_decoded_selected_node'];
  }
  else {
    $object = islandora_object_load($form_state['islandora_manuscript_id']);
  }
  if (!isset($finding_aid_object)) {
    islandora_manuscript_clear_all_finding_aid_links($object);

    if (isset($form_state['values']['finding_aid']) && trim($form_state['values']['finding_aid']) != '') {
      $finding_aid_object = islandora_object_load($form_state['values']['finding_aid']);
    }
    elseif (isset($step_storage['finding_aid']) && trim($step_storage['finding_aid']) != '') {
      $finding_aid_object = islandora_object_load($step_storage['finding_aid']);
    }

    if (isset($finding_aid_object)) {
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
 * Undo submission; get rid of our xlink fun.
 */
function islandora_manuscript_link_to_finding_aid_form_undo_submit(&$form, &$form_state) {
  $object = islandora_ingest_form_get_object($form_state);
  $shared_storage = islandora_ingest_form_get_shared_storage($form_state);
  if (!isset($shared_storage['islandora_manuscript']['finding_aid'])) {
    islandora_manuscript_clear_all_finding_aid_links($object);
  }
  if (isset($object['MODS'])) {
    islandora_manuscript_link_cleanup_mods($object['MODS']);
  }
}

/**
 * Nuke links to finding aids.
 */
function islandora_manuscript_clear_all_finding_aid_links(AbstractObject $object) {
  while ($finding_aid_object = islandora_manuscript_has_finding_aid_link($object)) {
    $object->relationships->remove(FEDORA_RELS_EXT_URI, 'isMemberOf', $finding_aid_object->id);
  }
}

/**
 * Update the MODS with the given info, first getting rid of any old elements.
 *
 * @param AbstractDatastream $mods
 *   A MODS datastream.
 * @param array $info
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
  $related_item->setAttributeNS('http://www.w3.org/1999/xlink', 'href', "info:fedora/{$info['finding_aid']}/EAD" . (isset($info['selected']->parent) ? "#{$info['selected']->parent}" : ''));
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

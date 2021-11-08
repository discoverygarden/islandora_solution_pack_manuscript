<?php
/**
 * @file
 * Holds block helper functions.
 */

/**
 * Constructs a container list using jsTree for viewing.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing an object within Fedora.
 *
 * @return array
 *   A renderable array.
 */
function islandora_manuscript_container_list($object) {
  module_load_include('inc', 'islandora_manuscript', 'includes/link');
  $module_path = drupal_get_path('module', 'islandora_manuscript');
  $library_path = libraries_get_path('jstree');
  $id = drupal_html_id('islandora_manuscript_container_list_tree');
  $form['tree'] = array(
    '#type' => 'markup',
    '#markup' => "<div id='$id'></div>",
    '#attached' => array(
      'css' => array(
        "$library_path/dist/themes/default/style.min.css",
        "$module_path/css/container_list_jstree.css",
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
        "$module_path/js/container_list_jstree.js" => array(
          'group' => JS_LIBRARY,
        ),
      ),
    ),
  );
  module_load_include('inc', 'islandora_manuscript', 'includes/jstreebuilder');
  $builder = new \Islandora\Manuscript\ContainerListJSTreeBuilder($object);
  $form['tree']['#attached']['js'][] = array(
    'type' => 'setting',
    'data' => array(
      'islandora_manuscript' => array(
        'jstree' => array(
          'info' => array(
            $id => array(
              'core' => array(
                'multiple' => FALSE,
                'data' => $builder->getTreeStructure(),
              ),
              'plugins' => array('types'),
              'types' => array(
                // TODO: Get some icons to differentiate.
                'default' => array(
                  'select_node' => FALSE,
                ),
                'recordgrp' => array(
                  'valid_children' => array(
                    'subgrp',
                  ),
                ),
                'subgrp' => array(
                  'valid_children' => array(
                    'series',
                  ),
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
              ),
            ),
          ),
        ),
      ),
    ),
  );
  return $form;
}

<?php

/**
 * @file
 * Manuscript listing callback.
 */

/**
 * Page callback; setup an Islandora Solr display.
 */
function islandora_manuscript_list_manuscripts(AbstractObject $object) {
  module_load_include('inc', 'islandora_solr', 'includes/utilities');
  $query = format_string('!field:("info:fedora/!value" OR "!value")', array(
    '!field' => islandora_solr_lesser_escape(variable_get('islandora_solr_member_of_field', 'RELS_EXT_isMemberOf_uri_ms')),
    '!value' => $object->id,
  ));
  return islandora_solr($query);
}

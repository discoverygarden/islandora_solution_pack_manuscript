<?php

/**
 * @file
 * Utility functions.
 */

/**
 * Get the transform associated with a manuscript.
 *
 * @param AbstractObject $manuscript_object
 *   The object to get the transform for.
 *
 * @return mixed
 *   The return of islandora_object_load for the associated transform object.
 */
function islandora_manuscript_get_manuscript_transform_object(AbstractObject $manuscript_object) {
  module_load_include('inc', 'islandora_basic_collection', 'includes/utilities');
  module_load_include('inc', 'islandora_manuscript', 'includes/link');
  $ead_object = islandora_manuscript_has_finding_aid_link($manuscript_object);
  if ($ead_object) {
    $direct_transform_pid = islandora_manuscript_get_transform_pid($ead_object);
    if ($direct_transform_pid) {
      $transform = islandora_object_load($direct_transform_pid);
      if ($transform) {
        return $transform;
      }
    }
  }
  else {
    $parents = islandora_basic_collection_get_parent_pids($manuscript_object);
    foreach ($parents as $parent_pid) {
      $parent = islandora_object_load($parent_pid);
      if ($parent) {
        $indirect_transform_pid = islandora_manuscript_get_transform_pid($parent);
        if ($indirect_transform_pid) {
          $transform = islandora_object_load($indirect_transform_pid);
          if ($transform) {
            return $transform;
          }
        }
      }
    }
    return islandora_object_load(ISLANDORA_MANUSCRIPT_DEFAULT_TRANSFORM);
  }
}

/**
 * Set the related transform for an object.
 *
 * @param AbstractObject $object
 *   The object to set the relation on.
 * @param string $transform_pid
 *   The PID to set the relation to.
 */
function islandora_manuscript_set_transform(AbstractObject $object, $transform_pid) {
  $relations = $object->relationships;
  $relations->autoCommit = FALSE;

  islandora_manuscript_remove_transform($object);
  $relations->add(
    ISLANDORA_RELS_EXT_URI,
    ISLANDORA_MANUSCRIPT_HAS_TRANSFORM_PREDICATE,
    $transform_pid
  );

  $relations->commitRelationships();
}

/**
 * Remove the related transform for an object.
 *
 * @param AbstractObject $object
 *   The object to remove the relation from.
 */
function islandora_manuscript_remove_transform(AbstractObject $object) {
  $relations = $object->relationships;
  $relations->remove(
    ISLANDORA_RELS_EXT_URI,
    ISLANDORA_MANUSCRIPT_HAS_TRANSFORM_PREDICATE
  );
}

/**
 * Get the associated transform.
 *
 * @param AbstractObject $object
 *   The object to get the transform for.
 *
 * @return mixed
 *   The PID of the transform object associated with this object.
 *   FALSE if there is not one.
 */
function islandora_manuscript_get_transform_pid($object) {
  $retrieved_rel = $object->relationships->get(
    ISLANDORA_RELS_EXT_URI,
    ISLANDORA_MANUSCRIPT_HAS_TRANSFORM_PREDICATE
  );
  if ($retrieved_rel) {
    $reset_rel = reset($retrieved_rel);
    return $reset_rel['object']['value'];
  }
  return FALSE;
}

/**
 * Get all transforms.
 *
 * @return array
 *   A list of all transform objects.
 */
function islandora_manuscript_get_transforms() {
  $tuque = islandora_get_tuque_connection();
  $query = <<<EOQ
  PREFIX dc: <http://purl.org/dc/elements/1.1/>
  SELECT ?transform ?label ?description
  FROM <#ri>
  WHERE {
    ?transform <fedora-model:hasModel> <info:fedora/islandora:transformCModel>
    OPTIONAL {
      ?transform <fedora-model:label> ?label
    }
    OPTIONAL {
      ?transform dc:description ?description
    }
  }
EOQ;
  $raw_results = $tuque->repository->ri->sparqlQuery($query);
  $results = array();
  foreach ($raw_results as $result) {
    $results[$result['transform']['value']] = array(
      'description' => isset($result['description']['value']) ? $result['description']['value'] : '',
      'label' => isset($result['label']['value']) ? $result['label']['value'] : NULL,
    );
  }
  return $results;
}

/**
 * Converts the given EAD document from the DTD format to the W3C Schema format.
 *
 * @param DOMDocument $doc
 *   The EAD document to transform.
 *
 * @return DOMDocument
 *   The transformed EAD document if successful, false otherwise, unless the
 *   given document was already in the W3C Schema format. In which case the
 *   function will return the given document unchanged.
 */
function islandora_manuscript_ead_dtd_to_xsd(DOMDocument $doc) {
  // If this document does not declare a DOCTYPE we should not transform it. As
  // it is already assumed to be in the correct format.
  if ($doc->doctype !== NULL) {
    $module_path = drupal_get_path('module', 'islandora_manuscript');
    $xsl = new DOMDocument();
    $xsl->load("{$module_path}/data/transforms/dtd2schema.xsl");
    $proc = new XSLTProcessor();
    $proc->importStylesheet($xsl);
    return $proc->transformToDoc($doc);
  }
  return $doc;
}

/**
 * Converts the given EAD document to Dublin Core Document.
 *
 * @param DOMDocument $doc
 *   An EAD document to crosswalk to DC.
 *
 * @return DOMDocument
 *   The transformed DC if successful, false otherwise.
 */
function islandora_manuscript_ead_to_dc(DOMDocument $doc) {
  $module_path = drupal_get_path('module', 'islandora_manuscript');
  $xsl = new DOMDocument();
  $xsl->load("{$module_path}/data/transforms/ead_to_dc.xsl");
  $proc = new XSLTProcessor();
  $proc->importStylesheet($xsl);
  return $proc->transformToDoc($doc);
}

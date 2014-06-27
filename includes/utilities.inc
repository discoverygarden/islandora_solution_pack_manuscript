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
 *   The transform object associated with the manuscript.
 *   FALSE if there is not one.
 */
function islandora_manuscript_get_manuscript_transform_object(AbstractObject $manuscript_object) {
  $islandora_namespace = ISLANDORA_RELS_EXT_URI;
  $transform_predicate = ISLANDORA_MANUSCRIPT_HAS_TRANSFORM_PREDICATE;
  $ead_predicate = ISLANDORA_MANUSCRIPT_HAS_EAD_PREDICATE;
  $query = <<<EOQ
    PREFIX islandora: <$islandora_namespace>
    SELECT ?ead ?collection ?ead_transform ?collection_transform
    FROM <#ri>
    WHERE {
      {
        <info:fedora/{$manuscript_object->id}> <fedora-rels-ext:$ead_predicate> ?ead .
        ?ead <islandora:$transform_predicate> ?ead_transform
      }
      UNION {
        <info:fedora/{$manuscript_object->id}> <fedora-rels-ext:isMemberOfCollection> ?collection .
        ?collection <islandora:$transform_predicate> ?collection_transform
      }
    }
EOQ;
  $raw_results = reset($manuscript_object->repository->ri->sparqlQuery($query));
  if ($raw_results['ead']['value']) {
    $transform_pid = $raw_results['ead_transform']['value'];
  }
  elseif ($raw_results['collection']['value']) {
    $transform_pid = $raw_results['collection_transform']['value'];
  }
  if (isset($transform_inheritee)) {
    return islandora_object_load($transform_pid);
  }
  return FALSE;
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

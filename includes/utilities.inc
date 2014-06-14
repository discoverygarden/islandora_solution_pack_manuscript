<?php

/**
 * @file
 * Utility functions.
 */

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

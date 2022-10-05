<?php
/**
 * @file
 * Displays a TOC and list of containers from EAD.
 *
 * Available variables:
 * - $attributes: Provided by template_process().
 * - $object: An AbstractObject containing an "EAD" datastream.
 * - $xslt_functions: An array of functions to allow the XSLT to run, as
 *     accepted by XSLTProcessor::registerPhpFunctions().
 * - $xslt_parameters: An associative array mapping namespace URIs to
 *     associative arrays of parameters proper.
 * - $doc: A DOMDocument containing the parsed EAD datastream.
 * - $xslt_doc: A DOMDocument containing the parsed XSLT to run.
 * - $processor: The XSLTProcessor instance which was used.
 * - $markup_doc: A DOMDocument containing the markup to output, after
 *     this function has run.  If the cache was used, this will be null!
 * - $rendered_ead_html: The rendered HTML from the $markup_doc transform
 */
?>
<div <?php echo $attributes; ?> <?php echo drupal_attributes(array('class' => $classes)); ?>>
  <?php echo $rendered_ead_html; ?>
</div>

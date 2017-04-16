<?php
/**
 * @file
 * Template file to style output.
 */
?>
<?php
  print $manuscript_object_id ? l(t('Return to Manuscript View'), "islandora/object/{$manuscript_object_id}") : t('Orphaned page (no associated manuscript)');
?>
<?php print theme('islandora_solr_search_return_link'); ?>
<?php if (isset($viewer)): ?>
  <div id="manuscript-page-viewer">
    <?php print $viewer; ?>
  </div>
<?php elseif (isset($object['JPG']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['JPG'])): ?>
  <div id="manuscript-page-image">
    <?php
      $params = array(
        'path' => url("islandora/object/{$object->id}/datastream/JPG/view"),
        'attributes' => array(),
      );
      print theme('image', $params);
    ?>
  </div>
<?php endif; ?>

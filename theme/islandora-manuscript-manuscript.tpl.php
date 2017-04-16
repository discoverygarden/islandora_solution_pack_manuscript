<?php
/**
 * @file
 * Template file to style output.
 */
?>
<?php print theme('islandora_solr_search_return_link'); ?>
<?php if (isset($viewer)): ?>
  <div id="manuscript-viewer">
    <?php print $viewer; ?>
  </div>
<?php endif; ?>
<?php if ($islandora_manuscript_metadata): ?>
    <div class="islandora-manuscript-metadata">
      <?php print $description; ?>
      <?php if ($parent_collections): ?>
          <div>
              <h2><?php print t('In collections'); ?></h2>
              <ul>
                <?php foreach ($parent_collections as $collection): ?>
                    <li><?php print l($collection->label, "islandora/object/{$collection->id}"); ?></li>
                <?php endforeach; ?>
              </ul>
          </div>
      <?php endif; ?>
      <?php print $metadata; ?>
    </div>
<?php endif; ?>

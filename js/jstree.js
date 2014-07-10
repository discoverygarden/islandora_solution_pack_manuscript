/**
 * @file
 * Create a jsTree for use in rendering a manuscript.
 */

(function ($) {
  Drupal.behaviors.islandora_manuscript_jstree = {
    attach: function(context, settings) {
      for (var id in settings.islandora_manuscript.jstree.info) {
        $('#' + id).jstree(settings.islandora_manuscript.jstree.info[id]).bind('select_node.jstree', function (evt, data) {
          
        });
      }
    }
  };
})(jQuery.noConflict(true));

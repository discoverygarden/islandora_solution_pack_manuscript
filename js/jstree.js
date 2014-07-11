/**
 * @file
 * Create a jsTree for use in rendering a manuscript.
 */

(function ($) {
  Drupal.behaviors.islandora_manuscript_jstree = {
    attach: function (context, settings) {
      if (typeof settings.islandora_manuscript != 'undefined') {
        for (var id in settings.islandora_manuscript.jstree.info) {
          $('#' + id).once('islandora-manuscript-jstree', function () {
            $.extend(true, settings.islandora_manuscript.jstree.info[id], {
              'conditionalselect': function (node) {
                return this.get_type(node, true)['select_node'];
              }
            });
            $(this)
              .jstree(settings.islandora_manuscript.jstree.info[id])
              .bind('select_node.jstree', function (evt, data) {
                // Stash the selected node, so we can get associated info when
                //processing PHP-side.
                data.instance.element.closest('form').find('input[name="selected_node"]').val(JSON.stringify(data.node));
              })
              .closest('form')
              .find('input[name="selected_node"]')
              .val('');
          });
        }
      }
    }
  };
})(jQuery.noConflict(true));

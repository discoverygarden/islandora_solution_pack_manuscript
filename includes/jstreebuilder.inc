<?php
/**
 * @file
 * Builders for jstree structures.
 */

namespace Islandora\Manuscript;
use AbstractObject, DOMDocument, DOMXPath, DOMElement;

class JSTreeBuilder {
  protected $object;
  protected $xpath;

  /**
   * Constructor.
   *
   * @param AbstractObject $object
   *   The object for which the tree is to be built.
   */
  public function __construct(AbstractObject $object) {
    $this->object = $object;

    $ead_doc = new DOMDocument();
    $ead_doc->loadXML($this->object['EAD']->content);
    $this->xpath = new DOMXPath($ead_doc);
    $this->xpath->registerNamespace('ead', 'urn:isbn:1-931666-22-9');
  }

  /**
   * Start building the required structure for jstree.
   *
   * @return array
   *   An indexed array, as could be passed to jstree core.data. We build out
   *   the hierarchical structure, as opposed to the flat structure with IDs.
   *
   * @see JSTreeBuilder::getLevel()
   */
  public function getTreeStructure() {
    return $this->getLevel('//ead:dsc/ead:c | //ead:dsc/ead:c01');
  }

  /**
   * Build out the list of nodes at this level.
   *
   * @param string $query
   *   A XPath query to select nodes to process.
   * @param DOMElement|NULL $node
   *   The query provided will be performed relative to this node; NULL will
   *   result in the query being performed relative to the root node of the
   *   document.
   *
   * @return array
   *   An indexed array of associative arrays, each representing a component
   *   within the EAD, as returned by JSTreeBuilder::getComponentTree().
   *
   * @see JSTreeBuilder::getComponentTree()
   */
  protected function getLevel($query, DOMElement $node = NULL) {
    if ($node === NULL) {
      $node = $this->xpath->document->documentElement;
    }
    $list = $this->xpath->query($query, $node);
    $roots = array();
    foreach ($list as $component) {
      $roots[] = $this->getComponentTree($component);
    }
    return $roots;
  }

  /**
   * Build out a given component node for jstree.
   *
   * @param DOMElement $element
   *   An EAD component element.
   *
   * @return array
   *   An associative array containing:
   *   - id: A string containing the ID of the component. May be empty if there
   *     is none.
   *   - text: The text to display for the given node.
   *   - children: An array of child nodes, as returned by
   *     JSTreeBuilder::getLevel().
   *   - path: An xpath which could be used to get back to the given node...
   *     Never actually got used?
   */
  protected function getComponentTree(DOMElement $element) {
    $unitid = $this->stringifyNode('ead:did/ead:unitid', $element);
    $title = $this->stringifyNode('ead:did/ead:unittitle', $element);
    $date = $this->stringifyNode('ead:did/ead:unitdate', $element);
    $text = empty($date) ?
      $title :
      format_string('@title (@date)', array(
        '@title' => $title,
        '@date' => $date,
      ));
    if ($unitid) {
      $text = $unitid.' '.$text;
    }
    $info = array(
      'id' => $element->getAttribute('id'),
      'text' => $text,
      'type' => $element->getAttribute('level'),
      'children' => array_merge(
        $this->getLevel('ead:c01 | ead:c02 | ead:c03 | ead:c04 | ead:c05 | ead:c06 | ead:c07 | ead:c08 | ead:c09 | ead:c', $element),
        $this->getContainers($element)
      ),
      'path' => $element->getNodePath(),
    );
    return $info;
  }

  /**
   * Given the xpath to a single node in a DOMElement,
   * return the text content of the node and its children
   * @param string $xpath Path to a single node
   * @param DOMElement $element Element which may contain the xpath
   * @return string The textContent of the node, or an empty string if no node is found
   */
  protected function stringifyNode($xpath, DOMElement $element) {
    $node = $this->xpath->query($xpath, $element)->item(0);
    return $node ? $node->textContent : '';
  }

  /**
   * Get all containers of this component.
   */
  protected function getContainers(DOMElement $element) {
    $containers = $this->getParentedContainers($element);
    if (empty($containers)) {
      $containers = $this->getFlatContainers($element);
    }
    return array_map(array($this, 'processContainer'), $containers);
  }

  /**
   * Helper function; process containers and text into appropriate forms.
   */
  protected function processContainer(array $info) {
    $containers = array();
    foreach ($info['container'] as $container) {
      $containers[$container->getAttribute('type')] = array(
        'id' => $container->getAttribute('id'),
        'value' => $container->nodeValue,
        'path' => $container->getNodePath(),
      );
    }
    $info['container'] = $containers;
    $info['text'] = implode(', ', $info['text']);
    return $info;
  }

  /**
   * Get containers which are related with parent => id references.
   */
  protected function getParentedContainers(DOMElement $element) {
    $to_return = array();
    foreach ($this->xpath->query('ead:did/ead:container[@parent and (starts-with(@type, "Folder") or starts-with(@type, "folder"))]', $element) as $container) {
      $info = array(
        'type' => 'container',
      );
      $parent_id = $container->getAttribute('parent');
      $parent_containers = $this->xpath->query("//ead:container[@id='$parent_id']");
      if ($parent_containers->length !== 1) {
        // If there's more or less than one, something is going terribly wrong?
        continue;
      }
      $parent_container = $parent_containers->item(0);
      $info['container'] = array(
        $parent_container,
        $container,
      );
      $subs = array(
        '@parent_type' => $parent_container->getAttribute('type'),
        '@parent_text' => $parent_container->nodeValue,
        '@current_type' => $container->getAttribute('type'),
        '@current_text' => $container->nodeValue,
      );
      $info['text'] = array(
        format_string('@parent_type @parent_text', $subs),
        format_string('@current_type @current_text', $subs),
      );
      $to_return[] = $info;
    }
    return $to_return;
  }

  /**
   * Get containers which listed side by side.
   */
  protected function getFlatContainers(DOMElement $element) {
    $containers = array();
    $container_info = NULL;
    foreach ($this->xpath->query('ead:did/ead:container', $element) as $container) {
      $raw_type = $container->getAttribute('type');
      $container_type = strtolower($raw_type);
      // Some collections list multiple boxes side by side... We break them
      // apart as separate containers.
      if (strpos($container_type, 'box') === 0) {
        if (isset($container_info)) {
          $containers[] = $container_info;
        }
        $container_info = array(
          'type' => 'container',
          'text' => array(),
        );
      }
      $container_info['text'][] = format_string('@type @text', array(
        '@type' => $raw_type,
        '@text' => $container->nodeValue,
      ));
      $container_info['container'][] = $container;
    }
    if (isset($container_info)) {
      $containers[] = $container_info;
    }
    return $containers;
  }
}

class ContainerListJSTreeBuilder extends JSTreeBuilder {
  /**
   * Inherits.
   *
   * Since we now may have NULL entries, let's get rid of 'em.
   */
  protected function getLevel($query, DOMElement $node = NULL) {
    return array_filter(parent::getLevel($query, $node));
  }

  /**
   * Inherits.
   *
   * Only return results for series and subseries, and make a couple
   * adjustments to the data.
   */
  protected function getComponentTree(DOMElement $element) {
    $type = $element->getAttribute('level');
    if (in_array($type, array('series', 'subseries'))) {
      $to_return = parent::getComponentTree($element);
      $to_return['a_attr']['href'] = url("islandora/object/{$this->object->id}", array('fragment' => $to_return['id']));
      $to_return['id'] = "container_list_{$to_return['id']}";
      return $to_return;
    }
  }
}

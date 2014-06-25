<?php

/**
 * @file
 * Holds delete form for islandora_manuscript.
 */

/**
 * Submit handler for deleting transforms.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_manuscript_islandora_delete_object_form_submit(array $form, array &$form_state) {
  $object = $form_state['object'];
  $message_parameters = array('@object' => $object->id);
  batch_set(
    array(
      'progress_message' => t('Time elapsed: <strong>@elapsed</strong> <br/>
        Estimated time remaining: <strong>@estimate.</strong>'),
      'error_message' => t('An error has occurred.'),
      'file' => drupal_get_path('module', 'islandora_manuscript') . '/includes/delete.form.inc',
      'title' => t('Removing relations to @object', $message_parameters),
      'init_message' => t('Preparing to remove realtions to @object', $message_parameters),
      'operations' => array(
        array(
          'islandora_manuscript_remove_associations_batch_operation',
          array($object->id),
        ),
      ),
    )
  );
  // Called from within this submit handler rather than from the Drupal Form API
  // as we need the object to exist to generate the pages and if we run this.
  // batch operation from a submit handler any submit handlers to be called
  // afterwards will not get called, which is a bug/feature of the Form API.
  islandora_delete_object_form_submit($form, $form_state);
}

/**
 * Batch operation to removing transform association relations.
 *
 * @param string $transform_pid
 *   The PID of the object to remove relations to.
 * @param array $context
 *   The batch context.
 */
function islandora_manuscript_remove_associations_batch_operation($transform_pid, &$context) {
  $tuque = islandora_get_tuque_connection();
  $resource_index = $tuque->repository->ri;
  $islandora_namespace = ISLANDORA_RELS_EXT_URI;
  $transform_predicate = ISLANDORA_MANUSCRIPTI_HAS_TRANSFORM_PREDICATE;
  $step = 10;
  $query = <<<EOQ
  PREFIX islandora: <$islandora_namespace>
  SELECT ?transformee
  FROM <#ri>
  WHERE {
    ?transformee <islandora:$transform_predicate> <info:fedora/$transform_pid>
  }
EOQ;
  $current_count = $resource_index->countQuery($query, 'sparql');

  // Initial batch setup.
  if (!isset($context['sandbox']['total_count'])) {
    $context['sandbox']['total_count'] = $current_count;
    if ($current_count == 0) {
      drupal_set_message(
        t(
          'No relations to remove for @object.',
          array(
            '@object' => $transform_pid,
          )
        )
      );
    }
    $context['sandbox']['current'] = 0.0;
  }
  if ($current_count == 0) {
    $context['finished'] = 1;
    return;
  }

  // Perform operations.
  module_load_include('inc', 'islandora_manuscript', 'includes/utilities');
  $transformee_results = $resource_index->sparqlQuery("{$query}LIMIT $step");
  foreach ($transformee_results as $transformee_result) {
    islandora_manuscript_remove_transform(
      islandora_object_load($transformee_result['transformee']['value'])
    );
    $context['sandbox']['current']++;
    $context['finished'] = $context['sandbox']['current'] / $context['sandbox']['total_count'];
  }
}

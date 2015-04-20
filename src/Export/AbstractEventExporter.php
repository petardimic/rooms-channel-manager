<?php

/**
 * @file
 * Contains \Drupal\rooms_channel_manager\Export\EventExporter
 */

namespace Drupal\rooms_channel_manager\Export;

abstract class AbstractEventExporter implements EventExporterInterface {

  // Holds the actual configuration information.
  public $config;
  protected $source_name = '';

  public function __construct() {
    $this->config = new StdClass;
    $this->export_type = '';
  }

  public function save() {
    $object = array(
      'unit_id' => $this->config->unit_id,
      'module' => $this->config->module,
      'config' => serialize($this->config),
    );
    if (db_query_range("SELECT COUNT(unit_id) FROM {rooms_channel_manager_export} WHERE unit_id = :unit_id AND module = :module", 0, 1, array(':unit_id' => $this->config->unit_id, ':module' => $this->config->module))->fetchField() > 0) {
      drupal_write_record('rooms_channel_manager_export', $object, array('unit_id', 'module'));
    }
    else {
      drupal_write_record('rooms_channel_manager_export', $object);
    }
  }

  public function load() {
    if (isset($this->config->unit_id)) {
      if ($record = db_query("SELECT config FROM {rooms_channel_manager_export} WHERE unit_id = :unit_id AND module = :module", array(':unit_id' => $this->config->unit_id, ':module' => $this->config->module))->fetchObject()) {
        if (isset($record->config)) {
          $this->config = unserialize($record->config);
        }
      }
    }
  }

  /**
   * Provides base configuration form.
   */
  public function config_form() {
    $form[$this->export_type] = array(
      '#type' => 'fieldset',
      '#title' => t('%type availability export.', array('%type' => $this->export_type)),
    );

    $form[$this->export_type]['submit'] = array(
      '#type' => 'submit',
      '#weight' => 99,
      '#value' => t('Save changes'),
    );

    return $form;
  }

  /**
   * Return the year and the month of the last event of a specific unit.
   */
  public function get_last_event() {
    $this->load();

    $result = db_select('rooms_availability', 't')
      ->fields('t')
      ->condition('unit_id', (int) $this->config->unit_id)
      ->execute()
      ->fetchAll();

    $year = 0;
    $month = 0;
    foreach ($result as $booking) {
      if ($booking->year > $year) {
        $year = $booking->year;
      }
    }

    foreach ($result as $booking) {
      if ($booking->month > $month && $booking->year == $year) {
        $month = $booking->month;
      }
    }

    $event = array(
      'year' => $year,
      'month' => $month,
    );

    return $event;
  }
}

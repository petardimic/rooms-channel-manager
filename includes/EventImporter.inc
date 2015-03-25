<?php

/**
 * Defines EventImporter base class.
 */

class EventImporter {

  // Holds the actual configuration information.
  public $config;

  public function __construct() {
    $this->config = new StdClass;
  }

  public function save() {
    $object = array(
      'unit_id' => $this->config->unit_id,
      'module' => $this->config->module,
      'config' => serialize($this->config),
    );
    if (db_query_range("SELECT COUNT(unit_id) FROM {rooms_channel_manager_sources} WHERE unit_id = :unit_id AND module = :module", 0, 1, array(':unit_id' => $this->config->unit_id, ':module' => $this->config->module))->fetchField() > 0) {
      drupal_write_record('rooms_channel_manager_sources', $object, array('unit_id', 'module'));
    }
    else {
      drupal_write_record('rooms_channel_manager_sources', $object);
    }
  }

  public function load() {
    if (isset($this->config->unit_id)) {
      if ($record = db_query("SELECT config FROM {rooms_channel_manager_sources} WHERE unit_id = :unit_id AND module = :module", array(':unit_id' => $this->config->unit_id, ':module' => $this->config->module))->fetchObject()) {
        if (isset($record->config)) {
          $this->config = unserialize($record->config);
        }
      }
    }
  }


  /**
   * Fetch Events from remote source and import bookings.
   */
  public function importBookingsFromSource() {
    $events = $this->fetch();
    $this->importBookings($events);
  }

  /**
   * Fetch Events to import.
   *
   * @return array events
   */
  public function fetch() {}

  /**
   * Add remote bookings.
   */
  public function importBookings($events) {
    foreach ($events as $event) {
      $booking = rooms_booking_create(array('unit_id' => $this->config->unit_id,
                                            'type' => 'external_booking',
                                            'uid' => 0,
                                            'start_date' => $event['startDate'],
                                            'end_date' => $event['endDate']));
      $booking->created = time();
      $booking->name = $event['summary'];
      $booking->title = $event['summary'];
      $booking->status = 1;
      rooms_booking_save($booking);
    }
  }


}

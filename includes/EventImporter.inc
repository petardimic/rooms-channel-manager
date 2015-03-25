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
   * Determine if we can/should fetch updates from the remote source. The
   * default behavior is to wait at least 1 hour between updates.
   */
  public function updateNeeded() {
    if (isset($this->config->module) && isset($this->config->unit_id)) {
      $last_updated = db_query("SELECT last_updated FROM {rooms_channel_manager_sources} WHERE module = :module AND unit_id = :unit_id",
                         array(':module' => $this->config->module, ':unit_id' => $this->config->unit_id))->fetchField();
      if ((REQUEST_TIME - $last_updated) > 3600) {
        return TRUE;
      }
    }
    return FALSE;
  }

  /**
   * Fetch Events from remote source and import bookings.
   */
  public function importBookingsFromSource() {
    $events = $this->fetch();
    $this->importBookings($events);
    if (isset($this->config->module) && isset($this->config->unit_id)) {
      $record = array(
        'last_updated' => REQUEST_TIME,
        'module' => $this->config->module,
        'unit_id' => $this->config->unit_id,
      );
      drupal_write_record('rooms_channel_manager_sources', $record, array('module', 'unit_id'));
    }
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
                                            'booking_status' => ROOMS_ANON_BOOKED,
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

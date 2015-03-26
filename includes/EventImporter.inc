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
   * Find customer name for an event.
   */
  public function getCustomerName($event) {
    return '';
  }

  /**
   * Add remote bookings.
   */
  public function importBookings($events) {

    // Get this unit's availability calendar.
    $uc = new UnitCalendar($this->config->unit_id);

    // Import external events.
    foreach ($events as $event) {
      if ($event['type'] == 'booking') {

        $name = $this->getCustomerName($event);

        // Ensure that this is not a duplicate booking.
        $result = db_query("SELECT COUNT(booking_id) FROM {rooms_bookings} WHERE name = :name AND start_date = :start_date AND end_date = :end_date",
                     array(':name' => $name, ':start_date' => $event['startDate'], ':end_date' => $event['endDate']))->fetchField();
        if ($result > 0) {
          continue;
        }

        // Make a user ID for this customer.
        $account = new StdClass();
        $account->name = rooms_channel_manager_unique_username($name);
        $account->pass = user_password(20);
        $account->status = 1;
        user_save($account);

        // Generate commerce profile.
        $profile = commerce_customer_profile_new('billing', $account->uid);
        $profile_wrapper = entity_metadata_wrapper('commerce_customer_profile', $profile);
        $profile_wrapper->commerce_customer_address->name_line = $name;
        commerce_customer_profile_save($profile);

        // Create booking.
        $booking = rooms_booking_create(array('unit_id' => $this->config->unit_id,
                                              'type' => 'external_booking',
                                              'uid' => $account->uid,
                                              'booking_status' => ROOMS_ANON_BOOKED,
                                              'start_date' => $event['startDate'],
                                              'end_date' => $event['endDate']));
        $booking->created = time();
        $booking->name = $name;
        $booking->title = $event['summary'];
        $booking->status = 1;
        rooms_booking_save($booking);
      }
      else {
        $event_id = ROOMS_NOT_AVAILABLE;
        $startDateTime = DateTime::createFromFormat('Y-m-d', $event['startDate']);
        $endDateTime = DateTime::createFromFormat('Y-m-d', $event['endDate']);
        $be = new BookingEvent($this->config->unit_id, $event_id, $startDateTime, $endDateTime);
        $events = array($be);
        $response = $uc->updateCalendar($events);
        if ($response[$event_id] == ROOMS_BLOCKED) {
          drupal_set_message(t('Could not update calendar because a locked event is blocking the update - you need to unlock any locked events in that period.'), 'warning');
        }
        elseif ($response[$event_id] == ROOMS_UPDATED) {
          drupal_set_message(t('Calendar Updated'));
        }
      }
    }
  }

}

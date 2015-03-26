<?php

/**
 * iCal Event importer class.
 */

class AirBnbEventImporter extends iCalEventImporter {

  public function __construct() {
    parent::__construct();
    $this->config->module = 'rooms_airbnb';
  }

  /**
   * Override config form form Airbnb-specific configuration.
   */
  public function config_form() {
    $form = parent::config_form();
    $form['ical_url']['#title'] = t('Airbnb iCal link');
    return $form;
  }

  /**
   * Get the customer's name for an event.
   */
  public function getCustomerName($event) {
    preg_match('/((\w[\s]?)*)( \([A-Z0-9]{6}\)$)/', $event['summary'], $matches);
    return $matches[1];
  }


  public function fetch() {
    $events = parent::fetch();
    foreach ($events as &$event) {

      // Parse Airbnb's ical format to determine what events are actual bookings.
      // If the summary line ends with a booking reference, this is a booking.
      if (!preg_match('/ \([A-Z0-9]{6}\)$/', $event['summary'])) {
        $event['type'] = 'availability';
      }
    }

    return $events;
  }

}

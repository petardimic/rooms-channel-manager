<?php

/**
 * @file
 * Contains \Drupal\rooms_airbnb\Import\AirBnbEventImporter
 */

namespace Drupal\rooms_airbnb\Import;

class AirBnbEventImporter extends \Drupal\rooms_channel_manager\Import\iCalEventImporter {

  protected $source_name = 'Airbnb';

  public function __construct() {
    parent::__construct();
    $this->config->module = 'rooms_airbnb';
  }

  /**
   * Override config form with Airbnb-specific configuration.
   */
  public function loadConfigForm() {
    $form = parent::loadConfigForm();
    $form[$this->source_name]['ical_url']['#title'] = t('Airbnb iCal link');
    return $form;
  }

  /**
   * Get the customer's name for an event.
   */
  public function getCustomerName($event) {
    preg_match('/((\w[\s]?)*)( \([A-Z0-9]{6}\)$)/', $event['summary'], $matches);
    return $matches[1];
  }

  /**
   * Get the booking reference for an event.
   */
  public function getBookingReference($event) {
    preg_match('/((\w[\s]?)*)( \(([A-Z0-9]{6})\)$)/', $event['summary'], $matches);
    return $matches[4];
  }

  /**
   * Get the link for an event.
   */
  public function getBookingLink($event) {
    return 'https://www.airbnb.com/reservation/itinerary?code=' . $this->getBookingReference($event);
  }

  public function fetchEvents() {
    $events = parent::fetchEvents();
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

<?php

/**
 * @file
 * Contains \Drupal\rooms_channel_manager\Import\EventImporter
 */

namespace Drupal\rooms_channel_manager\Import;

interface EventImporterInterface {

  public function setConfig();
  public function getConfig();
  public function loadConfigForm();
  public function updateNeeded();
  public function importBookingsFromSource();
  public function fetchEvents();
  public function getCustomerName($event);
  public function importBookings($events);

}

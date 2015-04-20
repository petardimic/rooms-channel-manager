<?php

/**
 * @file
 * Contains \Drupal\rooms_channel_manager\Import\EventImporter
 */

namespace Drupal\rooms_channel_manager\Import;

interface EventImporterInterface {

  public function setConfig();
  public function getConfig();
  public function config_form();
  public function updateNeeded();
  public function importBookingsFromSource();
  public function fetch();
  public function getCustomerName($event);
  public function importBookings($events);

}

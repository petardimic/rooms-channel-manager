<?php

/**
 * iCal Event importer class.
 */

class AirBnbEventImporter extends iCalEventImporter {

  public function __construct() {
    parent::__construct();
    $this->config->module = 'rooms_airbnb';
  }

}

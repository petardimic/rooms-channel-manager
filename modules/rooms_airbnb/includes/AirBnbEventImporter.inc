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

}

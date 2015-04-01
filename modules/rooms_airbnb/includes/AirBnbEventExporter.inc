<?php

/**
 * Defines EventImporter base class.
 */

class AirBnbEventExporter extends iCalEventExporter {

  // Holds the actual configuration information.
  public $config;

  public function __construct() {
    parent::__construct();
    $this->config->module = 'rooms_airbnb';
  }

  /**
   * Provides base configuration form.
   */
  public function config_form() {
    $form = parent::config_form();

    // FIXME
    return $form;
  }

}

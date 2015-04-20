<?php

/**
 * @file
 * Contains \Drupal\rooms_airbnb\Export\AirBnbEventExporter
 */

namespace Drupal\rooms_airbnb\Export;

class AirBnbEventExporter extends \Drupal\rooms_channel_manager\Export\iCalEventExporter {

  // Holds the actual configuration information.
  public $config;

  public function __construct() {
    parent::__construct();
    $this->config->module = 'rooms_airbnb';
  }

  /**
   * Provides base configuration form.
   */
  public function loadConfigForm() {
    $form = parent::loadConfigForm();

    // FIXME
    return $form;
  }

}

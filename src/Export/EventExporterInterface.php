<?php

/**
 * @file
 * Contains \Drupal\rooms_channel_manager\Export\EventExporterInterface
 */

namespace Drupal\rooms_channel_manager\Export;

interface EventExporterInterface {

  public function save();
  public function load();
  public function config_form();

}

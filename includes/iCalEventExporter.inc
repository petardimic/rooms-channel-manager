<?php

/**
 * Defines EventImporter base class.
 */

class iCalEventExporter extends EventExporter {

  // Holds the actual configuration information.
  public $config;

  public function __construct() {
    $this->config = new StdClass;
    $this->export_type = 'iCal';
  }

  /**
   * Provides base configuration form.
   */
  public function config_form() {
    if ($unit = menu_get_object('rooms_unit', 4)) {
      $this->config->unit_id = $unit->unit_id;
    }
    $this->load();
    $form = parent::config_form();

    $state_options = array(
      '0' => t('Not Available'),
      '2' => t('On Request'),
      '3' => t('Anonymous Booking'),
      '-1' => t('Unconfirmed Booking'),
      '11' => t('Confirmed Booking'),
    );

    $default_states = array_keys($state_options);

    $form[$this->export_type]['states'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Select states to export'),
      '#options' => $state_options,
      '#default_value' => isset($this->config->states) ? $this->config->states : $default_states,
    );

    if (is_object($unit)) {
      $form[$this->export_type]['unit_id'] = array(
        '#type' => 'hidden',
        '#value' => $unit->unit_id,
      );
    }

    if (isset($this->config->uuid) && strlen($this->config->uuid)) {
      $ics_link = url('rooms/availability/' . $this->config->module . '/' . $this->config->unit_id . '/availability/' . $this->config->uuid, array('absolute' => TRUE));
      $form[$this->export_type]['export_link'] = array(
        '#markup' => '<div class="ical-export-link"><h3>iCal export link</h3>' .
                     $ics_link . ' (<a href="' . $ics_link . '">download ICS file</a>)' .
                     '</div><br />',
      );
    }

    return $form;
  }

  public function config_form_submit($form, &$form_state) {
    if (isset($form_state['values']['unit_id'])) {
      $this->config->unit_id = $form_state['values']['unit_id'];
      $this->load();
      $this->config->states = $form_state['values']['states'];

      if (!isset($this->config->uuid)) {
        $this->config->uuid = uuid_generate();
      }

      // Generate a UUID for this source so that the URI can not be guessed.
      $this->save();
    }
  }

  /**
   * Callback for availability export.
   */
  public static function export_availability($module, $unit, $uuid) {

    // Find the class name for this module.
    $info_callback = $module . '_rooms_channel_export';
    $info = current(call_user_func($info_callback));

    // As this is a static method, we must instantiate an object to work with.
    $object = new $info['handler']['class'];
    $object->config->unit_id = $unit->unit_id;
    $object->load();

    // Check if the uuid matches, and throw a 404 if not.
    if ($object->config->uuid != $uuid) {
      drupal_not_found();
    }

    // Looking for the last event.
    $last_event = $object->get_last_event($unit);
    $start = date('Y-m-d');

    // Setting end date to export 1 month over the last event.
    $end = new DateTime($last_event['year'] . '-' . $last_event['month'] . '-' . '01');
    $end = $end->add(new DateInterval('P1M'));
    $end = $end->format('Y-m-d');

    drupal_add_http_header('Content-Type', 'text/calendar; utf-8');
    print $object->generate_ics($unit, $object->config->states, $start, $end);
    exit(0);
  }

  /**
   * Generate the iCal data.
   */
  public function generate_ics(RoomsUnit $unit, $states, $start, $end) {
    $this->load();

    $config = array("unique_id" => $GLOBALS['base_url']);
    $vcalendar = new vcalendar($config);
    $vcalendar->setProperty("method", "PUBLISH");
    $vcalendar->setProperty("x-wr-calname", "Rooms Calendar");
    $vcalendar->setProperty("X-WR-CALDESC", "Rooms calendar .ics format");
    $uuid = $this->config->uuid;
    $vcalendar->setProperty("X-WR-RELCALID", $uuid);
    $vcalendar->setProperty("X-WR-TIMEZONE", date_default_timezone());

    $config = $vcalendar->getConfig();

    $start_date = explode('-', $start);
    $end_date = explode('-', $end);

    // Check json from the current date to the end date of last event.
    $json = rooms_availability_generate_json($unit, $start_date[0], $start_date[1], $start_date[2], $end_date[0], $end_date[1], $end_date[2], $event_style = ROOMS_AVAILABILITY_ADMIN_STYLE);

    foreach ($json as $event) {
      $start = new DateTime($event['start']);
      $start = $start->format('Ymd\T\13:00:00');
      $end = new DateTime($event['end']);
      $end = $end->format('Ymd\T\13:00:00');

      // Having an event of one single day, we have to set it like an 'All Day'
      // event. we must remove time from the start date and remove end date.
      $all_day = FALSE;
      if ($start == $end) {
        $all_day = TRUE;
      }

      // THE EVENT IS AN UNCONFIRMED EVENT.
      if ((int) $event['id'] < 0 && in_array('-1', $states)) {
        $vevent = new vevent($config);

        if ($all_day) {
          // This will set of one day as an All Day event.
          $vevent->setProperty("DTSTART", $start, array("VALUE" => "DATE"));
        }
        else {
          $vevent->setProperty("DTSTART", $start);
          $vevent->setProperty("DTEND", $end);
        }

        $vevent->setProperty("SUMMARY", $event['title']);
        $vevent->setProperty("LOCATION", $unit->name);

        $vcalendar->setComponent($vevent);
      }

      // THE EVENT IS A CONFIRMED EVENT.
      if ((int) $event['id'] > 11 && in_array('11', $states)) {
        $vevent = new vevent($config);

        if ($all_day) {
          // This will set of one day as an All Day event.
          $vevent->setProperty("DTSTART", $start, array("VALUE" => "DATE"));
        }
        else {
          $vevent->setProperty("DTSTART", $start);
          $vevent->setProperty("DTEND", $end);
        }

        $vevent->setProperty("SUMMARY", 'Booking: ' . $event['title']);
        $vevent->setProperty("LOCATION", $unit->name);

        $vcalendar->setComponent($vevent);
      }

      // THE EVENT IS: NOT-AVAILABLE, ON-REQUEST OR ANONYMOUS BOOKING.
      if (in_array($event['id'], $states)) {
        $vevent = new vevent($config);

        if ($all_day) {
          // This will set of one day as an All Day event.
          $vevent->setProperty("DTSTART", $start, array("VALUE" => "DATE"));
        }
        else {
          $vevent->setProperty("DTSTART", $start);
          $vevent->setProperty("DTEND", $end);
        }

        $vevent->setProperty("SUMMARY", $event['title']);
        $vevent->setProperty("LOCATION", $unit->name);

        $vcalendar->setComponent($vevent);
      }
    }

    // Create the calendar to export.
    $ics = $vcalendar->createCalendar();
    return $ics;
  }

}

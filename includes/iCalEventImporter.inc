<?php

/**
 * iCal Event importer class.
 */

class iCalEventImporter extends EventImporter {

  protected $source_name = 'iCal';

  /**
   * Fetch events from a remote iCal URL.
   */
  public function fetch() {

    $headers = array();
    // TODO
    // $headers = array('If-Modified-Since' => gmdate(DATE_RFC1123, $last_fetched));
    $response = drupal_http_request($this->config->url, array('headers' => $headers));
    if ($response->code !== '200') {
      throw new Exception('Did not receive expected response from remote ical URL.');
    }

    $data = $response->data;
    // Parse iCal data.
    $vcalendar = new vcalendar();
    $vcalendar->parse($data);
    $now = $datetime = new DateTime();

    $events = array();
    while ($vevent = $vcalendar->getComponent('vevent')) {

      // Get event start date.
      $dtstart_array = $vevent->getProperty('dtstart', 1, TRUE);
      $dtstart = $dtstart_array['value'];
      $startDate = $dtstart['year'] . '-' . sprintf('%02d', $dtstart['month']) . '-' . sprintf('%02d', $dtstart['day']);
      $startDateTime = DateTime::createFromFormat('Y-m-d', $startDate);
      if ($startDateTime > $now) {

        // This event is in the future.
        $dtend_array = $vevent->getProperty('dtend', 1, TRUE);
        $dtend = $dtend_array['value'];
        $endDate = $dtend['year'] . '-' . sprintf('%02d', $dtend['month']) . '-' . sprintf('%02d', $dtend['day']);
        $endDateTime = DateTime::createFromFormat('Y-m-d', $endDate);

        // A Rooms booking event expects the end date to be the date before
        // departure.
        $endDateTime->sub(new DateInterval('P1D'));

        $description = '';
        if (!empty($vevent->description)) {
          $description = $vevent->description[0]['value'];
        }

        $events[] = array(
          'type' => 'booking',
          'startDate' => $startDate,
          'endDate' => $endDate,
          'summary' => $vevent->summary['value'],
          'description' => $description,
        );
      }
    }

    return $events;
  }

  public function config_form() {
    $form = parent::config_form();

    $form[$this->source_name]['ical_url'] = array(
      '#type' => 'textfield',
      '#title' => t('iCal URL'),
      '#description' => t('Enter an iCal URL - availability will be synced from this calendar.'),
      '#default_value' => isset($this->config->url) ? $this->config->url : '',
    );

    if ($unit = menu_get_object('rooms_unit', 4)) {
      $form[$this->source_name]['unit_id'] = array(
        '#type' => 'hidden',
        '#value' => $unit->unit_id,
      );
    }

    return $form;
  }

  public function config_form_validate($form, &$form_state) {
    if (empty($form_state['values']['ical_url'])) {
      form_set_error('ical_url', t('Please enter a URL'));
    }
    if (!valid_url($form_state['values']['ical_url'], TRUE)) {
      form_set_error('ical_url', t('The URL %url is invalid. Enter a fully-qualified URL, such as http://www.example.com/feed.xml.', array('%url' => $form_state['values']['ical_url'])));
    }
  }

  public function config_form_submit($form, &$form_state) {
    $this->load();
    if (isset($form_state['values']['unit_id'])) {
      $this->config->unit_id = $form_state['values']['unit_id'];
      $this->config->confirm_bookings = $form_state['values']['confirm_bookings'];
      $this->config->url = $form_state['values']['ical_url'];
      $this->save();
    }
  }

}

<?php

/**
 * @file
 * Contains \Drupal\rooms_channel_manager\Import\iCalEventImporter
 */

namespace Drupal\rooms_channel_manager\Import;

class iCalEventImporter extends AbstractEventImporter {

  protected $source_name = 'iCal';

  /**
   * Fetch events from a remote iCal URL.
   */
  public function fetchEvents() {

    $headers = array();
    // TODO
    // $headers = array('If-Modified-Since' => gmdate(DATE_RFC1123, $last_fetched));
    $response = drupal_http_request($this->config->url, array('headers' => $headers));
    if ($response->code !== '200') {
      throw new Exception('Did not receive expected response from remote ical URL.');
    }

    $data = $response->data;
    $lines = explode("\n", $data);

    // Parse iCal data.
    $ical = new \ICal($lines);
    $now = $datetime = new \DateTime();

    $events = array();
    foreach ($ical->events() as $vevent) {

      // Get event start date.
      $dtstart = $vevent['DTSTART'];
      $startDateTime = \DateTime::createFromFormat('Ymd', $dtstart);
      $startDate = $startDateTime->format('Y-m-d');
      if ($startDateTime > $now) {

        // This event is in the future.
        $dtend = $vevent['DTEND'];
        $endDateTime = \DateTime::createFromFormat('Ymd', $dtend);
        $endDate = $endDateTime->format('Y-m-d');

        // A Rooms booking event expects the end date to be the date before
        // departure.
        $endDateTime->sub(new \DateInterval('P1D'));

        $description = '';
        if (!empty($vevent['DESCRIPTION'])) {
          $description = $vevent['DESCRIPTION'];
        }

        $events[] = array(
          'type' => 'booking',
          'startDate' => $startDate,
          'endDate' => $endDate,
          'summary' => $vevent['SUMMARY'],
          'description' => $description,
        );
      }
    }

    return $events;
  }

  public function loadConfigForm() {
    $form = parent::loadConfigForm();

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

  public function configFormValidate($form, &$form_state) {
    if (empty($form_state['values']['ical_url'])) {
      form_set_error('ical_url', t('Please enter a URL'));
    }
    if (!valid_url($form_state['values']['ical_url'], TRUE)) {
      form_set_error('ical_url', t('The URL %url is invalid. Enter a fully-qualified URL, such as http://www.example.com/feed.xml.', array('%url' => $form_state['values']['ical_url'])));
    }
  }

  /**
   * Save iCal import configuration settings.
   */
  public function configFormSubmit($form, &$form_state) {
    $this->getConfig();
    if (isset($form_state['values']['unit_id'])) {
      $this->config->unit_id = $form_state['values']['unit_id'];
      $this->config->confirm_bookings = $form_state['values']['confirm_bookings'];
      $this->config->url = $form_state['values']['ical_url'];
      $this->setConfig();
    }
  }

}

<?php
/**
 * @file
 * Availability Calendars: code to edit the calendar and per node settings.
 *
 * @author Dan Karran (geodaniel) <dan at karran dot net>
 * @author Nicholas Alipaz (nicholas.alipaz)
 * @author Erwin Derksen (http://drupal.org/user/750928)
 */

module_load_include('inc', 'availability_calendars', 'availability_calendars');

/**
 * Alters the form for node edit forms for supported content types.
 *
 * Called by our hook_form_alter implementation. For parameters
 * @see http://api.drupal.org/api/drupal/developer--hooks--core.php/function/hook_form_alter/6
 */
function availability_calendars_node_form_alter(&$form, &$form_state, $form_id) {
  $settings = availability_calendars_get_settings('node', $form['nid']['#value']);

  $form['availability_calendars'] = array(
    '#type' => 'fieldset',
    '#title' => t('Availability calendar settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['availability_calendars']['showteaser'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show availability calendars within teasers'),
    '#default_value' => $settings->showteaser,
  );
  // Allow users to choose whether a key should be shown on the node on a per node basis
  $form['availability_calendars']['showkey'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show a key for the availability calendars'),
    '#default_value' => $settings->showkey,
  );
  // Whether to use only the first letter for the day of the week or not
  $form['availability_calendars']['firstletter'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use only the first letter from the day of the week'),
    '#default_value' => $settings->firstletter,
  );
  $form['availability_calendars']['hideold'] = array(
    '#type' => 'checkbox',
    '#title' => t('Do not show availability state of dates in the past'),
    '#default_value' => $settings->hideold,
  );
  $form['availability_calendars']['splitday'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow split day statuses'),
    '#default_value' => $settings->splitday,
  );
  $form['availability_calendars']['startofweek'] = array(
    '#type' => 'select',
    '#title' => t('First day of week'),
    '#default_value' => $settings->startofweek,
    '#options' => array(
      6 => t('Monday'),
      5 => t('Tuesday'),
      4 => t('Wednesday'),
      3 => t('Thursday'),
      2 => t('Friday'),
      1 => t('Saturday'),
      0 => t('Sunday'),
    ),
  );
  $form['availability_calendars']['monthcount'] = array(
    '#type' => 'textfield',
    '#title' => t('Number of months to display'),
    '#default_value' => $settings->monthcount,
    '#description' => t("Your calendars will show this number of months to all users except those with the 'edit own availability calendars' or 'edit availability calendars' who will always see 3 extra months on the calendars they can edit. This is to allow them to enter information into future calendars before it is made publicly available."),
  );
  $form['availability_calendars']['defaultstatus'] = array(
    '#type' => 'select',
    '#title' => t('Default status for each day'),
    '#default_value' => $settings->defaultstatus,
    '#options' => availability_calendars_options(),
  );
}

/**
 * Show the availability calendar. (callback for path availability-calendars/%)
 *
 * @param int $nid the node id to show the calendar for.
 * @param int|NULL $year the year to show the calendar for. If NULL, show from current month onwards.
 * @param int|NULL $month the month to show the calendar for. If NULL, show the whole year
 * @param boolean $edit Whether to show the edit form for the given year and month?
 * @return string themed calendar or edit calendar form
 */
function availability_calendars_page($nid = NULL, $year = NULL, $month = NULL, $edit = FALSE) {
  if (is_numeric($nid)) {
    $node = node_load($nid);
    drupal_set_title(t('Availability for !name', array('!name' => $node->title)));

    if (is_numeric($year) && is_numeric($month)) {
      drupal_set_title(t('Availability for !name in !date', array('!name' => $node->title, '!date' => format_date(mktime(12, 0, 0, $month, 1, $year), 'custom', 'F Y'))));
      if ($edit == 'edit') {
        // Display the edit form for the availability calendar on the node
        return availability_calendars_node_edit($node, $year, $month);
      }
      else {
        // Display availability calendar for single month
        $monthstodisplay = 1;
        return theme('availability_calendars_node', $node, $year, $month, $monthstodisplay);
      }
    }
    elseif (is_numeric($year)) {
      // Display availability calendar for a whole year
      $month = 1;
      $monthstodisplay = 12;
      return theme('availability_calendars_node', $node, $year, $month, $monthstodisplay);
    }
    else {
      // Display rolling availability calendar from this point onwards
      $year = date('Y');
      $month = date('m');
      $settings = availability_calendars_get_settings();
      $monthstodisplay = $settings->monthcount;
      return theme('availability_calendars_node', $node, $year, $month, $monthstodisplay);
    }
  }
  else {
    print drupal_not_found();
  }
}

/**
 * Produces the edit calendar form for the given node in the given year and month.
 *
 * @param object $node
 * @param int $year
 * @param int $month
 * @return string themed form to edit the calendar.
 */
function availability_calendars_node_edit($node, $year, $month) {
  return drupal_get_form('availability_calendars_node_edit_form', $node, $year, $month);
}

/**
 * Populates the node edit form.
 *
 * @return array
 */
function availability_calendars_node_edit_form($form_state, $node, $year, $month) {
  $form = array();
  $nid = $node->nid;
  $settings = availability_calendars_get_settings('node', $nid);
  $form['nid'] = array(
    '#type' => 'hidden',
    '#default_value' => $nid,
  );
  $form['year'] = array(
    '#type' => 'hidden',
    '#default_value' => $year,
  );
  $form['month'] = array(
    '#type' => 'hidden',
    '#default_value' => $month,
  );

  $month_meta = availability_calendars_month_meta($year, $month, $settings);

  // find all entries in database for this month (notes and availability) and pre-populate
  $notes = availability_calendars_get_node_notes($nid, $year, $month);
  $states = availability_calendars_get_node_states($nid, $year, $month, $settings);

  $day = 1;
  for ($week = 1; $week <= $month_meta['weeksinmonth']; $week++) {
    $form['week-' . $week] = array(
      '#type' => 'fieldset',
      '#title' => t('Week @week', array('@week' => $week)),
    );
    $form['week-' . $week]['note-' . $week] = array(
      '#type' => 'textfield',
      '#title' => t('Note'),
      '#default_value' => $notes[$week],
      '#description' => t('This will be displayed beside the week in the calendar. It could include, for example, a weekly price.'),
    );
    if ($week == 1) {
      $daysinweekremaining = 7 - $month_meta['firstday'];
    }
    else {
      $daysinweekremaining = 7;
    }
    while ($daysinweekremaining > 0 && $day <= $month_meta['daysinmonth']) {
      $form['week-' . $week]['day-' . $day] = array(
        '#type' => 'select',
        '#title' => format_date(mktime(12, 0, 0, $month, $day, $year), 'custom', 'l j F'),
        '#options' => availability_calendars_options(),
        '#default_value' => $states[$day],
      );
      $day++;
      $daysinweekremaining--;
    }
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
  );

  return $form;
}

/**
 * Callback function for submitting a node edit form.
 *
 * @param array $form
 * @param array $form_state
 */
function availability_calendars_node_edit_form_submit($form, &$form_state) {
  $nid = $form_state['values']['nid'];
  $year = $form_state['values']['year'];
  $month = $form_state['values']['month'];

  // extract weekly notes from the form
  $notes = array();
  $week = 1;
  $nomoreweeks = FALSE;
  while (!$nomoreweeks) {
    if (isset($form_state['values']['note-' . $week])) {
      $notes[$week] = $form_state['values']['note-' . $week];
      $week++;
    }
    else {
      $nomoreweeks = TRUE;
    }
  }

  // extract states from the form
  $states = array();
  $day = 1;
  $nomoredays = FALSE;
  while (!$nomoredays) {
    if (isset($form_state['values']['day-' . $day])) {
      $states[$day] = $form_state['values']['day-' . $day];
      $day++;
    }
    else {
      $nomoredays = TRUE;
    }
  }

  // update
  availability_calendars_update_node_notes($nid, $year, $month, $notes);
  availability_calendars_update_node_states($nid, $year, $month, $states);

  // Clear the page and block caches (#747992).
  // @todo: this line copied from node_save, a previous revision called node_save itself,
  //   this revision only cleans the cache but doesn't update the fields 'changed' and 'timestamp' of the node.
  //   If that is what we want, call node_save(), but note that node_save will call
  //   availability_calendars_node_save() without the settings being set/available: E_NOTICE's
  cache_clear_all();

  drupal_set_message(t('Availability information saved.'));
}

/**
 * Saves a set of settings associated with the node after inserting/updating it.
 *
 * @param object $node Node settings information to save into the database.
 */
function availability_calendars_save_node_settings($node) {
  variable_set('availability_calendars_settings_node_' . $node->nid . '_showteaser', $node->showteaser);
  variable_set('availability_calendars_settings_node_' . $node->nid . '_showkey', $node->showkey);
  variable_set('availability_calendars_settings_node_' . $node->nid . '_firstletter', $node->firstletter);
  variable_set('availability_calendars_settings_node_' . $node->nid . '_hideold', $node->hideold);
  variable_set('availability_calendars_settings_node_' . $node->nid . '_splitday', $node->splitday);
  variable_set('availability_calendars_settings_node_' . $node->nid . '_startofweek', $node->startofweek);
  variable_set('availability_calendars_settings_node_' . $node->nid . '_monthcount', $node->monthcount);
  variable_set('availability_calendars_settings_node_' . $node->nid . '_defaultstatus', $node->defaultstatus);
}

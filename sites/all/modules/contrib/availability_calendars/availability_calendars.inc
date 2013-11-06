<?php
/**
 * @file
 * General helper methods for Availability Calendars, like database access and settings.
 *
 * @author Dan Karran (geodaniel) <dan at karran dot net>
 * @author Nicholas Alipaz (nicholas.alipaz)
 * @author Erwin Derksen (http://drupal.org/user/750928)
 */

/**
 * Utility function to create an array of meta data for the month.
 *
 * @param int $year
 * @param int $month
 * @param object $settings
 * @return array
 */
function availability_calendars_month_meta($year, $month, $settings) {
  $month_meta['daysinmonth'] = date("t", mktime(0, 0, 0, $month, 1, $year));
  $month_meta['firstday'] = date("w", mktime(0, 0, 0, $month, 1, $year)) + $settings->startofweek;
  $temp_days = $month_meta['firstday'] + $month_meta['daysinmonth']; // padding
  $month_meta['weeksinmonth'] = ceil($temp_days / 7);

  // Stop empty weeks occuring at start of month
  if ($month_meta['firstday'] > 6) {
    $month_meta['firstday'] = $month_meta['firstday'] - 7;
    $month_meta['weeksinmonth'] --;
  }
  return $month_meta;
}

/**
 * availability_calendars status options.
 * note: we return unescaped labels as they might be used as options in a select where they get escaped again
 *
 * @return array array with the classes as the keys and the translated but unescaped labels as values
 */
function availability_calendars_options() {
  static $ret = NULL;
  if ($ret === NULL) {
    $ret = array();
    $settings = availability_calendars_get_settings();
    $statuses = availability_calendars_get_states();
    foreach ($statuses as $class => $state) {
      $ret[$class] = $state['label'];
    }
    if ($settings->splitday === 1) {
      foreach ($statuses as $class => $state) {
        $sub = $statuses;
        unset($sub[$class]);
        foreach ($sub as $subclass => $substate) {
          $ret["$class-am $subclass-pm"] = t('!a (am)/!b (pm)', array('!a' => $state['label'], '!b' => $substate['label']));
        }
      }
    }
  }
  return $ret;
}


/**
 * Utility function to get settings related to nodes or administration.
 *
 * @param string $scope type of settings to get: node or system
 * @param int|NULL $arg nid
 * @return object An object with all settings for the given scope
 */
function availability_calendars_get_settings($scope = NULL, $arg = NULL) {
  $settings = new stdClass();
  $settings->pernodeoverride = variable_get('availability_calendars_settings_system_pernodeoverride', 0);

  if ($settings->pernodeoverride && $scope === 'node' && $arg !== NULL) {
    // Add per node settings, NULL if not defined
    $settings->startofweek = variable_get('availability_calendars_settings_node_' . $arg . '_startofweek', NULL);
    $settings->showteaser = variable_get('availability_calendars_settings_node_' . $arg . '_showteaser', NULL);
    $settings->showkey = variable_get('availability_calendars_settings_node_' . $arg . '_showkey', NULL);
    $settings->firstletter = variable_get('availability_calendars_settings_node_' . $arg . '_firstletter', NULL);
    $settings->hideold = variable_get('availability_calendars_settings_node_' . $arg . '_hideold', NULL);
    $settings->defaultstatus = variable_get('availability_calendars_settings_node_' . $arg . '_defaultstatus', NULL);
    $settings->monthcount = variable_get('availability_calendars_settings_node_' . $arg . '_monthcount', NULL);
    $settings->splitday = variable_get('availability_calendars_settings_node_' . $arg . '_splitday', NULL);
  }

  // Add system wide settings (with their hard coded defaults) as far as not set
  if (!isset($settings->startofweek)) {
    $settings->startofweek = variable_get('availability_calendars_settings_system_startofweek', 1);
  }
  if (!isset($settings->showteaser)) {
    $settings->showteaser = variable_get('availability_calendars_settings_system_showteaser', 1);
  }
  if (!isset($settings->showkey)) {
    $settings->showkey = variable_get('availability_calendars_settings_system_showkey', 1);
  }
  if (!isset($settings->firstletter)) {
    $settings->firstletter = variable_get('availability_calendars_settings_system_firstletter', 0);
  }
  if (!isset($settings->hideold)) {
    $settings->hideold = variable_get('availability_calendars_settings_system_hideold', 0);
  }
  if (!isset($settings->monthcount)) {
    $settings->monthcount = variable_get('availability_calendars_settings_system_monthcount', 12);
  }
  if (!isset($settings->splitday)) {
    $settings->splitday = variable_get('availability_calendars_settings_system_splitday', 0);
  }
  if (!isset($settings->defaultstatus)) {
    $defaultstatus = reset(availability_calendars_get_states());
    $settings->defaultstatus = variable_get('availability_calendars_settings_system_defaultstatus', $defaultstatus['class']);
  }

  // And add the 'system wide only' setting(s)
  $settings->nodeview = variable_get('availability_calendars_settings_system_nodeview', 1);

  return $settings;
}

/*
 * DATABASE ACCESS FUNCTIONS
 * -------------------------
 */

/**
 * Returns an array of records for all states.
 *
 * @return array array of records keyed by the class
 */
function availability_calendars_get_states() {
  static $states = NULL;
  if ($states === NULL) {
    $states = array();
    $result = db_query("SELECT * FROM {availability_calendars_states} ORDER BY weight");
    while ($row = db_fetch_array($result)) {
      $states[$row['class']] = $row;
    }
  }
  return $states;
}

/**
 * Updates the set of states
 *
 * @param array $states array with the new state records (class, label and weight values)
 */
function availability_calendars_update_states($states) {
  $existing_States = availability_calendars_get_states();
  if ($states != $existing_States) {
    // update states: delete all existing, insert all new states
    db_query("DELETE FROM {availability_calendars_states}");
    foreach ($states as $state) {
      db_query("INSERT INTO {availability_calendars_states} (class,label,weight) VALUES ('%s','%s',%d)", $state['class'], $state['label'], $state['weight']);
    }
  }
}

/**
 * Returns the notes for the calendar for the given node in the given month.
 * The returned array will be completely filled, so no checking is necessary.
 *
 * @param int $nid
 * @param int $year
 * @param int $month
 * @return array array with 6 week entries of week number (int) => note (string) (possibly empty string)
 */
function availability_calendars_get_node_notes($nid, $year, $month) {
  $notes = array_fill(1, 6, "");
  $result = db_query('SELECT week, note FROM {availability_calendars_week} WHERE nid = %d AND year = %d and month = %d', $nid, $year, $month);
  while ($note = db_fetch_array($result)) {
    $notes[$note['week']] = $note['note'];
  }
  return $notes;
}

/**
 * Updates the calendar notes for the given node in the given month.
 *
 * @param int $nid
 * @param int $year
 * @param int $month
 * @param array $notes the (possibly empty) notes keyed by week number
 */
function availability_calendars_update_node_notes($nid, $year, $month, $notes) {
  db_query('DELETE FROM {availability_calendars_week} WHERE nid = %d AND year = %d AND month = %d', $nid, $year, $month);
  foreach ($notes as $week => $note) {
    if (!empty($note)) {
      db_query("INSERT INTO {availability_calendars_week} (nid, year, month, week, note) VALUES (%d, %d, %d, %d, '%s')", $nid, $year, $month, $week, $note);
    }
  }
}

/**
 * Returns the states for the calendar for the given node in the given month.
 * The returned array will be completely filled, so no checking is necessary.
 *
 * @param int $nid
 * @param int $year
 * @param int $month
 * @param object $settings settings (containing among others the default status)
 * @return array array with 28 to 31 day states (string) keyed by the day ofg the month number (int).
 */
function availability_calendars_get_node_states($nid, $year, $month, $settings) {
  $days_in_month = date("t", mktime(0, 0, 0, $month, 1, $year));
  $states = array_fill(1, $days_in_month, $settings->defaultstatus);
  $result = db_query('SELECT day, status FROM {availability_calendars_day} WHERE nid = %d AND year = %d AND month = %d', $nid, $year, $month);
  while ($state = db_fetch_array($result)) {
    $states[$state['day']] = $state['status'];
  }
  return $states;
}

/**
 * Update the states for the calendar for the given node in the given month.
 *
 * @param int $nid
 * @param int $year
 * @param int $month
 * $param array $states array with 28 to 31 day states (string) keyed by the day of the month number (int).
 */
function availability_calendars_update_node_states($nid, $year, $month, $states) {
  db_query('DELETE FROM {availability_calendars_day} WHERE nid = %d AND year = %d AND month = %d', $nid, $year, $month);
  foreach ($states as $day => $status) {
    db_query("INSERT INTO {availability_calendars_day} (nid, year, month, day, status, date) VALUES (%d, %d, %d, %d, '%s', '%s')", $nid, $year, $month, $day, $status, "$year-$month-$day 12:00:00");
  }
}

/**
 * Deletes all calendar information for the given node:
 * - notes
 * - day states
 * - settings
 *
 * @param int $nid
 */
function availability_calendars_delete_node($nid) {
  db_query('DELETE FROM {availability_calendars_week} WHERE nid = %d', $nid);
  db_query('DELETE FROM {availability_calendars_day} WHERE nid = %d', $nid);
  availability_calendars_delete_node_settings($nid);
}

/**
 * Removes the per node settings for one or all nodes.
 *
 * param int|NULL $nid node id
 */
function availability_calendars_delete_node_settings($nid = NULL) {
  if ($nid === NULL) {
    // Remove per node settings for all nodes
    $nid = '%';
  }
  db_query("DELETE FROM {variable} WHERE name LIKE 'availability_calendars_settings_node_%s_%'", $nid);
  cache_clear_all('variables', 'cache');
}

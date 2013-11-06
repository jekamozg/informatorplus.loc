<?php
/**
 * @file
 * Availability Calendars: theming code.
 * - theme calendar
 * - theme 1 month of a calendar
 * - theme key to the calendar
 *
 * @author Dan Karran (geodaniel) <dan at karran dot net>
 * @author Nicholas Alipaz (nicholas.alipaz)
 * @author Erwin Derksen (http://drupal.org/user/750928)
 */

module_load_include('inc', 'availability_calendars', 'availability_calendars');

/**
 * Actual inplementation of D7 hook_node_view.
 * @param object $node
 * @param string $view_mode 'full' or 'teaser'
 */
function availability_calendars_node_node_view($node, $view_mode) {
  $settings = availability_calendars_get_settings('node', $node->nid);
  if (($settings->nodeview === 1 && ($view_mode == 'full' || ($view_mode == 'teaser' && $settings->showteaser == 1)))) {
    $year = date('Y');
    $month = date('m');
    // if this is a translated node, use calendar of original node
    $calendar_node = $node->tnid && $node->tnid != $node->nid ? node_load($node->tnid) : $node;
    $node->content['availability_calendars'] = array(
      '#value' => theme('availability_calendars_node', $calendar_node, $year, $month, $settings->monthcount),
      '#weight' => 10,
    );
  }
}

/**
 * Themes the given number of months of the calendar for the given node.
 *
 * @param object $node
 * @param int $year first year to display
 * @param int $month first month to display
 * @param int $monthstodisplay
 * @return string
 */
function theme_availability_calendars_node($node, $year = 2008, $month = 01, $monthstodisplay = 12) {
  $rows = array();
  $settings = availability_calendars_get_settings('node', $node->nid);
  // Calendar code based on example at http://evolt.org/node/60673 :
  $day = 1;
  $output = '';

  // Create our key for the availability calendar if the node has it set to do so
  if ($settings->showkey === 1) {
    $output .= theme('availability_calendars_key');
  }

  $monthsremaining = $monthstodisplay;
  while ($monthsremaining > 0) {
    $output .= theme('availability_calendars_month', $node, $year, $month, $settings);
    $monthsremaining--;
    $month++;
    if ($month > 12) {
      $month = 1;
      $year++;
    }
  }

  // add an extra three months for editors only
  if (availability_can_edit($node->nid)) {
    $monthsremaining = 3;
    while ($monthsremaining > 0) {
      $output .= theme('availability_calendars_month', $node, $year, $month, $settings);
      $monthsremaining--;
      $month++;
      if ($month > 12) {
        $month = 1;
        $year++;
      }
    }
  }

  return $output;
}

/**
 * Themes the calendar for a given month.
 *
 * @param object $node
 * @param int $year
 * @param int $month
 * @param object $settings
 * @return string
 */
function theme_availability_calendars_month($node, $year, $month, $settings) {
  $month_meta = availability_calendars_month_meta($year, $month, $settings);
  /**
   * here we list all the days of the week, an array of 14 (two full weeks) so
   * that if users select monday as the first day we still get a full week in
   * our following loop .
   */
  $days = array(
    13 => t('Mon'),
    12 => t('Tue'),
    11 => t('Wed'),
    10 => t('Thu'),
    9 => t('Fri'),
    8 => t('Sat'),
    7 => t('Sun'),
    6 => t('Mon'),
    5 => t('Tue'),
    4 => t('Wed'),
    3 => t('Thu'),
    2 => t('Fri'),
    1 => t('Sat'),
    0 => t('Sun'),
  );
  $counter = -$month_meta['firstday'];
  for ($j = 0; $j < $month_meta['weeksinmonth']; $j++) {
    for ($i = 0; $i < 7; $i++) {
      $counter++;
      $week[$j][$i] = $counter;
      // offset the days
      if (($week[$j][$i] < 1) || ($week[$j][$i] > $month_meta['daysinmonth'])) {
        $week[$j][$i] = "";
      }
    }
  }
  // #1093408: Summer/winter time offsets: don't use midnight sharp
  $month_title = format_date(mktime(12, 0, 0, $month, 1, $year), 'custom', 'F Y');
  if (availability_can_edit($node->nid)) {
    // add edit link to month_title
    $month_title .=  ' ' . l(t('edit'), 'availability-calendars/' . $node->nid . '/' . date('Y/m', mktime(0, 0, 0, $month, 1, $year)) . '/edit', array('query' => array('destination' => 'node/' . $node->nid)));
  }
  $headers = array(); // container for header row
  array_push($headers, array('data' => '&nbsp;', 'class' => 'calempty')); // Add one empty cell for the notes column
  // add a header row showing the day of the week, we do some odd backwards looping through this...
  // because the options in the node's form are set in a way that requires it
  for ($i = $settings->startofweek + 7; $i > $settings->startofweek && $i <= (7 + $settings->startofweek); $i--) {
    $day = $settings->firstletter == 0 ? $days[$i] : drupal_substr($days[$i], 0, 1);
    array_push($headers, array('data' => $day, 'class' => 'dayofweek'));
  }

  // find all entries in database for this month ($availability, $notes) and pre-populate
  $notes = availability_calendars_get_node_notes($node->nid, $year, $month);
  $states = availability_calendars_get_node_states($node->nid, $year, $month, $settings);

  $today = mktime(0, 0, 0, date('m'), date('j'), date('Y'));
  $rows = array(); // our container for rows
  $cells = array(); // our container for cells
  foreach ($week as $key => $val) {
    $weeknumber = $key + 1;
    array_push($cells, array('data' => check_plain($notes[$weeknumber]), 'class' => 'calweeknote')); // Add the week note cell to the cells array
    for ($i = 0; $i < 7; $i++) {
      $day = $week[$key][$i];
      $daystamp = mktime(0, 0, 0, (int) $month, (int) $day, (int) $year);
      // if there's a date, it's part of this month
      if ($day) {
        $status = $states[$day];
        $classes = array();
        if ($today == $daystamp) {
          $classes[] = 'caltoday'; // today
        }
        else if ($daystamp < $today) {
          if ($settings->hideold == 1) {
            // Replace status information for days in the past
            $status = 'calpastdate';
          }
          else {
            // Just add a class to days in the past
            $classes[] = 'calpastdate';
          }
        }
        $classes[] = $status;
        $day_cell = availability_calenders_day($day, $status);
        $classes = implode(' ', $classes);
        array_push($cells, array('data' => $day_cell, 'class' => $classes));
      }
      else { // empty, typically row 1 or 5 in a month
        array_push($cells, array('data' => '<span></span>', 'class' => 'calother'));
      }
    }
    array_push($rows, array('data' => $cells, 'class' => 'calweek'));
    $cells = array(); // clear out our $cells array before running the next week
  }
  if ($weeknumber == 5) {
    array_push($cells, array('data' => '&nbsp;', 'class' => 'calweeknote')); // Add the week note cell to the cells array
    for ($i = 0; $i < 7; $i++) {
      array_push($cells, array('data' => '<span></span>'));
    }
    array_push($rows, array('data' => $cells));
  }
  $output = theme('table', $headers, $rows, array('class' => 'cal'), $month_title); // Our final table
  // Wrap the table to allow for better styling
  return '<div class="calmonth-wrapper">' . $output . '</div>';
}

/**
 * Helper function that returns the html for 1 day.
 *
 * @param int $day The day number.
 * @param string $status The status for this day.
 */
function availability_calenders_day($day, $status) {
  if (strpos($status, ' ') === FALSE) {
    // Whole day (in the future, when we get editable cells, this may become the same as split day)
    $result = "<span>$day</span>";
  }
  else {
    // Split day: the inner span will be positioned absolute and will take care of the background.
    // However, to be able to position it absolute, it needs to be within another positioned element.
    // That's why we use an outer span: td's are not and cannot be positioned.
    // Choice between span and div: span is used less often than div in Drupal, so styles that end with "... > span > span"
    // are more specific.
    $result = "<span>$day<span></span></span>";
  }
  return $result;
}

/**
 * Themes the key for our calendars.
 *
 * @return string
 */
function theme_availability_calendars_key() {
  // @todo: use caption instead of a non-related div tag (key-title)
  //use all the same classes for cells and table, so it styles the same as the calendars
  $keytitle = t('Key');
  $headers = array(
    t('Availability')
  );
  $rows = array();
  $states = availability_calendars_get_states();
  foreach ($states as $class => $state) {
    $rows[] = array(
      array(
        'data' => '<span>' . check_plain(t($state['label'])) . '</span>',
        'class' => "$class keystatus", // keystatus class used to allow better styling of the status column
      ),
    );
  }
  $rows[] = array(
    array(
      'data' => '<span>' . t('Current Day') . '</span>',
      'class' => 'caltoday keystatus',
    ),
  );
  $key = theme('table', $headers, $rows, array('class' => 'cal'), $keytitle);
  return '<div class="calmonth-wrapper key-wrapper">' . $key . '</div>';
}

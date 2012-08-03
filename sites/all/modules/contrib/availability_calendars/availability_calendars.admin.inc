<?php
/**
 * @file
 * Availability Calendars: admin settings form code
 * - define form
 * - validate
 * - submit
 *
 * @author Dan Karran (geodaniel) <dan at karran dot net>
 * @author Nicholas Alipaz (nicholas.alipaz)
 * @author Erwin Derksen (http://drupal.org/user/750928)
 */

module_load_include('inc', 'availability_calendars', 'availability_calendars');

/**
 * Retrieve form callback for the admin/settings/availability-calendars/settings page.
 *
 * @return array the form
 */
function availability_calendars_admin_settings() {
  drupal_add_css(drupal_get_path('module', 'availability_calendars') . '/availability_calendars.admin.css', 'module', 'all', FALSE);

  $form = array();
  $form['#validate'][] = 'availability_calendars_admin_settings_validate';
  $form['#submit'][] = 'availability_calendars_admin_settings_submit';

  $settings = availability_calendars_get_settings();
  $form['availability_calendars_settings_system_nodeview'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show automatically on node page'),
    '#default_value' => $settings->nodeview,
    '#description' => t("Check whether you want availability calendars to be displayed on the node page. If you don't check this, you will need to output it manually using the %func function.", array('%func' => "theme('availability_calendars_node')")),
  );
  $form['availability_calendars_settings_system_pernodeoverride'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow to override settings per node'),
    '#description' => t("If you check this setting, all settings in the '!block_name' block below will be overridden on a per node basis.", array('!block_name' => t('Global settings'))),
    '#default_value' => $settings->pernodeoverride,
  );
  $form['availability_calendars'] = array(
    '#type' => 'fieldset',
    '#title' => t('Global settings'),
    '#description' => t('The following settings define the look & feel of the calendars on a system wide basis.'),
  );
  $form['availability_calendars']['availability_calendars_settings_system_showteaser'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show availability calendars within teasers'),
    '#default_value' => $settings->showteaser,
  );
  // Allow users to choose whether a key should be shown on the node on a per node basis
  $form['availability_calendars']['availability_calendars_settings_system_showkey'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show a key for the availability calendars'),
    '#default_value' => $settings->showkey,
  );
  // Whether to use only the first letter for the day of the week or not
  $form['availability_calendars']['availability_calendars_settings_system_firstletter'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use only the first letter from the day of the week'),
    '#default_value' => $settings->firstletter,
  );
  $form['availability_calendars']['availability_calendars_settings_system_hideold'] = array(
    '#type' => 'checkbox',
    '#title' => t('Do not show availability state of dates in the past'),
    '#default_value' => $settings->hideold,
  );
  $form['availability_calendars']['availability_calendars_settings_system_splitday'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow split day statuses'),
    '#default_value' => $settings->splitday,
  );
  $form['availability_calendars']['availability_calendars_settings_system_startofweek'] = array(
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
  $form['availability_calendars']['availability_calendars_settings_system_monthcount'] = array(
    '#type' => 'textfield',
    '#title' => t('Number of months to display'),
    '#default_value' => $settings->monthcount,
    '#description' => t("Your calendars will show this number of months to all users except those with the 'edit own availability calendars' or 'edit availability calendars' who will always see 3 extra months on the calendars they can edit. This is to allow them to enter information into future calendars before it is made publicly available."),
  );

  // Add states
  $form['states'] = array(
    '#type' => 'fieldset',
    '#title' => t('States'),
    '#tree' => TRUE,
    '#description' => t('<p>You can modify the availability states here.</p>
      <ul>
        <li>The label is what visitors will see in the legend and what editors will see when editing the calendar.</li>
        <li>The class must be unique and will be used for the css.</li>
        <li>Default defines the status to use for days that have no state yet set.</li>
        <li>The weight defines the order in the legend and in the dropdown on the edit form.</li>
        <li>Make a label empty to remove the row.</li>
        <li>If there are no more empty lines to add new states, save the form and you will be able to add another state.</li>
        <li><strong>Changes made to existing classes will not be updated in existing calendars!</strong> So do not change on live sites.</li>
        <li>If you define new classes, visit the styling page to define its colors.</li>
      </ul>
    '),
    '#attributes' => array('class' => 'state-list'),
  );

  $element = &$form['states'];
  $states = availability_calendars_get_states();
  $i = 0;
  foreach ($states as $state) {
    availability_calendars_admin_settings_add_state($element, $i, $state, $settings);
    $i++;
  }

  // Show a minimum of 4 available states with at least one empty state
  do {
    availability_calendars_admin_settings_add_state($element, $i, array('class' => '', 'label' => '', 'weight' => 0), $settings);
    $i++;
  } while ($i < 4);

  return system_settings_form($form);
}

/**
 * Helper function to add a state item to a form.
 * Only the first item gets labels, as the fields will be presented below each other.
 *
 * @param array $element the form element to add the state item to
 * @param int $i the state item count
 * @param array $state array containing a state record
 * @param array $settings array containing general (i.e. non per state) settings
 */
function availability_calendars_admin_settings_add_state(&$element, $i, $state, $settings) {
  static $max_weight = 0;
  $element[$i]['label'] = array(
    '#type' => 'textfield',
    '#title' => $i == 0 ? t('Label') : '',
    '#default_value' => $state['label'],
    '#size' => 40,
    '#prefix' => '<div class="state-item">',
  );
  $element[$i]['class'] = array(
    '#type' => 'textfield',
    '#title' => $i == 0 ? t('Class') : '',
    '#default_value' => $state['class'],
    '#size' => 24,
  );
  // Bit difficult to add a set of related radio button's that do not appear together
  $element[$i]['defaultstatus'] = array(
    '#type' => 'radio',
    '#tree' => FALSE,
    // Use our own label format (as with a radio button the label appears after the radio button)
    // and wrap it in our own div.form-item
    '#prefix' => $i == 0 ? '<div class="cal-state-label form-item"><label for="edit-defaultstatus">' . t('Default') . ': </label>' : '',
    '#suffix' => $i == 0 ? '</div>' : '',
    '#return_value' => $i,
    '#default_value' => !empty($state['class']) && $state['class'] == $settings->defaultstatus ? $i : 0,
    // use the name attribute to group the radio buttons scattered over the items/rows
    '#attributes' => array('name' => 'defaultstatus'),
  );
  $element[$i]['weight'] = array(
    '#type' => 'select',
    '#title' => $i == 0 ? t('Weight') : '',
    '#default_value' => $state['weight'] > 0 ? $state['weight'] : ++$max_weight,
    '#options' => array_combine(range(1, 20, 1), range(1, 20, 1)),
    '#suffix' => '</div>',
  );
  if ($state['weight'] > $max_weight) {
    $max_weight = $state['weight'];
  }
}

/**
 * Form validate callback for the admin_settings form
 * - at least one label should be filled
 * - entered classes should be a valid css class
 */
function availability_calendars_admin_settings_validate($form, &$form_state) {
  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';
  if ($op == t('Save configuration')) {
    $element = $form_state['values']['states'];
    $all_empty = TRUE;
    foreach ($element as $i => $state_fields) {
      if (!empty($state_fields['label'])) {
        $all_empty = FALSE;
      }
      if (!empty($state_fields['class']) && !availability_calendars_is_safe_id($state_fields['class'])) {
        form_set_error("states][$i][class", t('Class should be usable as a valid css class.'));
      }
    }

    if ($all_empty) {
      form_set_error('states][0][label', t('At least 1 state should be defined.'));
    }

    $default = $form_state['values']['defaultstatus'];
    if (empty($element[$default]['label'])) {
      form_set_error("states][$default][label", t('The default state may not be empty or deleted.'));
    }
  }
}

/**
 * Form submit callback for the admin_settings form
 *
 * Processes the submitted form. The states are non system settings, and are handled here.
 * Other values are handled by the default system settings form handling.
 */
function availability_calendars_admin_settings_submit($form, &$form_state) {
  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';
  if ($op == t('Save configuration')) {
    // Process the state settings here and do not pass them to the default submit handler for system settings forms
    $element = $form_state['values']['states'];
    unset($form_state['values']['states']);

    // Extract the states
    $states = array();
    foreach ($element as $i => $state_fields) {
      // Only add non-empty labels
      if (!empty($state_fields['label'])) {
        if (empty($state_fields['class'])) {
          $state_fields['class'] = availability_calendars_string_to_safe_id($state_fields['label']);
        }
        $states[$state_fields['class']] = array(
          'class' => $state_fields['class'],
          'label' => $state_fields['label'],
          'weight' => $state_fields['weight'],
        );
      }
    }

    $default = $form_state['values']['defaultstatus'];
    if (!empty($element[$default]['class'])) {
      $form_state['values']['availability_calendars_settings_system_defaultstatus'] = $element[$default]['class'];
    }

    // finally, save the states
    availability_calendars_update_states($states);

    // Remove all per node settings if the setting changes from yes to no
    if ($form_state['values']['availability_calendars_settings_system_pernodeoverride'] == 0) {
      $settings = availability_calendars_get_settings();
      if ($settings->pernodeoverride) {
        availability_calendars_delete_node_settings();
      }
    }
  }
}

/**
 * Converts a string to a valid html id/class attribute.
 *
 * http://www.w3.org/TR/html4/struct/global.html#h-7.5.2 specifies what makes a
 * valid id/class attribute in HTML. This function:
 *
 * - Ensure an ID starts with an alpha character by prefixing with 'cal'.
 * - Replaces any character except a-z, A-Z, numbers, and underscores with dashes.
 * - Converts entire string to lowercase.
 *
 * @param string $string
 * @return string the converted string
 */
function availability_calendars_string_to_safe_id($string) {
  return 'cal' . strtolower(preg_replace('/[^a-zA-Z0-9_-]+/', '-', $string));
}

/**
 * Checks whether the given string can be used as an html id or class
 *
 * @param string $string
 * @return boolean
 */
function availability_calendars_is_safe_id($string) {
  return preg_match('/^[a-zA-Z][a-zA-Z0-9_-]*$/', $string) > 0;
}

/**
 * Called via hook_form_FORM_ID_alter() for FORM_ID = 'node_type_form' (i.e. the edit content type form).
 */
function availability_calendars_admin_form_node_type_form_alter(&$form, &$form_state) {
  // @todo: - list of content types on own admin settings page?
  //   - see Webform (field "Webform-enabled content types"), but take is_new and disabled into account?
  //   - What about per content type settings (between system-wide versus per node)
  $enabled = availability_calendars_is_supported_type($form['#node_type']->type);
  $form['availability_calendars'] = array(
    '#type' => 'fieldset',
    '#title' => t('Availability calendars'),
    '#collapsible' => TRUE,
    '#collapsed' => !$enabled,
  );
  $form['availability_calendars']['availability_calendars_settings_system-type'] = array(
    '#type' => 'checkbox',
    '#title' => t('Availability calendar support'),
    '#default_value' => $enabled,
    '#description' => t('Enable or disable availability support for this content type. If enabled, node owner will be able to specify node availability.'),
  );
}

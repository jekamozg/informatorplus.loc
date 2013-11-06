<?php
/**
 * @file
 * Availability Calendars: styles settings form code
 * - define form
 * - validate
 * - submit
 * - generate CSS
 *
 * This file uses many helper methods to ease the task of adding new style settings,
 * or enhancing a current style type (length, color, select field), and remaining a
 * standardized look & feel on the admin styles screen.
 *
 * Adding a property:
 * - Add the property to the correct fieldset in class AvailabilityCalendarsStylesFormBuilder.
 * - If necessary, add validation of the property to class AvailabilityCalendarsStylesFormValidator.
 * - Add the property to the CSS generator, i.e. class AvailabilityCalendarsCssGenerator.
 * - Optionally, add a sensible default to the function availability_calendars_install
 *   (in file availability_calendars.install). An update should normally not be necessary
 * - If necessary, remove the property from the base css file: availability_calendars.css in the module directory.
 * - Test
 * - Create an issue on the issue queue including a patch for the above changes.
 *
 * @author Erwin Derksen (http://drupal.org/user/750928)
 * @author Nicholas Alipaz (nicholas.alipaz)
 */

module_load_include('inc', 'availability_calendars', 'availability_calendars');

/**
 * Callback to retrieve a form for the admin/settings/availability-calendars/styles page.
 *
 * @return array The form
 */
function availability_calendars_styles() {
  drupal_add_css(drupal_get_path('module', 'availability_calendars') . '/availability_calendars.admin.css', 'module', 'all', FALSE);

  $form_builder = new AvailabilityCalendarsStylesFormBuilder(availability_calendars_get_styles());
  $form = $form_builder->exec();
  $form['#validate'][] = 'availability_calendars_styles_validate';
  $form = system_settings_form($form);
  // We want our #submit handler to run after the system submit handler, so the generator can simply use the variable API
  $form['#submit'][] = 'availability_calendars_styles_generate';
  return $form;
}

/**
 * Callback to validate a form for the styles form.
 */
function availability_calendars_styles_validate($form, &$form_state) {
  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';
  if ($op == t('Save configuration')) {
    // @todo: validate colors, lengths (note that Drupal validates select's for existing values)
    $css_validator = new AvailabilityCalendarsStylesFormValidator($form_state['values']);
    $css_validator->exec();
  }
}

/**
 * Callback to process form submission for the styles form.
 */
function availability_calendars_styles_generate($form, &$form_state) {
  if (variable_get('availability_calendars_settings_system_generate', 0)) {
    $css_generator = new AvailabilityCalendarsCssGenerator(availability_calendars_get_styles());
    $result = $css_generator->exec();
    if ($result) {
      drupal_set_message(t('The CSS file for Availaibility Calendars has been succesfully generated.'), 'status', FALSE);
    }
    else {
      drupal_set_message(t('An error occurred while generating the CSS file for Availaibility Calendars.'), 'error', FALSE);
    }
  }
}

/**
 * Returns the CSS styles defined for the calendars.
 *
 * @return array list of defined styles.
 */
function availability_calendars_get_styles() {
  $styles = variable_get('availability_calendars_styles', array());
  $styles['availability_calendars_settings_system_generate'] = variable_get('availability_calendars_settings_styles_generate', 1);
  return $styles;
}

class AvailabilityCalendarsStylesFormBuilder {
  /* @var $form array */
  protected $form = array();
  /* @var $currentFieldset string */
  protected $currentFieldset = '';
  /* @var $styles array */
  protected $styles = NULL;

  public function __construct($styles) {
    $this->styles = $styles;
  }

  public function exec() {
    $this->fieldsetTable();
    $this->fieldsetCaption();
    $this->fieldsetHeader();
    $this->fieldsetWeekNotes();
    $this->fieldsetDays();
    $this->fieldsetStates();
    // place all fieldsets within another fieldset and add the genereate checkbox
    $this->form = array(
      'availability_calendars_settings_system_generate' => array(
        '#type' => 'checkbox',
        '#title' => t('Generate the css file.'),
        '#default_value' => $this->styles['availability_calendars_settings_system_generate'],
        '#description' => t("Check whether you want to generate a CSS file based on the styles on this page. If you don't check this, the CSS file won't be generated nor included. As a consequence, you will need to define all styling in your theme."),
      ),
      'availability_calendars_styles' => array(
        '#type' => 'fieldset',
        '#title' => t('CSS Styles'),
        '#tree' => TRUE,
        '#description' => t("Below you can fill in a number of basic styles that define how your calendar will be displayed. If a style is left empty (or @n for selects) that style won't be generated.", array('@n' => '<none>')),
      ) + $this->form,
    );
    return $this->form;
  }

  /**
   * Helper method to return the fieldset for the table styles
   *
   * @return array an array with a number of form elements in a fieldset
   */
  protected function fieldsetTable() {
    $this->currentFieldset = 'table';
    $this->form[$this->currentFieldset] = array(
      '#type' => 'fieldset',
      '#title' => t('Table'),
      '#description' => t('Styles that define how the table will be displayed.'),
    );
    $this->selectField('font-size', array('larger', 'smaller', 'xx-small', 'x-small', 'small', 'medium', 'large', 'x-large', 'xx-large'));
    $this->colorField('color');
    $this->colorField('background-color');
    $this->lengthField('border-width');
    $this->colorField('border-color');
  }

  /**
   * Helper method to return the fieldset for the caption styles
   *
   * @return array an array with a number of form elements in a fieldset
   */
  protected function fieldsetCaption() {
    $this->currentFieldset = 'caption';
    $this->form[$this->currentFieldset] = array(
      '#type' => 'fieldset',
      '#title' => t('Caption'),
      '#description' => t('Styles that define how the name of the month will be displayed.'),
    );
    $this->selectField('text-align', array('left', 'right', 'center', 'inherit'));
    $this->selectField('font-weight', array('normal', 'bold', 'bolder', 'lighter', 'inherit'));
    $this->selectField('font-style', array('normal', 'italic', 'oblique', 'inherit'));
    $this->selectField('font-size', array('larger', 'smaller', 'xx-small', 'x-small', 'small', 'medium', 'large', 'x-large', 'xx-large', 'inherit'));
  }

  /**
   * Helper method to return the fieldset for the table header styles
   *
   * @return array an array with a number of form elements in a fieldset
   */
  protected function fieldsetHeader() {
    $this->currentFieldset = 'header';
    $this->form[$this->currentFieldset] = array(
      '#type' => 'fieldset',
      '#title' => t('Day names'),
      '#description' => t('Styles that define how the names of the weekdays will be displayed.'),
    );
    $this->lengthField('height');
    $this->selectField('text-align', array('left', 'right', 'center', 'inherit'));
    $this->selectField('font-weight', array('normal', 'bold', 'bolder', 'lighter', 'inherit'));
    $this->selectField('font-style', array('normal', 'italic', 'oblique', 'inherit'));
    $this->selectField('font-size', array('larger', 'smaller', 'xx-small', 'x-small', 'small', 'medium', 'large', 'x-large', 'xx-large', 'inherit'));
  }

  /**
   * Helper method to return the fieldset for the week note styles
   *
   * @return array an array with a number of form elements in a fieldset
   */
  protected function fieldsetWeekNotes() {
    $this->currentFieldset = 'week_notes';
    $this->form[$this->currentFieldset] = array(
      '#type' => 'fieldset',
      '#title' => t('Week notes'),
      '#description' => t('Styles that define how a week note cell will be displayed.'),
    );
    $this->lengthField('width');
  }

  /**
   * Helper method to return the fieldset for the day styles
   *
   * @return array an array with a number of form elements in a fieldset
   */
  protected function fieldsetDays() {
    $this->currentFieldset = 'days';
    $this->form[$this->currentFieldset] = array(
      '#type' => 'fieldset',
      '#title' => t('Days'),
      '#description' => t('Styles that define how a day cell will be displayed.'),
    );
    $this->lengthField('width');
    $this->lengthField('height');
    $this->selectField('text-align', array('left', 'right', 'center', 'inherit'));
    $this->selectField('vertical-align', array('baseline', 'sub', 'super', 'top', 'text-top', 'middle', 'bottom', 'text-bottom'));
  }

  /**
   * Helper method to return the fieldset for the states styles
   *
   * @return array an array with a number of form elements in a fieldset
   */
  protected function fieldsetStates() {
    $this->currentFieldset = 'states';
    $this->form[$this->currentFieldset] = array(
      '#type' => 'fieldset',
      '#title' => t('States'),
      '#description' => t('Styles that define how the states will be displayed.'),
    );
    $this->selectField('split-day', array('/', '\\', '|', '―'));
    // translate the title 'Split day' but only after creating the form element (or this value
    // won't be retrieved from the variable if showing this screen in another language).
    $this->form[$this->currentFieldset]['Split day']['#title'] = t('How to render a split day');
    $states = availability_calendars_get_states();
    foreach ($states as $class => $state) {
      $this->colorField(array($state['class'] => $state['label']));
    }
  }

  /**
   * Helper method to add a length field to a given fieldset.
   * By extracting this, we get standardized settings for length fields.
   * E.g: for now only pixels are allowed, but this function could add a dropdown for unit selection.
   *
   * @param string|array $css_property The name of the css color property to add.
   *   If this is not the same as the form field name, pass in an array with 1 element: <field name> => <css property>
   */
  protected function lengthField($cssProperty) {
    if (is_array($cssProperty)) {
      list($fieldName, $cssProperty) = each($cssProperty);
    }
    else {
      $fieldName = $cssProperty;
    }
    $this->form[$this->currentFieldset][$fieldName] = array(
      '#type' => 'textfield',
      '#title' => $cssProperty,
      '#default_value' => $this->getStyle($fieldName),
      '#size' => 8,
      '#maxlength' => 6,
      '#field_suffix' => 'px',
    );
  }

  /**
   * Helper method to add a color field to a given fieldset.
   * By extracting this, we get standardized settings and handling for color fields.
   * E.g: for now only color codes are allowed, but this function could add support for other
   * notations (http://www.w3.org/TR/CSS21/syndata.html#color-units) or a color picker.
   *
   * @param string|array $css_property The name of the css color property to add.
   *   If this is not the same as the form field name, pass in an array with 1 element: <field name> => <css property>
   */
  protected function colorField($cssProperty) {
    if (is_array($cssProperty)) {
      list($fieldName, $cssProperty) = each($cssProperty);
    }
    else {
      $fieldName = $cssProperty;
    }
    $this->form[$this->currentFieldset][$fieldName] = array(
      '#type' => 'textfield',
      '#title' => $cssProperty,
      '#default_value' => $this->getStyle($fieldName),
      '#size' => 8,
      '#maxlength' => 7,
      '#field_prefix' => '#',
    );
  }

  /**
   * Helper method to add a select field to a given fieldset.
   *
   * @param string|array $css_property The name of the css color property to add.
   *   If this is not the same as the form field name, pass in an array with 1 element: <field name> => <css property>
   * @param array $options  the options to present. An option <none> will be added in front of
   *   this list, to allow to select for not setting and thus not generating this property.
   */
  protected function selectField($cssProperty, $options) {
    if (is_array($cssProperty)) {
      list($fieldName, $cssProperty) = each($cssProperty);
    }
    else {
      $fieldName = $cssProperty;
    }
    array_unshift($options, '<none>');
    $options = array_combine($options, $options);
    $this->form[$this->currentFieldset][$fieldName] = array(
      '#type' => 'select',
      '#title' => $cssProperty,
      '#default_value' => $this->getStyle($fieldName),
      '#options' => $options,
    );
  }

  /**
   * Helper method to return 1 style setting, (to not repeat the taking care of not being set,
   * defaults, etc).
   *
   * @param string $name
   */
  protected function getStyle($name) {
    $category = $this->currentFieldset;
    return isset($this->styles[$category][$name]) ? $this->styles[$category][$name] : '';
  }
}

class AvailabilityCalendarsStylesFormValidator {
  /* @var $styles array */
  protected $styles = NULL;
  /* @var $currentFieldset string */
  protected $currentFieldset = '';

  public function __construct($styles) {
    $this->styles = $styles;
  }

  /**
   * Validates the styles settings.
   * Form errors are set on error.
   */
  public function exec() {
    $this->fieldsetTable();
    $this->fieldsetCaption();
    $this->fieldsetHeader();
    $this->fieldsetWeekNotes();
    $this->fieldsetDays();
    $this->fieldsetStates();
  }

  protected function fieldsetTable() {
    $this->currentFieldset = 'table';
    $this->colorField('color');
    $this->colorField('background-color');
    $this->lengthField('border-width');
    $this->colorField('border-color');
  }

  protected function fieldsetCaption() {
    $this->currentFieldset = 'caption';
  }

  protected function fieldsetHeader() {
    $this->currentFieldset = 'header';
    $this->lengthField('height');
  }

  protected function fieldsetWeekNotes() {
    $this->currentFieldset = 'week_notes';
    $this->lengthField('width');
  }

  protected function fieldsetDays() {
    $this->currentFieldset = 'days';
    $this->lengthField('width');
    $this->lengthField('height');
  }

  protected function fieldsetStates() {
    $this->currentFieldset = 'states';

    if ($this->getStyle('split-day') != '') {
      // We need cell width and height for split days
      if ($this->getStyle('width', 'days') != '') {
        form_set_error('days][width', t('The day cell width is needed to generate styles for split days.'));
      }
      if ($this->getStyle('height', 'days') != '') {
        form_set_error('days][height', t('The day cell height is needed to generate styles for split days.'));
      }
    }
    $states = availability_calendars_get_states();
    foreach ($states as $class => $state) {
      $this->colorField($state['class']);
    }
  }

  protected function lengthField($name) {
    $value = $this->getStyle($name);
    if (!empty($value) && !$this->validateLengthValue($value)) {
      form_set_error($this->currentFieldset . '][' . $name, t('Not a valid width or height specification.'));
    }
  }

  protected function colorField($name) {
    $value = $this->getStyle($name);
    if (!empty($value) && !$this->validateColorValue($value)) {
      form_set_error($this->currentFieldset . '][' . $name, t('Not a valid color code.'));
    }
  }

  /**
   * Check for a CSS length declaration:
   * We check for a part of http://www.w3.org/TR/CSS21/syndata.html#numbers
   * and http://www.w3.org/TR/CSS21/syndata.html#length-units:
   * - only non-signed pixel values are allowed
   *
   * @param string $value
   * @return boolean
   */
  protected function validateLengthValue($value) {
    $pattern = '/^(\d*\.)?\d+(px)?$/';
    return preg_match($pattern, $value) === 1;
  }


  /**
   * Check for a CSS color declaration:
   * We check for a part of http://www.w3.org/TR/CSS21/syndata.html#color-units
   * - only hex code formats are allowed
   *
   * @param string $value
   * @return boolean
   */
  protected function validateColorValue($value) {
    $pattern = '/^#?[0-9a-fA-F]{3}[0-9a-fA-F]{3}?$/';
    return preg_match($pattern, $value) === 1;
  }

  /**
   * Helper method to return 1 style setting, (to not repeat the taking care of not being set,
   * defaults, etc).
   *
   * @param string $name
   * @param string $category the name of the category, NULL for the current fieldset
   */
  protected function getStyle($name, $category = NULL) {
    if ($category === NULL) {
      $category = $this->currentFieldset;
    }
    return isset($this->styles[$category][$name]) ? $this->styles[$category][$name] : '';
  }
}

class AvailabilityCalendarsCssGenerator {
  /* @var $styles array */
  protected $styles = NULL;

  public function __construct($styles) {
    $this->styles = $styles;
  }

  /**
   * Creates and writes the CSS file for Availability Calendars
   *
   * @return boolean Whether the generation was successful.
   */
  public function exec() {
    $css = $this->createCss();
    $result = $this->writeCss($css);
    return $result;
  }

  /**
   * Creates the css.
   *
   * @return string The created css
   */
  protected function createCss() {
    $css = '';
    $css .= $this->createTableCss();
    $css .= $this->createCaptionCss();
    $css .= $this->createHeaderCss();
    $css .= $this->createDaysCss();
    $css .= $this->createStatesCss();
    $css .= $this->createWeekNotesCss();
    return $css;
  }

  /**
   * Writes the created CSS to a file.
   *
   * @param string $css
   * @return boolean Whether the generation was successful.
   */
  protected function writeCss($css) {
    $result = false;
    $path = file_directory_path() . '/availability_calendars';
    if ($result = file_check_directory($path, FILE_CREATE_DIRECTORY)) {
      $file = $path . '/' . 'availability_calendars.css';
      if ($result = (file_save_data($css, $file, FILE_EXISTS_REPLACE) === $file)) {
        // Set file permissions for webserver-generated files. I use 0666 as often, the ftp account is not the webserver nor in its group
        @chmod($file, 0666);
      }
    }
    return $result;
  }

  /**
   * @return string The CSS for the table
   */
  protected function createTableCss() {
    $category = 'table';
    $css = '';
    $css .= $this->cssSelector(".cal");
    $css .= $this->addCssColorDeclaration($category, 'color');
    $css .= $this->addCssColorDeclaration($category, 'background-color');
    $css .= $this->addCssLengthDeclaration($category, 'border-width');
    $css .= $this->addCssColorDeclaration($category, 'border-color');
    $css .= "}\n";
    return $css;
  }

  /**
   * @return string The CSS for the caption
   */
  protected function createCaptionCss() {
    $category = 'caption';
    $css = '';
    $css .= $this->cssSelector(".cal caption");
    $css .= $this->addCssDeclaration($category, 'text-align');
    $css .= $this->addCssDeclaration($category, 'font-weight');
    $css .= $this->addCssDeclaration($category, 'font-style');
    $css .= $this->addCssDeclaration($category, 'font-size');
    $css .= $this->cssSelectorEnd();
    return $css;
  }

  /**
   * @return string The CSS for the header
   */
  protected function createHeaderCss() {
    $category = 'header';
    $css = '';
    $css .= $this->cssSelector(".cal thead th");
    $css .= $this->addCssLengthDeclaration($category, 'height');
    $css .= $this->addCssDeclaration($category, 'font-size');
    $css .= $this->cssSelectorEnd();
    return $css;
  }

  /**
   * @return string The CSS for the WeekNotes
   */
  protected function createWeekNotesCss() {
    $category = 'week_notes';
    $css = '';
    $css .= $this->cssSelector("td.calweeknote");
    $css .= $this->addCssLengthDeclaration($category, 'width');
    $css .= $this->cssSelectorEnd();
    return $css;
  }

  /**
   * @return string The CSS for the day cells
   */
  protected function createDaysCss() {
    $category = 'days';
    $css = '';
    $css .= $this->cssSelector(".cal td");
    $css .= $this->addCssLengthDeclaration($category, 'width');
    $css .= $this->addCssLengthDeclaration($category, 'height');
    $css .= $this->addCssDeclaration($category, 'text-align');
    $css .= $this->addCssDeclaration($category, 'vertical-align');
    $css .= $this->cssSelectorEnd();
    return $css;
  }

  /**
   * @return string The CSS for the states
   */
  protected function createStatesCss() {
    $category = 'states';
    $splitDay = $this->getStyle($category, 'split-day');
    $states = availability_calendars_get_states();

    $css = empty($splitDay)
      ? "/* Whole days only*/\n"
      : "/* Whole days, -pm states are IE6 fallback: IE6 does not display split states: show PM state as whole day state */\n";

    foreach ($states as $class => $stateRecord) {
      $state = $stateRecord['class'];
      // Whole day state
      $css .= $this->cssSelector(empty($splitDay) ? ".$state > span" : ".$state > span, .$state-pm > span");
      $css .= $this->addCssColorDeclaration($category, array($class => 'background-color'));
      $css .= $this->cssSelectorEnd();
    }
    $splitDay = $this->getStyle($category, 'split-day');
    if (!empty($splitDay)) {
      $pattern = '/^([0-9.]+)([a-zA-Z%]*)$/';
      $value = $this->getStyle('days', 'width');
      $parts = array();
      preg_match($pattern, $value, $parts);
      $cellWidth = $parts[1];
      $cellWidthUnit = count($parts) > 2 ? $parts[2] : 'px';
      $value = $this->getStyle('days', 'height');
      $parts = array();
      preg_match($pattern, $value, $parts);
      $cellHeight = $parts[1];
      $cellHeightUnit = count($parts) > 2 ? $parts[2] : 'px';

      $css .= "/* Split days */\n";

      // Styles for the outer span within the td that contains the day number and an absolutle positioned span
      $css .= $this->cssSelector('.cal td > span');
      $css .= $this->cssDeclaration('display', 'block');
      $css .= $this->cssDeclaration('position', 'relative');
      $css .= $this->cssDeclaration('width', $cellWidth . $cellWidthUnit);
      $css .= $this->cssDeclaration('height', $cellHeight . $cellHeightUnit);
      $css .= $this->cssDeclaration('line-height', $cellHeight . $cellHeightUnit);
      $css .= $this->addCssDeclaration('days', 'text-align');
      $css .= $this->addCssDeclaration('days', 'vertical-align');
      $css .= $this->cssDeclaration('z-index', 1);
      $css .= $this->cssSelectorEnd();
      // Styles for the inner span that takes care of the background coloring of split days
      $css .= $this->cssSelector('.cal td > span > span');
      $css .= $this->cssDeclaration('position', 'absolute');
      $css .= $this->cssDeclaration('top', '0');
      $css .= $this->cssDeclaration('left', '0');
      $css .= $this->cssDeclaration('z-index', -1);
      $css .= $this->cssDeclaration('border-style', 'solid');
      switch ($splitDay) {
        case '/':
          $css .= $this->cssDeclaration('width', '0');
          $css .= $this->cssDeclaration('height', '0');
          $css .= $this->cssDeclaration('border-left-width', floor($cellWidth / 2) . $cellWidthUnit);
          $css .= $this->cssDeclaration('border-top-width', floor($cellHeight / 2) . $cellHeightUnit);
          $css .= $this->cssDeclaration('border-right-width', ceil($cellWidth / 2) . $cellWidthUnit);
          $css .= $this->cssDeclaration('border-bottom-width', ceil($cellHeight / 2) . $cellHeightUnit);
          break;
        case '\\':
          $css .= $this->cssDeclaration('width', '0');
          $css .= $this->cssDeclaration('height', '0');
          $css .= $this->cssDeclaration('border-left-width', floor($cellWidth / 2) . $cellWidthUnit);
          $css .= $this->cssDeclaration('border-bottom-width', floor($cellHeight / 2) . $cellHeightUnit);
          $css .= $this->cssDeclaration('border-right-width', ceil($cellWidth / 2) . $cellWidthUnit);
          $css .= $this->cssDeclaration('border-top-width', ceil($cellHeight / 2) . $cellHeightUnit);
          break;
          case '|':
          $css .= $this->cssDeclaration('width', '0');
          $css .= $this->addCssLengthDeclaration('days', 'height');
          $css .= $this->cssDeclaration('border-left-width', floor($cellWidth / 2) . $cellWidthUnit);
          $css .= $this->cssDeclaration('border-right-width', ceil($cellWidth / 2) . $cellWidthUnit);
          break;
        case '―':
          $css .= $this->addCssLengthDeclaration('days', 'width');
          $css .= $this->cssDeclaration('height', '0');
          $css .= $this->cssDeclaration('border-top-width', floor($cellHeight / 2) . $cellHeightUnit);
          $css .= $this->cssDeclaration('border-bottom-width', ceil($cellHeight / 2) . $cellHeightUnit);
          break;
      }
      $css .= $this->cssSelectorEnd();
      foreach ($states as $class => $stateRecord) {
        $state = $stateRecord['class'];
        $cssAm = $this->cssSelector(".$state-am > span > span");
        $cssPm = $this->cssSelector(".$state-pm > span > span");
        switch ($splitDay) {
          case '/':
            $cssAm .= $this->addCssColorDeclaration($category, array($state => 'border-left-color'));
            $cssAm .= $this->addCssColorDeclaration($category, array($state => 'border-top-color'));
            $cssPm .= $this->addCssColorDeclaration($category, array($state => 'border-right-color'));
            $cssPm .= $this->addCssColorDeclaration($category, array($state => 'border-bottom-color'));
            break;
          case '\\':
            $cssAm .= $this->addCssColorDeclaration($category, array($state => 'border-left-color'));
            $cssAm .= $this->addCssColorDeclaration($category, array($state => 'border-bottom-color'));
            $cssPm .= $this->addCssColorDeclaration($category, array($state => 'border-right-color'));
            $cssPm .= $this->addCssColorDeclaration($category, array($state => 'border-top-color'));
            break;
          case '|':
            $cssAm .= $this->addCssColorDeclaration($category, array($state => 'border-left-color'));
            $cssPm .= $this->addCssColorDeclaration($category, array($state => 'border-right-color'));
            break;
          case '―':
            $cssAm .= $this->addCssColorDeclaration($category, array($state => 'border-top-color'));
            $cssPm .= $this->addCssColorDeclaration($category, array($state => 'border-bottom-color'));
            break;
        }
        $cssAm .= $this->cssSelectorEnd();
        $cssPm .= $this->cssSelectorEnd();
        $css .= $cssAm;
        $css .= $cssPm;
      }
    }
    return $css;
  }

  /**
   * Single place that specifies how to format selectors.
   */
  protected function cssSelector($selector) {
    return "$selector {\n";
  }

  /**
   * Single place that specifies how to format selectors ends.
   */
  protected function cssSelectorEnd() {
    return "}\n\n";
  }

  /**
   * Creates a CSS declaration for a length property.
   * @see addCssDeclaration for param and return info
   */
  protected function addCssLengthDeclaration($category, $name) {
    $value = $this->getStyle($category, $name);
    // Pixels are the default for numeric values
    if (!empty($value) && is_numeric($value)) {
      $value .= 'px';
    }
    return $this->cssDeclaration($name, $value);
  }

  /**
   * Creates a CSS declaration for a color property.
   * @see addCssDeclaration for param and return info
   */
  protected function addCssColorDeclaration($category, $name) {
    $value = $this->getStyle($category, $name);
    // Color codes may be given without #
    if (!empty($value) && ctype_xdigit($value)) {
      $value = '#' . $value;
    }
    return $this->cssDeclaration($name, $value);
  }

  /**
   * Creates a CSS declaration.
   *
   * @param string $category The name of the category in which the setting is stored.
   * @param string|array $name The name of the setting and property. If $name is an array it should
   *   be an array with 1 element of the form <setting name> => <property name>. Use this if the
   *   setting name is not the name of the property.
   * @return string The generated CSS declaration: "  property: value;"
   */
  protected function addCssDeclaration($category, $name) {
    $value = $this->getStyle($category, $name);
    return $this->cssDeclaration($name, $value);
  }

  /**
   * Helper method to specify in one single place whether and how to output spaces and new lines.
   */
  protected function cssDeclaration($property, $value) {
    if (is_array($property)) {
      $property = current($property);
    }
    return $value != '' ? "  $property: $value;\n" : '';
  }

  /**
   * Helper method to return 1 style setting (to not repeat the taking care of not being set,
   * defaults, etc).
   *
   * @param string $category
   * @param string|array $name
   * @param mixed $default
   */
  protected function getStyle($category, $name, $default = '') {
    if (is_array($name)) {
      $name = key($name);
    }
    return isset($this->styles[$category][$name]) && $this->styles[$category][$name] !== '<none>' ? $this->styles[$category][$name] : $default;
  }
}

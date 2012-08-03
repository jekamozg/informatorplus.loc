<?php
// $Id: template.php,v 1.4 2010/07/02 23:14:21 eternalistic Exp $
drupal_add_css(drupal_get_path('theme', 'acquia_slate').'/css/style.css','theme','all');
// Override theme_button for expanding graphic buttons
function acquia_slate_button($element) {
  // Make sure not to overwrite classes.
  if (isset($element['#attributes']['class'])) {
    $element['#attributes']['class'] = 'form-'. $element['#button_type'] .' '. $element['#attributes']['class'];
  }
  else {
    $element['#attributes']['class'] = 'form-'. $element['#button_type'];
  }
  
  // Wrap visible inputs with span tags for button graphics
  if (stristr($element['#attributes']['style'], 'display: none;') || stristr($element['#attributes']['class'], 'fivestar-submit') || (is_array($element["#upload_validators"]))) {
    return '<input type="submit" '. (empty($element['#name']) ? '' : 'name="'. $element['#name'] .'" ')  .'id="'. $element['#id'].'" value="'. check_plain($element['#value']) .'" '. drupal_attributes($element['#attributes']) ." />\n";
  }
  else {
    return '<span class="button-wrapper"><span class="button"><span><input type="submit" '. (empty($element['#name']) ? '' : 'name="'. $element['#name'] .'" ')  .'id="'. $element['#id'].'" value="'. check_plain($element['#value']) .'" '. drupal_attributes($element['#attributes']) ." /></span></span></span>\n";
  }
}

//function acquia_slate_menu_item($link, $has_children, $menu = '', $in_active_trail = FALSE, $extra_class = NULL) {
//  $class = ($menu ? 'expanded' : ($has_children ? 'collapsed' : 'leaf'));
//  if (!empty($extra_class)) {
//    $class .= ' '. $extra_class;
//  }
//  if ($in_active_trail) {
//    $class .= ' active-trail';
//  }
//  return '<div class="menu_arrow"></div><li class="'. $class .'">'. $link . $menu ."</li>\n";
//}
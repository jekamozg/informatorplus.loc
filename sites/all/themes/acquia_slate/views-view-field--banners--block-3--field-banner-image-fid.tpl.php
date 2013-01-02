<?php
$node_title = $row->node_title;
$banner_image = theme('image', $output, $node_title, $node_title, array('width' => 470, 'height' => 70), false);
if (isset($row->node_data_field_banner_place_field_banner_link_url)) {
    $link_options = array(
        'attributes' => array('target' => '_blank'),
        'html' => true,
        'absolute' => true,
    );
    print l($banner_image, link_cleanup_url($row->node_data_field_banner_place_field_banner_link_url), $link_options);
} else {
    print $banner_image;
}
?>
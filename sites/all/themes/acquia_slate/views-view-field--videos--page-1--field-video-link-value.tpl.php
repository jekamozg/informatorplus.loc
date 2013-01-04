<?php 
    $url = drupal_parse_url($output);
    if($url && isset($url['query']) && isset($url['query']['v'])) {
        $youtubeThumb = "http://img.youtube.com/vi/{$url['query']['v']}/0.jpg";
    }
?>
<a href="<?php print ($output); ?>" rel="lightvideo[][<?php print $row->node_title;?>]">
    <img src="<?php print $youtubeThumb;?>"/>
</a>
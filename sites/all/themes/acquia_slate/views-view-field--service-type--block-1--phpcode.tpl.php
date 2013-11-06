<?php

$query = "SELECT field_service_subtype_nid AS subtype_nid, node.title FROM content_field_service_subtype ".
         "LEFT JOIN node ON node.nid = content_field_service_subtype.field_service_subtype_nid ".
         "WHERE content_field_service_subtype.nid = {$output} ".
         "ORDER BY node.title ASC"
;
$result = db_query($query);
?>
<?php while($row = db_fetch_array($result)):?>
    <a href="<?php print base_path().'autoservices?field_a_service_subtype_nid='.$row['subtype_nid'];?>"><?php print $row['title'];?></a>
<?php endwhile;?>
<?php
// $Id: comment.tpl.php,v 1.1 2010/12/03 15:31:21 marianojofre Exp $
?>

<div class="comment <?php print $comment_classes;?> clear-block">
  
<div class="usuario">  
  
  <div class="comment-id">
  <?php print $id ?>
  </div>
  
  
  <div class="comment-author">
  <?php print $author ?>
  </div>
  
  
  <div class="comment-picture">
  <?php print $picture ?>
  </div>

  <div class="comment-date">
  <?php print $date ?>
  </div>

</div>
  
<div class="comentario">
  
  <div class="content">
    <?php print $content ?>
  </div>
  
</div>

<?php if ($links): ?>
  <div class="links">
    <?php print $links ?>
  </div>
  <?php endif; ?>
 
</div><!-- /comment -->
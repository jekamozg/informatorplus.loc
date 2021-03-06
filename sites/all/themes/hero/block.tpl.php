<?php
// $Id: block.tpl.php,v 1.1 2010/12/14 18:00:40 marianojofre Exp $
?>

<div class="block-outer">
  <div id="block-<?php print $block->module .'-'. $block->delta; ?>" class="block block-<?php print $block->module ?> <?php print $block_zebra; ?> <?php print $position; ?> <?php print $skinr; ?>">
    <div class="inner clearfix">
      <?php if (isset($edit_links)): ?>
      <?php print $edit_links; ?>
      <?php endif; ?>
      <?php if ($block->subject): ?>
      <h2 class="title block-title"><?php print $block->subject ?></h2>
      <?php endif;?>
      <div class="content clearfix">
        <?php print $block->content ?>
      </div>
    </div><!-- /block-inner -->
  </div><!-- /block -->
</div><!-- /block-outer -->
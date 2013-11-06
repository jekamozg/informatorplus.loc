<?php
// $Id: node.tpl.php,v 1.3 2010/07/02 23:14:21 eternalistic Exp $
//print_r (ymaps_get_node_markers($node));
?>
<div id="node-<?php print $node->nid; ?>" class="node <?php print $node_classes; ?> node-center-inner">
    <div class="inner">
        <?php print $picture ?>

        <?php if ($page == 0): ?>
            <h2 class="title"><a href="<?php print $node_url ?>" title="<?php print $title ?>"><?php print $title ?></a></h2>
        <?php endif; ?>
        <!--
        <?php if ($submitted): ?>
                                                            <div class="meta">
                                                              <span class="submitted"><?php print $submitted ?></span>
                                                            </div>
        <?php endif; ?>
        -->


        <div class="content clearfix">
            <?php if ($node_top && !$teaser): ?>
                <div id="node-top" class="node-top row nested">
                    <div id="node-top-inner" class="node-top-inner inner">
                        <?php print $node_top; ?>
                    </div><!-- /node-top-inner -->
                </div><!-- /node-top -->
            <?php endif; ?>
            <?php print $content ?>
        </div>

        <?php if ($terms): ?>
            <div class="terms">
                <div class="terms-inner">
                    <?php print $terms; ?>
                </div>
            </div>
        <?php endif; ?>

        <?php if ($links): ?>
            <div class="links">
                <div class="links-inner">
                    <?php print $links; ?>
                </div>
            </div>
        <?php endif; ?>
    </div><!-- /inner -->

    <?php if ($node_bottom && !$teaser): ?>
        <div id="node-bottom" class="node-bottom row nested">
            <div id="node-bottom-inner" class="node-bottom-inner inner">
                <?php print $node_bottom; ?>
            </div><!-- /node-bottom-inner -->
        </div><!-- /node-bottom -->
    <?php endif; ?>
</div><!-- /node-<?php print $node->nid; ?> -->

<div class="node-right-inner">
    <div class="node-block">
        <fieldset>
            <legend><?php print $node->title?></legend>
            <div>
                <?php print $node->addFields['field_logo_autoclub']; ?>
            </div>
            <div class="group-join">
                <?php print $node->addFields['group_button']; ?>
            </div>
        </fieldset>
    </div>

    <div class="node-block">
        <fieldset>
            <legend>Участники</legend>
            <?php print $node->addFields['view_autoclub_users']; ?>
        </fieldset>
    </div>
    <div class="node-block">
        <fieldset>
            <legend>Фотоальбомы</legend>
            <?php print $node->addFields['view_autoclub_photoalbums']; ?>
            <?php print $node->adminLinks['add_photoalbum'];?>
        </fieldset>
    </div>
    <div class="node-block">
        <fieldset>
            <legend>Видео</legend>
            <?php print $node->addFields['view_autoclub_videos']; ?>
            <?php print $node->adminLinks['add_video'];?>
        </fieldset>
    </div>
</div>

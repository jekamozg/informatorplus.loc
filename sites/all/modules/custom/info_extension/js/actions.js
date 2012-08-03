$(document).ready(function(){
    $('.info_extension_menu_item').click(function(){
        var current_menu_item = $(this);
        if(!current_menu_item.hasClass('active')) {
            var rel = current_menu_item.attr('rel');
            $('.info_extension_content_item').slideUp(500);
            $('.info_extension_content_item[rel="'+rel+'"]').slideDown(500);
            $('.info_extension_menu_item').removeClass('active');
            current_menu_item.addClass('active');
        }
    });
    info_extension_pager_events();
    info_extension_show_image_description();
});

function info_extension_show_image_description(){
    $('.field-field-c-photo .field-item a.imagefield-field_c_photo').each(function(){
       var current_alt = $('img', this).attr('alt');
       $(this).after('<div class="info_extension_center info_extension_label">'+current_alt+'</div>');
    });
}

function info_extension_pager_rewrite() {
    $('.info_extension_content_item .pager').each(function(){
        var current_item = $(this).html();
        $(this).empty();
        $(this).html(current_item);
    });
}

function info_extension_smooth_slider(item_number) {
    var field = $('.field-field-a-photo');
    var width = 0;
    var items = [];
    var offset = 0;
    var field_items = field.children('.field-items');
    var max_width = field.width();
    function px_to_value(css_value) {
        css_value = parseFloat(css_value);
        return css_value;
    }
    field_items.children('.field-item').each(function(){
        var current_width = $(this).width();
        current_width += px_to_value($(this).css('margin-left'));
        current_width += px_to_value($(this).css('margin-right'));
        current_width += px_to_value($(this).css('padding-left'));
        current_width += px_to_value($(this).css('padding-right'));
        items.push(current_width);
        width += current_width;
        console.log(current_width);
    });
    if(width>field.width()) {
        field_items.before('<div class="smooth_pager field_item_prev">«</div>');
        field_items.after('<div class="smooth_pager field_item_next">»</div>');
    }
    function offset_filter(current_offset, all_width){
        if(current_offset > all_width - items[items.length-1] - (max_width - items[items.length-1])) {
            current_offset = all_width - items[items.length-1]- (max_width - items[items.length-1]);
        }
        if (current_offset < 0) {
            current_offset = 0;
        }
        return current_offset;
    }
    $('.field_item_prev').click(function(){
        offset = offset_filter(offset - items[0], width);
        console.log(offset);
        field_items.animate({
            marginLeft: -offset +'px'
        }, 500);
//        field_items.css({
//            'margin-left': -offset +'px'
//        });
    });
    $('.field_item_next').click(function(){
        offset = offset_filter(offset + items[0], width);
        console.log(offset);
        field_items.animate({
            marginLeft: -offset +'px'
        }, 500);
//        field_items.css({
//            'margin-left': -offset +'px'
//        });
    });
    field_items.css('width', width+'px');
}

function info_extension_pager_events(){
    info_extension_pager_rewrite();
    $('.info_extension_content_item .views-row').click(function(){
        var href = $('.views-field-title a',this).attr('href');
        window.location = href;
    });
    $('.info_extension_content_item .pager a').click(function(){
        var info_item_id = $(this).parents('.info_extension_content_item').attr('info_item_id');
        var href = ($(this).attr('href'));
        var page = href.substring(href.lastIndexOf('?page=') + 6, href.lastIndexOf('?page=') + 7);
        var current_type = $('.info_extension_content_item[info_item_id="'+info_item_id+'"]');
        current_type.animate({
            opacity: 0.2
        }, 1000);
        var height = current_type.height();
        $.ajax({
            type: 'post',
            data: {
                page: page,
                info_item_id: info_item_id,
                info_extension_nid: info_extension_nid
            },
            url: '/info_extension/ajax/events_view/',
            //        dataType: 'json',
            success: function(data) {
                current_type.css({
                    opacity: 0.2
                });
                current_type.html(data);
                current_type.animate({
                    opacity: 1
                }, "slow");
                info_extension_pager_events();
                setTimeout(function(){
                    info_extension_pager_events();
                }, 100);
            }
        });
        return false;
    });
}

$(window).load(function() {
    info_extension_smooth_slider(2);
});
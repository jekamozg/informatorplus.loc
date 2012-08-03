function global_fixes() {
    $('#pid-admin-user-user-list #user-admin-account .tableSelect-processed tbody tr').each(function(){
        $('td',this).each(function(){
           if($('a',this).text() == 'admin') {
               $(this).parent().remove();
           }
        });
    });
}

function autoservice_menu() {
    $('.block-menu .menu li').click(function(){
        $('ul', this).toggle('middle');
    });
}

function autoservice_notitle_event() {
    $('#view-id-autoservices-page_1 .views-row').each(function(){
        //        alert($('.views-field-field-a-settings-value .field-content',this).text());
        if($('.views-field-field-a-settings-value .field-content',this).text() == 'Не показывать заголовок и адрес') {
            $('.views-field-field-logo-fid',this).hide();
            $('.views-field-field-logo-fid-1',this).show();
            $('.views-field-title',this).hide();
            $('.views-field-phpcode', this).hide();
            $('.views-field-street', this).hide();
            $(this).contents().filter(function() {
                return this.nodeType == 3; //Node.TEXT_NODE
            }).remove();
        }
    });
}

function menu_add_plus(){
    $('.block-menu .content > .menu > li.expanded').before('<div class="menu_arrow"></div>');
}

$(document).ready(function(){
    autoservice_menu();
    autoservice_notitle_event();
    global_fixes();
//    menu_add_plus();
});

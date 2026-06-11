#include "utils.h"
#include <stdlib.h>
#include <stdio.h>

void force_eink_refresh() {
    system("eips -c");
    gtk_widget_queue_draw(main_window);
}

GtkWidget* create_chart_card(const char* title, const char* subtitle, GtkWidget** content_area) {
    GtkWidget *event_box = gtk_event_box_new();
    GdkColor white;
    gdk_color_parse("#ffffff", &white);
    gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &white);
    
    GtkWidget *frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
    gtk_container_add(GTK_CONTAINER(event_box), frame);

    GtkWidget *vbox = gtk_vbox_new(FALSE, 6);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);
    gtk_container_add(GTK_CONTAINER(frame), vbox);
    
    GtkWidget *header_hbox = gtk_hbox_new(FALSE, 10);
    GtkWidget *title_vbox = gtk_vbox_new(FALSE, 2);
    char markup[256];
    
    sprintf(markup, "<span size='15000' weight='bold'>%s</span>", title); 
    GtkWidget *lbl_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_title), markup);
    gtk_misc_set_alignment(GTK_MISC(lbl_title), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(title_vbox), lbl_title, FALSE, FALSE, 0);
    
    if (subtitle) {
        sprintf(markup, "<span size='11000' color='#505050'>%s</span>", subtitle);
        GtkWidget *lbl_sub = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(lbl_sub), markup);
        gtk_misc_set_alignment(GTK_MISC(lbl_sub), 0.0, 0.5);
        gtk_box_pack_start(GTK_BOX(title_vbox), lbl_sub, FALSE, FALSE, 0);
    }
    gtk_box_pack_start(GTK_BOX(header_hbox), title_vbox, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), header_hbox, FALSE, FALSE, 0);
    
    *content_area = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), *content_area, TRUE, TRUE, 0);
    
    return event_box;
}

#include "ui_history.h"
#include "utils.h"
#include <cairo.h>

static gboolean on_expose_timeline(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    cairo_t *cr = gdk_cairo_create(widget->window);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    
    double cx = widget->allocation.width / 2.0;
    
    // Draw vertical black line
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_set_line_width(cr, 2.0);
    cairo_move_to(cr, cx, 15);
    cairo_line_to(cr, cx, widget->allocation.height - 15);
    cairo_stroke(cr);
    
    // Draw 3 dots
    double y_positions[] = {25.0, 85.0, 145.0};
    for (int i = 0; i < 3; ++i) {
        cairo_arc(cr, cx, y_positions[i], 4.0, 0, 2 * G_PI);
        cairo_fill(cr);
    }
    
    cairo_destroy(cr);
    return FALSE;
}

GtkWidget* create_history_page() {
    GtkWidget *vbox = gtk_vbox_new(FALSE, 16);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 16);
    
    // Today Overview
    GtkWidget *overview_content;
    GtkWidget *overview_card = create_chart_card("今日阅读概览", NULL, &overview_content);
    
    GtkWidget *stats_hbox = gtk_hbox_new(TRUE, 8);
    const char* stat_vals[] = {"2.5 h", "3 次", "85 页"};
    const char* stat_lbls[] = {"今日阅读", "阅读次数", "翻页数"};
    for(int i = 0; i < 3; ++i) {
        GtkWidget *svbox = gtk_vbox_new(FALSE, 4);
        gtk_container_set_border_width(GTK_CONTAINER(svbox), 8);
        
        char markup[128];
        sprintf(markup, "<span size='18000' weight='bold'>%s</span>", stat_vals[i]);
        GtkWidget *lbl_val = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(lbl_val), markup);
        gtk_misc_set_alignment(GTK_MISC(lbl_val), 0.5, 0.5);
        gtk_box_pack_start(GTK_BOX(svbox), lbl_val, TRUE, TRUE, 0);

        sprintf(markup, "<span size='11000' color='#505050'>%s</span>", stat_lbls[i]);
        GtkWidget *lbl_sub = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(lbl_sub), markup);
        gtk_misc_set_alignment(GTK_MISC(lbl_sub), 0.5, 0.5);
        gtk_box_pack_start(GTK_BOX(svbox), lbl_sub, TRUE, TRUE, 0);
        
        gtk_box_pack_start(GTK_BOX(stats_hbox), svbox, TRUE, TRUE, 0);
    }
    gtk_box_pack_start(GTK_BOX(overview_content), stats_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), overview_card, FALSE, FALSE, 0);
    
    // Timeline Module
    GtkWidget *time_content;
    GtkWidget *time_card = create_chart_card("阅读时间轴", NULL, &time_content);
    
    GtkWidget *timeline_hbox = gtk_hbox_new(FALSE, 10);
    
    // Left: timeline graphic
    GtkWidget *timeline_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(timeline_da, 30, 180);
    g_signal_connect(timeline_da, "expose-event", G_CALLBACK(on_expose_timeline), NULL);
    gtk_box_pack_start(GTK_BOX(timeline_hbox), timeline_da, FALSE, FALSE, 0);
    
    // Right: Event labels
    GtkWidget *events_vbox = gtk_vbox_new(TRUE, 0);
    
    const char* times[] = {"14:30 - 15:45", "10:15 - 11:00", "08:00 - 08:30"};
    const char* descs[] = {"阅读《三体全集》，75分钟", "阅读《设计心理学》，45分钟", "阅读《Steve Jobs》，30分钟"};
    
    for (int i = 0; i < 3; ++i) {
        GtkWidget *evbox = gtk_vbox_new(FALSE, 2);
        char markup[256];
        
        sprintf(markup, "<span size='13000' weight='bold'>%s</span>", times[i]);
        GtkWidget *lbl_time = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(lbl_time), markup);
        gtk_misc_set_alignment(GTK_MISC(lbl_time), 0.0, 0.5);
        gtk_box_pack_start(GTK_BOX(evbox), lbl_time, FALSE, FALSE, 0);
        
        sprintf(markup, "<span size='11000' color='#505050'>%s</span>", descs[i]);
        GtkWidget *lbl_desc = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(lbl_desc), markup);
        gtk_misc_set_alignment(GTK_MISC(lbl_desc), 0.0, 0.5);
        gtk_box_pack_start(GTK_BOX(evbox), lbl_desc, FALSE, FALSE, 0);
        
        gtk_box_pack_start(GTK_BOX(events_vbox), evbox, TRUE, TRUE, 0);
    }
    
    gtk_box_pack_start(GTK_BOX(timeline_hbox), events_vbox, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(time_content), timeline_hbox, FALSE, FALSE, 10);
    
    gtk_box_pack_start(GTK_BOX(vbox), time_card, TRUE, TRUE, 0);
    
    return vbox;
}

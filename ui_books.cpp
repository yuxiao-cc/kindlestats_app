#include "ui_books.h"
#include <cairo.h>
#include <stdio.h>
#include <stdlib.h>

static void destroy_double_ptr(gpointer data, GClosure *closure) {
    g_free(data);
}

// Draw the book cover outline
static gboolean on_expose_book_cover(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    cairo_t *cr = gdk_cairo_create(widget->window);
    
    // Background
    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
    cairo_paint(cr);
    
    // Border
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_set_line_width(cr, 2.0);
    cairo_rectangle(cr, 1, 1, widget->allocation.width - 2, widget->allocation.height - 2);
    cairo_stroke(cr);
    
    // Text
    cairo_set_font_size(cr, 12.0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_text_extents_t extents;
    cairo_text_extents(cr, "封面", &extents);
    cairo_move_to(cr, widget->allocation.width/2.0 - extents.width/2.0, widget->allocation.height/2.0 + extents.height/2.0);
    cairo_show_text(cr, "封面");
    
    cairo_destroy(cr);
    return FALSE;
}

// Draw the black and white progress bar
static gboolean on_expose_progress_bar(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    double progress = *(double*)data; // e.g., 0.75
    
    cairo_t *cr = gdk_cairo_create(widget->window);
    
    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
    cairo_paint(cr);
    
    double w = widget->allocation.width;
    double h = widget->allocation.height;
    
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_rectangle(cr, 0, 0, w * progress, h);
    cairo_fill(cr);
    
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_set_line_width(cr, 1.0);
    cairo_rectangle(cr, 0, 0, w, h);
    cairo_stroke(cr);
    
    cairo_destroy(cr);
    return FALSE;
}

GtkWidget* create_book_item(const char* title, const char* author, const char* time_str, double progress) {
    GtkWidget *hbox = gtk_hbox_new(FALSE, 16);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 8);
    
    // Book Cover
    GtkWidget *cover_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(cover_da, 60, 80);
    g_signal_connect(cover_da, "expose-event", G_CALLBACK(on_expose_book_cover), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), cover_da, FALSE, FALSE, 0);
    
    // Details
    GtkWidget *vbox = gtk_vbox_new(FALSE, 4);
    
    char markup[256];
    sprintf(markup, "<span size='14000' weight='bold'>%s</span>", title);
    GtkWidget *lbl_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_title), markup);
    gtk_misc_set_alignment(GTK_MISC(lbl_title), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), lbl_title, FALSE, FALSE, 0);
    
    sprintf(markup, "<span size='11000' color='#505050'>%s</span>", author);
    GtkWidget *lbl_author = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_author), markup);
    gtk_misc_set_alignment(GTK_MISC(lbl_author), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), lbl_author, FALSE, FALSE, 0);
    
    sprintf(markup, "<span size='11000'>最后阅读: %s</span>", time_str);
    GtkWidget *lbl_time = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_time), markup);
    gtk_misc_set_alignment(GTK_MISC(lbl_time), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), lbl_time, FALSE, FALSE, 0);
    
    // Progress Bar Area
    GtkWidget *prog_hbox = gtk_hbox_new(FALSE, 10);
    
    GtkWidget *prog_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(prog_da, 200, 8); // Thin progress bar
    double *p_val = g_new(double, 1);
    *p_val = progress;
    g_signal_connect_data(prog_da, "expose-event", G_CALLBACK(on_expose_progress_bar), p_val, (GClosureNotify)destroy_double_ptr, (GConnectFlags)0);
    gtk_box_pack_start(GTK_BOX(prog_hbox), prog_da, FALSE, FALSE, 0);
    
    sprintf(markup, "<span size='11000' weight='bold'>%d%%</span>", (int)(progress * 100));
    GtkWidget *lbl_pct = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_pct), markup);
    gtk_box_pack_start(GTK_BOX(prog_hbox), lbl_pct, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(vbox), prog_hbox, FALSE, FALSE, 4);
    
    gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
    
    return hbox;
}

GtkWidget* create_books_page() {
    GtkWidget *vbox = gtk_vbox_new(FALSE, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    
    // Pagination Bar
    GtkWidget *page_hbox = gtk_hbox_new(FALSE, 0);
    
    GtkWidget *btn_prev = gtk_button_new_with_label("上一页");
    gtk_box_pack_start(GTK_BOX(page_hbox), btn_prev, FALSE, FALSE, 0);
    
    GtkWidget *lbl_page = gtk_label_new("<span size='13000' weight='bold'>1 / 4 页</span>");
    gtk_label_set_use_markup(GTK_LABEL(lbl_page), TRUE);
    gtk_box_pack_start(GTK_BOX(page_hbox), lbl_page, TRUE, TRUE, 0);
    
    GtkWidget *btn_next = gtk_button_new_with_label("下一页");
    gtk_box_pack_end(GTK_BOX(page_hbox), btn_next, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(vbox), page_hbox, FALSE, FALSE, 5);
    
    GtkWidget *separator = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 0);
    
    // Mock Book List
    GtkWidget *b1 = create_book_item("三体全集", "刘慈欣", "今天 15:30", 0.45);
    gtk_box_pack_start(GTK_BOX(vbox), b1, FALSE, FALSE, 0);
    
    GtkWidget *b2 = create_book_item("Steve Jobs", "Walter Isaacson", "昨天 21:15", 0.82);
    gtk_box_pack_start(GTK_BOX(vbox), b2, FALSE, FALSE, 0);
    
    GtkWidget *b3 = create_book_item("设计心理学", "Donald A. Norman", "周一 09:00", 0.12);
    gtk_box_pack_start(GTK_BOX(vbox), b3, FALSE, FALSE, 0);
    
    GtkWidget *b4 = create_book_item("C++ Primer", "Stanley B. Lippman", "上周", 0.05);
    gtk_box_pack_start(GTK_BOX(vbox), b4, FALSE, FALSE, 0);
    
    return vbox;
}

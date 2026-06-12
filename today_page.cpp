#include "shared.h"
#include "draw_callbacks.h"
#include "today_page.h"
#include <stdio.h>
#include <string.h>

// Build one timeline row (dot only, no line)
static GtkWidget* create_timeline_row(int idx, SessionData* sess) {
    GtkWidget *eb = gtk_event_box_new();
    GdkColor white;
    gdk_color_parse("#ffffff", &white);
    gtk_widget_modify_bg(eb, GTK_STATE_NORMAL, &white);

    GtkWidget *hbox = gtk_hbox_new(FALSE, 6);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);
    gtk_container_add(GTK_CONTAINER(eb), hbox);

    // Dot drawing area
    GtkWidget *dot_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(dot_da, 24, 40);
    g_signal_connect(dot_da, "expose-event", G_CALLBACK(on_expose_tl_dot), GINT_TO_POINTER(idx == 0 ? 1 : 0));
    gtk_box_pack_start(GTK_BOX(hbox), dot_da, FALSE, FALSE, 0);

    // Content bubble
    GtkWidget *bubble = gtk_vbox_new(FALSE, 3);
    char m[1024];

    // Header: time on left, duration on right
    GtkWidget *hdr = gtk_hbox_new(FALSE, 8);
    sprintf(m, "<span size='12000' weight='bold'>%s</span>", sess->time);
    GtkWidget *lbl_t = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_t), m);
    gtk_misc_set_alignment(GTK_MISC(lbl_t), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(hdr), lbl_t, FALSE, FALSE, 0);

    sprintf(m, "<span size='11000' weight='bold'>读了 %s</span>", sess->duration);
    GtkWidget *lbl_d = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_d), m);
    gtk_misc_set_alignment(GTK_MISC(lbl_d), 1.0, 0.5);
    gtk_box_pack_end(GTK_BOX(hdr), lbl_d, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(bubble), hdr, FALSE, FALSE, 0);

    // Book
    sprintf(m, "<span size='13000' weight='bold'>《%s》</span>", sess->book);
    GtkWidget *lbl_b = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_b), m);
    gtk_misc_set_alignment(GTK_MISC(lbl_b), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(bubble), lbl_b, FALSE, FALSE, 0);

    // Progress
    sprintf(m, "<span size='11000' color='#505050'>进度: %s</span>", sess->progress);
    GtkWidget *lbl_p = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_p), m);
    gtk_misc_set_alignment(GTK_MISC(lbl_p), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(bubble), lbl_p, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(hbox), bubble, TRUE, TRUE, 0);

    return eb;
}

// Timeline background line drawing callback
typedef struct {
    int count;
} TimelineBgData;

static gboolean on_expose_tl_bg(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    TimelineBgData *bg = (TimelineBgData*)data;
    cairo_t *cr = gdk_cairo_create(widget->window);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    if (bg->count > 1) {
        double cx = 12.0; // Center of 24px dot area
        double row_h = 48.0; // 40px dot area + 8px spacing
        
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_set_line_width(cr, 2);
        cairo_move_to(cr, cx, 16); // Start from first dot position
        cairo_line_to(cr, cx, 16 + (bg->count - 1) * row_h); // To last dot position
        cairo_stroke(cr);
    }

    cairo_destroy(cr);
    return FALSE;
}

void rebuild_timeline() {
    if (!g_today_timeline_container) return;

    // Clear
    GList *children = gtk_container_get_children(GTK_CONTAINER(g_today_timeline_container));
    for (GList *l = children; l; l = l->next) gtk_widget_destroy(GTK_WIDGET(l->data));
    g_list_free(children);

    SessionData* sessions = g_sessions_today;
    int count = NUM_SESSIONS_TODAY;

    if (count == 0) {
        GtkWidget *empty = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(empty),
            "<span size='16000' weight='bold'>今日无阅读记录</span>\n"
            "<span size='13000' color='#505050'>今天还没有阅读记录，快去读书吧！</span>");
        gtk_misc_set_alignment(GTK_MISC(empty), 0.5, 0.5);
        gtk_box_pack_start(GTK_BOX(g_today_timeline_container), empty, TRUE, TRUE, 20);
    } else {
        // Create overlay container for background line + rows
        GtkWidget *overlay = gtk_fixed_new();
        gtk_box_pack_start(GTK_BOX(g_today_timeline_container), overlay, TRUE, TRUE, 0);

        // Background line drawing area
        GtkWidget *bg_da = gtk_drawing_area_new();
        gtk_widget_set_size_request(bg_da, 24, 16 + (count - 1) * 48);
        TimelineBgData *bg_data = g_new(TimelineBgData, 1);
        bg_data->count = count;
        g_signal_connect_data(bg_da, "expose-event", G_CALLBACK(on_expose_tl_bg),
                              bg_data, (GClosureNotify)g_free, (GConnectFlags)0);
        gtk_fixed_put(GTK_FIXED(overlay), bg_da, 0, 0);

        // Timeline rows
        for (int i = 0; i < count; i++) {
            GtkWidget *row = create_timeline_row(i, &sessions[i]);
            gtk_fixed_put(GTK_FIXED(overlay), row, 0, i * 48);
        }
    }

    gtk_widget_show_all(g_today_timeline_container);
}

GtkWidget* create_today_page() {
    GtkWidget *page_vbox = gtk_vbox_new(FALSE, 6);
    gtk_container_set_border_width(GTK_CONTAINER(page_vbox), 6);

    // Header with date
    GtkWidget *hdr = gtk_hbox_new(FALSE, 8);
    gtk_container_set_border_width(GTK_CONTAINER(hdr), 4);
    
    g_today_date_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(g_today_date_label), 
        "<span size='15000' weight='bold'>今天 (06月12日)</span>");
    gtk_misc_set_alignment(GTK_MISC(g_today_date_label), 0.5, 0.5);
    gtk_box_pack_start(GTK_BOX(hdr), g_today_date_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(page_vbox), hdr, FALSE, FALSE, 0);

    // Separator
    GtkWidget *sep = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(page_vbox), sep, FALSE, FALSE, 0);

    // Timeline container
    g_today_timeline_container = gtk_vbox_new(FALSE, 2);
    gtk_box_pack_start(GTK_BOX(page_vbox), g_today_timeline_container, TRUE, TRUE, 0);

    rebuild_timeline();
    return page_vbox;
}

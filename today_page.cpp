#include "shared.h"
#include "draw_callbacks.h"
#include "today_page.h"
#include <stdio.h>
#include <string.h>

// Build one timeline row
static GtkWidget* create_timeline_row(int idx, SessionData* sess) {
    GtkWidget *eb = gtk_event_box_new();
    GdkColor white;
    gdk_color_parse("#ffffff", &white);
    gtk_widget_modify_bg(eb, GTK_STATE_NORMAL, &white);

    GtkWidget *hbox = gtk_hbox_new(FALSE, 6);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);
    gtk_container_add(GTK_CONTAINER(eb), hbox);

    // Dot + line drawing area
    GtkWidget *dot_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(dot_da, 24, 40);
    g_signal_connect(dot_da, "expose-event", G_CALLBACK(on_expose_tl_dot), GINT_TO_POINTER(idx == 0));
    gtk_box_pack_start(GTK_BOX(hbox), dot_da, FALSE, FALSE, 0);

    // Content bubble
    GtkWidget *bubble = gtk_vbox_new(FALSE, 3);
    char m[1024];

    // Header: time + duration
    GtkWidget *hdr = gtk_hbox_new(FALSE, 8);
    sprintf(m, "<span size='15000' weight='bold'>%s</span>", sess->time);
    GtkWidget *lbl_t = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_t), m);
    gtk_misc_set_alignment(GTK_MISC(lbl_t), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(hdr), lbl_t, FALSE, FALSE, 0);

    sprintf(m, "<span size='13000' weight='bold'>读了 %s</span>", sess->duration);
    GtkWidget *lbl_d = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_d), m);
    gtk_misc_set_alignment(GTK_MISC(lbl_d), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(hdr), lbl_d, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(bubble), hdr, FALSE, FALSE, 0);

    // Book
    sprintf(m, "<span size='16000' weight='bold'>《%s》</span>", sess->book);
    GtkWidget *lbl_b = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_b), m);
    gtk_misc_set_alignment(GTK_MISC(lbl_b), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(bubble), lbl_b, FALSE, FALSE, 0);

    // Progress
    sprintf(m, "<span size='13000' color='#505050'>进度: %s</span>", sess->progress);
    GtkWidget *lbl_p = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_p), m);
    gtk_misc_set_alignment(GTK_MISC(lbl_p), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(bubble), lbl_p, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(hbox), bubble, TRUE, TRUE, 0);

    // Separator
    GtkWidget *sep = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(eb), sep, FALSE, FALSE, 0);

    return eb;
}

static void update_date_label() {
    if (!g_today_date_label) return;
    const char* text;
    if (g_today_day_idx == 0) text = "今天 (06月10日)";
    else if (g_today_day_idx == 1) text = "昨天 (06月09日)";
    else text = "前天 (06月08日)";
    char m[128];
    sprintf(m, "<span size='15000' weight='bold'>%s</span>", text);
    gtk_label_set_markup(GTK_LABEL(g_today_date_label), m);
}

void rebuild_timeline() {
    if (!g_today_timeline_container) return;
    update_date_label();

    // Clear
    GList *children = gtk_container_get_children(GTK_CONTAINER(g_today_timeline_container));
    for (GList *l = children; l; l = l->next) gtk_widget_destroy(GTK_WIDGET(l->data));
    g_list_free(children);

    SessionData* sessions = NULL;
    int count = 0;
    if (g_today_day_idx == 0) { sessions = g_sessions_today; count = NUM_SESSIONS_TODAY; }
    else if (g_today_day_idx == 1) { sessions = g_sessions_yesterday; count = NUM_SESSIONS_YESTERDAY; }
    else { sessions = g_sessions_2days; count = NUM_SESSIONS_2DAYS; }

    if (count == 0) {
        GtkWidget *empty = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(empty),
            "<span size='16000' weight='bold'>该日无阅读记录</span>\n"
            "<span size='13000' color='#505050'>你可以点击上方切换按钮查看其他有阅读记录的日期。</span>");
        gtk_misc_set_alignment(GTK_MISC(empty), 0.5, 0.5);
        gtk_box_pack_start(GTK_BOX(g_today_timeline_container), empty, TRUE, TRUE, 20);
    } else {
        for (int i = 0; i < count; i++) {
            gtk_box_pack_start(GTK_BOX(g_today_timeline_container),
                               create_timeline_row(i, &sessions[i]), FALSE, FALSE, 2);
        }
    }

    gtk_widget_show_all(g_today_timeline_container);
}

static void on_prev_day(GtkButton *btn, gpointer data) {
    if (g_today_day_idx < 2) { g_today_day_idx++; rebuild_timeline(); }
}

static void on_next_day(GtkButton *btn, gpointer data) {
    if (g_today_day_idx > 0) { g_today_day_idx--; rebuild_timeline(); }
}

GtkWidget* create_today_page() {
    GtkWidget *page_vbox = gtk_vbox_new(FALSE, 6);
    gtk_container_set_border_width(GTK_CONTAINER(page_vbox), 6);

    // Header
    GtkWidget *hdr = gtk_hbox_new(FALSE, 8);
    gtk_container_set_border_width(GTK_CONTAINER(hdr), 4);

    GtkWidget *prev = gtk_button_new_with_label("[ < 切换前一日 ]");
    GtkWidget *next = gtk_button_new_with_label("[ 切换后一日 > ]");
    g_today_date_label = gtk_label_new(NULL);
    gtk_misc_set_alignment(GTK_MISC(g_today_date_label), 0.5, 0.5);

    g_signal_connect(prev, "clicked", G_CALLBACK(on_prev_day), NULL);
    g_signal_connect(next, "clicked", G_CALLBACK(on_next_day), NULL);

    gtk_box_pack_start(GTK_BOX(hdr), prev, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hdr), g_today_date_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hdr), next, FALSE, FALSE, 0);
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

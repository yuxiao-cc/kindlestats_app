#include "shared.h"
#include "draw_callbacks.h"
#include "today_page.h"
#include <stdio.h>
#include <string.h>

#define ROW_HEIGHT 80
#define DOT_RADIUS 8

// Build one timeline row
static GtkWidget* create_timeline_row(int global_idx, int idx, int count, SessionData* sess) {
    GtkWidget *eb = gtk_event_box_new();
    GdkColor white;
    gdk_color_parse("#ffffff", &white);
    gtk_widget_modify_bg(eb, GTK_STATE_NORMAL, &white);

    GtkWidget *hbox = gtk_hbox_new(FALSE, 12);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 8);
    gtk_container_add(GTK_CONTAINER(eb), hbox);

    // Dot + line drawing area (full height of row)
    GtkWidget *dot_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(dot_da, 30, ROW_HEIGHT);
    int is_global_first = (global_idx == 0) ? 1 : 0;
    int is_last = (idx == count - 1) ? 1 : 0;
    int is_page_first_no_top = (global_idx > 0 && idx == 0) ? 1 : 0;
    int flags = is_global_first | (is_last << 1) | (is_page_first_no_top << 2);
    g_signal_connect(dot_da, "expose-event", G_CALLBACK(on_expose_tl_dot), GINT_TO_POINTER(flags));
    gtk_box_pack_start(GTK_BOX(hbox), dot_da, FALSE, FALSE, 0);

    // Content
    GtkWidget *vbox = gtk_vbox_new(FALSE, 4);
    char m[1024];

    // Time + duration
    GtkWidget *hdr = gtk_hbox_new(FALSE, 8);
    sprintf(m, "<span size='12000' weight='bold'>%s</span>", sess->time);
    GtkWidget *lbl_t = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_t), m);
    gtk_misc_set_alignment(GTK_MISC(lbl_t), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(hdr), lbl_t, FALSE, FALSE, 0);

    sprintf(m, "<span size='11000' color='#505050'>读了 %s</span>", sess->duration);
    GtkWidget *lbl_d = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_d), m);
    gtk_misc_set_alignment(GTK_MISC(lbl_d), 1.0, 0.5);
    gtk_box_pack_end(GTK_BOX(hdr), lbl_d, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hdr, FALSE, FALSE, 0);

    // Book
    sprintf(m, "<span size='14000' weight='bold'>《%s》</span>", sess->book);
    GtkWidget *lbl_b = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_b), m);
    gtk_misc_set_alignment(GTK_MISC(lbl_b), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), lbl_b, FALSE, FALSE, 0);

    // Progress
    sprintf(m, "<span size='11000' color='#505050'>进度: %s</span>", sess->progress);
    GtkWidget *lbl_p = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_p), m);
    gtk_misc_set_alignment(GTK_MISC(lbl_p), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), lbl_p, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);

    return eb;
}

static void on_prev_page(GtkButton *btn, gpointer data) {
    if (g_today_page > 0) { g_today_page--; rebuild_timeline(); }
}

static void on_next_page(GtkButton *btn, gpointer data) {
    g_today_page++;
    rebuild_timeline();
}

void rebuild_timeline() {
    if (!g_today_timeline_container) return;

    // Clear
    GList *children = gtk_container_get_children(GTK_CONTAINER(g_today_timeline_container));
    for (GList *l = children; l; l = l->next) gtk_widget_destroy(GTK_WIDGET(l->data));
    g_list_free(children);

    int count = NUM_SESSIONS_TODAY;
    int total_pages = (count + SESSIONS_PER_PAGE - 1) / SESSIONS_PER_PAGE;
    if (total_pages == 0) total_pages = 1;
    if (g_today_page >= total_pages) g_today_page = total_pages - 1;
    if (g_today_page < 0) g_today_page = 0;

    if (count == 0) {
        GtkWidget *empty = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(empty),
            "<span size='16000' weight='bold'>今日无阅读记录</span>\n"
            "<span size='13000' color='#505050'>今天还没有阅读记录，快去读书吧！</span>");
        gtk_misc_set_alignment(GTK_MISC(empty), 0.5, 0.5);
        gtk_box_pack_start(GTK_BOX(g_today_timeline_container), empty, TRUE, TRUE, 20);
    } else {
        int start = g_today_page * SESSIONS_PER_PAGE;
        int end = start + SESSIONS_PER_PAGE;
        if (end > count) end = count;
        int page_count = end - start;

        for (int i = 0; i < page_count; i++) {
            gtk_box_pack_start(GTK_BOX(g_today_timeline_container),
                               create_timeline_row(start + i, i, page_count, &g_sessions_today[start + i]),
                               FALSE, FALSE, 0);
        }
    }

    // Update pagination label
    if (g_today_page_label) {
        char buf[64];
        char m[128];
        if (total_pages <= 0) sprintf(buf, "第 0 / 0 页");
        else sprintf(buf, "第 %d / %d 页", g_today_page + 1, total_pages);
        sprintf(m, "<span size='14000' weight='bold'>%s</span>", buf);
        gtk_label_set_markup(GTK_LABEL(g_today_page_label), m);
    }

    gtk_widget_show_all(g_today_timeline_container);
}

GtkWidget* create_today_page() {
    GtkBuilder *builder = load_ui("today_page.ui");
    if (!builder) return gtk_vbox_new(FALSE, 0);

    GtkWidget *page = GTK_WIDGET(gtk_builder_get_object(builder, "today_page"));
    if (!page || !GTK_IS_WIDGET(page)) { g_object_unref(builder); return gtk_vbox_new(FALSE, 0); }
    g_object_ref_sink(page);

    // Assign globals from builder
    g_today_date_label = GTK_WIDGET(gtk_builder_get_object(builder, "today_date_label"));
    g_today_timeline_container = GTK_WIDGET(gtk_builder_get_object(builder, "today_timeline_container"));
    g_today_page_label = GTK_WIDGET(gtk_builder_get_object(builder, "today_page_label"));

    gtk_label_set_markup(GTK_LABEL(g_today_date_label),
        "<span size='15000' weight='bold'>今天 (06月12日)</span>");
    gtk_label_set_markup(GTK_LABEL(g_today_page_label),
        "<span size='14000' weight='bold'>第 1 / 1 页</span>");

    g_object_ref(g_today_date_label);
    g_object_ref(g_today_timeline_container);
    g_object_ref(g_today_page_label);

    // Connect pagination signals
    GtkWidget *prev = GTK_WIDGET(gtk_builder_get_object(builder, "today_prev_btn"));
    GtkWidget *next = GTK_WIDGET(gtk_builder_get_object(builder, "today_next_btn"));
    g_signal_connect(prev, "clicked", G_CALLBACK(on_prev_page), NULL);
    g_signal_connect(next, "clicked", G_CALLBACK(on_next_page), NULL);

    g_object_unref(builder);
    rebuild_timeline();
    return page;
}
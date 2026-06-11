#include "shared.h"
#include "draw_callbacks.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// ==================== Global state for heatmap ====================
static GtkWidget *g_heat_drawing_area;
static GtkWidget *g_heat_stats_label;
static GtkWidget *g_heat_year_lbl;
static GtkWidget *g_heat_month_lbl;
static int g_heat_year = 2026;
static int g_heat_month = 6;

// ==================== Stats calculation ====================

static int get_total_reading_minutes() {
    return 400 + (int)((sin(g_heat_month) + 1) * 300);
}

static int get_active_days() {
    return 10 + (int)((cos(g_heat_month) + 1) * 8);
}

static int get_max_streak() {
    return 2 + (int)((sin(g_heat_month * 2) + 1) * 3);
}

static int get_avg_daily_min() {
    int total = get_total_reading_minutes();
    int days = get_active_days();
    return days > 0 ? total / days : 0;
}

static void refresh_heat_stats() {
    if (!g_heat_stats_label) return;
    char m[768];
    int total = get_total_reading_minutes();
    int active = get_active_days();
    int streak = get_max_streak();
    int avg = get_avg_daily_min();
    sprintf(m,
        "<span size='9500' color='#505050'>累计阅读</span>\n"
        "<span size='12000' weight='bold'>%.1f 小时</span>\n"
        "<span size='9500' color='#505050'>活跃天数</span>\n"
        "<span size='12000' weight='bold'>%d 天</span>\n"
        "<span size='9500' color='#505050'>最长连读</span>\n"
        "<span size='12000' weight='bold'>%d 天</span>\n"
        "<span size='9500' color='#505050'>日均阅读</span>\n"
        "<span size='12000' weight='bold'>%d 分钟</span>",
        total / 60.0, active, streak, avg);
    gtk_label_set_markup(GTK_LABEL(g_heat_stats_label), m);
}

static void refresh_heat_date_labels() {
    if (g_heat_year_lbl) {
        char m[64];
        sprintf(m, "<span size='11000' weight='bold'>%d年</span>", g_heat_year);
        gtk_label_set_markup(GTK_LABEL(g_heat_year_lbl), m);
    }
    if (g_heat_month_lbl) {
        char m[64];
        sprintf(m, "<span size='11000' weight='bold'>%d月</span>", g_heat_month);
        gtk_label_set_markup(GTK_LABEL(g_heat_month_lbl), m);
    }
}

static void refresh_heat_canvas() {
    refresh_heat_date_labels();
    refresh_heat_stats();
    if (g_heat_drawing_area) gtk_widget_queue_draw(g_heat_drawing_area);
}

// ==================== Monthly heatmap drawing ====================

static gboolean on_expose_heatmap_monthly(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    cairo_t *cr = gdk_cairo_create(widget->window);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    double w = widget->allocation.width;
    double h = widget->allocation.height;

    int fw = first_weekday_of_month(g_heat_year, g_heat_month);
    int dm = days_in_month(g_heat_year, g_heat_month);

    // Tighter horizontal padding to give cells more room
    double pad_x = 4.0, pad_y = 4.0;
    double header_h = 28.0;
    double cw = (w - pad_x * 2) / 7.0;
    double ch = (h - pad_y * 2 - header_h) / 6.0;
    if (cw < 8) cw = 8;
    if (ch < 8) ch = 8;

    // Weekday headers - larger font
    const char* dh[] = {"一","二","三","四","五","六","日"};
    cairo_set_font_size(cr, 20);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    for (int i = 0; i < 7; i++) {
        cairo_text_extents_t ext;
        cairo_text_extents(cr, dh[i], &ext);
        cairo_move_to(cr, pad_x + i * cw + cw/2 - ext.width/2, pad_y + 20);
        cairo_show_text(cr, dh[i]);
    }

    // Separator under weekday header
    cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
    cairo_set_line_width(cr, 1);
    cairo_move_to(cr, pad_x, pad_y + header_h - 4);
    cairo_line_to(cr, w - pad_x, pad_y + header_h - 4);
    cairo_stroke(cr);

    // Day cells (no day numbers, just heat color)
    int row = 0, col = fw;
    for (int day = 1; day <= dm; day++) {
        double x = pad_x + col * cw;
        double y = pad_y + header_h + row * ch;

        double seed = sin(day + g_heat_month + g_heat_year);
        double gc;
        if (seed > 0.7) gc = 0.0;
        else if (seed > 0.4) gc = 0.4;
        else if (seed > 0.1) gc = 0.66;
        else if (seed > -0.3) gc = 0.86;
        else gc = 1.0;

        cairo_set_source_rgb(cr, gc, gc, gc);
        cairo_rectangle(cr, x + 1, y + 1, cw - 2, ch - 2);
        cairo_fill(cr);

        cairo_set_source_rgb(cr, 0.75, 0.75, 0.75);
        cairo_set_line_width(cr, 0.5);
        cairo_rectangle(cr, x + 1, y + 1, cw - 2, ch - 2);
        cairo_stroke(cr);

        col++;
        if (col >= 7) { col = 0; row++; }
    }

    cairo_destroy(cr);
    return FALSE;
}

// ==================== Heatmap control callbacks ====================

static void on_heat_year_prev(GtkButton *b, gpointer d) {
    g_heat_year--;
    refresh_heat_canvas();
}

static void on_heat_year_next(GtkButton *b, gpointer d) {
    g_heat_year++;
    refresh_heat_canvas();
}

static void on_heat_month_prev(GtkButton *b, gpointer d) {
    g_heat_month--;
    if (g_heat_month < 1) { g_heat_month = 12; g_heat_year--; }
    refresh_heat_canvas();
}

static void on_heat_month_next(GtkButton *b, gpointer d) {
    g_heat_month++;
    if (g_heat_month > 12) { g_heat_month = 1; g_heat_year++; }
    refresh_heat_canvas();
}

// ==================== Create a chart card (compact fonts) ====================

static GtkWidget* create_chart_card_compact(const char* title, const char* subtitle,
                                            GtkWidget** content_area) {
    GtkWidget *event_box = gtk_event_box_new();
    GdkColor white;
    gdk_color_parse("#ffffff", &white);
    gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &white);

    GtkWidget *frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
    gtk_container_add(GTK_CONTAINER(event_box), frame);

    GtkWidget *vbox = gtk_vbox_new(FALSE, 4);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);
    gtk_container_add(GTK_CONTAINER(frame), vbox);

    GtkWidget *header_hbox = gtk_hbox_new(FALSE, 8);
    GtkWidget *title_vbox = gtk_vbox_new(FALSE, 1);
    char markup[256];

    sprintf(markup, "<span size='13000' weight='bold'>%s</span>", title);
    GtkWidget *lbl_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_title), markup);
    gtk_misc_set_alignment(GTK_MISC(lbl_title), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(title_vbox), lbl_title, FALSE, FALSE, 0);

    if (subtitle) {
        sprintf(markup, "<span size='10000'>%s</span>", subtitle);
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

// ==================== Create a stat card (smaller fonts) ====================

static GtkWidget* create_stat_card(const char* val, const char* lbl) {
    GtkWidget *event_box = gtk_event_box_new();
    GdkColor white;
    gdk_color_parse("#ffffff", &white);
    gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &white);

    GtkWidget *frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
    gtk_container_add(GTK_CONTAINER(event_box), frame);

    GtkWidget *vbox = gtk_vbox_new(FALSE, 1);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);
    gtk_container_add(GTK_CONTAINER(frame), vbox);

    char m_val[128];
    sprintf(m_val, "<span size='18000' weight='bold'>%s</span>", val);
    GtkWidget *lbl_val = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_val), m_val);
    gtk_misc_set_alignment(GTK_MISC(lbl_val), 0.5, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), lbl_val, TRUE, TRUE, 0);

    char m_sub[128];
    sprintf(m_sub, "<span size='10000'>%s</span>", lbl);
    GtkWidget *lbl_sub = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_sub), m_sub);
    gtk_misc_set_alignment(GTK_MISC(lbl_sub), 0.5, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), lbl_sub, TRUE, TRUE, 0);

    return event_box;
}

// ==================== Build dashboard page ====================

GtkWidget* create_dashboard_page() {
    GtkWidget *vbox = gtk_vbox_new(FALSE, 4);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

    // ===== 3 stat cards =====
    GtkWidget *stats_hbox = gtk_hbox_new(TRUE, 4);
    const char* stat_vals[] = {"128.5 h", "24 本", "46 min"};
    const char* stat_lbls[] = {"累计阅读", "已读书籍", "日均阅读"};
    for (int i = 0; i < 3; i++) {
        gtk_box_pack_start(GTK_BOX(stats_hbox), create_stat_card(stat_vals[i], stat_lbls[i]), TRUE, TRUE, 0);
    }
    gtk_box_pack_start(GTK_BOX(vbox), stats_hbox, FALSE, FALSE, 0);

    // ===== Heatmap card: [title on left, year/month controls on right] =====
    GtkWidget *heat_eb = gtk_event_box_new();
    GdkColor white;
    gdk_color_parse("#ffffff", &white);
    gtk_widget_modify_bg(heat_eb, GTK_STATE_NORMAL, &white);

    GtkWidget *heat_frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(heat_frame), GTK_SHADOW_OUT);
    gtk_container_add(GTK_CONTAINER(heat_eb), heat_frame);

    GtkWidget *heat_vbox = gtk_vbox_new(FALSE, 3);
    gtk_container_set_border_width(GTK_CONTAINER(heat_vbox), 6);
    gtk_container_add(GTK_CONTAINER(heat_frame), heat_vbox);

    // Header row: title (left, expandable) + year/month controls (right)
    GtkWidget *heat_hdr = gtk_hbox_new(FALSE, 3);

    GtkWidget *heat_title_lbl = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(heat_title_lbl), "<span size='12000' weight='bold'>阅读热力图</span>");
    gtk_misc_set_alignment(GTK_MISC(heat_title_lbl), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(heat_hdr), heat_title_lbl, TRUE, TRUE, 0);

    // Year group: wrap in EventBox for e-ink + fixed-width label to prevent overlap
    GtkWidget *yr_eb = gtk_event_box_new();
    GdkColor ebg;
    gdk_color_parse("#ffffff", &ebg);
    gtk_widget_modify_bg(yr_eb, GTK_STATE_NORMAL, &ebg);

    GtkWidget *yr_hbox = gtk_hbox_new(FALSE, 0);
    GtkWidget *yr_prev = gtk_button_new();
    GtkWidget *yr_prev_lbl = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(yr_prev_lbl), "<span size='10000'>&lt;&lt;</span>");
    gtk_container_add(GTK_CONTAINER(yr_prev), yr_prev_lbl);
    g_signal_connect(yr_prev, "clicked", G_CALLBACK(on_heat_year_prev), NULL);
    gtk_box_pack_start(GTK_BOX(yr_hbox), yr_prev, FALSE, FALSE, 0);

    g_heat_year_lbl = gtk_label_new(NULL);
    gtk_widget_set_size_request(g_heat_year_lbl, 100, -1);
    gtk_misc_set_alignment(GTK_MISC(g_heat_year_lbl), 0.5, 0.5);
    gtk_box_pack_start(GTK_BOX(yr_hbox), g_heat_year_lbl, FALSE, FALSE, 0);

    GtkWidget *yr_next = gtk_button_new();
    GtkWidget *yr_next_lbl = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(yr_next_lbl), "<span size='10000'>&gt;&gt;</span>");
    gtk_container_add(GTK_CONTAINER(yr_next), yr_next_lbl);
    g_signal_connect(yr_next, "clicked", G_CALLBACK(on_heat_year_next), NULL);
    gtk_box_pack_start(GTK_BOX(yr_hbox), yr_next, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(yr_eb), yr_hbox);
    gtk_box_pack_start(GTK_BOX(heat_hdr), yr_eb, FALSE, FALSE, 0);

    // Spacer
    GtkWidget *sp1 = gtk_label_new(" ");
    gtk_box_pack_start(GTK_BOX(heat_hdr), sp1, FALSE, FALSE, 0);

    // Month group: same structure
    GtkWidget *mo_eb = gtk_event_box_new();
    gtk_widget_modify_bg(mo_eb, GTK_STATE_NORMAL, &ebg);

    GtkWidget *mo_hbox = gtk_hbox_new(FALSE, 0);
    GtkWidget *mo_prev = gtk_button_new();
    GtkWidget *mo_prev_lbl = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(mo_prev_lbl), "<span size='10000'>&lt;&lt;</span>");
    gtk_container_add(GTK_CONTAINER(mo_prev), mo_prev_lbl);
    g_signal_connect(mo_prev, "clicked", G_CALLBACK(on_heat_month_prev), NULL);
    gtk_box_pack_start(GTK_BOX(mo_hbox), mo_prev, FALSE, FALSE, 0);

    g_heat_month_lbl = gtk_label_new(NULL);
    gtk_widget_set_size_request(g_heat_month_lbl, 60, -1);
    gtk_misc_set_alignment(GTK_MISC(g_heat_month_lbl), 0.5, 0.5);
    gtk_box_pack_start(GTK_BOX(mo_hbox), g_heat_month_lbl, FALSE, FALSE, 0);

    GtkWidget *mo_next = gtk_button_new();
    GtkWidget *mo_next_lbl = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(mo_next_lbl), "<span size='10000'>&gt;&gt;</span>");
    gtk_container_add(GTK_CONTAINER(mo_next), mo_next_lbl);
    g_signal_connect(mo_next, "clicked", G_CALLBACK(on_heat_month_next), NULL);
    gtk_box_pack_start(GTK_BOX(mo_hbox), mo_next, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(mo_eb), mo_hbox);
    gtk_box_pack_start(GTK_BOX(heat_hdr), mo_eb, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(heat_vbox), heat_hdr, FALSE, FALSE, 0);

    // Content: left chart + right stats
    GtkWidget *heat_content = gtk_hbox_new(FALSE, 6);
    gtk_box_pack_start(GTK_BOX(heat_vbox), heat_content, TRUE, TRUE, 0);

    // Left: heatmap drawing area (capped at ~70% width via size_request)
    g_heat_drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(g_heat_drawing_area, 600, 120);
    g_signal_connect(g_heat_drawing_area, "expose-event", G_CALLBACK(on_expose_heatmap_monthly), NULL);
    gtk_box_pack_start(GTK_BOX(heat_content), g_heat_drawing_area, TRUE, TRUE, 0);

    // Right: compact stats text (4 items, smaller font, wider)
    g_heat_stats_label = gtk_label_new(NULL);
    gtk_misc_set_alignment(GTK_MISC(g_heat_stats_label), 0.0, 0.0);
    gtk_widget_set_size_request(g_heat_stats_label, 180, -1);
    gtk_box_pack_start(GTK_BOX(heat_content), g_heat_stats_label, FALSE, FALSE, 0);

    // Apply initial state
    refresh_heat_canvas();

    gtk_box_pack_start(GTK_BOX(vbox), heat_eb, FALSE, FALSE, 0);

    // ===== Gold Hour (24h bar) - subtitle on right to give chart more space =====
    GtkWidget *gold_eb = gtk_event_box_new();
    GdkColor goldbg;
    gdk_color_parse("#ffffff", &goldbg);
    gtk_widget_modify_bg(gold_eb, GTK_STATE_NORMAL, &goldbg);

    GtkWidget *gold_frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(gold_frame), GTK_SHADOW_OUT);
    gtk_container_add(GTK_CONTAINER(gold_eb), gold_frame);

    GtkWidget *gold_vbox = gtk_vbox_new(FALSE, 3);
    gtk_container_set_border_width(GTK_CONTAINER(gold_vbox), 6);
    gtk_container_add(GTK_CONTAINER(gold_frame), gold_vbox);

    // Header: title (left) + subtitle (right)
    GtkWidget *gold_hdr = gtk_hbox_new(FALSE, 4);
    GtkWidget *gold_title_lbl = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(gold_title_lbl), "<span size='12000' weight='bold'>最爱阅读时段</span>");
    gtk_misc_set_alignment(GTK_MISC(gold_title_lbl), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(gold_hdr), gold_title_lbl, TRUE, TRUE, 0);

    GtkWidget *gold_sub_lbl = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(gold_sub_lbl), "<span size='10000' color='#505050'>最常在 18:00 - 21:00</span>");
    gtk_misc_set_alignment(GTK_MISC(gold_sub_lbl), 1.0, 0.5);
    gtk_box_pack_end(GTK_BOX(gold_hdr), gold_sub_lbl, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(gold_vbox), gold_hdr, FALSE, FALSE, 0);

    // Bar chart (taller now, ~160px to give bars more vertical room)
    GtkWidget *bar24_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(bar24_da, -1, 160);
    g_signal_connect(bar24_da, "expose-event", G_CALLBACK(on_expose_bar24), NULL);
    gtk_box_pack_start(GTK_BOX(gold_vbox), bar24_da, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), gold_eb, FALSE, FALSE, 0);

    // ===== 7-day trend hbar (no subtitle, simpler header) =====
    GtkWidget *hbar_eb = gtk_event_box_new();
    GdkColor hbarbg;
    gdk_color_parse("#ffffff", &hbarbg);
    gtk_widget_modify_bg(hbar_eb, GTK_STATE_NORMAL, &hbarbg);

    GtkWidget *hbar_frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(hbar_frame), GTK_SHADOW_OUT);
    gtk_container_add(GTK_CONTAINER(hbar_eb), hbar_frame);

    GtkWidget *hbar_vbox = gtk_vbox_new(FALSE, 3);
    gtk_container_set_border_width(GTK_CONTAINER(hbar_vbox), 6);
    gtk_container_add(GTK_CONTAINER(hbar_frame), hbar_vbox);

    GtkWidget *hbar_title_lbl = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(hbar_title_lbl), "<span size='12000' weight='bold'>近7日阅读趋势</span>");
    gtk_misc_set_alignment(GTK_MISC(hbar_title_lbl), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(hbar_vbox), hbar_title_lbl, FALSE, FALSE, 0);

    // hbar drawing area (taller for more spacing between rows)
    GtkWidget *hbar_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(hbar_da, -1, 180);
    g_signal_connect(hbar_da, "expose-event", G_CALLBACK(on_expose_hbar), NULL);
    gtk_box_pack_start(GTK_BOX(hbar_vbox), hbar_da, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbar_eb, FALSE, FALSE, 0);

    return vbox;
}

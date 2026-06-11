#include "shared.h"
#include "draw_callbacks.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// ==================== Global state for heatmap ====================
static GtkWidget *g_heat_drawing_area;
static GtkWidget *g_heat_stats_label;
static int g_heat_year = 2026;
static int g_heat_month = 6; // 0 = annual view, 1-12 = month view

static gboolean is_monthly_view() { return g_heat_month >= 1 && g_heat_month <= 12; }

static int get_total_reading_minutes() {
    if (is_monthly_view()) {
        return 400 + (int)((sin(g_heat_month) + 1) * 300);
    }
    // Annual: sum of 12 months
    int total = 0;
    for (int m = 1; m <= 12; m++) total += 400 + (int)((sin(m) + 1) * 300);
    return total;
}

static int get_active_days() {
    if (is_monthly_view()) {
        return 10 + (int)((cos(g_heat_month) + 1) * 8);
    }
    return 120 + (int)((cos(g_heat_year) + 1) * 30);
}

static int get_max_streak() {
    if (is_monthly_view()) {
        return 2 + (int)((sin(g_heat_month * 2) + 1) * 3);
    }
    return 8 + (int)((sin(g_heat_year) + 1) * 5);
}

static void refresh_heat_stats() {
    if (!g_heat_stats_label) return;
    char m[512];
    int total = get_total_reading_minutes();
    int active = get_active_days();
    int streak = get_max_streak();
    const char* unit = is_monthly_view() ? "本月" : "全年";
    sprintf(m,
        "<span size='13000'><b>%s统计</b></span>\n\n"
        "<span size='11000'>• 累计阅读</span>\n"
        "<span size='15000' weight='bold'>%.1f 小时</span>\n\n"
        "<span size='11000'>• 活跃天数</span>\n"
        "<span size='15000' weight='bold'>%d 天</span>\n\n"
        "<span size='11000'>• 最长连读</span>\n"
        "<span size='15000' weight='bold'>%d 天</span>",
        unit, total / 60.0, active, streak);
    gtk_label_set_markup(GTK_LABEL(g_heat_stats_label), m);
}

static void refresh_heat_canvas() {
    refresh_heat_stats();
    if (g_heat_drawing_area) gtk_widget_queue_draw(g_heat_drawing_area);
}

// ==================== Heatmap drawing (annual or monthly) ====================

static gboolean on_expose_heatmap_v2(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    cairo_t *cr = gdk_cairo_create(widget->window);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    double w = widget->allocation.width;
    double h = widget->allocation.height;

    if (is_monthly_view()) {
        // Monthly view: 7 columns x ~6 rows
        int fw = first_weekday_of_month(g_heat_year, g_heat_month);
        int dm = days_in_month(g_heat_year, g_heat_month);
        double pad_x = 30.0, pad_y = 5.0;
        double cw = (w - pad_x * 2) / 7.0;
        double ch = (h - pad_y * 2 - 18) / 6.0;

        // Day headers
        const char* dh[] = {"一","二","三","四","五","六","日"};
        cairo_set_font_size(cr, 11);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
        for (int i = 0; i < 7; i++) {
            cairo_text_extents_t ext;
            cairo_text_extents(cr, dh[i], &ext);
            cairo_move_to(cr, pad_x + i * cw + cw/2 - ext.width/2, pad_y + 12);
            cairo_show_text(cr, dh[i]);
        }

        // Day cells
        cairo_set_font_size(cr, 11);
        int row = 0, col = fw;
        for (int day = 1; day <= dm; day++) {
            double x = pad_x + col * cw;
            double y = pad_y + 20 + row * ch;

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

            char ds[4];
            sprintf(ds, "%d", day);
            cairo_set_source_rgb(cr, gc < 0.5 ? 1 : 0, gc < 0.5 ? 1 : 0, gc < 0.5 ? 1 : 0);
            cairo_text_extents_t ext;
            cairo_text_extents(cr, ds, &ext);
            cairo_move_to(cr, x + cw/2 - ext.width/2, y + ch/2 + ext.height/2);
            cairo_show_text(cr, ds);

            col++;
            if (col >= 7) { col = 0; row++; }
        }
    } else {
        // Annual view: 53 columns x 7 rows (GitHub-style)
        double pad_x = 20.0, pad_y = 5.0;
        double grid_w = w - pad_x * 2;
        double cell = (grid_w - 52 * 2.0) / 53.0;
        if (cell < 4) cell = 4;

        // Weekday labels
        const char* wd[] = {"", "一", "", "三", "", "五", ""};
        cairo_set_font_size(cr, 9);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_source_rgb(cr, 0.4, 0.4, 0.4);
        for (int row = 0; row < 7; row++) {
            if (wd[row][0]) {
                cairo_move_to(cr, 2, pad_y + row * (cell + 2) + cell - 1);
                cairo_show_text(cr, wd[row]);
            }
        }

        for (int col = 0; col < 53; col++) {
            for (int row = 0; row < 7; row++) {
                double x = pad_x + col * (cell + 2.0);
                double y = pad_y + row * (cell + 2.0);
                double seed = sin(col * 7 + row + g_heat_year);
                if (seed > 0.85) cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
                else if (seed > 0.55) cairo_set_source_rgb(cr, 0.4, 0.4, 0.4);
                else if (seed > 0.25) cairo_set_source_rgb(cr, 0.66, 0.66, 0.66);
                else if (seed > -0.15) cairo_set_source_rgb(cr, 0.86, 0.86, 0.86);
                else cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
                cairo_rectangle(cr, x, y, cell, cell);
                cairo_fill_preserve(cr);
                cairo_set_source_rgb(cr, 0.88, 0.88, 0.88);
                cairo_set_line_width(cr, 0.5);
                cairo_stroke(cr);
            }
        }
    }

    cairo_destroy(cr);
    return FALSE;
}

// ==================== Heatmap control callbacks ====================

static void on_heat_year_prev(GtkButton *b, gpointer d) {
    if (is_monthly_view()) g_heat_year--;
    else g_heat_year--;
    refresh_heat_canvas();
}

static void on_heat_year_next(GtkButton *b, gpointer d) {
    if (is_monthly_view()) g_heat_year++;
    else g_heat_year++;
    refresh_heat_canvas();
}

static void on_heat_month_prev(GtkButton *b, gpointer d) {
    if (is_monthly_view()) {
        g_heat_month--;
        if (g_heat_month < 1) { g_heat_month = 12; g_heat_year--; }
    }
    refresh_heat_canvas();
}

static void on_heat_month_next(GtkButton *b, gpointer d) {
    if (is_monthly_view()) {
        g_heat_month++;
        if (g_heat_month > 12) { g_heat_month = 1; g_heat_year++; }
    }
    refresh_heat_canvas();
}

static void on_heat_toggle_view(GtkButton *b, gpointer d) {
    GtkWidget *lbl = GTK_WIDGET(d);
    if (is_monthly_view()) {
        g_heat_month = 0; // switch to annual
        gtk_label_set_markup(GTK_LABEL(lbl), "<span size='10000'>切换为月度</span>");
    } else {
        g_heat_month = 6; // switch to monthly, default to June
        gtk_label_set_markup(GTK_LABEL(lbl), "<span size='10000'>切换为年度</span>");
    }
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
    sprintf(m_val, "<span size='20000' weight='bold'>%s</span>", val);
    GtkWidget *lbl_val = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_val), m_val);
    gtk_misc_set_alignment(GTK_MISC(lbl_val), 0.5, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), lbl_val, TRUE, TRUE, 0);

    char m_sub[128];
    sprintf(m_sub, "<span size='11000'>%s</span>", lbl);
    GtkWidget *lbl_sub = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_sub), m_sub);
    gtk_misc_set_alignment(GTK_MISC(lbl_sub), 0.5, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), lbl_sub, TRUE, TRUE, 0);

    return event_box;
}

// ==================== Build dashboard page ====================

GtkWidget* create_dashboard_page() {
    GtkWidget *vbox = gtk_vbox_new(FALSE, 8);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);

    // ===== 3 stat cards =====
    GtkWidget *stats_hbox = gtk_hbox_new(TRUE, 6);
    const char* stat_vals[] = {"128.5 h", "24 本", "46 min"};
    const char* stat_lbls[] = {"累计阅读", "已读书籍", "日均阅读"};
    for (int i = 0; i < 3; i++) {
        gtk_box_pack_start(GTK_BOX(stats_hbox), create_stat_card(stat_vals[i], stat_lbls[i]), TRUE, TRUE, 0);
    }
    gtk_box_pack_start(GTK_BOX(vbox), stats_hbox, FALSE, FALSE, 0);

    // ===== Heatmap card with [left: chart, right: stats text] =====
    GtkWidget *heat_eb = gtk_event_box_new();
    GdkColor white;
    gdk_color_parse("#ffffff", &white);
    gtk_widget_modify_bg(heat_eb, GTK_STATE_NORMAL, &white);

    GtkWidget *heat_frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(heat_frame), GTK_SHADOW_OUT);
    gtk_container_add(GTK_CONTAINER(heat_eb), heat_frame);

    GtkWidget *heat_vbox = gtk_vbox_new(FALSE, 4);
    gtk_container_set_border_width(GTK_CONTAINER(heat_vbox), 8);
    gtk_container_add(GTK_CONTAINER(heat_frame), heat_vbox);

    // Title row
    GtkWidget *heat_title_hbox = gtk_hbox_new(FALSE, 8);
    GtkWidget *heat_title_lbl = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(heat_title_lbl), "<span size='13000' weight='bold'>阅读热力图</span>");
    gtk_misc_set_alignment(GTK_MISC(heat_title_lbl), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(heat_title_hbox), heat_title_lbl, TRUE, TRUE, 0);

    // View toggle button
    GtkWidget *toggle_btn = gtk_button_new();
    GtkWidget *toggle_lbl = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(toggle_lbl), "<span size='10000'>切换为月度</span>");
    gtk_container_add(GTK_CONTAINER(toggle_btn), toggle_lbl);
    g_signal_connect(toggle_btn, "clicked", G_CALLBACK(on_heat_toggle_view), toggle_lbl);
    gtk_box_pack_end(GTK_BOX(heat_title_hbox), toggle_btn, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(heat_vbox), heat_title_hbox, FALSE, FALSE, 0);

    // Content: left chart + right stats
    GtkWidget *heat_content = gtk_hbox_new(FALSE, 12);
    gtk_box_pack_start(GTK_BOX(heat_vbox), heat_content, TRUE, TRUE, 0);

    // Left: chart with year/month controls
    GtkWidget *left_vbox = gtk_vbox_new(FALSE, 2);
    gtk_box_pack_start(GTK_BOX(heat_content), left_vbox, TRUE, TRUE, 0);

    // Year/Month controls row
    GtkWidget *ctrl_hbox = gtk_hbox_new(FALSE, 4);
    GtkWidget *yr_prev = gtk_button_new();
    GtkWidget *yr_prev_lbl = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(yr_prev_lbl), "<span size='10000'>[ &lt; ]</span>");
    gtk_container_add(GTK_CONTAINER(yr_prev), yr_prev_lbl);
    GtkWidget *yr_next = gtk_button_new();
    GtkWidget *yr_next_lbl = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(yr_next_lbl), "<span size='10000'>[ &gt; ]</span>");
    gtk_container_add(GTK_CONTAINER(yr_next), yr_next_lbl);
    GtkWidget *mo_prev = gtk_button_new();
    GtkWidget *mo_prev_lbl = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(mo_prev_lbl), "<span size='10000'>月 &lt;</span>");
    gtk_container_add(GTK_CONTAINER(mo_prev), mo_prev_lbl);
    GtkWidget *mo_next = gtk_button_new();
    GtkWidget *mo_next_lbl = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(mo_next_lbl), "<span size='10000'>> 月</span>");
    gtk_container_add(GTK_CONTAINER(mo_next), mo_next_lbl);

    g_signal_connect(yr_prev, "clicked", G_CALLBACK(on_heat_year_prev), NULL);
    g_signal_connect(yr_next, "clicked", G_CALLBACK(on_heat_year_next), NULL);
    g_signal_connect(mo_prev, "clicked", G_CALLBACK(on_heat_month_prev), NULL);
    g_signal_connect(mo_next, "clicked", G_CALLBACK(on_heat_month_next), NULL);

    gtk_box_pack_start(GTK_BOX(ctrl_hbox), yr_prev, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(ctrl_hbox), yr_next, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(ctrl_hbox), mo_prev, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(ctrl_hbox), mo_next, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(left_vbox), ctrl_hbox, FALSE, FALSE, 0);

    // Heatmap drawing area
    g_heat_drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(g_heat_drawing_area, -1, 200);
    g_signal_connect(g_heat_drawing_area, "expose-event", G_CALLBACK(on_expose_heatmap_v2), NULL);
    gtk_box_pack_start(GTK_BOX(left_vbox), g_heat_drawing_area, TRUE, TRUE, 0);

    // Right: stats text
    g_heat_stats_label = gtk_label_new(NULL);
    gtk_misc_set_alignment(GTK_MISC(g_heat_stats_label), 0.0, 0.0);
    gtk_box_pack_start(GTK_BOX(heat_content), g_heat_stats_label, FALSE, FALSE, 0);

    // Apply initial state
    g_heat_month = 0; // start in annual view
    refresh_heat_canvas();

    gtk_box_pack_start(GTK_BOX(vbox), heat_eb, FALSE, FALSE, 0);

    // ===== Gold Hour (24h bar) =====
    GtkWidget *gold_content;
    GtkWidget *gold_card = create_chart_card_compact("最爱阅读时段", "最常在 18:00 - 21:00 分布", &gold_content);
    GtkWidget *bar24_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(bar24_da, -1, 130);
    g_signal_connect(bar24_da, "expose-event", G_CALLBACK(on_expose_bar24), NULL);
    gtk_box_pack_start(GTK_BOX(gold_content), bar24_da, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), gold_card, FALSE, FALSE, 0);

    // ===== 7-day trend hbar (no "(分)" in title) =====
    GtkWidget *hbar_content;
    GtkWidget *hbar_card = create_chart_card_compact("近7日阅读趋势", NULL, &hbar_content);
    GtkWidget *hbar_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(hbar_da, -1, 150);
    g_signal_connect(hbar_da, "expose-event", G_CALLBACK(on_expose_hbar), NULL);
    gtk_box_pack_start(GTK_BOX(hbar_content), hbar_da, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbar_card, FALSE, FALSE, 0);

    return vbox;
}

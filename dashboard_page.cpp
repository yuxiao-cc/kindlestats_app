#include "shared.h"
#include "draw_callbacks.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// ==================== Global state for heatmap ====================
static GtkWidget *g_heat_drawing_area;
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

    double pad_x = 4.0, pad_y = 4.0;
    double header_h = 28.0;
    double cw = (w - pad_x * 2) / 7.0;
    double ch = (h - pad_y * 2 - header_h) / 6.0;
    if (cw < 8) cw = 8;
    if (ch < 8) ch = 8;

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

    cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
    cairo_set_line_width(cr, 1);
    cairo_move_to(cr, pad_x, pad_y + header_h - 4);
    cairo_line_to(cr, w - pad_x, pad_y + header_h - 4);
    cairo_stroke(cr);

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

// ==================== Create a stat cell (used in 2x2 grid) ====================

static GtkWidget* create_stat_cell(const char* label, const char* value) {
    GtkWidget *eb = gtk_event_box_new();
    GdkColor white;
    gdk_color_parse("#ffffff", &white);
    gtk_widget_modify_bg(eb, GTK_STATE_NORMAL, &white);

    GtkWidget *vbox = gtk_vbox_new(TRUE, 2);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
    gtk_container_add(GTK_CONTAINER(eb), vbox);

    char ml[128], mv[128];
    sprintf(ml, "<span size='10000' color='#505050'>%s</span>", label);
    sprintf(mv, "<span size='14000' weight='bold'>%s</span>", value);

    GtkWidget *lbl = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl), ml);
    gtk_misc_set_alignment(GTK_MISC(lbl), 0.5, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), lbl, TRUE, TRUE, 0);

    GtkWidget *val = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(val), mv);
    gtk_misc_set_alignment(GTK_MISC(val), 0.5, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), val, TRUE, TRUE, 0);

    return eb;
}

// ==================== Create a top stat card ====================

static GtkWidget* create_stat_card(const char* val, const char* lbl) {
    GtkWidget *event_box = gtk_event_box_new();
    GdkColor white;
    gdk_color_parse("#ffffff", &white);
    gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &white);

    GtkWidget *frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
    gtk_container_add(GTK_CONTAINER(event_box), frame);

    GtkWidget *vbox = gtk_vbox_new(FALSE, 1);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);
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

// ==================== Apply e-ink white backgrounds ====================

static void apply_eink_bg(GtkBuilder *builder, const char *id) {
    GtkWidget *w = GTK_WIDGET(gtk_builder_get_object(builder, id));
    if (w) {
        GdkColor white;
        gdk_color_parse("#ffffff", &white);
        gtk_widget_modify_bg(w, GTK_STATE_NORMAL, &white);
    }
}

// ==================== Build dashboard page ====================

GtkWidget* create_dashboard_page() {
    GtkBuilder *builder = load_ui("dashboard_page.ui");
    if (!builder) return gtk_vbox_new(FALSE, 0);

    GtkWidget *page = GTK_WIDGET(gtk_builder_get_object(builder, "dashboard_page"));
    if (!page || !GTK_IS_WIDGET(page)) { g_object_unref(builder); return gtk_vbox_new(FALSE, 0); }
    g_object_ref_sink(page);

    // Apply e-ink backgrounds
    apply_eink_bg(builder, "heat_eb");
    apply_eink_bg(builder, "yr_eb");
    apply_eink_bg(builder, "mo_eb");
    apply_eink_bg(builder, "gold_eb");
    apply_eink_bg(builder, "hbar_eb");

    // Set markup on labels
    GtkWidget *heat_title_lbl = GTK_WIDGET(gtk_builder_get_object(builder, "heat_title_lbl"));
    gtk_label_set_markup(GTK_LABEL(heat_title_lbl), "<span size='12000' weight='bold'>阅读热力图</span>");
    GtkWidget *gold_title_lbl = GTK_WIDGET(gtk_builder_get_object(builder, "gold_title_lbl"));
    gtk_label_set_markup(GTK_LABEL(gold_title_lbl), "<span size='12000' weight='bold'>最爱阅读时段</span>");
    GtkWidget *gold_sub_lbl = GTK_WIDGET(gtk_builder_get_object(builder, "gold_sub_lbl"));
    gtk_label_set_markup(GTK_LABEL(gold_sub_lbl), "<span size='10000' color='#505050'>最常在 18:00 - 21:00 分布</span>");
    GtkWidget *hbar_title_lbl = GTK_WIDGET(gtk_builder_get_object(builder, "hbar_title_lbl"));
    gtk_label_set_markup(GTK_LABEL(hbar_title_lbl), "<span size='12000' weight='bold'>近7日阅读趋势</span>");

    // Assign globals from builder
    g_heat_drawing_area = GTK_WIDGET(gtk_builder_get_object(builder, "heat_drawing_area"));
    g_heat_year_lbl = GTK_WIDGET(gtk_builder_get_object(builder, "heat_year_lbl"));
    g_heat_month_lbl = GTK_WIDGET(gtk_builder_get_object(builder, "heat_month_lbl"));

    g_object_ref(g_heat_drawing_area);
    g_object_ref(g_heat_year_lbl);
    g_object_ref(g_heat_month_lbl);

    // Connect expose-event callbacks
    g_signal_connect(g_heat_drawing_area, "expose-event",
                     G_CALLBACK(on_expose_heatmap_monthly), NULL);
    GtkWidget *bar24_da = GTK_WIDGET(gtk_builder_get_object(builder, "bar24_da"));
    g_signal_connect(bar24_da, "expose-event", G_CALLBACK(on_expose_bar24), NULL);
    GtkWidget *hbar_da = GTK_WIDGET(gtk_builder_get_object(builder, "hbar_da"));
    g_signal_connect(hbar_da, "expose-event", G_CALLBACK(on_expose_hbar), NULL);

    // Connect year/month navigation buttons
    GtkWidget *yr_prev = GTK_WIDGET(gtk_builder_get_object(builder, "yr_prev"));
    GtkWidget *yr_next = GTK_WIDGET(gtk_builder_get_object(builder, "yr_next"));
    GtkWidget *mo_prev = GTK_WIDGET(gtk_builder_get_object(builder, "mo_prev"));
    GtkWidget *mo_next = GTK_WIDGET(gtk_builder_get_object(builder, "mo_next"));
    g_signal_connect(yr_prev, "clicked", G_CALLBACK(on_heat_year_prev), NULL);
    g_signal_connect(yr_next, "clicked", G_CALLBACK(on_heat_year_next), NULL);
    g_signal_connect(mo_prev, "clicked", G_CALLBACK(on_heat_month_prev), NULL);
    g_signal_connect(mo_next, "clicked", G_CALLBACK(on_heat_month_next), NULL);

    // 3 top stat cards (dynamic, created in code)
    GtkWidget *stats_hbox = GTK_WIDGET(gtk_builder_get_object(builder, "stats_hbox"));
    const char* stat_vals[] = {"128.5 h", "24 本", "46 min"};
    const char* stat_lbls[] = {"累计阅读", "已读书籍", "日均阅读"};
    for (int i = 0; i < 3; i++) {
        gtk_box_pack_start(GTK_BOX(stats_hbox), create_stat_card(stat_vals[i], stat_lbls[i]), TRUE, TRUE, 0);
    }

    // 2x2 stats grid (dynamic values)
    GtkWidget *stats_grid = GTK_WIDGET(gtk_builder_get_object(builder, "heat_stats_grid"));
    char val_total[32], val_active[32], val_streak[32], val_avg[32];
    sprintf(val_total, "%.1f 小时", get_total_reading_minutes() / 60.0);
    sprintf(val_active, "%d 天", get_active_days());
    sprintf(val_streak, "%d 天", get_max_streak());
    sprintf(val_avg, "%d 分钟", get_avg_daily_min());

    GtkWidget *c1 = create_stat_cell("累计阅读", val_total);
    GtkWidget *c2 = create_stat_cell("活跃天数", val_active);
    GtkWidget *c3 = create_stat_cell("最长连读", val_streak);
    GtkWidget *c4 = create_stat_cell("日均阅读", val_avg);
    gtk_table_attach_defaults(GTK_TABLE(stats_grid), c1, 0, 1, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(stats_grid), c2, 1, 2, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(stats_grid), c3, 0, 1, 1, 2);
    gtk_table_attach_defaults(GTK_TABLE(stats_grid), c4, 1, 2, 1, 2);

    refresh_heat_canvas();
    g_object_unref(builder);
    return page;
}
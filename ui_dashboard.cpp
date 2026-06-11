#include "ui_dashboard.h"
#include "utils.h"
#include <cairo.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

static int current_month_for_modal = 1;

// ------------------- Monthly Heatmap Dialog -------------------

static gboolean on_expose_monthly_heatmap(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    cairo_t *cr = gdk_cairo_create(widget->window);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    
    double pad_x = 5.0, pad_y = 5.0;
    double cell_size = 18.0;
    double gap = 3.0;
    
    // Draw day headers (Mon - Sun)
    const char* days[] = {"一", "二", "三", "四", "五", "六", "日"};
    cairo_set_font_size(cr, 10.0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
    for (int i = 0; i < 7; ++i) {
        cairo_text_extents_t extents;
        cairo_text_extents(cr, days[i], &extents);
        double x = pad_x + i * (cell_size + gap);
        cairo_move_to(cr, x + cell_size/2.0 - extents.width/2.0, pad_y + 10.0);
        cairo_show_text(cr, days[i]);
    }
    
    double grid_y_start = pad_y + 15.0;
    
    // Mock days layout logic
    int first_day_of_week = (current_month_for_modal + 2) % 7; // Mock
    int days_in_month = 30;
    if (current_month_for_modal == 2) days_in_month = 28;
    else if (current_month_for_modal == 1 || current_month_for_modal == 3 || current_month_for_modal == 5 || current_month_for_modal == 7 || current_month_for_modal == 8 || current_month_for_modal == 10 || current_month_for_modal == 12) days_in_month = 31;
    
    for (int day = 1; day <= days_in_month; ++day) {
        int pos = first_day_of_week + day - 1;
        int col = pos % 7;
        int row = pos / 7;
        
        double x = pad_x + col * (cell_size + gap);
        double y = grid_y_start + row * (cell_size + gap);
        
        double seed = sin(day + current_month_for_modal + 2026.0);
        if (seed > 0.7) cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        else if (seed > 0.4) cairo_set_source_rgb(cr, 0.4, 0.4, 0.4);
        else if (seed > 0.1) cairo_set_source_rgb(cr, 0.66, 0.66, 0.66);
        else if (seed > -0.3) cairo_set_source_rgb(cr, 0.86, 0.86, 0.86);
        else cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
        
        cairo_rectangle(cr, x, y, cell_size, cell_size);
        cairo_fill_preserve(cr);
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_set_line_width(cr, 1.0);
        cairo_stroke(cr);
    }
    
    cairo_destroy(cr);
    return FALSE;
}

static void on_modal_close(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = GTK_WIDGET(data);
    gtk_widget_destroy(dialog);
    force_eink_refresh();
}

static void show_monthly_dialog(int month) {
    current_month_for_modal = month;
    
    GtkWidget *dialog = gtk_dialog_new();
    gtk_window_set_decorated(GTK_WINDOW(dialog), FALSE);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    
    GdkColor white, black;
    gdk_color_parse("#ffffff", &white);
    gdk_color_parse("#000000", &black);
    gtk_widget_modify_bg(dialog, GTK_STATE_NORMAL, &white);
    
    // Create a 2px black border around the dialog content
    GtkWidget *frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
    
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_add(GTK_CONTAINER(content_area), frame);
    
    GtkWidget *main_vbox = gtk_vbox_new(FALSE, 10);
    gtk_container_set_border_width(GTK_CONTAINER(main_vbox), 20);
    gtk_container_add(GTK_CONTAINER(frame), main_vbox);
    
    // Title
    char title_buf[64];
    sprintf(title_buf, "<span size='16000' weight='bold'>2026年 %d月 阅读概况</span>", month);
    GtkWidget *title_lbl = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title_lbl), title_buf);
    gtk_box_pack_start(GTK_BOX(main_vbox), title_lbl, FALSE, FALSE, 0);
    
    // Horizontal Layout for Grid and Stats
    GtkWidget *split_hbox = gtk_hbox_new(FALSE, 20);
    
    // Left: Monthly Heatmap Grid
    GtkWidget *monthly_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(monthly_da, 160, 140);
    g_signal_connect(monthly_da, "expose-event", G_CALLBACK(on_expose_monthly_heatmap), NULL);
    gtk_box_pack_start(GTK_BOX(split_hbox), monthly_da, FALSE, FALSE, 0);
    
    // Right: Stats
    GtkWidget *stats_vbox = gtk_vbox_new(FALSE, 8);
    gtk_box_pack_start(GTK_BOX(split_hbox), stats_vbox, TRUE, TRUE, 0);
    
    int total_h = 6 + (month % 3) * 2;
    int active_d = 10 + month;
    int max_s = 2 + month % 4;
    
    char stat_buf[256];
    
    sprintf(stat_buf, "<span size='12000' weight='bold'>本月统计汇总:</span>");
    GtkWidget *s1 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(s1), stat_buf);
    gtk_misc_set_alignment(GTK_MISC(s1), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(stats_vbox), s1, FALSE, FALSE, 0);
    
    sprintf(stat_buf, "<span size='12000'>• 累计阅读: <b>%d 小时</b></span>", total_h);
    GtkWidget *s2 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(s2), stat_buf);
    gtk_misc_set_alignment(GTK_MISC(s2), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(stats_vbox), s2, FALSE, FALSE, 0);
    
    sprintf(stat_buf, "<span size='12000'>• 活跃天数: <b>%d 天</b> / 30天</span>", active_d);
    GtkWidget *s3 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(s3), stat_buf);
    gtk_misc_set_alignment(GTK_MISC(s3), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(stats_vbox), s3, FALSE, FALSE, 0);
    
    sprintf(stat_buf, "<span size='12000'>• 最长连读: <b>%d 天</b></span>", max_s);
    GtkWidget *s4 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(s4), stat_buf);
    gtk_misc_set_alignment(GTK_MISC(s4), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(stats_vbox), s4, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(main_vbox), split_hbox, TRUE, TRUE, 10);
    
    // Close Button
    GtkWidget *close_btn = gtk_button_new();
    GtkWidget *close_lbl = gtk_label_new("<span size='13000' weight='bold'>关闭 [X]</span>");
    gtk_label_set_markup(GTK_LABEL(close_lbl), "<span size='13000' weight='bold'>关闭 [X]</span>");
    gtk_container_add(GTK_CONTAINER(close_btn), close_lbl);
    g_signal_connect(close_btn, "clicked", G_CALLBACK(on_modal_close), dialog);
    
    GtkWidget *close_hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_end(GTK_BOX(close_hbox), close_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), close_hbox, FALSE, FALSE, 0);
    
    gtk_widget_show_all(dialog);
    
    force_eink_refresh();
    gtk_dialog_run(GTK_DIALOG(dialog));
}

static gboolean on_month_label_press(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    int month = GPOINTER_TO_INT(data);
    show_monthly_dialog(month);
    return TRUE;
}

// ------------------- Annual Heatmap -------------------

static gboolean on_expose_heatmap(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    cairo_t *cr = gdk_cairo_create(widget->window);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    
    double pad_x = 0.0;
    double pad_y = 5.0;
    
    double grid_y = pad_y;
    double grid_width = widget->allocation.width - pad_x * 2;
    double cell_size = (grid_width - 52 * 2.0) / 53.0;
    
    for (int col = 0; col < 53; ++col) {
        for (int row = 0; row < 7; ++row) {
            double x = pad_x + col * (cell_size + 2.0);
            double y = grid_y + row * (cell_size + 2.0);
            double seed = sin(col * 7 + row + 2026.0);
            if (seed > 0.85) cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
            else if (seed > 0.55) cairo_set_source_rgb(cr, 0.4, 0.4, 0.4);
            else if (seed > 0.25) cairo_set_source_rgb(cr, 0.66, 0.66, 0.66);
            else if (seed > -0.15) cairo_set_source_rgb(cr, 0.86, 0.86, 0.86);
            else cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
            
            cairo_rectangle(cr, x, y, cell_size, cell_size);
            cairo_fill_preserve(cr);
            cairo_set_source_rgb(cr, 0.88, 0.88, 0.88);
            cairo_set_line_width(cr, 1.0);
            cairo_stroke(cr);
        }
    }
    
    cairo_destroy(cr);
    return FALSE;
}

// ------------------- 24h Bar Chart -------------------

static gboolean on_expose_bar24(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    cairo_t *cr = gdk_cairo_create(widget->window);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    
    double pad_x = 10.0, pad_y = 5.0;
    double area_w = widget->allocation.width - pad_x * 2;
    double area_h = widget->allocation.height - pad_y * 2 - 20.0;
    double base_y = widget->allocation.height - pad_y - 15.0;
    
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_set_line_width(cr, 2.0);
    cairo_move_to(cr, pad_x, base_y);
    cairo_line_to(cr, widget->allocation.width - pad_x, base_y);
    cairo_stroke(cr);
    
    double mock_data[24] = {5, 2, 0, 0, 0, 0, 5, 15, 30, 20, 10, 15, 25, 40, 20, 10, 15, 25, 60, 80, 95, 85, 40, 15};
    double bar_w = (area_w - 23 * 2.0) / 24.0;
    
    cairo_set_font_size(cr, 11.0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    
    for (int i = 0; i < 24; ++i) {
        double x = pad_x + i * (bar_w + 2.0);
        double h = (mock_data[i] / 100.0) * area_h;
        double y = base_y - h;
        
        cairo_rectangle(cr, x, y, bar_w, h);
        cairo_fill(cr);
        
        if (i % 4 == 0 || i == 23) {
            char lbl[8];
            sprintf(lbl, "%d", i);
            cairo_text_extents_t extents;
            cairo_text_extents(cr, lbl, &extents);
            cairo_move_to(cr, x + bar_w/2.0 - extents.width/2.0, base_y + 12.0);
            cairo_show_text(cr, lbl);
        }
    }
    
    cairo_destroy(cr);
    return FALSE;
}

// ------------------- 7-Day Trend HBar Chart -------------------

static gboolean on_expose_hbar(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    cairo_t *cr = gdk_cairo_create(widget->window);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    
    const char* days[] = {"周五", "周六", "周日", "周一", "周二", "周三", "今日"};
    double values[] = {45, 12, 90, 35, 50, 70, 80};
    double max_val = 100.0;
    
    double start_y = 5.0;
    double row_h = (widget->allocation.height - start_y - 5.0) / 7.0;
    
    cairo_set_font_size(cr, 12.0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    
    for (int i = 0; i < 7; ++i) {
        double y = start_y + i * row_h;
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_move_to(cr, 15, y + row_h/2.0 + 4.0);
        cairo_show_text(cr, days[i]);
        
        double track_x = 65.0;
        double track_w = widget->allocation.width - 130.0;
        double track_h = 10.0;
        double track_y = y + (row_h - track_h)/2.0;
        
        double fill_w = track_w * (values[i] / max_val);
        if (i % 2 == 0) cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
        else cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        
        cairo_rectangle(cr, track_x, track_y, fill_w, track_h);
        cairo_fill_preserve(cr);
        
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_set_line_width(cr, 1.0);
        cairo_stroke(cr);
        
        char val_str[16];
        sprintf(val_str, "%.0fm", values[i]);
        cairo_move_to(cr, track_x + track_w + 10.0, y + row_h/2.0 + 4.0);
        cairo_show_text(cr, val_str);
    }
    
    cairo_destroy(cr);
    return FALSE;
}

// ------------------- Main Dashboard Page Constructor -------------------

GtkWidget* create_dashboard_page() {
    GtkWidget *dashboard_vbox = gtk_vbox_new(FALSE, 8);
    gtk_container_set_border_width(GTK_CONTAINER(dashboard_vbox), 4);
    
    // Top 3 Stats
    GtkWidget *stats_hbox = gtk_hbox_new(TRUE, 8);
    const char* stat_vals[] = {"128.5 h", "24 本", "46 min"};
    const char* stat_lbls[] = {"累计阅读", "已读书籍", "日均阅读"};
    for(int i=0; i<3; ++i) {
        GtkWidget *vbox = gtk_vbox_new(FALSE, 2);
        gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
        
        char m_val[128];
        sprintf(m_val, "<span size='20000' weight='bold'>%s</span>", stat_vals[i]);
        GtkWidget *lbl_val = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(lbl_val), m_val);
        gtk_misc_set_alignment(GTK_MISC(lbl_val), 0.5, 0.5);
        gtk_box_pack_start(GTK_BOX(vbox), lbl_val, TRUE, TRUE, 0);

        char m_sub[128];
        sprintf(m_sub, "<span size='11000' color='#505050'>%s</span>", stat_lbls[i]);
        GtkWidget *lbl_sub = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(lbl_sub), m_sub);
        gtk_misc_set_alignment(GTK_MISC(lbl_sub), 0.5, 0.5);
        gtk_box_pack_start(GTK_BOX(vbox), lbl_sub, TRUE, TRUE, 0);
        
        gtk_box_pack_start(GTK_BOX(stats_hbox), vbox, TRUE, TRUE, 0);
    }
    gtk_box_pack_start(GTK_BOX(dashboard_vbox), stats_hbox, FALSE, FALSE, 4);

    // Heatmap Module
    GtkWidget *heat_content;
    GtkWidget *heat_card = create_chart_card("年度阅读热力图", NULL, &heat_content);
    
    // Month Buttons Header
    GtkWidget *months_hbox = gtk_hbox_new(TRUE, 2);
    for (int i = 1; i <= 12; ++i) {
        GtkWidget *evt_box = gtk_event_box_new();
        GdkColor white; gdk_color_parse("#ffffff", &white);
        gtk_widget_modify_bg(evt_box, GTK_STATE_NORMAL, &white);
        
        char lbl_buf[32];
        sprintf(lbl_buf, "<span size='11000' underline='single'>%d月</span>", i);
        GtkWidget *lbl = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(lbl), lbl_buf);
        gtk_container_add(GTK_CONTAINER(evt_box), lbl);
        
        g_signal_connect(evt_box, "button-press-event", G_CALLBACK(on_month_label_press), GINT_TO_POINTER(i));
        gtk_box_pack_start(GTK_BOX(months_hbox), evt_box, TRUE, TRUE, 0);
    }
    gtk_box_pack_start(GTK_BOX(heat_content), months_hbox, FALSE, FALSE, 4);

    GtkWidget *heatmap_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(heatmap_da, -1, 140); 
    g_signal_connect(heatmap_da, "expose-event", G_CALLBACK(on_expose_heatmap), NULL);
    gtk_box_pack_start(GTK_BOX(heat_content), heatmap_da, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(dashboard_vbox), heat_card, FALSE, FALSE, 0);

    // Gold Hour Module
    GtkWidget *gold_content;
    GtkWidget *gold_card = create_chart_card("最爱阅读时段", "最常在 18:00 - 21:00 分布", &gold_content);
    GtkWidget *bar24_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(bar24_da, -1, 130);
    g_signal_connect(bar24_da, "expose-event", G_CALLBACK(on_expose_bar24), NULL);
    gtk_box_pack_start(GTK_BOX(gold_content), bar24_da, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(dashboard_vbox), gold_card, FALSE, FALSE, 0);
    
    // 7-Day Trend Module
    GtkWidget *hbar_content;
    GtkWidget *hbar_card = create_chart_card("近7日阅读趋势", NULL, &hbar_content);
    GtkWidget *hbar_da_widget = gtk_drawing_area_new();
    gtk_widget_set_size_request(hbar_da_widget, -1, 160);
    g_signal_connect(hbar_da_widget, "expose-event", G_CALLBACK(on_expose_hbar), NULL);
    gtk_box_pack_start(GTK_BOX(hbar_content), hbar_da_widget, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(dashboard_vbox), hbar_card, FALSE, FALSE, 0);

    return dashboard_vbox;
}

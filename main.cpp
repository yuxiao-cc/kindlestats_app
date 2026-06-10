#include <gtk/gtk.h>
#include <cairo.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

// 状态变量
static int heatmap_mode = 0; // 0: 年度, 1: 月度
static int goldhour_mode = 0; // 0: 雷达图, 1: 24小时柱状图

// 控件引用
static GtkWidget *heatmap_da;
static GtkWidget *radar_da;
static GtkWidget *bar24_da;
static GtkWidget *goldhour_btn_lbl;
static GtkWidget *heatmap_btn_lbl;

// 切换热力图回调
static void on_toggle_heatmap(GtkWidget *widget, gpointer data) {
    heatmap_mode = 1 - heatmap_mode;
    if (heatmap_mode == 0) {
        gtk_label_set_markup(GTK_LABEL(heatmap_btn_lbl), "<span size='16000'>年度</span>");
    } else {
        gtk_label_set_markup(GTK_LABEL(heatmap_btn_lbl), "<span size='16000'>月度</span>");
    }
    gtk_widget_queue_draw(heatmap_da);
    system("eips -c");
}

// 切换最爱阅读时段回调
static void on_toggle_goldhour(GtkWidget *widget, gpointer data) {
    goldhour_mode = 1 - goldhour_mode;
    if (goldhour_mode == 0) {
        gtk_widget_hide(bar24_da);
        gtk_widget_show(radar_da);
        gtk_label_set_markup(GTK_LABEL(goldhour_btn_lbl), "<span size='16000'>雷达图</span>");
    } else {
        gtk_widget_hide(radar_da);
        gtk_widget_show(bar24_da);
        gtk_label_set_markup(GTK_LABEL(goldhour_btn_lbl), "<span size='16000'>24h 柱状</span>");
    }
    system("eips -c");
}

// 绘制热力图的回调函数
static gboolean on_expose_heatmap(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    cairo_t *cr = gdk_cairo_create(widget->window);
    
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr); // 白底，不画黑框
    
    double pad_x = 0.0;
    double pad_y = 5.0;
    
    if (heatmap_mode == 0) {
        // 年度
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
    } else {
        // 月度模拟
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 28.0);
        cairo_move_to(cr, widget->allocation.width / 2.0 - 100, widget->allocation.height / 2.0);
        cairo_show_text(cr, "[ 月度热力图展示区 ]");
    }
    
    cairo_destroy(cr);
    return FALSE;
}

// 绘制雷达图
static gboolean on_expose_radar(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    cairo_t *cr = gdk_cairo_create(widget->window);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    
    double cx = widget->allocation.width / 2.0;
    double cy = widget->allocation.height / 2.0;
    double r_max = 110.0;
    
    double angles[8];
    for (int i = 0; i < 8; ++i) {
        angles[i] = -M_PI / 2.0 + i * (M_PI / 4.0);
    }
    
    for (int ring = 1; ring <= 3; ++ring) {
        double r = r_max * (ring / 3.0);
        cairo_set_source_rgb(cr, 0.8 - (ring-1)*0.1, 0.8 - (ring-1)*0.1, 0.8 - (ring-1)*0.1);
        cairo_set_line_width(cr, ring == 3 ? 1.5 : 1.0);
        for (int i = 0; i < 8; ++i) {
            double x = cx + r * cos(angles[i]);
            double y = cy + r * sin(angles[i]);
            if (i == 0) cairo_move_to(cr, x, y);
            else cairo_line_to(cr, x, y);
        }
        cairo_close_path(cr);
        cairo_stroke(cr);
    }
    
    cairo_set_source_rgb(cr, 0.6, 0.6, 0.6);
    cairo_set_line_width(cr, 0.75);
    double dashes[] = {2.0, 2.0};
    cairo_set_dash(cr, dashes, 2, 0);
    for (int i = 0; i < 4; ++i) {
        cairo_move_to(cr, cx + r_max * cos(angles[i]), cy + r_max * sin(angles[i]));
        cairo_line_to(cr, cx + r_max * cos(angles[i+4]), cy + r_max * sin(angles[i+4]));
    }
    cairo_stroke(cr);
    cairo_set_dash(cr, NULL, 0, 0);
    
    double data_r[8] = {85, 45, 55, 35, 55, 95, 105, 75}; 
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.25);
    for (int i = 0; i < 8; ++i) {
        double x = cx + data_r[i] * cos(angles[i]);
        double y = cy + data_r[i] * sin(angles[i]);
        if (i == 0) cairo_move_to(cr, x, y);
        else cairo_line_to(cr, x, y);
    }
    cairo_close_path(cr);
    cairo_fill_preserve(cr);
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_set_line_width(cr, 2.0);
    cairo_stroke(cr);
    
    const char* labels[] = {"0:00", "3:00", "6:00", "9:00", "12:00", "15:00", "18:00", "21:00"};
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_set_font_size(cr, 20.0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    for (int i = 0; i < 8; ++i) {
        double offset_r = r_max + 25.0;
        double x = cx + offset_r * cos(angles[i]);
        double y = cy + offset_r * sin(angles[i]);
        cairo_text_extents_t extents;
        cairo_text_extents(cr, labels[i], &extents);
        cairo_move_to(cr, x - extents.width/2.0, y + extents.height/2.0);
        cairo_show_text(cr, labels[i]);
    }
    
    cairo_destroy(cr);
    return FALSE;
}

// 绘制 24h 柱状图
static gboolean on_expose_bar24(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    cairo_t *cr = gdk_cairo_create(widget->window);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    
    double pad_x = 10.0;
    double pad_y = 10.0;
    double area_w = widget->allocation.width - pad_x * 2;
    double area_h = widget->allocation.height - pad_y * 2 - 30.0;
    double base_y = widget->allocation.height - pad_y - 20.0;
    
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_set_line_width(cr, 2.0);
    cairo_move_to(cr, pad_x, base_y);
    cairo_line_to(cr, widget->allocation.width - pad_x, base_y);
    cairo_stroke(cr);
    
    double mock_data[24] = {5, 2, 0, 0, 0, 0, 5, 15, 30, 20, 10, 15, 25, 40, 20, 10, 15, 25, 60, 80, 95, 85, 40, 15};
    double bar_w = (area_w - 23 * 2.0) / 24.0;
    
    cairo_set_font_size(cr, 16.0);
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
            cairo_move_to(cr, x + bar_w/2.0 - extents.width/2.0, base_y + 18.0);
            cairo_show_text(cr, lbl);
        }
    }
    
    cairo_destroy(cr);
    return FALSE;
}

// 绘制横向柱状图
static gboolean on_expose_hbar(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    cairo_t *cr = gdk_cairo_create(widget->window);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    
    const char* days[] = {"周五", "周六", "周日", "周一", "周二", "周三", "今日"};
    double values[] = {45, 12, 90, 35, 50, 70, 80};
    double max_val = 100.0;
    
    double start_y = 15.0;
    double row_h = (widget->allocation.height - start_y - 15.0) / 7.0;
    
    cairo_set_font_size(cr, 20.0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    
    for (int i = 0; i < 7; ++i) {
        double y = start_y + i * row_h;
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_move_to(cr, 15, y + row_h/2.0 + 6.0);
        cairo_show_text(cr, days[i]);
        
        double track_x = 85.0;
        double track_w = widget->allocation.width - 170.0;
        double track_h = 16.0;
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
        cairo_move_to(cr, track_x + track_w + 15.0, y + row_h/2.0 + 6.0);
        cairo_show_text(cr, val_str);
    }
    
    cairo_destroy(cr);
    return FALSE;
}

// 封装创建图表卡片
GtkWidget* create_chart_card(const char* title, const char* subtitle, const char* btn_label, GtkWidget** content_area, GtkWidget** btn_lbl_ptr, GCallback toggle_cb) {
    GtkWidget *event_box = gtk_event_box_new();
    GdkColor white;
    gdk_color_parse("#ffffff", &white);
    gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &white);
    
    GtkWidget *frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
    gtk_container_add(GTK_CONTAINER(event_box), frame);
    
    GtkWidget *vbox = gtk_vbox_new(FALSE, 8);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 12);
    gtk_container_add(GTK_CONTAINER(frame), vbox);
    
    GtkWidget *header_hbox = gtk_hbox_new(FALSE, 10);
    GtkWidget *title_vbox = gtk_vbox_new(FALSE, 2);
    char markup[256];
    sprintf(markup, "<span size='26000' weight='bold'>%s</span>", title);
    GtkWidget *lbl_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_title), markup);
    gtk_misc_set_alignment(GTK_MISC(lbl_title), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(title_vbox), lbl_title, FALSE, FALSE, 0);
    
    if (subtitle) {
        sprintf(markup, "<span size='18000' foreground='#505050'>%s</span>", subtitle);
        GtkWidget *lbl_sub = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(lbl_sub), markup);
        gtk_misc_set_alignment(GTK_MISC(lbl_sub), 0.0, 0.5);
        gtk_box_pack_start(GTK_BOX(title_vbox), lbl_sub, FALSE, FALSE, 0);
    }
    gtk_box_pack_start(GTK_BOX(header_hbox), title_vbox, TRUE, TRUE, 0);
    
    if (btn_label) {
        GtkWidget *btn = gtk_button_new();
        *btn_lbl_ptr = gtk_label_new(NULL);
        sprintf(markup, "<span size='16000'>%s</span>", btn_label);
        gtk_label_set_markup(GTK_LABEL(*btn_lbl_ptr), markup);
        gtk_container_add(GTK_CONTAINER(btn), *btn_lbl_ptr);
        g_signal_connect(btn, "clicked", toggle_cb, NULL);
        gtk_box_pack_end(GTK_BOX(header_hbox), btn, FALSE, FALSE, 0);
    }
    
    gtk_box_pack_start(GTK_BOX(vbox), header_hbox, FALSE, FALSE, 0);
    
    *content_area = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), *content_area, TRUE, TRUE, 0);
    
    return event_box;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "L:A_N:application_PC:N_ID:kindlestats");
    gtk_window_fullscreen(GTK_WINDOW(window));
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *main_vbox = gtk_vbox_new(FALSE, 10);
    gtk_container_set_border_width(GTK_CONTAINER(main_vbox), 20);
    gtk_container_add(GTK_CONTAINER(window), main_vbox);

    GtkWidget *header_hbox = gtk_hbox_new(FALSE, 0);
    GtkWidget *title_label = gtk_label_new("<span size='20000' weight='bold'>KindleStats</span>");
    gtk_label_set_use_markup(GTK_LABEL(title_label), TRUE);
    
    GtkWidget *exit_btn = gtk_button_new();
    GtkWidget *exit_lbl = gtk_label_new("<span size='16000'>退出 [X]</span>");
    gtk_label_set_use_markup(GTK_LABEL(exit_lbl), TRUE);
    gtk_container_add(GTK_CONTAINER(exit_btn), exit_lbl);
    g_signal_connect(exit_btn, "clicked", G_CALLBACK(gtk_main_quit), NULL);
    
    gtk_box_pack_start(GTK_BOX(header_hbox), title_label, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(header_hbox), exit_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), header_hbox, FALSE, FALSE, 5);

    GtkWidget *tab_hbox = gtk_hbox_new(TRUE, 8);
    GtkWidget *tab1 = gtk_button_new();
    GtkWidget *lbl1 = gtk_label_new("<span size='20000' weight='bold'>数据概览</span>");
    gtk_label_set_use_markup(GTK_LABEL(lbl1), TRUE);
    gtk_container_add(GTK_CONTAINER(tab1), lbl1);
    
    GtkWidget *tab2 = gtk_button_new();
    GtkWidget *lbl2 = gtk_label_new("<span size='20000'>我的书籍</span>");
    gtk_label_set_use_markup(GTK_LABEL(lbl2), TRUE);
    gtk_container_add(GTK_CONTAINER(tab2), lbl2);
    
    GtkWidget *tab3 = gtk_button_new();
    GtkWidget *lbl3 = gtk_label_new("<span size='20000'>今日阅读</span>");
    gtk_label_set_use_markup(GTK_LABEL(lbl3), TRUE);
    gtk_container_add(GTK_CONTAINER(tab3), lbl3);
    
    gtk_box_pack_start(GTK_BOX(tab_hbox), tab1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(tab_hbox), tab2, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(tab_hbox), tab3, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), tab_hbox, FALSE, FALSE, 10);

    GtkWidget *content_vbox = gtk_vbox_new(FALSE, 16);
    gtk_box_pack_start(GTK_BOX(main_vbox), content_vbox, TRUE, TRUE, 0);

    // 三大统计卡片
    GtkWidget *stats_hbox = gtk_hbox_new(TRUE, 10);
    const char* stat_vals[] = {"128.5 h", "24 本", "46 min"};
    const char* stat_lbls[] = {"累计阅读", "已读书籍", "日均阅读"};
    for(int i=0; i<3; ++i) {
        GtkWidget *event_box = gtk_event_box_new();
        GdkColor white;
        gdk_color_parse("#ffffff", &white);
        gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &white);
        
        GtkWidget *frame = gtk_frame_new(NULL);
        gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
        gtk_container_add(GTK_CONTAINER(event_box), frame);
        
        GtkWidget *vbox = gtk_vbox_new(FALSE, 2);
        gtk_container_set_border_width(GTK_CONTAINER(vbox), 12);
        
        char markup[256];
        sprintf(markup, "<span size='36000' weight='bold'>%s</span>\n<span size='20000' foreground='#505050'>%s</span>", stat_vals[i], stat_lbls[i]);
        GtkWidget *lbl = gtk_label_new(NULL);
        gtk_label_set_use_markup(GTK_LABEL(lbl), TRUE);
        gtk_label_set_justify(GTK_LABEL(lbl), GTK_JUSTIFY_CENTER);
        
        gtk_box_pack_start(GTK_BOX(vbox), lbl, TRUE, TRUE, 0);
        gtk_container_add(GTK_CONTAINER(frame), vbox);
        gtk_box_pack_start(GTK_BOX(stats_hbox), event_box, TRUE, TRUE, 0);
    }
    gtk_box_pack_start(GTK_BOX(content_vbox), stats_hbox, FALSE, FALSE, 0);

    // 热力图模块
    GtkWidget *heat_content;
    GtkWidget *heat_card = create_chart_card("2026年阅读热力图", NULL, "年度", &heat_content, &heatmap_btn_lbl, G_CALLBACK(on_toggle_heatmap));
    heatmap_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(heatmap_da, -1, 160); 
    g_signal_connect(heatmap_da, "expose-event", G_CALLBACK(on_expose_heatmap), NULL);
    gtk_box_pack_start(GTK_BOX(heat_content), heatmap_da, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content_vbox), heat_card, FALSE, FALSE, 0);

    // 最爱阅读时段模块
    GtkWidget *gold_content;
    GtkWidget *gold_card = create_chart_card("最爱阅读时段", "最常在 18:00 - 21:00 分布", "雷达图", &gold_content, &goldhour_btn_lbl, G_CALLBACK(on_toggle_goldhour));
    
    radar_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(radar_da, -1, 260);
    g_signal_connect(radar_da, "expose-event", G_CALLBACK(on_expose_radar), NULL);
    
    bar24_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(bar24_da, -1, 260);
    g_signal_connect(bar24_da, "expose-event", G_CALLBACK(on_expose_bar24), NULL);
    
    gtk_box_pack_start(GTK_BOX(gold_content), radar_da, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(gold_content), bar24_da, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content_vbox), gold_card, FALSE, FALSE, 0);
    
    // 近7日阅读趋势模块
    GtkWidget *hbar_content;
    GtkWidget *hbar_card = create_chart_card("近7日阅读趋势 (分)", NULL, NULL, &hbar_content, NULL, NULL);
    GtkWidget *hbar_da_widget = gtk_drawing_area_new();
    gtk_widget_set_size_request(hbar_da_widget, -1, 220);
    g_signal_connect(hbar_da_widget, "expose-event", G_CALLBACK(on_expose_hbar), NULL);
    gtk_box_pack_start(GTK_BOX(hbar_content), hbar_da_widget, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content_vbox), hbar_card, FALSE, FALSE, 0);

    GdkColor bg_color;
    gdk_color_parse("#f5f4ef", &bg_color);
    gtk_widget_modify_bg(window, GTK_STATE_NORMAL, &bg_color);

    gtk_widget_show_all(window);
    
    // 隐藏 24h 柱状图
    if (goldhour_mode == 0) gtk_widget_hide(bar24_da);
    
    system("eips -c");
    gtk_main();
    return 0;
}

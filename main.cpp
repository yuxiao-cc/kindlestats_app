#include <gtk/gtk.h>
#include <cairo.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

// 全局控件引用
static GtkWidget *main_window;
static GtkWidget *heatmap_da;
static GtkWidget *bar24_da;

// 绘制热力图的回调函数
static gboolean on_expose_heatmap(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    cairo_t *cr = gdk_cairo_create(widget->window);
    
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr); // 白底，不画黑框
    
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

// 绘制 24h 柱状图
static gboolean on_expose_bar24(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    cairo_t *cr = gdk_cairo_create(widget->window);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    
    double pad_x = 10.0;
    double pad_y = 5.0;
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
    
    cairo_set_font_size(cr, 11.0); // 调小字号
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

// 绘制横向柱状图
static gboolean on_expose_hbar(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    cairo_t *cr = gdk_cairo_create(widget->window);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    
    const char* days[] = {"周五", "周六", "周日", "周一", "周二", "周三", "今日"};
    double values[] = {45, 12, 90, 35, 50, 70, 80};
    double max_val = 100.0;
    
    double start_y = 5.0;
    double row_h = (widget->allocation.height - start_y - 5.0) / 7.0;
    
    cairo_set_font_size(cr, 12.0); // 调小字号
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

// 封装创建图表卡片（去除厚重边框和阴影，使用顶部细线分隔）
GtkWidget* create_chart_card(const char* title, const char* subtitle, GtkWidget** content_area) {
    GtkWidget *vbox = gtk_vbox_new(FALSE, 6);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
    
    // 顶部添加细线分隔符
    GtkWidget *separator = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 4);
    
    GtkWidget *header_hbox = gtk_hbox_new(FALSE, 10);
    GtkWidget *title_vbox = gtk_vbox_new(FALSE, 2);
    char markup[256];
    
    // 字体缩小至 15000
    sprintf(markup, "<span size='15000' weight='bold'>%s</span>", title); 
    GtkWidget *lbl_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_title), markup);
    gtk_misc_set_alignment(GTK_MISC(lbl_title), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(title_vbox), lbl_title, FALSE, FALSE, 0);
    
    if (subtitle) {
        // 副标题缩小至 11000
        sprintf(markup, "<span size='11000' color='#505050'>%s</span>", subtitle);
        GtkWidget *lbl_sub = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(lbl_sub), markup);
        gtk_misc_set_alignment(GTK_MISC(lbl_sub), 0.0, 0.5);
        gtk_box_pack_start(GTK_BOX(title_vbox), lbl_sub, FALSE, FALSE, 0);
    }
    gtk_box_pack_start(GTK_BOX(header_hbox), title_vbox, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), header_hbox, FALSE, FALSE, 0);
    
    *content_area = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), *content_area, TRUE, TRUE, 0);
    
    return vbox;
}

// 强制刷新整个屏幕
static void on_notebook_switch_page(GtkNotebook *notebook, GtkNotebookPage *page, guint page_num, gpointer user_data) {
    system("eips -c");
    gtk_widget_queue_draw(main_window);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "KindleStats");
    gtk_window_fullscreen(GTK_WINDOW(main_window));
    gtk_window_set_decorated(GTK_WINDOW(main_window), FALSE);
    g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GdkColor bg_color;
    gdk_color_parse("#ffffff", &bg_color); // 纯白底色
    gtk_widget_modify_bg(main_window, GTK_STATE_NORMAL, &bg_color);

    GtkWidget *main_vbox = gtk_vbox_new(FALSE, 10);
    gtk_container_set_border_width(GTK_CONTAINER(main_vbox), 20);
    gtk_container_add(GTK_CONTAINER(main_window), main_vbox);

    // Header
    GtkWidget *header_hbox = gtk_hbox_new(FALSE, 0);
    GtkWidget *title_label = gtk_label_new("<span size='14000' weight='bold'>KindleStats</span>");
    gtk_label_set_use_markup(GTK_LABEL(title_label), TRUE);
    
    GtkWidget *exit_btn = gtk_button_new();
    GtkWidget *exit_lbl = gtk_label_new("<span size='12000'>退出 [X]</span>");
    gtk_label_set_use_markup(GTK_LABEL(exit_lbl), TRUE);
    gtk_container_add(GTK_CONTAINER(exit_btn), exit_lbl);
    g_signal_connect(exit_btn, "clicked", G_CALLBACK(gtk_main_quit), NULL);
    
    gtk_box_pack_start(GTK_BOX(header_hbox), title_label, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(header_hbox), exit_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), header_hbox, FALSE, FALSE, 0);

    // 导航栏采用 GtkNotebook
    GtkWidget *notebook = gtk_notebook_new();
    gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
    g_signal_connect(notebook, "switch-page", G_CALLBACK(on_notebook_switch_page), NULL);
    gtk_box_pack_start(GTK_BOX(main_vbox), notebook, TRUE, TRUE, 0);

    // ================== 第一页：数据概览 ==================
    GtkWidget *dashboard_vbox = gtk_vbox_new(FALSE, 8);
    gtk_container_set_border_width(GTK_CONTAINER(dashboard_vbox), 4);
    
    // 三大统计卡片
    GtkWidget *stats_hbox = gtk_hbox_new(TRUE, 8);
    const char* stat_vals[] = {"128.5 h", "24 本", "46 min"};
    const char* stat_lbls[] = {"累计阅读", "已读书籍", "日均阅读"};
    for(int i=0; i<3; ++i) {
        GtkWidget *vbox = gtk_vbox_new(FALSE, 2);
        gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
        
        char m_val[128];
        sprintf(m_val, "<span size='20000' weight='bold'>%s</span>", stat_vals[i]); // 字体减小
        GtkWidget *lbl_val = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(lbl_val), m_val);
        gtk_misc_set_alignment(GTK_MISC(lbl_val), 0.5, 0.5);
        gtk_box_pack_start(GTK_BOX(vbox), lbl_val, TRUE, TRUE, 0);

        char m_sub[128];
        sprintf(m_sub, "<span size='11000' color='#505050'>%s</span>", stat_lbls[i]); // 字体减小
        GtkWidget *lbl_sub = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(lbl_sub), m_sub);
        gtk_misc_set_alignment(GTK_MISC(lbl_sub), 0.5, 0.5);
        gtk_box_pack_start(GTK_BOX(vbox), lbl_sub, TRUE, TRUE, 0);
        
        gtk_box_pack_start(GTK_BOX(stats_hbox), vbox, TRUE, TRUE, 0);
    }
    gtk_box_pack_start(GTK_BOX(dashboard_vbox), stats_hbox, FALSE, FALSE, 4);

    // 热力图模块
    GtkWidget *heat_content;
    GtkWidget *heat_card = create_chart_card("年度阅读热力图", NULL, &heat_content);
    heatmap_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(heatmap_da, -1, 140); 
    g_signal_connect(heatmap_da, "expose-event", G_CALLBACK(on_expose_heatmap), NULL);
    gtk_box_pack_start(GTK_BOX(heat_content), heatmap_da, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(dashboard_vbox), heat_card, FALSE, FALSE, 0);

    // 最爱阅读时段模块
    GtkWidget *gold_content;
    GtkWidget *gold_card = create_chart_card("最爱阅读时段", "最常在 18:00 - 21:00 分布", &gold_content);
    bar24_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(bar24_da, -1, 130);
    g_signal_connect(bar24_da, "expose-event", G_CALLBACK(on_expose_bar24), NULL);
    gtk_box_pack_start(GTK_BOX(gold_content), bar24_da, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(dashboard_vbox), gold_card, FALSE, FALSE, 0);
    
    // 近7日阅读趋势模块
    GtkWidget *hbar_content;
    GtkWidget *hbar_card = create_chart_card("近7日阅读趋势", NULL, &hbar_content);
    GtkWidget *hbar_da_widget = gtk_drawing_area_new();
    gtk_widget_set_size_request(hbar_da_widget, -1, 160);
    g_signal_connect(hbar_da_widget, "expose-event", G_CALLBACK(on_expose_hbar), NULL);
    gtk_box_pack_start(GTK_BOX(hbar_content), hbar_da_widget, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(dashboard_vbox), hbar_card, FALSE, FALSE, 0);

    GtkWidget *dashboard_tab_label = gtk_label_new("<span size='14000' weight='bold'> 数据概览 </span>");
    gtk_label_set_use_markup(GTK_LABEL(dashboard_tab_label), TRUE);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), dashboard_vbox, dashboard_tab_label);

    // ================== 第二页：我的书籍 (Placeholder) ==================
    GtkWidget *books_vbox = gtk_vbox_new(FALSE, 8);
    GtkWidget *books_lbl = gtk_label_new("【我的书籍】列表内容待实现");
    gtk_box_pack_start(GTK_BOX(books_vbox), books_lbl, TRUE, TRUE, 0);
    GtkWidget *books_tab_label = gtk_label_new("<span size='14000' weight='bold'> 我的书籍 </span>");
    gtk_label_set_use_markup(GTK_LABEL(books_tab_label), TRUE);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), books_vbox, books_tab_label);

    // ================== 第三页：今日阅读 (Placeholder) ==================
    GtkWidget *history_vbox = gtk_vbox_new(FALSE, 8);
    GtkWidget *history_lbl = gtk_label_new("【今日阅读】时间轴内容待实现");
    gtk_box_pack_start(GTK_BOX(history_vbox), history_lbl, TRUE, TRUE, 0);
    GtkWidget *history_tab_label = gtk_label_new("<span size='14000' weight='bold'> 今日阅读 </span>");
    gtk_label_set_use_markup(GTK_LABEL(history_tab_label), TRUE);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), history_vbox, history_tab_label);

    gtk_widget_show_all(main_window);
    
    system("eips -c");
    gtk_main();
    return 0;
}

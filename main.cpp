#include <gtk/gtk.h>
#include <cairo.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

// 绘制热力图的回调函数
static gboolean on_expose_heatmap(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    cairo_t *cr = gdk_cairo_create(widget->window);
    
    // 容器白底与边框
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_set_line_width(cr, 2.0);
    cairo_rectangle(cr, 0, 0, widget->allocation.width, widget->allocation.height);
    cairo_stroke(cr);
    
    // 内边距
    double pad_x = 12.0;
    double pad_y = 10.0;
    
    // 绘制标题
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 24.0);
    cairo_move_to(cr, pad_x, pad_y + 24.0);
    cairo_show_text(cr, "2026年年度阅读热力图");
    
    // 画一根虚线
    cairo_set_line_width(cr, 1.0);
    double dashes[] = {2.0, 2.0};
    cairo_set_dash(cr, dashes, 2, 0);
    cairo_move_to(cr, pad_x, pad_y + 35.0);
    cairo_line_to(cr, widget->allocation.width - pad_x, pad_y + 35.0);
    cairo_stroke(cr);
    cairo_set_dash(cr, NULL, 0, 0);
    
    // 绘制点阵
    double grid_y = pad_y + 50.0;
    double grid_width = widget->allocation.width - pad_x * 2;
    double cell_size = (grid_width - 52 * 2.0) / 53.0; // 53 列，间隔 2px
    
    for (int col = 0; col < 53; ++col) {
        for (int row = 0; row < 7; ++row) {
            double x = pad_x + col * (cell_size + 2.0);
            double y = grid_y + row * (cell_size + 2.0);
            
            // 模拟随机分布，这里用正弦函数产生固定的模拟数据
            double seed = sin(col * 7 + row + 2026.0);
            if (seed > 0.85) cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);       // 黑
            else if (seed > 0.55) cairo_set_source_rgb(cr, 0.4, 0.4, 0.4);  // 深灰
            else if (seed > 0.25) cairo_set_source_rgb(cr, 0.66, 0.66, 0.66); // 中灰
            else if (seed > -0.15) cairo_set_source_rgb(cr, 0.86, 0.86, 0.86); // 浅灰
            else cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // 白
            
            cairo_rectangle(cr, x, y, cell_size, cell_size);
            cairo_fill_preserve(cr);
            
            // 边框
            cairo_set_source_rgb(cr, 0.88, 0.88, 0.88);
            cairo_set_line_width(cr, 1.0);
            cairo_stroke(cr);
        }
    }
    
    cairo_destroy(cr);
    return FALSE;
}

// 绘制雷达图的回调函数
static gboolean on_expose_radar(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    cairo_t *cr = gdk_cairo_create(widget->window);
    
    // 白底与边框
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_set_line_width(cr, 2.0);
    cairo_rectangle(cr, 0, 0, widget->allocation.width, widget->allocation.height);
    cairo_stroke(cr);
    
    // 标题文字
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 24.0);
    cairo_move_to(cr, 10, 30);
    cairo_show_text(cr, "最爱阅读时段");
    
    cairo_set_font_size(cr, 18.0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
    cairo_move_to(cr, 10, 55);
    cairo_show_text(cr, "最常在 18:00 - 21:00 分布");
    
    // 计算雷达图中心点
    double cx = widget->allocation.width / 2.0;
    double cy = 70.0 + (widget->allocation.height - 70.0) / 2.0;
    double r_max = 110.0;
    
    // 八角形顶点角度
    double angles[8];
    for (int i = 0; i < 8; ++i) {
        angles[i] = -M_PI / 2.0 + i * (M_PI / 4.0);
    }
    
    // 绘制 3 层同心八边形网格
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
    
    // 绘制 4 条轴线
    cairo_set_source_rgb(cr, 0.6, 0.6, 0.6);
    cairo_set_line_width(cr, 0.75);
    double dashes[] = {2.0, 2.0};
    cairo_set_dash(cr, dashes, 2, 0);
    for (int i = 0; i < 4; ++i) {
        cairo_move_to(cr, cx + r_max * cos(angles[i]), cy + r_max * sin(angles[i]));
        cairo_line_to(cr, cx + r_max * cos(angles[i+4]), cy + r_max * sin(angles[i+4]));
    }
    cairo_stroke(cr);
    cairo_set_dash(cr, NULL, 0, 0); // 恢复实线
    
    // 填充数据多边形
    double data_r[8] = {85, 45, 55, 35, 55, 95, 105, 75}; // 模拟各时段数据
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.25);
    for (int i = 0; i < 8; ++i) {
        double x = cx + data_r[i] * cos(angles[i]);
        double y = cy + data_r[i] * sin(angles[i]);
        if (i == 0) cairo_move_to(cr, x, y);
        else cairo_line_to(cr, x, y);
    }
    cairo_close_path(cr);
    cairo_fill_preserve(cr);
    
    // 数据边界线
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_set_line_width(cr, 2.0);
    cairo_stroke(cr);
    
    // 绘制外圈刻度标签
    const char* labels[] = {"0:00", "3:00", "6:00", "9:00", "12:00", "15:00", "18:00", "21:00"};
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_set_font_size(cr, 16.0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    for (int i = 0; i < 8; ++i) {
        double offset_r = r_max + 20.0;
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

// 绘制横向柱状图的回调函数
static gboolean on_expose_hbar(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    cairo_t *cr = gdk_cairo_create(widget->window);
    
    // 白底与边框
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_set_line_width(cr, 2.0);
    cairo_rectangle(cr, 0, 0, widget->allocation.width, widget->allocation.height);
    cairo_stroke(cr);
    
    // 标题文字
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 24.0);
    cairo_move_to(cr, 10, 30);
    cairo_show_text(cr, "近7日阅读趋势 (分)");
    
    // 条形数据
    const char* days[] = {"周五", "周六", "周日", "周一", "周二", "周三", "今日"};
    double values[] = {45, 12, 90, 35, 50, 70, 80};
    double max_val = 100.0;
    
    double start_y = 60.0;
    double row_h = (widget->allocation.height - start_y - 15.0) / 7.0;
    
    cairo_set_font_size(cr, 18.0);
    for (int i = 0; i < 7; ++i) {
        double y = start_y + i * row_h;
        
        // 星期标签
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_move_to(cr, 10, y + row_h/2.0 + 6.0);
        cairo_show_text(cr, days[i]);
        
        // 条形轨道
        double track_x = 75.0;
        double track_w = widget->allocation.width - 150.0;
        double track_h = 16.0;
        double track_y = y + (row_h - track_h)/2.0;
        
        // 填充条形
        double fill_w = track_w * (values[i] / max_val);
        if (i % 2 == 0) cairo_set_source_rgb(cr, 0.3, 0.3, 0.3); // 深灰交替
        else cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        
        cairo_rectangle(cr, track_x, track_y, fill_w, track_h);
        cairo_fill_preserve(cr);
        
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_set_line_width(cr, 1.0);
        cairo_stroke(cr);
        
        // 数据文字
        char val_str[16];
        sprintf(val_str, "%.0fm", values[i]);
        cairo_move_to(cr, track_x + track_w + 15.0, y + row_h/2.0 + 6.0);
        cairo_show_text(cr, val_str);
    }
    
    cairo_destroy(cr);
    return FALSE;
}

// 退出事件处理
static gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    gtk_main_quit();
    return TRUE;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // 创建主窗口
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    
    // 【关键】修改标题为 PC:N，在 Kindle 上隐藏原生状态栏
    gtk_window_set_title(GTK_WINDOW(window), "L:A_N:application_PC:N_ID:kindlestats");
    gtk_window_fullscreen(GTK_WINDOW(window));
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // 主容器 (带内边距)
    GtkWidget *main_vbox = gtk_vbox_new(FALSE, 10);
    gtk_container_set_border_width(GTK_CONTAINER(main_vbox), 20);
    gtk_container_add(GTK_CONTAINER(window), main_vbox);

    // 头部标题与退出按钮
    GtkWidget *header_hbox = gtk_hbox_new(FALSE, 0);
    GtkWidget *title_label = gtk_label_new("<span size='20000' weight='bold'>KindleStats</span>");
    gtk_label_set_use_markup(GTK_LABEL(title_label), TRUE);
    
    GtkWidget *exit_btn = gtk_button_new();
    GtkWidget *exit_lbl = gtk_label_new("<span size='18000'>退出 [X]</span>");
    gtk_label_set_use_markup(GTK_LABEL(exit_lbl), TRUE);
    gtk_container_add(GTK_CONTAINER(exit_btn), exit_lbl);
    g_signal_connect(exit_btn, "clicked", G_CALLBACK(gtk_main_quit), NULL);
    
    gtk_box_pack_start(GTK_BOX(header_hbox), title_label, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(header_hbox), exit_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), header_hbox, FALSE, FALSE, 5);

    // 选项卡（按钮形式模拟）
    GtkWidget *tab_hbox = gtk_hbox_new(TRUE, 8);
    
    GtkWidget *tab1 = gtk_button_new();
    GtkWidget *lbl1 = gtk_label_new("<span size='24000' weight='bold'>数据概览</span>");
    gtk_label_set_use_markup(GTK_LABEL(lbl1), TRUE);
    gtk_container_add(GTK_CONTAINER(tab1), lbl1);
    
    GtkWidget *tab2 = gtk_button_new();
    GtkWidget *lbl2 = gtk_label_new("<span size='24000'>我的书籍</span>");
    gtk_label_set_use_markup(GTK_LABEL(lbl2), TRUE);
    gtk_container_add(GTK_CONTAINER(tab2), lbl2);
    
    GtkWidget *tab3 = gtk_button_new();
    GtkWidget *lbl3 = gtk_label_new("<span size='24000'>今日阅读</span>");
    gtk_label_set_use_markup(GTK_LABEL(lbl3), TRUE);
    gtk_container_add(GTK_CONTAINER(tab3), lbl3);
    
    gtk_box_pack_start(GTK_BOX(tab_hbox), tab1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(tab_hbox), tab2, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(tab_hbox), tab3, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), tab_hbox, FALSE, FALSE, 10);

    // 内容区容器
    GtkWidget *content_vbox = gtk_vbox_new(FALSE, 16);
    gtk_box_pack_start(GTK_BOX(main_vbox), content_vbox, TRUE, TRUE, 0);

    // 顶部三大统计卡片
    GtkWidget *stats_hbox = gtk_hbox_new(TRUE, 10);
    const char* stat_vals[] = {"128.5 h", "24 本", "46 min"};
    const char* stat_lbls[] = {"累计阅读", "已读书籍", "日均阅读"};
    for(int i=0; i<3; ++i) {
        GtkWidget *frame = gtk_frame_new(NULL);
        gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
        
        GtkWidget *vbox = gtk_vbox_new(FALSE, 2);
        gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);
        
        char markup[256];
        sprintf(markup, "<span size='36000' weight='bold'>%s</span>\n<span size='20000' color='#505050'>%s</span>", stat_vals[i], stat_lbls[i]);
        GtkWidget *lbl = gtk_label_new(NULL);
        gtk_label_set_use_markup(GTK_LABEL(lbl), TRUE);
        gtk_label_set_justify(GTK_LABEL(lbl), GTK_JUSTIFY_CENTER);
        
        gtk_box_pack_start(GTK_BOX(vbox), lbl, TRUE, TRUE, 0);
        gtk_container_add(GTK_CONTAINER(frame), vbox);
        gtk_box_pack_start(GTK_BOX(stats_hbox), frame, TRUE, TRUE, 0);
    }
    gtk_box_pack_start(GTK_BOX(content_vbox), stats_hbox, FALSE, FALSE, 0);

    // 热力图绘图区 (Cairo)
    GtkWidget *heatmap_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(heatmap_da, -1, 240);
    g_signal_connect(heatmap_da, "expose-event", G_CALLBACK(on_expose_heatmap), NULL);
    gtk_box_pack_start(GTK_BOX(content_vbox), heatmap_da, FALSE, FALSE, 0);

    // 雷达图绘图区 (Cairo)
    GtkWidget *radar_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(radar_da, -1, 320);
    g_signal_connect(radar_da, "expose-event", G_CALLBACK(on_expose_radar), NULL);
    gtk_box_pack_start(GTK_BOX(content_vbox), radar_da, FALSE, FALSE, 0);
    
    // 条状图绘图区 (Cairo)
    GtkWidget *hbar_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(hbar_da, -1, 280);
    g_signal_connect(hbar_da, "expose-event", G_CALLBACK(on_expose_hbar), NULL);
    gtk_box_pack_start(GTK_BOX(content_vbox), hbar_da, FALSE, FALSE, 0);

    // 设置全局背景色为纸张色 #f5f4ef
    GdkColor bg_color;
    gdk_color_parse("#f5f4ef", &bg_color);
    gtk_widget_modify_bg(window, GTK_STATE_NORMAL, &bg_color);

    // 显示界面
    gtk_widget_show_all(window);
    
    // GTK 渲染完毕后，调用底层的 eips 强制刷新整屏以去除残影
    system("eips -c");
    
    gtk_main();
    
    return 0;
}

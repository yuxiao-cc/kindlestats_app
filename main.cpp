
#include <gtk/gtk.h>
#include <cairo.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void log_debug(const char* msg) {
    FILE *f = fopen("/mnt/us/kindlestats_debug.log", "a");
    if (!f) f = fopen("kindlestats_debug.log", "a");
    if (f) {
        fprintf(f, "%s\n", msg);
        fclose(f);
    }
}


GtkWidget *main_window;
GtkWidget *dashboard_page;
GtkWidget *books_page;
GtkWidget *history_page;

GtkWidget* create_chart_card(const char* title, const char* subtitle, GtkWidget** content_area);

GtkWidget* create_chart_card(const char* title, const char* subtitle, GtkWidget** content_area) {
    GtkWidget *vbox = gtk_vbox_new(FALSE, 6);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
    
    GtkWidget *separator = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 4);
    
    GtkWidget *header_hbox = gtk_hbox_new(FALSE, 10);
    GtkWidget *title_vbox = gtk_vbox_new(FALSE, 2);
    char markup[256];
    
    sprintf(markup, "<span size='15000' weight='bold'>%s</span>", title); 
    GtkWidget *lbl_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_title), markup);
    gtk_misc_set_alignment(GTK_MISC(lbl_title), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(title_vbox), lbl_title, FALSE, FALSE, 0);
    
    if (subtitle) {
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


static gboolean on_expose_timeline(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    log_debug("on_expose_timeline called");
    cairo_t *cr = gdk_cairo_create(widget->window);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    
    double cx = widget->allocation.width / 2.0;
    
    // Draw vertical black line
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_set_line_width(cr, 2.0);
    cairo_move_to(cr, cx, 15);
    cairo_line_to(cr, cx, widget->allocation.height - 15);
    cairo_stroke(cr);
    
    // Draw 3 dots
    double y_positions[] = {25.0, 85.0, 145.0};
    for (int i = 0; i < 3; ++i) {
        cairo_arc(cr, cx, y_positions[i], 4.0, 0, 2 * G_PI);
        cairo_fill(cr);
    }
    
    cairo_destroy(cr);
    return FALSE;
}

GtkWidget* create_history_page() {
    GtkWidget *vbox = gtk_vbox_new(FALSE, 16);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 16);
    
    // Today Overview
    GtkWidget *overview_content;
    GtkWidget *overview_card = create_chart_card("今日阅读概览", NULL, &overview_content);
    
    GtkWidget *stats_hbox = gtk_hbox_new(TRUE, 8);
    const char* stat_vals[] = {"2.5 h", "3 次", "85 页"};
    const char* stat_lbls[] = {"今日阅读", "阅读次数", "翻页数"};
    for(int i = 0; i < 3; ++i) {
        GtkWidget *svbox = gtk_vbox_new(FALSE, 4);
        gtk_container_set_border_width(GTK_CONTAINER(svbox), 8);
        
        char markup[128];
        sprintf(markup, "<span size='18000' weight='bold'>%s</span>", stat_vals[i]);
        GtkWidget *lbl_val = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(lbl_val), markup);
        gtk_misc_set_alignment(GTK_MISC(lbl_val), 0.5, 0.5);
        gtk_box_pack_start(GTK_BOX(svbox), lbl_val, TRUE, TRUE, 0);

        sprintf(markup, "<span size='11000' color='#505050'>%s</span>", stat_lbls[i]);
        GtkWidget *lbl_sub = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(lbl_sub), markup);
        gtk_misc_set_alignment(GTK_MISC(lbl_sub), 0.5, 0.5);
        gtk_box_pack_start(GTK_BOX(svbox), lbl_sub, TRUE, TRUE, 0);
        
        gtk_box_pack_start(GTK_BOX(stats_hbox), svbox, TRUE, TRUE, 0);
    }
    gtk_box_pack_start(GTK_BOX(overview_content), stats_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), overview_card, FALSE, FALSE, 0);
    
    // Timeline Module
    GtkWidget *time_content;
    GtkWidget *time_card = create_chart_card("阅读时间轴", NULL, &time_content);
    
    GtkWidget *timeline_hbox = gtk_hbox_new(FALSE, 10);
    
    // Left: timeline graphic
    GtkWidget *timeline_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(timeline_da, 30, 180);
    g_signal_connect(timeline_da, "expose-event", G_CALLBACK(on_expose_timeline), NULL);
    gtk_box_pack_start(GTK_BOX(timeline_hbox), timeline_da, FALSE, FALSE, 0);
    
    // Right: Event labels
    GtkWidget *events_vbox = gtk_vbox_new(TRUE, 0);
    
    const char* times[] = {"14:30 - 15:45", "10:15 - 11:00", "08:00 - 08:30"};
    const char* descs[] = {"阅读《三体全集》，75分钟", "阅读《设计心理学》，45分钟", "阅读《Steve Jobs》，30分钟"};
    
    for (int i = 0; i < 3; ++i) {
        GtkWidget *evbox = gtk_vbox_new(FALSE, 2);
        char markup[256];
        
        sprintf(markup, "<span size='13000' weight='bold'>%s</span>", times[i]);
        GtkWidget *lbl_time = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(lbl_time), markup);
        gtk_misc_set_alignment(GTK_MISC(lbl_time), 0.0, 0.5);
        gtk_box_pack_start(GTK_BOX(evbox), lbl_time, FALSE, FALSE, 0);
        
        sprintf(markup, "<span size='11000' color='#505050'>%s</span>", descs[i]);
        GtkWidget *lbl_desc = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(lbl_desc), markup);
        gtk_misc_set_alignment(GTK_MISC(lbl_desc), 0.0, 0.5);
        gtk_box_pack_start(GTK_BOX(evbox), lbl_desc, FALSE, FALSE, 0);
        
        gtk_box_pack_start(GTK_BOX(events_vbox), evbox, TRUE, TRUE, 0);
    }
    
    gtk_box_pack_start(GTK_BOX(timeline_hbox), events_vbox, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(time_content), timeline_hbox, FALSE, FALSE, 10);
    
    gtk_box_pack_start(GTK_BOX(vbox), time_card, TRUE, TRUE, 0);
    
    return vbox;
}


static void destroy_double_ptr(gpointer data, GClosure *closure) {
    g_free(data);
}

// Draw the book cover outline
static gboolean on_expose_book_cover(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    cairo_t *cr = gdk_cairo_create(widget->window);
    
    // Background
    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
    cairo_paint(cr);
    
    // Border
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_set_line_width(cr, 2.0);
    cairo_rectangle(cr, 1, 1, widget->allocation.width - 2, widget->allocation.height - 2);
    cairo_stroke(cr);
    
    // Text
    cairo_set_font_size(cr, 12.0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_text_extents_t extents;
    cairo_text_extents(cr, "封面", &extents);
    cairo_move_to(cr, widget->allocation.width/2.0 - extents.width/2.0, widget->allocation.height/2.0 + extents.height/2.0);
    cairo_show_text(cr, "封面");
    
    cairo_destroy(cr);
    return FALSE;
}

// Draw the black and white progress bar
static gboolean on_expose_progress_bar(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    double progress = *(double*)data; // e.g., 0.75
    
    cairo_t *cr = gdk_cairo_create(widget->window);
    
    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
    cairo_paint(cr);
    
    double w = widget->allocation.width;
    double h = widget->allocation.height;
    
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_rectangle(cr, 0, 0, w * progress, h);
    cairo_fill(cr);
    
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_set_line_width(cr, 1.0);
    cairo_rectangle(cr, 0, 0, w, h);
    cairo_stroke(cr);
    
    cairo_destroy(cr);
    return FALSE;
}

GtkWidget* create_book_item(const char* title, const char* author, const char* time_str, double progress) {
    GtkWidget *hbox = gtk_hbox_new(FALSE, 16);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 8);
    
    // Book Cover
    GtkWidget *cover_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(cover_da, 60, 80);
    g_signal_connect(cover_da, "expose-event", G_CALLBACK(on_expose_book_cover), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), cover_da, FALSE, FALSE, 0);
    
    // Details
    GtkWidget *vbox = gtk_vbox_new(FALSE, 4);
    
    char markup[256];
    sprintf(markup, "<span size='14000' weight='bold'>%s</span>", title);
    GtkWidget *lbl_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_title), markup);
    gtk_misc_set_alignment(GTK_MISC(lbl_title), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), lbl_title, FALSE, FALSE, 0);
    
    sprintf(markup, "<span size='11000' color='#505050'>%s</span>", author);
    GtkWidget *lbl_author = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_author), markup);
    gtk_misc_set_alignment(GTK_MISC(lbl_author), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), lbl_author, FALSE, FALSE, 0);
    
    sprintf(markup, "<span size='11000'>最后阅读: %s</span>", time_str);
    GtkWidget *lbl_time = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_time), markup);
    gtk_misc_set_alignment(GTK_MISC(lbl_time), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), lbl_time, FALSE, FALSE, 0);
    
    // Progress Bar Area
    GtkWidget *prog_hbox = gtk_hbox_new(FALSE, 10);
    
    GtkWidget *prog_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(prog_da, 200, 8); // Thin progress bar
    double *p_val = g_new(double, 1);
    *p_val = progress;
    g_signal_connect_data(prog_da, "expose-event", G_CALLBACK(on_expose_progress_bar), p_val, (GClosureNotify)destroy_double_ptr, (GConnectFlags)0);
    gtk_box_pack_start(GTK_BOX(prog_hbox), prog_da, FALSE, FALSE, 0);
    
    sprintf(markup, "<span size='11000' weight='bold'>%d%%</span>", (int)(progress * 100));
    GtkWidget *lbl_pct = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_pct), markup);
    gtk_box_pack_start(GTK_BOX(prog_hbox), lbl_pct, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(vbox), prog_hbox, FALSE, FALSE, 4);
    
    gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
    
    return hbox;
}

GtkWidget* create_books_page() {
    GtkWidget *vbox = gtk_vbox_new(FALSE, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    
    // Pagination Bar
    GtkWidget *page_hbox = gtk_hbox_new(FALSE, 0);
    
    GtkWidget *btn_prev = gtk_button_new_with_label("上一页");
    gtk_box_pack_start(GTK_BOX(page_hbox), btn_prev, FALSE, FALSE, 0);
    
    GtkWidget *lbl_page = gtk_label_new("<span size='13000' weight='bold'>1 / 4 页</span>");
    gtk_label_set_use_markup(GTK_LABEL(lbl_page), TRUE);
    gtk_box_pack_start(GTK_BOX(page_hbox), lbl_page, TRUE, TRUE, 0);
    
    GtkWidget *btn_next = gtk_button_new_with_label("下一页");
    gtk_box_pack_end(GTK_BOX(page_hbox), btn_next, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(vbox), page_hbox, FALSE, FALSE, 5);
    
    GtkWidget *separator = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 0);
    
    // Mock Book List
    GtkWidget *b1 = create_book_item("三体全集", "刘慈欣", "今天 15:30", 0.45);
    gtk_box_pack_start(GTK_BOX(vbox), b1, FALSE, FALSE, 0);
    
    GtkWidget *b2 = create_book_item("Steve Jobs", "Walter Isaacson", "昨天 21:15", 0.82);
    gtk_box_pack_start(GTK_BOX(vbox), b2, FALSE, FALSE, 0);
    
    GtkWidget *b3 = create_book_item("设计心理学", "Donald A. Norman", "周一 09:00", 0.12);
    gtk_box_pack_start(GTK_BOX(vbox), b3, FALSE, FALSE, 0);
    
    GtkWidget *b4 = create_book_item("C++ Primer", "Stanley B. Lippman", "上周", 0.05);
    gtk_box_pack_start(GTK_BOX(vbox), b4, FALSE, FALSE, 0);
    
    return vbox;
}




// ------------------- Annual Heatmap -------------------

static gboolean on_expose_heatmap(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    log_debug("on_expose_heatmap called");
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
    log_debug("on_expose_bar24 called");
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
    log_debug("on_expose_hbar called");
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



static void on_tab_clicked(GtkWidget *widget, gpointer data) {
    int tab_index = GPOINTER_TO_INT(data);
    
    gtk_widget_hide(dashboard_page);
    gtk_widget_hide(books_page);
    gtk_widget_hide(history_page);
    
    if (tab_index == 1) gtk_widget_show(dashboard_page);
    else if (tab_index == 2) gtk_widget_show(books_page);
    else if (tab_index == 3) gtk_widget_show(history_page);
}

int main(int argc, char *argv[]) {
    log_debug("=== Starting KindleStats ==="); gtk_init(&argc, &argv); log_debug("gtk_init done");

    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "KindleStats");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 1072, 1448);
    g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GdkColor bg_color;
    gdk_color_parse("#ffffff", &bg_color); // Pure white background
    gtk_widget_modify_bg(main_window, GTK_STATE_NORMAL, &bg_color);

    log_debug("Creating main window and vbox..."); GtkWidget *main_vbox = gtk_vbox_new(FALSE, 10);
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

    // Old-school Tab HBox
    GtkWidget *tab_hbox = gtk_hbox_new(TRUE, 10);
    
    GtkWidget *tab1 = gtk_button_new();
    GtkWidget *lbl1 = gtk_label_new("<span size='16000'>数据概览</span>");
    gtk_label_set_use_markup(GTK_LABEL(lbl1), TRUE);
    gtk_container_add(GTK_CONTAINER(tab1), lbl1);
    g_signal_connect(tab1, "clicked", G_CALLBACK(on_tab_clicked), GINT_TO_POINTER(1));
    
    GtkWidget *tab2 = gtk_button_new();
    GtkWidget *lbl2 = gtk_label_new("<span size='16000'>我的书籍</span>");
    gtk_label_set_use_markup(GTK_LABEL(lbl2), TRUE);
    gtk_container_add(GTK_CONTAINER(tab2), lbl2);
    g_signal_connect(tab2, "clicked", G_CALLBACK(on_tab_clicked), GINT_TO_POINTER(2));
    
    GtkWidget *tab3 = gtk_button_new();
    GtkWidget *lbl3 = gtk_label_new("<span size='16000'>今日阅读</span>");
    gtk_label_set_use_markup(GTK_LABEL(lbl3), TRUE);
    gtk_container_add(GTK_CONTAINER(tab3), lbl3);
    g_signal_connect(tab3, "clicked", G_CALLBACK(on_tab_clicked), GINT_TO_POINTER(3));
    
    gtk_box_pack_start(GTK_BOX(tab_hbox), tab1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(tab_hbox), tab2, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(tab_hbox), tab3, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), tab_hbox, FALSE, FALSE, 10);

    // Container for Pages
    GtkWidget *content_vbox = gtk_vbox_new(FALSE, 16);
    gtk_box_pack_start(GTK_BOX(main_vbox), content_vbox, TRUE, TRUE, 0);

    log_debug("Creating dashboard_page..."); dashboard_page = create_dashboard_page();
    log_debug("Creating books_page..."); books_page = create_books_page();
    log_debug("Creating history_page..."); history_page = create_history_page();
    
    gtk_box_pack_start(GTK_BOX(content_vbox), dashboard_page, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content_vbox), books_page, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content_vbox), history_page, TRUE, TRUE, 0);

    log_debug("Calling gtk_widget_show_all..."); gtk_widget_show_all(main_window);
    
    // Hide pages except dashboard initially
    gtk_widget_hide(books_page);
    gtk_widget_hide(history_page);
    
    log_debug("Waiting for GTK to render...");
    while (gtk_events_pending()) gtk_main_iteration();

    log_debug("Entering gtk_main...");
    gtk_main();
    return 0;
}

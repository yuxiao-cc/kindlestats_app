#include "shared.h"
#include "draw_callbacks.h"
#include "month_dialog.h"
#include <stdio.h>
#include <string.h>

// Forward decl from month_dialog.cpp
GtkWidget* make_month_label(const char* text, int month_num);

// Create a chart card with GtkEventBox + GtkFrame (CRITICAL for e-ink)
static GtkWidget* create_chart_card(const char* title, const char* subtitle,
                                     const char* btn_label, GtkWidget** content_area,
                                     GtkWidget** btn_lbl_ptr, GCallback toggle_cb) {
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

    sprintf(markup, "<span size='20000' weight='bold'>%s</span>", title);
    GtkWidget *lbl_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_title), markup);
    gtk_misc_set_alignment(GTK_MISC(lbl_title), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(title_vbox), lbl_title, FALSE, FALSE, 0);

    if (subtitle) {
        sprintf(markup, "<span size='13000'>%s</span>", subtitle);
        GtkWidget *lbl_sub = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(lbl_sub), markup);
        gtk_misc_set_alignment(GTK_MISC(lbl_sub), 0.0, 0.5);
        gtk_box_pack_start(GTK_BOX(title_vbox), lbl_sub, FALSE, FALSE, 0);
    }
    gtk_box_pack_start(GTK_BOX(header_hbox), title_vbox, TRUE, TRUE, 0);

    if (btn_label && btn_lbl_ptr) {
        GtkWidget *btn = gtk_button_new();
        *btn_lbl_ptr = gtk_label_new(NULL);
        sprintf(markup, "<span size='13000'>%s</span>", btn_label);
        gtk_label_set_markup(GTK_LABEL(*btn_lbl_ptr), markup);
        gtk_container_add(GTK_CONTAINER(btn), *btn_lbl_ptr);
        if (toggle_cb) g_signal_connect(btn, "clicked", toggle_cb, NULL);
        gtk_box_pack_end(GTK_BOX(header_hbox), btn, FALSE, FALSE, 0);
    }

    gtk_box_pack_start(GTK_BOX(vbox), header_hbox, FALSE, FALSE, 0);

    *content_area = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), *content_area, TRUE, TRUE, 0);

    return event_box;
}

// Create a stat card (3 across the top)
static GtkWidget* create_stat_card(const char* val, const char* lbl) {
    GtkWidget *event_box = gtk_event_box_new();
    GdkColor white;
    gdk_color_parse("#ffffff", &white);
    gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &white);

    GtkWidget *frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
    gtk_container_add(GTK_CONTAINER(event_box), frame);

    GtkWidget *vbox = gtk_vbox_new(FALSE, 2);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_container_add(GTK_CONTAINER(frame), vbox);

    char m_val[128];
    sprintf(m_val, "<span size='28000' weight='bold'>%s</span>", val);
    GtkWidget *lbl_val = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_val), m_val);
    gtk_misc_set_alignment(GTK_MISC(lbl_val), 0.5, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), lbl_val, TRUE, TRUE, 0);

    char m_sub[128];
    sprintf(m_sub, "<span size='14000'>%s</span>", lbl);
    GtkWidget *lbl_sub = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_sub), m_sub);
    gtk_misc_set_alignment(GTK_MISC(lbl_sub), 0.5, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), lbl_sub, TRUE, TRUE, 0);

    return event_box;
}

GtkWidget* create_dashboard_page() {
    GtkWidget *vbox = gtk_vbox_new(FALSE, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);

    // ===== 3 stat cards =====
    GtkWidget *stats_hbox = gtk_hbox_new(TRUE, 8);
    const char* stat_vals[] = {"128.5 h", "24 本", "46 min"};
    const char* stat_lbls[] = {"累计阅读", "已读书籍", "日均阅读"};
    for (int i = 0; i < 3; i++) {
        gtk_box_pack_start(GTK_BOX(stats_hbox), create_stat_card(stat_vals[i], stat_lbls[i]), TRUE, TRUE, 0);
    }
    gtk_box_pack_start(GTK_BOX(vbox), stats_hbox, FALSE, FALSE, 0);

    // ===== Heatmap card =====
    GtkWidget *heat_content;
    GtkWidget *heat_card = create_chart_card("年度阅读热力图", "点击月份查看详情", NULL,
                                              &heat_content, NULL, NULL);

    // Month labels row (clickable)
    GtkWidget *months_hbox = gtk_hbox_new(TRUE, 0);
    const char* month_names[] = {"1月","2月","3月","4月","5月","6月","7月","8月","9月","10月","11月","12月"};
    for (int i = 0; i < 12; i++) {
        gtk_box_pack_start(GTK_BOX(months_hbox), make_month_label(month_names[i], i + 1), TRUE, TRUE, 0);
    }
    gtk_box_pack_start(GTK_BOX(heat_content), months_hbox, FALSE, FALSE, 2);

    // Heatmap drawing
    GtkWidget *heatmap_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(heatmap_da, -1, 170);
    g_signal_connect(heatmap_da, "expose-event", G_CALLBACK(on_expose_heatmap), NULL);
    gtk_box_pack_start(GTK_BOX(heat_content), heatmap_da, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), heat_card, FALSE, FALSE, 0);

    // ===== Gold Hour (24h bar) =====
    GtkWidget *gold_content;
    GtkWidget *gold_card = create_chart_card("最爱阅读时段", "最常在 18:00 - 21:00 分布", NULL,
                                               &gold_content, NULL, NULL);
    GtkWidget *bar24_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(bar24_da, -1, 150);
    g_signal_connect(bar24_da, "expose-event", G_CALLBACK(on_expose_bar24), NULL);
    gtk_box_pack_start(GTK_BOX(gold_content), bar24_da, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), gold_card, FALSE, FALSE, 0);

    // ===== 7-day trend hbar =====
    GtkWidget *hbar_content;
    GtkWidget *hbar_card = create_chart_card("近7日阅读趋势 (分)", NULL, NULL,
                                              &hbar_content, NULL, NULL);
    GtkWidget *hbar_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(hbar_da, -1, 180);
    g_signal_connect(hbar_da, "expose-event", G_CALLBACK(on_expose_hbar), NULL);
    gtk_box_pack_start(GTK_BOX(hbar_content), hbar_da, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbar_card, FALSE, FALSE, 0);

    return vbox;
}

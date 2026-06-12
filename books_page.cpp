#include "shared.h"
#include "draw_callbacks.h"
#include "books_page.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>

// Forward decl for lambda callback
static void on_book_row_clicked(GtkWidget *w, GdkEventButton *e, gpointer d) {
    if (e->type == GDK_BUTTON_PRESS) {
        show_book_detail(GPOINTER_TO_INT(d));
    }
}

static void on_back_btn_clicked(GtkButton *b, gpointer d) {
    back_to_book_list();
}

// Sort comparators
static int cmp_progress_desc(const void* a, const void* b) {
    int ia = *(const int*)a, ib = *(const int*)b;
    return g_books[ib].progress - g_books[ia].progress;
}
static int cmp_time_desc(const void* a, const void* b) {
    int ia = *(const int*)a, ib = *(const int*)b;
    return g_books[ib].timeMin - g_books[ia].timeMin;
}
static int cmp_alpha_asc(const void* a, const void* b) {
    int ia = *(const int*)a, ib = *(const int*)b;
    return strcmp(g_books[ia].title, g_books[ib].title);
}

// Create one book row
static GtkWidget* create_book_row(int book_idx) {
    GtkWidget *eb = gtk_event_box_new();
    GdkColor white;
    gdk_color_parse("#ffffff", &white);
    gtk_widget_modify_bg(eb, GTK_STATE_NORMAL, &white);

    GtkWidget *hbox = gtk_hbox_new(FALSE, 12);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 6);
    gtk_container_add(GTK_CONTAINER(eb), hbox);

    // Cover
    GtkWidget *cover_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(cover_da, 48, 64);
    g_signal_connect(cover_da, "expose-event", G_CALLBACK(on_expose_cover), GINT_TO_POINTER(book_idx));
    gtk_box_pack_start(GTK_BOX(hbox), cover_da, FALSE, FALSE, 0);

    // Info column
    GtkWidget *vbox = gtk_vbox_new(FALSE, 2);

    // Title + author + time line
    char m1[512];
    int h = g_books[book_idx].timeMin / 60;
    int m = g_books[book_idx].timeMin % 60;
    sprintf(m1, "<span size='14000' weight='bold'>%s</span>  <span size='11000' color='#505050'>%s</span>",
            g_books[book_idx].title, g_books[book_idx].author);
    GtkWidget *lbl_ta = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_ta), m1);
    gtk_misc_set_alignment(GTK_MISC(lbl_ta), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), lbl_ta, FALSE, FALSE, 0);

    // Progress row - long and thin
    GtkWidget *pbox = gtk_hbox_new(FALSE, 4);
    GtkWidget *prog_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(prog_da, 300, 6);
    double *pval = (double*)g_malloc(sizeof(double));
    *pval = g_books[book_idx].progress / 100.0;
    g_signal_connect_data(prog_da, "expose-event", G_CALLBACK(on_expose_progress),
                          pval, (GClosureNotify)destroy_double_ptr, (GConnectFlags)0);
    gtk_box_pack_start(GTK_BOX(pbox), prog_da, TRUE, TRUE, 0);

    char m3[64];
    sprintf(m3, "<span size='10000' weight='bold'>%d%%</span>", g_books[book_idx].progress);
    GtkWidget *lbl_pct = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_pct), m3);
    gtk_box_pack_start(GTK_BOX(pbox), lbl_pct, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), pbox, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);

    // Time on right
    char m2[64];
    sprintf(m2, "<span size='12000' weight='bold'>%d时%d分</span>", h, m);
    GtkWidget *lbl_t = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_t), m2);
    gtk_misc_set_alignment(GTK_MISC(lbl_t), 1.0, 0.5);
    gtk_box_pack_end(GTK_BOX(hbox), lbl_t, FALSE, FALSE, 0);

    // Click → show detail
    g_signal_connect(eb, "button-press-event", G_CALLBACK(on_book_row_clicked), GINT_TO_POINTER(book_idx));

    return eb;
}

static void refresh_pagination_label(GtkWidget *label, int total_pages) {
    char buf[64];
    char m[128];
    if (total_pages <= 0) sprintf(buf, "第 0 / 0 页");
    else sprintf(buf, "第 %d / %d 页", g_book_page + 1, total_pages);
    sprintf(m, "<span size='14000' weight='bold'>%s</span>", buf);
    gtk_label_set_markup(GTK_LABEL(label), m);
}

// Public: rebuild the list
void rebuild_book_list() {
    if (!g_books_list_container) return;

    // Clear existing
    GList *children = gtk_container_get_children(GTK_CONTAINER(g_books_list_container));
    for (GList *l = children; l; l = l->next) gtk_widget_destroy(GTK_WIDGET(l->data));
    g_list_free(children);

    // Build index array
    int indices[NUM_BOOKS];
    int n = 0;
    for (int i = 0; i < NUM_BOOKS; i++) {
        if (g_filter_unfinished && g_books[i].finished) continue;
        indices[n++] = i;
    }

    // Sort
    if (g_sort_mode == 0) qsort(indices, n, sizeof(int), cmp_progress_desc);
    else if (g_sort_mode == 1) qsort(indices, n, sizeof(int), cmp_time_desc);
    else qsort(indices, n, sizeof(int), cmp_alpha_asc);

    int total_pages = (n + BOOKS_PER_PAGE - 1) / BOOKS_PER_PAGE;
    if (total_pages == 0) total_pages = 1;
    if (g_book_page >= total_pages) g_book_page = total_pages - 1;
    if (g_book_page < 0) g_book_page = 0;

    if (n == 0) {
        GtkWidget *empty = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(empty), "<span size='16000' weight='bold'>没有找到匹配的书籍</span>");
        gtk_misc_set_alignment(GTK_MISC(empty), 0.5, 0.5);
        gtk_box_pack_start(GTK_BOX(g_books_list_container), empty, TRUE, TRUE, 20);
    } else {
        int start = g_book_page * BOOKS_PER_PAGE;
        int end = start + BOOKS_PER_PAGE;
        if (end > n) end = n;
        for (int i = start; i < end; i++) {
            gtk_box_pack_start(GTK_BOX(g_books_list_container),
                               create_book_row(indices[i]), FALSE, FALSE, 4);
        }
    }

    // Update pagination label
    GtkWidget *page_lbl = (GtkWidget*)g_object_get_data(G_OBJECT(g_books_page_widget), "page-label");
    if (page_lbl) refresh_pagination_label(page_lbl, total_pages);

    gtk_widget_show_all(g_books_list_container);
}

// Callbacks
static void on_filter_toggled(GtkToggleButton *btn, gpointer data) {
    g_filter_unfinished = gtk_toggle_button_get_active(btn);
    g_book_page = 0;
    rebuild_book_list();
}

static void on_sort_clicked(GtkButton *btn, gpointer data) {
    g_sort_mode = GPOINTER_TO_INT(data);
    g_book_page = 0;
    rebuild_book_list();
}

static void on_prev_page(GtkButton *btn, gpointer data) {
    if (g_book_page > 0) { g_book_page--; rebuild_book_list(); }
}

static void on_next_page(GtkButton *btn, gpointer data) {
    g_book_page++;
    rebuild_book_list(); // will clamp
}

// Detail page
void update_book_detail_content(int idx) {
    if (!g_books_detail_container || idx < 0 || idx >= NUM_BOOKS) return;

    GList *children = gtk_container_get_children(GTK_CONTAINER(g_books_detail_container));
    for (GList *l = children; l; l = l->next) gtk_widget_destroy(GTK_WIDGET(l->data));
    g_list_free(children);

    // Back button
    GtkWidget *back_btn = gtk_button_new_with_label("[ < 返回书籍列表 ]");
    g_signal_connect(back_btn, "clicked", G_CALLBACK(on_back_btn_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(g_books_detail_container), back_btn, FALSE, FALSE, 4);

    // Header: large cover + title block
    GtkWidget *hdr = gtk_hbox_new(FALSE, 16);
    gtk_container_set_border_width(GTK_CONTAINER(hdr), 8);

    GtkWidget *cover_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(cover_da, 72, 96);
    g_signal_connect(cover_da, "expose-event", G_CALLBACK(on_expose_cover), GINT_TO_POINTER(idx));
    gtk_box_pack_start(GTK_BOX(hdr), cover_da, FALSE, FALSE, 0);

    GtkWidget *title_vbox = gtk_vbox_new(FALSE, 4);
    char m[512];
    sprintf(m, "<span size='22000' weight='bold'>%s</span>", g_books[idx].title);
    GtkWidget *lbl_t = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_t), m);
    gtk_misc_set_alignment(GTK_MISC(lbl_t), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(title_vbox), lbl_t, FALSE, FALSE, 0);

    sprintf(m, "<span size='14000' color='#505050'>作者: %s</span>", g_books[idx].author);
    GtkWidget *lbl_a = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_a), m);
    gtk_misc_set_alignment(GTK_MISC(lbl_a), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(title_vbox), lbl_a, FALSE, FALSE, 0);

    // Progress
    GtkWidget *pbox = gtk_hbox_new(FALSE, 8);
    GtkWidget *prog_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(prog_da, 250, 12);
    double *pval = (double*)g_malloc(sizeof(double));
    *pval = g_books[idx].progress / 100.0;
    g_signal_connect_data(prog_da, "expose-event", G_CALLBACK(on_expose_progress),
                          pval, (GClosureNotify)destroy_double_ptr, (GConnectFlags)0);
    gtk_box_pack_start(GTK_BOX(pbox), prog_da, FALSE, FALSE, 0);
    sprintf(m, "<span size='14000' weight='bold'>%d%%</span>", g_books[idx].progress);
    GtkWidget *lbl_p = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_p), m);
    gtk_box_pack_start(GTK_BOX(pbox), lbl_p, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(title_vbox), pbox, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(hdr), title_vbox, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(g_books_detail_container), hdr, FALSE, FALSE, 0);

    // Stats grid: 2 columns
    GtkWidget *grid = gtk_table_new(3, 2, FALSE);
    gtk_table_set_col_spacings(GTK_TABLE(grid), 20);
    gtk_table_set_row_spacings(GTK_TABLE(grid), 8);

    int h = g_books[idx].timeMin / 60;
    int m_ = g_books[idx].timeMin % 60;
    int rem_h = (100 - g_books[idx].progress) * h / (g_books[idx].progress > 0 ? g_books[idx].progress : 1);

    const char* lbls[] = {"阅读总时长", "最后阅读", "阅读进度", "已完成", "起始日期", "今日趋势"};
    char vals[6][256];
    sprintf(vals[0], "%d小时%d分", h, m_);
    sprintf(vals[1], "%s", g_books[idx].lastRead);
    sprintf(vals[2], "%d%%", g_books[idx].progress);
    sprintf(vals[3], "%s", g_books[idx].finished ? "是" : "否");
    sprintf(vals[4], "%s", g_books[idx].finished ? "已读完" : "进行中");
    sprintf(vals[5], "见下方图表");

    for (int i = 0; i < 6; i++) {
        GtkWidget *eb = gtk_event_box_new();
        GdkColor white; gdk_color_parse("#ffffff", &white);
        gtk_widget_modify_bg(eb, GTK_STATE_NORMAL, &white);
        GtkWidget *vb = gtk_vbox_new(FALSE, 2);
        char lm[128];
        sprintf(lm, "<span size='12000' color='#505050'>%s</span>", lbls[i]);
        GtkWidget *ll = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(ll), lm);
        gtk_misc_set_alignment(GTK_MISC(ll), 0.0, 0.0);
        gtk_box_pack_start(GTK_BOX(vb), ll, FALSE, FALSE, 0);
        char vm[128];
        sprintf(vm, "<span size='16000' weight='bold'>%s</span>", vals[i]);
        GtkWidget *vl = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(vl), vm);
        gtk_misc_set_alignment(GTK_MISC(vl), 0.0, 0.0);
        gtk_box_pack_start(GTK_BOX(vb), vl, FALSE, FALSE, 0);
        gtk_container_add(GTK_CONTAINER(eb), vb);
        gtk_table_attach_defaults(GTK_TABLE(grid), eb, i%2, i%2+1, i/2, i/2+1);
    }
    gtk_box_pack_start(GTK_BOX(g_books_detail_container), grid, FALSE, FALSE, 8);

    // Mini trend chart
    GtkWidget *trend_eb = gtk_event_box_new();
    GdkColor white; gdk_color_parse("#ffffff", &white);
    gtk_widget_modify_bg(trend_eb, GTK_STATE_NORMAL, &white);
    GtkWidget *trend_da = gtk_drawing_area_new();
    gtk_widget_set_size_request(trend_da, -1, 100);
    struct TrendData *td = (struct TrendData*)g_malloc(sizeof(struct TrendData));
    memcpy(td->trend, g_books[idx].trend, sizeof(td->trend));
    g_signal_connect_data(trend_da, "expose-event", G_CALLBACK(on_expose_mini_trend),
                          td, (GClosureNotify)g_free, (GConnectFlags)0);
    gtk_container_add(GTK_CONTAINER(trend_eb), trend_da);
    gtk_box_pack_start(GTK_BOX(g_books_detail_container), trend_eb, TRUE, TRUE, 4);

    // Label above trend
    GtkWidget *trend_lbl = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(trend_lbl), "<span size='13000' weight='bold'>近7次阅读时长 (分钟)</span>");
    gtk_misc_set_alignment(GTK_MISC(trend_lbl), 0.0, 0.5);

    gtk_widget_show_all(g_books_detail_container);
}

void show_book_detail(int book_idx) {
    g_current_book_detail = book_idx;
    update_book_detail_content(book_idx);
    gtk_widget_hide(g_books_list_container);
    gtk_widget_show(g_books_detail_container);
}

void back_to_book_list() {
    gtk_widget_hide(g_books_detail_container);
    gtk_widget_show(g_books_list_container);
}

GtkWidget* create_books_page() {
    GtkWidget *page_vbox = gtk_vbox_new(FALSE, 6);
    gtk_container_set_border_width(GTK_CONTAINER(page_vbox), 6);
    g_books_page_widget = page_vbox;

    // Control bar
    GtkWidget *ctrl = gtk_hbox_new(FALSE, 8);
    gtk_container_set_border_width(GTK_CONTAINER(ctrl), 4);

    GtkWidget *filter_btn = gtk_toggle_button_new_with_label("仅显示未读完");
    g_signal_connect(filter_btn, "toggled", G_CALLBACK(on_filter_toggled), NULL);
    gtk_box_pack_start(GTK_BOX(ctrl), filter_btn, FALSE, FALSE, 0);

    // Spacer
    GtkWidget *spacer = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(ctrl), spacer, TRUE, TRUE, 0);

    GtkWidget *sort_lbl = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(sort_lbl), "<span size='11000'>排序:</span>");
    gtk_box_pack_end(GTK_BOX(ctrl), sort_lbl, FALSE, FALSE, 0);

    GtkWidget *s0 = gtk_button_new_with_label("进度");
    GtkWidget *s1 = gtk_button_new_with_label("时长");
    GtkWidget *s2 = gtk_button_new_with_label("字母");
    g_signal_connect(s0, "clicked", G_CALLBACK(on_sort_clicked), GINT_TO_POINTER(0));
    g_signal_connect(s1, "clicked", G_CALLBACK(on_sort_clicked), GINT_TO_POINTER(1));
    g_signal_connect(s2, "clicked", G_CALLBACK(on_sort_clicked), GINT_TO_POINTER(2));
    gtk_box_pack_end(GTK_BOX(ctrl), s2, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(ctrl), s1, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(ctrl), s0, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(page_vbox), ctrl, FALSE, FALSE, 0);

    // List container
    g_books_list_container = gtk_vbox_new(FALSE, 2);
    gtk_box_pack_start(GTK_BOX(page_vbox), g_books_list_container, TRUE, TRUE, 0);

    // Detail container (hidden initially)
    g_books_detail_container = gtk_vbox_new(FALSE, 4);
    gtk_box_pack_start(GTK_BOX(page_vbox), g_books_detail_container, TRUE, TRUE, 0);
    gtk_widget_hide(g_books_detail_container);

    // Pagination bar
    GtkWidget *pgbar = gtk_hbox_new(FALSE, 12);
    gtk_container_set_border_width(GTK_CONTAINER(pgbar), 4);
    GtkWidget *prev = gtk_button_new_with_label("上一页");
    GtkWidget *page_lbl = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(page_lbl), "<span size='14000' weight='bold'>第 1 / 1 页</span>");
    GtkWidget *next = gtk_button_new_with_label("下一页");
    g_signal_connect(prev, "clicked", G_CALLBACK(on_prev_page), NULL);
    g_signal_connect(next, "clicked", G_CALLBACK(on_next_page), NULL);
    gtk_box_pack_start(GTK_BOX(pgbar), prev, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pgbar), page_lbl, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(pgbar), next, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(page_vbox), pgbar, FALSE, FALSE, 0);

    g_object_set_data(G_OBJECT(page_vbox), "page-label", page_lbl);

    rebuild_book_list();
    return page_vbox;
}

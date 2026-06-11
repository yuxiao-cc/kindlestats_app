#include <gtk/gtk.h>
#include <stdlib.h>
#include "utils.h"
#include "ui_dashboard.h"
#include "ui_books.h"
#include "ui_history.h"

GtkWidget *main_window;

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

    // Notebook for Tabs
    GtkWidget *notebook = gtk_notebook_new();
    gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
    gtk_box_pack_start(GTK_BOX(main_vbox), notebook, TRUE, TRUE, 0);

    // Page 1: Dashboard
    GtkWidget *dashboard_page = create_dashboard_page();
    GtkWidget *lbl1 = gtk_label_new("<span size='14000' weight='bold'> 数据概览 </span>");
    gtk_label_set_use_markup(GTK_LABEL(lbl1), TRUE);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), dashboard_page, lbl1);

    // Page 2: My Books
    GtkWidget *books_page = create_books_page();
    GtkWidget *lbl2 = gtk_label_new("<span size='14000' weight='bold'> 我的书籍 </span>");
    gtk_label_set_use_markup(GTK_LABEL(lbl2), TRUE);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), books_page, lbl2);

    // Page 3: Today's Reading History
    GtkWidget *history_page = create_history_page();
    GtkWidget *lbl3 = gtk_label_new("<span size='14000' weight='bold'> 今日阅读 </span>");
    gtk_label_set_use_markup(GTK_LABEL(lbl3), TRUE);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), history_page, lbl3);

    gtk_widget_show_all(main_window);
    
    // 不再使用 eips -c 暴力清屏，依赖 Kindle XDamage 局部刷新
    gtk_main();
    return 0;
}

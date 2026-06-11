#include <gtk/gtk.h>
#include <stdlib.h>
#include "utils.h"
#include "ui_dashboard.h"
#include "ui_books.h"
#include "ui_history.h"

GtkWidget *main_window;
GtkWidget *dashboard_page;
GtkWidget *books_page;
GtkWidget *history_page;

static void on_tab_clicked(GtkWidget *widget, gpointer data) {
    int tab_index = GPOINTER_TO_INT(data);
    
    gtk_widget_hide(dashboard_page);
    gtk_widget_hide(books_page);
    gtk_widget_hide(history_page);
    
    if (tab_index == 1) gtk_widget_show(dashboard_page);
    else if (tab_index == 2) gtk_widget_show(books_page);
    else if (tab_index == 3) gtk_widget_show(history_page);
    
    force_eink_refresh();
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "KindleStats");
    gtk_window_fullscreen(GTK_WINDOW(main_window));
    gtk_window_set_decorated(GTK_WINDOW(main_window), FALSE);
    g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GdkColor bg_color;
    gdk_color_parse("#f5f4ef", &bg_color); // Revert to old background color to be 100% safe
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

    dashboard_page = create_dashboard_page();
    books_page = create_books_page();
    history_page = create_history_page();
    
    gtk_box_pack_start(GTK_BOX(content_vbox), dashboard_page, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content_vbox), books_page, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content_vbox), history_page, TRUE, TRUE, 0);

    gtk_widget_show_all(main_window);
    
    // Hide pages except dashboard initially
    gtk_widget_hide(books_page);
    gtk_widget_hide(history_page);
    
    system("eips -c"); // Initial clear is fine now that we're explicitly triggering refresh on tab clicks
    gtk_main();
    return 0;
}

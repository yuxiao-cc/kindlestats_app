#include "shared.h"
#include "dashboard_page.h"
#include "books_page.h"
#include "today_page.h"
#include <stdio.h>

static GtkWidget* make_tab_label(const char* text) {
    GtkWidget *lbl = gtk_label_new(NULL);
    char m[128];
    sprintf(m, "<span size='13000' weight='bold'>%s</span>", text);
    gtk_label_set_markup(GTK_LABEL(lbl), m);
    return lbl;
}

int main(int argc, char *argv[]) {
    log_debug("=== Starting KindleStats ===");
    gtk_init(&argc, &argv);
    log_debug("gtk_init done");

    g_main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(g_main_window), "L:A_N:application_PC:N_ID:kindlestats");
    gtk_window_set_default_size(GTK_WINDOW(g_main_window), 1080, 1440);
    gtk_window_set_decorated(GTK_WINDOW(g_main_window), FALSE);
    g_signal_connect(g_main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GdkColor bg;
    gdk_color_parse("#f5f4ef", &bg);
    gtk_widget_modify_bg(g_main_window, GTK_STATE_NORMAL, &bg);

    log_debug("Building UI...");

    // Main vertical container
    GtkWidget *main_vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(g_main_window), main_vbox);

    // Header bar
    GtkWidget *header = gtk_hbox_new(FALSE, 8);
    gtk_container_set_border_width(GTK_CONTAINER(header), 8);
    GtkWidget *title_lbl = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title_lbl), "<span size='17000' weight='bold'>KindleStats</span>");
    gtk_misc_set_alignment(GTK_MISC(title_lbl), 0.0, 0.5);
    gtk_box_pack_start(GTK_BOX(header), title_lbl, TRUE, TRUE, 0);

    GtkWidget *exit_btn = gtk_button_new();
    GtkWidget *exit_lbl = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(exit_lbl), "<span size='10000'>退出 [X]</span>");
    gtk_container_add(GTK_CONTAINER(exit_btn), exit_lbl);
    g_signal_connect(exit_btn, "clicked", G_CALLBACK(gtk_main_quit), NULL);
    gtk_box_pack_end(GTK_BOX(header), exit_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), header, FALSE, FALSE, 0);

    // Bottom border for header
    GtkWidget *hdr_sep = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(main_vbox), hdr_sep, FALSE, FALSE, 0);

    // Notebook with 3 tabs
    GtkWidget *nb = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(nb), GTK_POS_TOP);
    gtk_box_pack_start(GTK_BOX(main_vbox), nb, TRUE, TRUE, 0);

    log_debug("Creating dashboard page...");
    gtk_notebook_append_page(GTK_NOTEBOOK(nb), create_dashboard_page(), make_tab_label("数据概览"));
    log_debug("Creating books page...");
    gtk_notebook_append_page(GTK_NOTEBOOK(nb), create_books_page(), make_tab_label("我的书籍"));
    log_debug("Creating today page...");
    gtk_notebook_append_page(GTK_NOTEBOOK(nb), create_today_page(), make_tab_label("今日阅读"));

    log_debug("Showing window...");
    gtk_widget_show_all(g_main_window);

    // Initial e-ink full refresh (proven pattern from working version)
    system("eips -c");

    log_debug("Entering gtk_main...");
    gtk_main();

    log_debug("Exiting.");
    return 0;
}

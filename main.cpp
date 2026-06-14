#include "shared.h"
#include "dashboard_page.h"
#include "books_page.h"
#include "today_page.h"
#include <stdio.h>
#include <string.h>
#ifdef __linux__
#include <unistd.h>
#endif

GtkBuilder* load_ui(const char *filename) {
    GtkBuilder *builder = gtk_builder_new();
    GError *err = NULL;
    char path[512];

    // Try relative to executable first
    char exe_path[512] = {0};
#ifdef __linux__
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len > 0) {
        exe_path[len] = '\0';
        char *last_slash = strrchr(exe_path, '/');
        if (last_slash) {
            *last_slash = '\0';
            snprintf(path, sizeof(path), "%s/ui/%s", exe_path, filename);
            if (gtk_builder_add_from_file(builder, path, &err)) return builder;
            g_error_free(err); err = NULL;
        }
    }
#endif
    // Fallback: relative to cwd
    snprintf(path, sizeof(path), "ui/%s", filename);
    if (!gtk_builder_add_from_file(builder, path, &err)) {
        log_debug(err->message);
        g_error_free(err);
        g_object_unref(builder);
        return NULL;
    }
    return builder;
}

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

    GtkBuilder *builder = load_ui("main_window.ui");
    if (!builder) return 1;

    g_main_window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    g_signal_connect(g_main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GdkColor bg;
    gdk_color_parse("#f5f4ef", &bg);
    gtk_widget_modify_bg(g_main_window, GTK_STATE_NORMAL, &bg);

    GtkWidget *title_lbl = GTK_WIDGET(gtk_builder_get_object(builder, "title_lbl"));
    gtk_label_set_markup(GTK_LABEL(title_lbl), "<span size='17000' weight='bold'>KindleStats</span>");

    GtkWidget *exit_btn = GTK_WIDGET(gtk_builder_get_object(builder, "exit_btn"));
    GtkWidget *exit_child = gtk_bin_get_child(GTK_BIN(exit_btn));
    if (exit_child && GTK_IS_LABEL(exit_child)) {
        gtk_label_set_markup(GTK_LABEL(exit_child), "<span size='10000'>退出 [X]</span>");
    }
    g_signal_connect(exit_btn, "clicked", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *nb = GTK_WIDGET(gtk_builder_get_object(builder, "notebook"));
    g_object_unref(builder);

    log_debug("Creating dashboard page...");
    gtk_notebook_append_page(GTK_NOTEBOOK(nb), create_dashboard_page(), make_tab_label("数据概览"));
    log_debug("Creating books page...");
    gtk_notebook_append_page(GTK_NOTEBOOK(nb), create_books_page(), make_tab_label("我的书籍"));
    log_debug("Creating today page...");
    gtk_notebook_append_page(GTK_NOTEBOOK(nb), create_today_page(), make_tab_label("今日阅读"));

    log_debug("Showing window...");
    gtk_widget_show_all(g_main_window);

    system("eips -c");

    log_debug("Entering gtk_main...");
    gtk_main();

    log_debug("Exiting.");
    return 0;
}
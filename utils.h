#ifndef UTILS_H
#define UTILS_H

#include <gtk/gtk.h>

extern GtkWidget *main_window; // Define in main.cpp

GtkWidget* create_chart_card(const char* title, const char* subtitle, GtkWidget** content_area);
void force_eink_refresh();

#endif

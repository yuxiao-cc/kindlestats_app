#ifndef KINDLESTATS_DRAW_CALLBACKS_H
#define KINDLESTATS_DRAW_CALLBACKS_H

#include <gtk/gtk.h>

// Shared struct types for drawing callbacks
struct MonthCalData {
    int year, month;
};

struct TrendData {
    int trend[7];
};

gboolean on_expose_heatmap(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_expose_bar24(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_expose_hbar(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_expose_cover(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_expose_progress(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_expose_tl_dot(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_expose_month_cal(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_expose_mini_trend(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean on_expose_pie(GtkWidget *widget, GdkEventExpose *event, gpointer data);

#endif

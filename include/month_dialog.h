#ifndef KINDLESTATS_MONTH_DIALOG_H
#define KINDLESTATS_MONTH_DIALOG_H

#include <gtk/gtk.h>

// Forward decl
struct MonthCalData;

void show_month_dialog(int month);
GtkWidget* make_month_label(const char* text, int month_num);

#endif

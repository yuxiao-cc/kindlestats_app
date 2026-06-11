#include "shared.h"
#include "draw_callbacks.h"
#include "month_dialog.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static gboolean on_month_label_clicked(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    if (event->type != GDK_BUTTON_PRESS) return FALSE;
    int month = GPOINTER_TO_INT(data);
    show_month_dialog(month);
    return TRUE;
}

// Helper: create a clickable month label
GtkWidget* make_month_label(const char* text, int month_num) {
    GtkWidget *eb = gtk_event_box_new();
    GdkColor white;
    gdk_color_parse("#ffffff", &white);
    gtk_widget_modify_bg(eb, GTK_STATE_NORMAL, &white);

    GtkWidget *lbl = gtk_label_new(NULL);
    char m[128];
    sprintf(m, "<span size='14000' weight='bold' underline='single'>%s</span>", text);
    gtk_label_set_markup(GTK_LABEL(lbl), m);
    gtk_misc_set_alignment(GTK_MISC(lbl), 0.5, 0.5);
    gtk_container_add(GTK_CONTAINER(eb), lbl);

    g_signal_connect(eb, "button-press-event", G_CALLBACK(on_month_label_clicked), GINT_TO_POINTER(month_num));
    return eb;
}

void show_month_dialog(int month) {
    if (month < 1 || month > 12) return;

    char title[64];
    sprintf(title, "2026年%d月 阅读概况", month);

    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        title, GTK_WINDOW(g_main_window),
        (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
        "关闭", GTK_RESPONSE_CLOSE, NULL);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 480, 480);

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content), 12);

    // Calendar drawing area
    GtkWidget *cal_eb = gtk_event_box_new();
    GdkColor white;
    gdk_color_parse("#ffffff", &white);
    gtk_widget_modify_bg(cal_eb, GTK_STATE_NORMAL, &white);

    GtkWidget *cal = gtk_drawing_area_new();
    gtk_widget_set_size_request(cal, -1, 250);
    struct MonthCalData *mdata = (struct MonthCalData*)g_malloc(sizeof(struct MonthCalData));
    mdata->year = 2026;
    mdata->month = month;
    g_signal_connect_data(cal, "expose-event", G_CALLBACK(on_expose_month_cal),
                          mdata, (GClosureNotify)g_free, (GConnectFlags)0);
    gtk_container_add(GTK_CONTAINER(cal_eb), cal);
    gtk_box_pack_start(GTK_BOX(content), cal_eb, FALSE, FALSE, 0);

    // Spacer
    gtk_box_pack_start(GTK_BOX(content), gtk_label_new(""), FALSE, FALSE, 6);

    // Stats (mock data based on month)
    int total_min = 400 + (int)((sin(month) + 1) * 300);
    int active_days = 10 + (int)((cos(month) + 1) * 8);
    int max_streak = 2 + (int)((sin(month * 2) + 1) * 3);

    char stats_text[512];
    sprintf(stats_text,
        "<span size='15000' weight='bold'>本月统计汇总:</span>\n\n"
        "<span size='15000'>• 累计阅读: <b>%.1f 小时</b></span>\n"
        "<span size='15000'>• 活跃天数: <b>%d 天</b></span>\n"
        "<span size='15000'>• 最长连读: <b>%d 天</b></span>",
        total_min / 60.0, active_days, max_streak);

    GtkWidget *stats_lbl = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(stats_lbl), stats_text);
    gtk_misc_set_alignment(GTK_MISC(stats_lbl), 0.0, 0.0);
    gtk_box_pack_start(GTK_BOX(content), stats_lbl, FALSE, FALSE, 4);

    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

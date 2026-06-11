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
    char dbg[64];
    sprintf(dbg, "Month %d clicked", month);
    log_debug(dbg);
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

    // Force white background on the dialog window (critical for e-ink)
    GdkColor white;
    gdk_color_parse("#ffffff", &white);
    gtk_widget_modify_bg(dialog, GTK_STATE_NORMAL, &white);

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content), 12);
    gtk_widget_modify_bg(content, GTK_STATE_NORMAL, &white);

    // Title label (wrapped in EventBox for e-ink)
    char title_markup[128];
    sprintf(title_markup, "<span size='20000' weight='bold'>%s</span>", title);
    GtkWidget *title_eb = gtk_event_box_new();
    gtk_widget_modify_bg(title_eb, GTK_STATE_NORMAL, &white);
    GtkWidget *title_lbl = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title_lbl), title_markup);
    gtk_misc_set_alignment(GTK_MISC(title_lbl), 0.0, 0.5);
    gtk_container_add(GTK_CONTAINER(title_eb), title_lbl);
    gtk_box_pack_start(GTK_BOX(content), title_eb, FALSE, FALSE, 0);

    // Separator
    GtkWidget *sep_eb = gtk_event_box_new();
    gtk_widget_modify_bg(sep_eb, GTK_STATE_NORMAL, &white);
    GtkWidget *sep = gtk_hseparator_new();
    gtk_container_add(GTK_CONTAINER(sep_eb), sep);
    gtk_box_pack_start(GTK_BOX(content), sep_eb, FALSE, FALSE, 4);

    // Calendar drawing area wrapped in EventBox + Frame for e-ink
    GtkWidget *cal_eb = gtk_event_box_new();
    gtk_widget_modify_bg(cal_eb, GTK_STATE_NORMAL, &white);
    GtkWidget *cal_frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(cal_frame), GTK_SHADOW_IN);
    gtk_container_add(GTK_CONTAINER(cal_eb), cal_frame);

    GtkWidget *cal = gtk_drawing_area_new();
    gtk_widget_set_size_request(cal, -1, 250);
    struct MonthCalData *mdata = (struct MonthCalData*)g_malloc(sizeof(struct MonthCalData));
    mdata->year = 2026;
    mdata->month = month;
    g_signal_connect_data(cal, "expose-event", G_CALLBACK(on_expose_month_cal),
                          mdata, (GClosureNotify)g_free, (GConnectFlags)0);
    gtk_container_add(GTK_CONTAINER(cal_frame), cal);
    gtk_box_pack_start(GTK_BOX(content), cal_eb, FALSE, FALSE, 0);

    // Spacer
    GtkWidget *sp_eb = gtk_event_box_new();
    gtk_widget_modify_bg(sp_eb, GTK_STATE_NORMAL, &white);
    gtk_container_add(GTK_CONTAINER(sp_eb), gtk_label_new(""));
    gtk_box_pack_start(GTK_BOX(content), sp_eb, FALSE, FALSE, 6);

    // Stats (wrapped in EventBox for e-ink)
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

    GtkWidget *stats_eb = gtk_event_box_new();
    gtk_widget_modify_bg(stats_eb, GTK_STATE_NORMAL, &white);
    GtkWidget *stats_lbl = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(stats_lbl), stats_text);
    gtk_misc_set_alignment(GTK_MISC(stats_lbl), 0.0, 0.0);
    gtk_container_add(GTK_CONTAINER(stats_eb), stats_lbl);
    gtk_box_pack_start(GTK_BOX(content), stats_eb, FALSE, FALSE, 4);

    // Get the action area and wrap the close button for e-ink
    GtkWidget *action_area = gtk_dialog_get_action_area(GTK_DIALOG(dialog));
    gtk_widget_modify_bg(action_area, GTK_STATE_NORMAL, &white);

    // Find and rewrap the close button
    GList *children = gtk_container_get_children(GTK_CONTAINER(action_area));
    for (GList *l = children; l; l = l->next) {
        if (GTK_IS_BUTTON(l->data)) {
            GtkWidget *btn = GTK_WIDGET(l->data);
            // Remove from action area
            gtk_container_remove(GTK_CONTAINER(action_area), btn);
            // Wrap in EventBox for e-ink
            GtkWidget *btn_eb = gtk_event_box_new();
            gtk_widget_modify_bg(btn_eb, GTK_STATE_NORMAL, &white);
            gtk_container_add(GTK_CONTAINER(btn_eb), btn);
            // Add back to action area
            gtk_box_pack_start(GTK_BOX(action_area), btn_eb, TRUE, FALSE, 0);
        }
    }
    g_list_free(children);

    gtk_widget_show_all(dialog);

    // Force e-ink full refresh so dialog appears
    while (gtk_events_pending()) gtk_main_iteration();
    system("eips -c");
    while (gtk_events_pending()) gtk_main_iteration();

    // Run dialog (blocking)
    gtk_dialog_run(GTK_DIALOG(dialog));

    // Force e-ink refresh after dialog closes
    gtk_widget_destroy(dialog);
    while (gtk_events_pending()) gtk_main_iteration();
    system("eips -c");
    while (gtk_events_pending()) gtk_main_iteration();
    gtk_widget_queue_draw(g_main_window);
}

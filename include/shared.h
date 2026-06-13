#ifndef KINDLESTATS_SHARED_H
#define KINDLESTATS_SHARED_H

#include <gtk/gtk.h>
#include <cairo.h>
#include <stdbool.h>

// ==================== Mock Data ====================

struct BookData {
    const char *title;
    const char *author;
    const char *lastRead;
    int timeMin;
    int progress;
    int trend[7];
    bool finished;
};

extern BookData g_books[];
extern const int NUM_BOOKS;

struct SessionData {
    const char *time;
    const char *duration;
    const char *book;
    const char *progress;
};

extern SessionData g_sessions_today[];
extern const int NUM_SESSIONS_TODAY;
extern SessionData g_sessions_yesterday[];
extern const int NUM_SESSIONS_YESTERDAY;
extern SessionData g_sessions_2days[];
extern const int NUM_SESSIONS_2DAYS;

// ==================== Globals ====================

extern GtkWidget *g_main_window;
extern GtkWidget *g_books_list_container;
extern GtkWidget *g_books_detail_container;
extern GtkWidget *g_books_page_widget;
extern GtkWidget *g_books_pgbar;
extern int g_book_page;
extern const int BOOKS_PER_PAGE;
extern bool g_filter_unfinished;
extern int g_sort_mode;
extern int g_current_book_detail;
extern int g_today_day_idx;
extern GtkWidget *g_today_date_label;
extern GtkWidget *g_today_timeline_container;
extern GtkWidget *g_today_page_label;
extern int g_today_page;
extern const int SESSIONS_PER_PAGE;

// ==================== Utilities ====================

void log_debug(const char* msg);
int days_in_month(int y, int m);
int first_weekday_of_month(int y, int m);
void parse_pango_color(const char* hex, GdkColor *out);
void destroy_double_ptr(gpointer data, GClosure *closure);

#endif

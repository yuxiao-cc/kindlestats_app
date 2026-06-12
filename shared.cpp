#include "shared.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// ==================== Mock Data ====================

BookData g_books[] = {
    {"三体", "刘慈欣", "今天 17:00", 2550, 85, {15,40,20,60,45,0,50}, false},
    {"卡拉马佐夫兄弟", "陀思妥耶夫斯基", "6月6日 20:30", 1695, 62, {30,0,45,15,60,25,0}, false},
    {"百年孤独", "加西亚·马尔克斯", "5月20日 18:00", 1125, 100, {20,30,40,20,15,10,25}, true},
    {"万历十五年", "黄仁宇", "6月8日 14:00", 860, 95, {10,15,30,0,45,25,15}, false},
    {"自私的基因", "理查德·道金斯", "6月5日 18:15", 610, 40, {15,20,0,30,10,15,20}, false},
    {"人类简史", "尤瓦尔·赫拉利", "5月28日 15:30", 535, 75, {0,40,20,30,0,15,10}, false},
    {"三体II：黑暗森林", "刘慈欣", "6月9日 17:15", 372, 15, {0,0,10,15,0,25,30}, false},
    {"红楼梦", "曹雪芹", "5月30日 20:00", 340, 8, {10,0,0,20,15,0,10}, false},
};
const int NUM_BOOKS = 8;

SessionData g_sessions_today[] = {
    {"20:05 - 20:35", "30 分钟", "三体", "82% → 85%"},
    {"09:30 - 10:15", "45 分钟", "卡拉马佐夫兄弟", "60% → 62%"},
    {"08:00 - 08:45", "45 分钟", "三体", "78% → 82%"},
    {"07:30 - 08:00", "30 分钟", "百年孤独", "95% → 100%"},
    {"22:00 - 22:30", "30 分钟", "万历十五年", "90% → 92%"},
    {"21:30 - 22:00", "30 分钟", "自私的基因", "35% → 38%"},
    {"15:00 - 15:45", "45 分钟", "人类简史", "70% → 75%"},
    {"14:00 - 14:30", "30 分钟", "三体II：黑暗森林", "10% → 15%"},
    {"12:00 - 12:30", "30 分钟", "红楼梦", "5% → 8%"},
    {"10:30 - 11:00", "30 分钟", "三体", "85% → 87%"},
};
const int NUM_SESSIONS_TODAY = 10;

SessionData g_sessions_yesterday[] = {
    {"21:00 - 21:50", "50 分钟", "三体", "75% → 80% (+5%)"},
    {"14:15 - 14:35", "20 分钟", "自私的基因", "38% → 40% (+2%)"},
    {"08:30 - 09:10", "40 分钟", "三体", "72% → 75% (+3%)"},
};
const int NUM_SESSIONS_YESTERDAY = 3;

SessionData g_sessions_2days[] = {
    {"19:00 - 19:55", "55 分钟", "万历十五年", "90% → 95% (+5%)"},
};
const int NUM_SESSIONS_2DAYS = 1;

// ==================== Globals ====================

GtkWidget *g_main_window = NULL;
GtkWidget *g_books_list_container = NULL;
GtkWidget *g_books_detail_container = NULL;
GtkWidget *g_books_page_widget = NULL;
int g_book_page = 0;
const int BOOKS_PER_PAGE = 4;
bool g_filter_unfinished = false;
int g_sort_mode = 0;
int g_current_book_detail = -1;
int g_today_day_idx = 0;
GtkWidget *g_today_date_label = NULL;
GtkWidget *g_today_timeline_container = NULL;
GtkWidget *g_today_page_label = NULL;
int g_today_page = 0;
const int SESSIONS_PER_PAGE = 5;

// ==================== Utilities ====================

void log_debug(const char* msg) {
    FILE *f = fopen("/mnt/us/kindlestats_debug.log", "a");
    if (!f) f = fopen("kindlestats_debug.log", "a");
    if (f) { fprintf(f, "%s\n", msg); fclose(f); }
}

int days_in_month(int y, int m) {
    int d[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (m == 2 && ((y%4==0 && y%100!=0) || y%400==0)) return 29;
    return d[m-1];
}

int first_weekday_of_month(int y, int m) {
    struct tm t = {};
    t.tm_year = y - 1900;
    t.tm_mon = m - 1;
    t.tm_mday = 1;
    mktime(&t);
    return (t.tm_wday + 6) % 7; // 0=Mon ... 6=Sun
}

void parse_pango_color(const char* hex, GdkColor *out) {
    gdk_color_parse(hex, out);
}

void destroy_double_ptr(gpointer data, GClosure *closure) {
    g_free(data);
}

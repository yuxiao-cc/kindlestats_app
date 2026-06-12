#include "shared.h"
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

// ==================== Export-backed library loading ====================

namespace {
struct SessionAgg {
    int total_minutes = 0;
    int trend[7] = {0, 0, 0, 0, 0, 0, 0};
    time_t last_end = 0;
};

static bool read_text_file(const char* path, std::string* out) {
    if (!path || !out) return false;
    std::ifstream ifs(path, std::ios::in | std::ios::binary);
    if (!ifs) return false;
    std::ostringstream ss;
    ss << ifs.rdbuf();
    *out = ss.str();
    return true;
}

static std::string extract_json_string(const std::string& src, const char* key) {
    std::string needle = std::string("\"") + key + "\"";
    size_t pos = src.find(needle);
    if (pos == std::string::npos) return "";
    pos = src.find(':', pos);
    if (pos == std::string::npos) return "";
    pos = src.find('"', pos);
    if (pos == std::string::npos) return "";
    size_t end = pos + 1;
    for (; end < src.size(); ++end) {
        if (src[end] == '"' && src[end - 1] != '\\') break;
    }
    if (end >= src.size()) return "";
    return src.substr(pos + 1, end - pos - 1);
}

static int extract_json_int(const std::string& src, const char* key, int def_value) {
    std::string needle = std::string("\"") + key + "\"";
    size_t pos = src.find(needle);
    if (pos == std::string::npos) return def_value;
    pos = src.find(':', pos);
    if (pos == std::string::npos) return def_value;
    ++pos;
    while (pos < src.size() && (src[pos] == ' ' || src[pos] == '\t')) ++pos;
    size_t end = pos;
    while (end < src.size() && ((src[end] >= '0' && src[end] <= '9') || src[end] == '-')) ++end;
    if (end == pos) return def_value;
    return atoi(src.substr(pos, end - pos).c_str());
}

static std::string format_timestamp(time_t ts) {
    char buf[64];
    struct tm *tmv = localtime(&ts);
    if (!tmv) {
        sprintf(buf, "%ld", (long)ts);
        return buf;
    }
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", tmv);
    return buf;
}

static const char* dup_cstr(const std::string& s) {
    return g_strdup(s.c_str());
}

static void parse_sessions_jsonl(std::unordered_map<std::string, SessionAgg>& sessions) {
    std::string raw;
    if (!read_text_file("E:\\extensions\\kindlestats\\data\\sessions.jsonl", &raw)) return;

    std::istringstream iss(raw);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.find("\"book_uuid\"") == std::string::npos) continue;
        std::string uuid = extract_json_string(line, "book_uuid");
        if (uuid.empty()) continue;

        int duration_sec = extract_json_int(line, "duration_sec", 0);
        int session_end = extract_json_int(line, "session_end", 0);

        SessionAgg &agg = sessions[uuid];
        agg.total_minutes += std::max(0, duration_sec / 60);
        if (session_end > 0 && (time_t)session_end > agg.last_end) {
            agg.last_end = (time_t)session_end;
        }

        time_t ts = (time_t)session_end;
        struct tm *tmv = localtime(&ts);
        if (tmv) {
            int wd = tmv->tm_wday; // 0=Sun ... 6=Sat
            if (wd < 0 || wd > 6) wd = 0;
            agg.trend[wd] += std::max(0, duration_sec / 60);
        }
    }
}

static void load_from_export() {
    std::string raw;
    if (!read_text_file("E:\\extensions\\kindlestats\\export\\books.json", &raw)) return;

    std::unordered_map<std::string, SessionAgg> sessions;
    parse_sessions_jsonl(sessions);
    std::unordered_set<std::string> seen;

    std::regex obj_re(R"(\{[^{}]*"book_uuid"[^{}]*\})");
    auto begin = std::sregex_iterator(raw.begin(), raw.end(), obj_re);
    auto end = std::sregex_iterator();

    for (auto it = begin; it != end; ++it) {
        const std::string obj = it->str();
        std::string uuid = extract_json_string(obj, "book_uuid");
        if (uuid.empty()) continue;
        if (seen.find(uuid) != seen.end()) continue;
        seen.insert(uuid);

        BookData book = {};
        book.uuid = dup_cstr(uuid);
        book.title = dup_cstr(extract_json_string(obj, "title"));
        book.author = dup_cstr(extract_json_string(obj, "authors"));
        book.thumbnail_path = dup_cstr(extract_json_string(obj, "thumbnail_path"));
        book.progress = extract_json_int(obj, "percent_finished", 0);
        book.finished = book.progress >= 100;

        int last_access = extract_json_int(obj, "last_access", 0);
        std::string last_read = "-";
        auto sit = sessions.find(uuid);
        if (sit != sessions.end()) {
            book.timeMin = std::max(0, sit->second.total_minutes);
            for (int i = 0; i < 7; ++i) book.trend[i] = sit->second.trend[i];
            if (sit->second.last_end > 0) {
                last_read = format_timestamp(sit->second.last_end);
            }
        }
        if (last_read == "-" && last_access > 0) {
            last_read = format_timestamp((time_t)last_access);
        }
        book.lastRead = dup_cstr(last_read);

        g_books.push_back(book);
    }

    if (!g_books.empty()) {
        log_debug("Loaded book library from export");
    }
}
} // namespace

// ==================== Mock Data ====================

// Real library entries are loaded at runtime from the Kindle export.
std::vector<BookData> g_books;

SessionData g_sessions_today[] = {
    {"20:05 - 20:35", "30 分钟", "三体", "82% -> 85%"},
    {"09:30 - 10:15", "45 分钟", "卡拉马佐夫兄弟", "60% -> 62%"},
    {"08:00 - 08:45", "45 分钟", "三体", "78% -> 82%"},
    {"07:30 - 08:00", "30 分钟", "百年孤独", "95% -> 100%"},
    {"22:00 - 22:30", "30 分钟", "万历十五年", "90% -> 92%"},
    {"21:30 - 22:00", "30 分钟", "自私的基因", "35% -> 38%"},
    {"15:00 - 15:45", "45 分钟", "人类简史", "70% -> 75%"},
    {"14:00 - 14:30", "30 分钟", "三体II：黑暗森林", "10% -> 15%"},
    {"12:00 - 12:30", "30 分钟", "红楼梦", "5% -> 8%"},
    {"10:30 - 11:00", "30 分钟", "三体", "85% -> 87%"},
};
const int NUM_SESSIONS_TODAY = 10;

SessionData g_sessions_yesterday[] = {
    {"21:00 - 21:50", "50 分钟", "三体", "75% -> 80% (+5%)"},
    {"14:15 - 14:35", "20 分钟", "自私的基因", "38% -> 40% (+2%)"},
    {"08:30 - 09:10", "40 分钟", "三体", "72% -> 75% (+3%)"},
};
const int NUM_SESSIONS_YESTERDAY = 3;

SessionData g_sessions_2days[] = {
    {"19:00 - 19:55", "55 分钟", "万历十五年", "90% -> 95% (+5%)"},
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

void load_book_library() {
    g_books.clear();
    load_from_export();
    if (g_books.empty()) {
        log_debug("Book export missing or empty");
    }
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

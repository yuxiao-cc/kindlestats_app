#include "shared.h"
#include "draw_callbacks.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// ==================== Heatmap (annual grid) ====================

gboolean on_expose_heatmap(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    cairo_t *cr = gdk_cairo_create(widget->window);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    double pad_x = 0.0, pad_y = 5.0;
    double grid_w = widget->allocation.width - pad_x * 2;
    double cell = (grid_w - 52 * 2.0) / 53.0;

    for (int col = 0; col < 53; ++col) {
        for (int row = 0; row < 7; ++row) {
            double x = pad_x + col * (cell + 2.0);
            double y = pad_y + row * (cell + 2.0);
            double seed = sin(col * 7 + row + 2026.0);
            if (seed > 0.85) cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
            else if (seed > 0.55) cairo_set_source_rgb(cr, 0.4, 0.4, 0.4);
            else if (seed > 0.25) cairo_set_source_rgb(cr, 0.66, 0.66, 0.66);
            else if (seed > -0.15) cairo_set_source_rgb(cr, 0.86, 0.86, 0.86);
            else cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
            cairo_rectangle(cr, x, y, cell, cell);
            cairo_fill_preserve(cr);
            cairo_set_source_rgb(cr, 0.88, 0.88, 0.88);
            cairo_set_line_width(cr, 1.0);
            cairo_stroke(cr);
        }
    }
    cairo_destroy(cr);
    return FALSE;
}

// ==================== 24h Bar Chart ====================

gboolean on_expose_bar24(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    cairo_t *cr = gdk_cairo_create(widget->window);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    double pad_x = 10.0, pad_y = 10.0;
    double area_w = widget->allocation.width - pad_x * 2;
    double area_h = widget->allocation.height - pad_y * 2 - 40.0;
    double base_y = widget->allocation.height - pad_y - 28.0;

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 2);
    cairo_move_to(cr, pad_x, base_y);
    cairo_line_to(cr, widget->allocation.width - pad_x, base_y);
    cairo_stroke(cr);

    double mock_data[24] = {5,2,0,0,0,0,5,15,30,20,10,15,25,40,20,10,15,25,60,80,95,85,40,15};
    double bar_w = (area_w - 23 * 2.0) / 24.0;

    cairo_set_font_size(cr, 22);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);

    for (int i = 0; i < 24; i++) {
        double x = pad_x + i * (bar_w + 2.0);
        double h = (mock_data[i] / 100.0) * area_h;
        cairo_rectangle(cr, x, base_y - h, bar_w, h);
        cairo_fill(cr);

        if (i % 4 == 0 || i == 23) {
            char lbl[8];
            sprintf(lbl, "%d", i);
            cairo_text_extents_t ext;
            cairo_text_extents(cr, lbl, &ext);
            // Position label clearly below the axis line with extra room
            cairo_move_to(cr, x + bar_w/2.0 - ext.width/2.0, base_y + 22.0);
            cairo_show_text(cr, lbl);
        }
    }
    cairo_destroy(cr);
    return FALSE;
}

// ==================== 7-day Horizontal Bar ====================

gboolean on_expose_hbar(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    cairo_t *cr = gdk_cairo_create(widget->window);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    const char* days[] = {"周五","周六","周日","周一","周二","周三","今日"};
    double vals[] = {45, 12, 90, 35, 50, 70, 80};

    double start_y = 16.0;
    double row_h = (widget->allocation.height - start_y - 10.0) / 7.0;

    cairo_set_font_size(cr, 26);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);

    for (int i = 0; i < 7; i++) {
        double y = start_y + i * row_h;
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_move_to(cr, 12, y + row_h/2.0 + 9.0);
        cairo_show_text(cr, days[i]);

        double tx = 110, tw = widget->allocation.width - 200, th = 26;
        double ty = y + (row_h - th)/2.0;
        double fw = tw * (vals[i] / 100.0);
        cairo_set_source_rgb(cr, i%2==0 ? 0.35 : 0.0, i%2==0 ? 0.35 : 0.0, i%2==0 ? 0.35 : 0.0);
        cairo_rectangle(cr, tx, ty, fw, th);
        cairo_fill_preserve(cr);
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_set_line_width(cr, 1.5);
        cairo_stroke(cr);

        char vs[16];
        sprintf(vs, "%.0fmin", vals[i]);
        cairo_text_extents_t ext;
        cairo_text_extents(cr, vs, &ext);
        cairo_move_to(cr, tx + tw + 10, y + row_h/2.0 + 9.0);
        cairo_show_text(cr, vs);
    }
    cairo_destroy(cr);
    return FALSE;
}

// ==================== Book Cover (mini) ====================

gboolean on_expose_cover(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    int idx = GPOINTER_TO_INT(data);
    if (idx < 0 || idx >= NUM_BOOKS) return FALSE;
    cairo_t *cr = gdk_cairo_create(widget->window);

    // Background
    cairo_set_source_rgb(cr, 0.88, 0.88, 0.88);
    cairo_paint(cr);
    // Border
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 2);
    cairo_rectangle(cr, 1, 1, widget->allocation.width - 2, widget->allocation.height - 2);
    cairo_stroke(cr);
    // Spine
    cairo_set_source_rgb(cr, 0.65, 0.65, 0.65);
    cairo_rectangle(cr, 1, 1, 5, widget->allocation.height - 2);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 1);
    cairo_move_to(cr, 6, 1);
    cairo_line_to(cr, 6, widget->allocation.height - 1);
    cairo_stroke(cr);

    // Title text
    cairo_set_font_size(cr, 9);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_text_extents_t ext;
    cairo_text_extents(cr, g_books[idx].title, &ext);
    double tx = widget->allocation.width / 2.0 + 2 - ext.width / 2.0;
    if (tx < 8) tx = 8;
    double ty = widget->allocation.height / 2.0 + ext.height / 2.0;
    cairo_move_to(cr, tx, ty);
    cairo_show_text(cr, g_books[idx].title);

    cairo_destroy(cr);
    return FALSE;
}

// ==================== Progress Bar ====================

gboolean on_expose_progress(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    double progress = *(double*)data;
    cairo_t *cr = gdk_cairo_create(widget->window);

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    double pw = widget->allocation.width;
    double ph = 15;

    cairo_set_source_rgb(cr, 0, 0, 0);
    double bar_y = (widget->allocation.height - ph) / 2.0;
    cairo_rectangle(cr, 0, bar_y, pw * progress, ph);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 1);
    cairo_rectangle(cr, 0, bar_y, pw, ph);
    cairo_stroke(cr);

    cairo_destroy(cr);
    return FALSE;
}

// ==================== Timeline Dot + Line ====================

gboolean on_expose_tl_dot(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    int flags = GPOINTER_TO_INT(data);
    cairo_t *cr = gdk_cairo_create(widget->window);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    double cx = widget->allocation.width / 2.0;
    double cy = widget->allocation.height / 2.0; // Center of row
    double h = widget->allocation.height;
    
    // flags: bit0=is_global_first, bit1=is_last(page), bit2=not_first_page_first
    int is_global_first = flags & 1;
    int is_last = (flags >> 1) & 1;
    int is_page_first_no_top = (flags >> 2) & 1;
    
    // Draw vertical line
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 2);
    if (is_global_first && is_last) {
        // Only one item total, no line
    } else if (is_page_first_no_top) {
        // First item on non-first page: only line down
        cairo_move_to(cr, cx, cy);
        cairo_line_to(cr, cx, h + 80);
    } else if (is_global_first) {
        // Global first: line from center to bottom
        cairo_move_to(cr, cx, cy);
        cairo_line_to(cr, cx, h + 80);
    } else if (is_last) {
        // Last: line from top to center
        cairo_move_to(cr, cx, -80);
        cairo_line_to(cr, cx, cy);
    } else {
        // Middle: line through entire height
        cairo_move_to(cr, cx, -80);
        cairo_line_to(cr, cx, h + 80);
    }
    cairo_stroke(cr);
    
    // Draw dot at center
    if (is_global_first) cairo_set_source_rgb(cr, 0, 0, 0);
    else cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_arc(cr, cx, cy, 8, 0, 2 * G_PI);
    cairo_fill_preserve(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 2);
    cairo_stroke(cr);

    cairo_destroy(cr);
    return FALSE;
}

// ==================== Monthly Calendar (in dialog) ====================

gboolean on_expose_month_cal(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    struct MonthCalData *md = (struct MonthCalData*)data;
    cairo_t *cr = gdk_cairo_create(widget->window);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    int fw = first_weekday_of_month(md->year, md->month);
    int dm = days_in_month(md->year, md->month);
    double w = widget->allocation.width;
    double h = widget->allocation.height;
    double cw = w / 7.0;
    double ch = (h - 28) / 6.0;

    // Day headers
    const char* dh[] = {"一","二","三","四","五","六","日"};
    cairo_set_font_size(cr, 14);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
    for (int i = 0; i < 7; i++) {
        cairo_text_extents_t ext;
        cairo_text_extents(cr, dh[i], &ext);
        cairo_move_to(cr, i * cw + cw/2 - ext.width/2, 18);
        cairo_show_text(cr, dh[i]);
    }

    // Separator
    cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
    cairo_set_line_width(cr, 1);
    cairo_move_to(cr, 5, 26);
    cairo_line_to(cr, w - 5, 26);
    cairo_stroke(cr);

    // Day cells
    cairo_set_font_size(cr, 13);
    int row = 0, col = fw;
    for (int day = 1; day <= dm; day++) {
        double x = col * cw;
        double y = 28 + row * ch;

        double seed = sin(day + md->month + md->year);
        double gc;
        if (seed > 0.7) gc = 0.0;
        else if (seed > 0.4) gc = 0.4;
        else if (seed > 0.1) gc = 0.66;
        else if (seed > -0.3) gc = 0.86;
        else gc = 1.0;

        cairo_set_source_rgb(cr, gc, gc, gc);
        cairo_rectangle(cr, x + 2, y + 2, cw - 4, ch - 4);
        cairo_fill(cr);

        cairo_set_source_rgb(cr, 0.75, 0.75, 0.75);
        cairo_set_line_width(cr, 0.5);
        cairo_rectangle(cr, x + 2, y + 2, cw - 4, ch - 4);
        cairo_stroke(cr);

        // Day number
        char ds[4];
        sprintf(ds, "%d", day);
        cairo_set_source_rgb(cr, gc < 0.5 ? 1 : 0, gc < 0.5 ? 1 : 0, gc < 0.5 ? 1 : 0);
        cairo_text_extents_t ext;
        cairo_text_extents(cr, ds, &ext);
        cairo_move_to(cr, x + cw/2 - ext.width/2, y + ch/2 + ext.height/2);
        cairo_show_text(cr, ds);

        col++;
        if (col >= 7) { col = 0; row++; }
    }

    cairo_destroy(cr);
    return FALSE;
}

// ==================== Mini Trend Bars (book detail) ====================

gboolean on_expose_mini_trend(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    struct TrendData *td = (struct TrendData*)data;
    cairo_t *cr = gdk_cairo_create(widget->window);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    double pw = widget->allocation.width;
    double ph = widget->allocation.height;
    double slot = (pw - 6 * 16) / 7;
    double bw = slot * 0.35;
    double maxv = 0;
    for (int i = 0; i < 7; i++) if (td->trend[i] > maxv) maxv = td->trend[i];
    if (maxv == 0) maxv = 1;

    // Title inside chart
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_font_size(cr, 16);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_move_to(cr, 8, 18);
    cairo_show_text(cr, "单本近7次阅读趋势");

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 1);
    cairo_move_to(cr, 8, ph - 20);
    cairo_line_to(cr, pw - 8, ph - 20);
    cairo_stroke(cr);

    cairo_set_font_size(cr, 14);

    for (int i = 0; i < 7; i++) {
        double x = 8 + i * (slot + 16) + (slot - bw) / 2;
        double h = (td->trend[i] / maxv) * (ph - 42);
        double y = ph - 20 - h;
        cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
        cairo_rectangle(cr, x, y, bw, h);
        cairo_fill(cr);
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_set_line_width(cr, 1);
        cairo_rectangle(cr, x, y, bw, h);
        cairo_stroke(cr);

        char lbl[8];
        if (td->trend[i] > 0) sprintf(lbl, "%dm", td->trend[i]);
        else strcpy(lbl, "-");
        cairo_text_extents_t ext;
        cairo_text_extents(cr, lbl, &ext);
        cairo_move_to(cr, x + bw/2 - ext.width/2, ph - 5);
        cairo_show_text(cr, lbl);
    }

    cairo_destroy(cr);
    return FALSE;
}

// ==================== Pie/Donut Chart (session time distribution) ====================

gboolean on_expose_pie(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    int *pidx = (int*)data;
    int idx = *pidx;
    if (idx < 0 || idx >= NUM_BOOKS) return FALSE;

    cairo_t *cr = gdk_cairo_create(widget->window);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    double cx = widget->allocation.width / 2.0;
    double cy = widget->allocation.height / 2.0;
    double r = MIN(cx, cy) - 10;

    // Background ring
    cairo_set_source_rgb(cr, 0.88, 0.88, 0.88);
    cairo_set_line_width(cr, 16);
    cairo_arc(cr, cx, cy, r, 0, 2 * G_PI);
    cairo_stroke(cr);

    // Progress arc
    double frac = g_books[idx].progress / 100.0;
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 16);
    cairo_arc(cr, cx, cy, r, -G_PI/2, -G_PI/2 + 2 * G_PI * frac);
    cairo_stroke(cr);

    // Center text
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    char buf[8];
    sprintf(buf, "%d%%", g_books[idx].progress);
    cairo_set_font_size(cr, 20);
    cairo_text_extents_t ext;
    cairo_text_extents(cr, buf, &ext);
    cairo_move_to(cr, cx - ext.width/2, cy + ext.height/2);
    cairo_show_text(cr, buf);

    cairo_destroy(cr);
    return FALSE;
}

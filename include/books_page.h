#ifndef KINDLESTATS_BOOKS_PAGE_H
#define KINDLESTATS_BOOKS_PAGE_H

#include <gtk/gtk.h>

GtkWidget* create_books_page();
void back_to_book_list();
void show_book_detail(int book_idx);
void rebuild_book_list();
void update_book_detail_content(int book_idx);

#endif

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <charl.h>
#include <host.h>
#include <crypto.h>

#include "gui.h"
#include "parser.h"
#include "operator.h"
#include "log.h"

/*******************************************************************************
 ** BEGIN CALLBACKS
 ******************************************************************************/

static void destroy (GtkWidget *src, gpointer data)
{
        gtk_widget_destroy(src);
        gtk_main_quit();
}

static void line (GtkWidget *src, gpointer data)
{
        GtkEntryBuffer *buf = gtk_entry_get_buffer(GTK_ENTRY(src));
        const char *content = gtk_entry_buffer_get_text(buf);

        printf("Line: %s\n", content);
        parser_out(content);

        gtk_entry_buffer_delete_text(buf, 0, strlen(content));
}

static void autoscroll (GtkWidget *src, GdkRectangle *rect, gpointer data)
{
        GtkScrolledWindow *scroll = data;
        GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(scroll);

        gdouble page_size = gtk_adjustment_get_page_size(adj);
        gdouble upper = gtk_adjustment_get_upper(adj);

        gtk_adjustment_set_value(adj, upper - page_size);
}

static gboolean update (gpointer data)
{
        if (connected) {
                parser_in();
        }

        log_refresh();
        return TRUE;
}

/*******************************************************************************
 ** END CALLBACKS
 ******************************************************************************/

/**
 *  Construct and maintain the primary interface. Create any necessary callbacks
 *  and signals or modules.
 */
void gui ()
{
        GtkWidget *win, *vbox, *hbox, *scroll;
        GtkWidget *field, *view, *list;
        GtkFunction func;

        gtk_init(NULL, NULL);
        crypto_init();

        win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(win), "Charl");
        gtk_container_set_border_width(GTK_CONTAINER(win), 10);
        gtk_window_set_default_size(GTK_WINDOW(win), 600, 300);
        g_signal_connect(win, "destroy", G_CALLBACK(destroy), NULL);

        vbox = gtk_vbox_new(FALSE, 5);
        hbox = gtk_hbox_new(FALSE, 5);

        field = gtk_entry_new_with_max_length(MAX_BUF - (MAX_NAME * 2) - 1);
        g_signal_connect(field, "activate", G_CALLBACK(line), NULL);

        view = gtk_text_view_new();
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD_CHAR);
        gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
        gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);
        gtk_text_view_set_left_margin(GTK_TEXT_VIEW(view), 3);
        gtk_widget_set_size_request(view, 300, 60);

        list = gtk_text_view_new();
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(list), GTK_WRAP_WORD_CHAR);
        gtk_text_view_set_editable(GTK_TEXT_VIEW(list), FALSE);
        gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(list), FALSE);
        gtk_text_view_set_left_margin(GTK_TEXT_VIEW(list), 3);
        gtk_widget_set_size_request(list, 150, 60);

        scroll = gtk_scrolled_window_new(NULL, NULL);
        gtk_container_add(GTK_CONTAINER(scroll), view);
        g_signal_connect(view, "size-allocate", G_CALLBACK(autoscroll), scroll);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                       GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

        gtk_box_pack_start(GTK_BOX(hbox), scroll, TRUE, TRUE, 5);
        gtk_box_pack_end(GTK_BOX(hbox), list, FALSE, FALSE, 5);

        gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 5);
        gtk_box_pack_end(GTK_BOX(vbox), field, FALSE, FALSE, 5);

        gtk_container_add(GTK_CONTAINER(win), vbox);

        gtk_widget_show(list);
        gtk_widget_show(view);
        gtk_widget_show(scroll);
        gtk_widget_show(field);
        gtk_widget_show(hbox);
        gtk_widget_show(vbox);
        gtk_widget_show(win);

        log_init(view, list);

        func = update;
        gtk_timeout_add(100, func, NULL);

        gtk_widget_grab_focus(field);

        gtk_main();
        crypto_close();
}
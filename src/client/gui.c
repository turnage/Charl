#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>

#include "charl.h"
#include "host.h"
#include "crypto.h"
#include "gui.h"
#include "parser.h"
#include "operator.h"
#include "log.h"

static char address[MAX_BUF / 3] = {0};
static char name[MAX_NAME] = {0};
static char pass[MAX_PASS] = {0};

static char channel[MAX_CHAN] = {0};
static char channel_pass[MAX_PASS] = {0};

static int open_connect = 0;
static int open_join = 0;

/*******************************************************************************
 ** BEGIN CALLBACKS
 ******************************************************************************/

static void destroy (GtkWidget *src, gpointer data)
{
        gtk_widget_destroy(src);
        gtk_main_quit();
}

static void soft_destroy (GtkWidget *src, gpointer data)
{
        gtk_widget_destroy((GtkWidget *)data);
        open_connect = 0;
        open_join = 0;
}

static void line (GtkWidget *src, gpointer data)
{
        GtkEntryBuffer *buf = gtk_entry_get_buffer(GTK_ENTRY(src));
        const char *content = gtk_entry_buffer_get_text(buf);

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

static void fieldcheck (GtkWidget *src, gpointer data)
{
        char *line = data;
        memset(line, 0, strlen(line));
        sprintf(line, "%s", gtk_entry_get_text(GTK_ENTRY(src)));
}

static void connect_button (GtkWidget *src, gpointer data)
{
        char command[MAX_BUF] = {0};

        sprintf(command, "/c %s", address);
        parser_out(command);
        memset(address, 0, MAX_BUF / 3);

        if (connected) {
                memset(command, 0, strlen(command));
                sprintf(command, "/login %s %s", name, pass);
                parser_out(command);
                memset(name, 0, MAX_NAME);
                memset(pass, 0, MAX_PASS);
        }

        open_connect = 0;
}

static void join_button (GtkWidget *src, gpointer data)
{
        char command[MAX_BUF] = {0};

        sprintf(command, "/hop %s %s", channel, channel_pass);
        parser_out(command);
        memset(channel, 0, MAX_CHAN);
        memset(channel_pass, 0, MAX_PASS);

        open_join = 0;
}

static void disconnect_button (GtkWidget *src, gpointer data)
{
        parser_out("/disconnect");
}

static void connect_win (GtkMenuItem *item, gpointer data)
{
        if (!open_connect && !open_join && !connected) {
                open_connect = 1;

                GtkWidget *win;
                GtkWidget *hbox, *vboxa, *vboxb, *vboxc;
                GtkWidget *field_address, *field_name, *field_pass;
                GtkWidget *label_address, *label_name, *label_pass;
                GtkWidget *submit;

                win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
                gtk_window_set_title(GTK_WINDOW(win), "Connect");
                gtk_container_set_border_width(GTK_CONTAINER(win), 10);
                gtk_window_set_default_size(GTK_WINDOW(win), 400, 200);
                gtk_window_set_resizable(GTK_WINDOW(win), FALSE);
                g_signal_connect(win, "destroy", G_CALLBACK(soft_destroy), win);

                field_address = gtk_entry_new_with_max_length(MAX_BUF / 3);
                g_signal_connect(field_address, "changed",
                                 G_CALLBACK(fieldcheck), address);

                field_name = gtk_entry_new_with_max_length(MAX_NAME - 1);
                g_signal_connect(field_name, "changed",
                                 G_CALLBACK(fieldcheck), name);

                field_pass = gtk_entry_new_with_max_length(MAX_PASS - 1);
                gtk_entry_set_visibility(GTK_ENTRY(field_pass), FALSE);
                gtk_entry_set_invisible_char(GTK_ENTRY(field_pass), '-');
                g_signal_connect(field_pass, "changed",
                                 G_CALLBACK(fieldcheck), pass);

                label_address = gtk_label_new("Address");
                label_name = gtk_label_new("Name");
                label_pass = gtk_label_new("Password");        

                submit = gtk_button_new_with_label("Connect");
                g_signal_connect(submit, "clicked",
                                 G_CALLBACK(connect_button), NULL);
                g_signal_connect(submit, "clicked",
                                 G_CALLBACK(soft_destroy), win);

                vboxa = gtk_vbox_new(FALSE, 5);
                vboxb = gtk_vbox_new(FALSE, 5);
                vboxc = gtk_vbox_new(FALSE, 5);
                hbox = gtk_hbox_new(FALSE, 5);

                gtk_container_add(GTK_CONTAINER(vboxa), label_address);
                gtk_container_add(GTK_CONTAINER(vboxa), label_name);
                gtk_container_add(GTK_CONTAINER(vboxa), label_pass);

                gtk_container_add(GTK_CONTAINER(vboxb), field_address);
                gtk_container_add(GTK_CONTAINER(vboxb), field_name);
                gtk_container_add(GTK_CONTAINER(vboxb), field_pass);

                gtk_container_add(GTK_CONTAINER(hbox), vboxa);
                gtk_container_add(GTK_CONTAINER(hbox), vboxb);

                gtk_container_add(GTK_CONTAINER(vboxc), hbox);
                gtk_container_add(GTK_CONTAINER(vboxc), submit);

                gtk_container_add(GTK_CONTAINER(win), vboxc);

                gtk_widget_show_all(win);
        } else if (connected)
                log_add(COL_ERROR, "You're already connected somewhere!\n");
}

static void join_win (GtkMenuItem *item, gpointer data)
{
        if (!open_join && !open_connect) {
                open_join = 1;

                GtkWidget *win;
                GtkWidget *hbox, *vboxa, *vboxb, *vboxc;
                GtkWidget *field_address, *field_pass;
                GtkWidget *label_channel, *label_pass;
                GtkWidget *submit;

                win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
                gtk_window_set_title(GTK_WINDOW(win), "Connect");
                gtk_container_set_border_width(GTK_CONTAINER(win), 10);
                gtk_window_set_default_size(GTK_WINDOW(win), 400, 200);
                gtk_window_set_resizable(GTK_WINDOW(win), FALSE);
                g_signal_connect(win, "destroy", G_CALLBACK(soft_destroy), win);

                field_address = gtk_entry_new_with_max_length(MAX_CHAN - 1);
                g_signal_connect(field_address, "changed",
                                 G_CALLBACK(fieldcheck), channel);

                field_pass = gtk_entry_new_with_max_length(MAX_PASS - 1);
                gtk_entry_set_visibility(GTK_ENTRY(field_pass), FALSE);
                gtk_entry_set_invisible_char(GTK_ENTRY(field_pass), '-');
                g_signal_connect(field_pass, "changed",
                                 G_CALLBACK(fieldcheck), channel_pass);

                label_channel = gtk_label_new("Channel");
                label_pass = gtk_label_new("Password");        

                submit = gtk_button_new_with_label("Connect");
                g_signal_connect(submit, "clicked",
                                 G_CALLBACK(join_button), NULL);
                g_signal_connect(submit, "clicked",
                                 G_CALLBACK(soft_destroy), win);

                vboxa = gtk_vbox_new(FALSE, 5);
                vboxb = gtk_vbox_new(FALSE, 5);
                vboxc = gtk_vbox_new(FALSE, 5);
                hbox = gtk_hbox_new(FALSE, 5);

                gtk_container_add(GTK_CONTAINER(vboxa), label_channel);
                gtk_container_add(GTK_CONTAINER(vboxa), label_pass);

                gtk_container_add(GTK_CONTAINER(vboxb), field_address);
                gtk_container_add(GTK_CONTAINER(vboxb), field_pass);

                gtk_container_add(GTK_CONTAINER(hbox), vboxa);
                gtk_container_add(GTK_CONTAINER(hbox), vboxb);

                gtk_container_add(GTK_CONTAINER(vboxc), hbox);
                gtk_container_add(GTK_CONTAINER(vboxc), submit);

                gtk_container_add(GTK_CONTAINER(win), vboxc);

                gtk_widget_show_all(win);
        }
}

/*******************************************************************************
 ** END CALLBACKS
 ******************************************************************************/

/**
 *  Construct and maintain the primary interface. Create any necessary callbacks
 *  and signals or modules.
 */
void gui (void)
{
        GtkWidget *win, *vbox, *hbox, *scroll;
        GtkWidget *field, *view, *list, *menu;
        GtkToolItem *connect, *join, *disconnect;
        GError *err;
        GtkFunction func;

        gtk_init(NULL, NULL);
        crypto_init();

        gtk_window_set_default_icon_from_file("icon.ico", &err);

        connect = gtk_tool_button_new_from_stock(GTK_STOCK_NETWORK);
        gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(connect), "Connect");
        g_signal_connect(connect, "clicked", G_CALLBACK(connect_win), NULL);

        join = gtk_tool_button_new_from_stock(GTK_STOCK_JUMP_TO);
        gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(join), "Join");
        g_signal_connect(join, "clicked", G_CALLBACK(join_win), NULL);

        disconnect = gtk_tool_button_new_from_stock(GTK_STOCK_CANCEL);
        gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(disconnect), "Leave");
        g_signal_connect(disconnect, "clicked",
                         G_CALLBACK(disconnect_button), NULL);

        menu = gtk_toolbar_new();
        gtk_toolbar_insert(GTK_TOOLBAR(menu), connect, -1);
        gtk_toolbar_insert(GTK_TOOLBAR(menu), join, -1);
        gtk_toolbar_insert(GTK_TOOLBAR(menu), disconnect, -1);
        gtk_toolbar_set_style(GTK_TOOLBAR(menu), GTK_TOOLBAR_ICONS);

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


        gtk_box_pack_start(GTK_BOX(vbox), menu, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 5);
        gtk_box_pack_end(GTK_BOX(vbox), field, FALSE, FALSE, 5);

        gtk_container_add(GTK_CONTAINER(win), vbox);

        gtk_widget_show_all(win);

        log_init(view, list);

        func = update;
        gtk_timeout_add(100, func, NULL);

        gtk_widget_grab_focus(field);

        gtk_main();
        crypto_close();
}

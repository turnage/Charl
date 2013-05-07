#include <string.h>
#include <stdarg.h>

#include "log.h"

static GtkTextBuffer *side;
static GtkTextView *view;
static GtkTextBuffer *buf;
static GtkTextIter it;

static const char *tags[COL_PRIV] = {0};
static color_id themes[MAX_BUF] = {0};
static char alog[MAX_BUF][MAX_BUF] = {{0}};
static int size = 0;
static char list[MAX_BUF] = {0};

/**
 *  Establish a connection to the log window.
 *  @wid: pointer to the log view
 *  @sid: pointer to the peer list
 */
void log_init (GtkWidget *wid, GtkWidget *sid)
{
        view = GTK_TEXT_VIEW(wid);
        buf = gtk_text_view_get_buffer(view);
        side = gtk_text_view_get_buffer(GTK_TEXT_VIEW(sid));

        /* primary window colors */
        gtk_text_buffer_create_tag(buf, "none", "foreground", "black", NULL);
        gtk_text_buffer_create_tag(buf, "error", "foreground", "red", NULL);
        gtk_text_buffer_create_tag(buf, "notif", "foreground", "blue", NULL);
        gtk_text_buffer_create_tag(buf, "anon", "foreground", "#CCCCCC", NULL);
        gtk_text_buffer_create_tag(buf, "name", "foreground", "#3D0000", NULL);
        gtk_text_buffer_create_tag(buf, "priv", "foreground", "#3B3B3B", NULL);

        /* side window colors */
        gtk_text_buffer_create_tag(side, "anon", "foreground", "#CCCCCC", NULL);
        gtk_text_buffer_create_tag(side, "name", "foreground", "#3D0000", NULL);


        tags[COL_NONE] = "none";
        tags[COL_ERROR] = "error";
        tags[COL_NOTIF] = "notif";
        tags[COL_ANON] = "anon";
        tags[COL_NAME] = "name";
        tags[COL_PRIV] = "priv";

        gtk_text_buffer_get_iter_at_offset(buf, &it, 0);
}

/**
 *  Add a line to the log with the requested appearance. This should not be
 *  called from any callbacks; only background work.
 *  @col: appearance of the content
 *  @msg: content to add
 */
void log_add (color_id col, const char *msg, ...)
{
        va_list li;
        va_start(li, msg);

        if (size < MAX_BUF) {
                memset(alog[size], 0, MAX_BUF);
                vsprintf(alog[size], msg, li);
                themes[size] = col;
                size += 1;
        }

        va_end(li);
}

/**
 *  Take a string (of size MAX_BUF) with a list of peers on the current channel
 *  so it can be displayed.
 *  @peers: string of peers
 */
void log_list (const char *peers)
{
        memcpy(list, peers, MAX_BUF);
}

void log_display (int i)
{
        char *name = NULL;
        char *content = NULL;

        if (themes[i] == COL_NONE) {
                if (name = strtok(alog[i], ":")) {
                        content = name + strlen(name) + 1;
                        gtk_text_buffer_insert_with_tags_by_name(buf, &it, name,
                                                                 -1,
                                                                 tags[COL_NAME],
                                                                 NULL);
                        gtk_text_buffer_insert_with_tags_by_name(buf, &it, ":",
                                                                 1,
                                                                 tags[COL_NONE],
                                                                 NULL);
                        gtk_text_buffer_insert_with_tags_by_name(buf, &it,
                                                                 content,
                                                                 -1,
                                                                 tags[COL_NONE],
                                                                 NULL);
                }
        } else if ((themes[i] == COL_NOTIF) && (alog[i][0] == '-') &&
                   (alog[i][1] == '-')) {
                gtk_text_buffer_insert_with_tags_by_name(buf, &it, alog[i], -1,
                                                         tags[COL_PRIV], NULL);
        } else
                gtk_text_buffer_insert_with_tags_by_name(buf, &it, alog[i], -1,
                                                         tags[themes[i]], NULL);
}

/**
 *  Display all the elements that have been added to the queue since the last
 *  call. If it's busy; skip it until next refresh.
 */
void log_refresh ()
{
        int i = 0;
        
        for (i = 0; i < size; i++) {
                log_display(i);
                memset(alog[i], 0, MAX_BUF);
                themes[i] = 0;
        }

        if (list[0]) {
                GtkTextIter mit;
                gtk_text_buffer_set_text(side, " ", 1);
                gtk_text_buffer_get_iter_at_offset(side, &mit, 0);
                gtk_text_buffer_insert_with_tags_by_name(side, &mit, list, -1,
                                                         tags[COL_NAME], NULL);
                memset(list, 0, MAX_BUF);
        }

        size = 0;
}
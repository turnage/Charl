#ifndef LOG_H
#define LOG_H

#include <gtk/gtk.h>
#include <charl.h>

/* Enable the log module. */
void log_init (GtkWidget *wid, GtkWidget *sid);

/* Update the peer list. */
void log_list (const char *peers);

/* Add a line to the log view. */
void log_add (color_id col, const char *msg, ...);

/* Display any messages waiting in queue. */
void log_refresh ();

#endif /* LOG_H */
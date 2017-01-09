#ifndef __GV_GRAPHICAL_APPLICATION_H__
#define __GV_GRAPHICAL_APPLICATION_H__

#include <glib-object.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

/* GObject declarations */

#define GV_TYPE_GRAPHICAL_APPLICATION gv_graphical_application_get_type()

G_DECLARE_FINAL_TYPE(GvGraphicalApplication, gv_graphical_application,
                     GV, GRAPHICAL_APPLICATION, GtkApplication)

/* Methods */

GApplication *gv_graphical_application_new(void);

#endif /* __GV_GRAPHICAL_APPLICATION_H__ */

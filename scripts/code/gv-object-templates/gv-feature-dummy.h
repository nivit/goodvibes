#ifndef __GV_DUMMY_H__
#define __GV_DUMMY_H__

#include <glib-object.h>

#include "framework/gv-feature.h"

/* GObject declarations */

#define GV_TYPE_DUMMY gv_dummy_get_type()

G_DECLARE_FINAL_TYPE(GvDummy, gv_dummy, GV, DUMMY, GvFeature)

#endif /* __GV_DUMMY_H__ */

#ifndef __OCK_DUMMY_H__
#define __OCK_DUMMY_H__

#include <glib-object.h>

#include "framework/ock-feature.h"

/* GObject declarations */

#define OCK_TYPE_DUMMY ock_dummy_get_type()

G_DECLARE_FINAL_TYPE(OckDummy, ock_dummy, OCK, DUMMY, OckFeature)

#endif /* __OCK_DUMMY_H__ */

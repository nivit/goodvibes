#ifndef __OCK_DUMMY_H__
#define __OCK_DUMMY_H__

#include <glib-object.h>

/* GObject declarations */

#define OCK_TYPE_DUMMY ock_dummy_get_type()

G_DECLARE_FINAL_TYPE(OckDummy, ock_dummy, OCK, DUMMY, GObject)

/* Data types */

/* Methods */

OckDummy *ock_dummy_new(void);

/* Property accessors */

#endif /* __OCK_DUMMY_H__ */

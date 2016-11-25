#include <glib.h>
#include <glib-object.h>

#include "additions/glib-object.h"

#include "framework/log.h"

#include "core/ock-dummy.h"

/*
 * Properties
 */

enum {
	/* Reserved */
	PROP_0,
	/* Properties */
	// TODO fill with your properties
	/* Number of properties */
	PROP_N
};

static GParamSpec *properties[PROP_N];

/*
 * Signals
 */

/*
 * GObject definitions
 */

struct _OckDummyPrivate {
	// TODO fill with your data
};

typedef struct _OckDummyPrivate OckDummyPrivate;

struct _OckDummy {
	/* Parent instance structure */
	GObject parent_instance;
	/* Private data */
	OckDummyPrivate *priv;
};

G_DEFINE_TYPE_WITH_PRIVATE(OckDummy, ock_dummy, G_TYPE_OBJECT)

/*
 * Helpers
 */

/*
 * Signal handlers & callbacks
 */

/*
 * Property accessors
 */

static void
ock_dummy_get_property(GObject    *object,
                       guint       property_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
	OckDummy *self = OCK_DUMMY(object);

	TRACE_GET_PROPERTY(object, property_id, value, pspec);

	// TODO handle properties
	(void) self;
	(void) value;

	switch (property_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void
ock_dummy_set_property(GObject      *object,
                       guint         property_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
	OckDummy *self = OCK_DUMMY(object);

	TRACE_SET_PROPERTY(object, property_id, value, pspec);

	// TODO handle properties
	(void) self;
	(void) value;

	switch (property_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

/*
 * Public methods
 */

OckDummy *
ock_dummy_new(void)
{
	return g_object_new(OCK_TYPE_DUMMY, NULL);
}

/*
 * GObject methods
 */

static void
ock_dummy_finalize(GObject *object)
{
	OckDummy *self = OCK_DUMMY(object);
	OckDummyPrivate *priv = self->priv;

	TRACE("%p", object);

	// TODO job to be done
	(void) priv;

	/* Chain up */
	G_OBJECT_CHAINUP_FINALIZE(ock_dummy, object);
}

static void
ock_dummy_constructed(GObject *object)
{
	OckDummy *self = OCK_DUMMY(object);
	OckDummyPrivate *priv = self->priv;

	TRACE("%p", object);

	/* Initialize properties */
	// TODO
	(void) priv;

	/* Chain up */
	G_OBJECT_CHAINUP_CONSTRUCTED(ock_dummy, object);
}

static void
ock_dummy_init(OckDummy *self)
{
	TRACE("%p", self);

	/* Initialize private pointer */
	self->priv = ock_dummy_get_instance_private(self);
}

static void
ock_dummy_class_init(OckDummyClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize = ock_dummy_finalize;
	object_class->constructed = ock_dummy_constructed;

	/* Properties */
	object_class->get_property = ock_dummy_get_property;
	object_class->set_property = ock_dummy_set_property;

	// TODO define your properties here
	//      use OCK_PARAM_DEFAULT_FLAGS

	g_object_class_install_properties(object_class, PROP_N, properties);
}

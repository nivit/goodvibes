#include <glib.h>
#include <glib-object.h>

#include "additions/glib-object.h"

#include "framework/log.h"

#include "core/gv-dummy.h"

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

struct _GvDummyPrivate {
	// TODO fill with your data
};

typedef struct _GvDummyPrivate GvDummyPrivate;

struct _GvDummy {
	/* Parent instance structure */
	GObject parent_instance;
	/* Private data */
	GvDummyPrivate *priv;
};

G_DEFINE_TYPE_WITH_PRIVATE(GvDummy, gv_dummy, G_TYPE_OBJECT)

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
gv_dummy_get_property(GObject    *object,
                      guint       property_id,
                      GValue     *value,
                      GParamSpec *pspec)
{
	GvDummy *self = GV_DUMMY(object);

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
gv_dummy_set_property(GObject      *object,
                      guint         property_id,
                      const GValue *value,
                      GParamSpec   *pspec)
{
	GvDummy *self = GV_DUMMY(object);

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

GvDummy *
gv_dummy_new(void)
{
	return g_object_new(GV_TYPE_DUMMY, NULL);
}

/*
 * GObject methods
 */

static void
gv_dummy_finalize(GObject *object)
{
	GvDummy *self = GV_DUMMY(object);
	GvDummyPrivate *priv = self->priv;

	TRACE("%p", object);

	// TODO job to be done
	(void) priv;

	/* Chain up */
	G_OBJECT_CHAINUP_FINALIZE(gv_dummy, object);
}

static void
gv_dummy_constructed(GObject *object)
{
	GvDummy *self = GV_DUMMY(object);
	GvDummyPrivate *priv = self->priv;

	TRACE("%p", object);

	/* Initialize properties */
	// TODO
	(void) priv;

	/* Chain up */
	G_OBJECT_CHAINUP_CONSTRUCTED(gv_dummy, object);
}

static void
gv_dummy_init(GvDummy *self)
{
	TRACE("%p", self);

	/* Initialize private pointer */
	self->priv = gv_dummy_get_instance_private(self);
}

static void
gv_dummy_class_init(GvDummyClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize = gv_dummy_finalize;
	object_class->constructed = gv_dummy_constructed;

	/* Properties */
	object_class->get_property = gv_dummy_get_property;
	object_class->set_property = gv_dummy_set_property;

	// TODO define your properties here
	//      use GV_PARAM_DEFAULT_FLAGS

	g_object_class_install_properties(object_class, PROP_N, properties);
}

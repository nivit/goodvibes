#include <glib.h>
#include <glib-object.h>

#include "framework/log.h"
#include "framework/ock-feature.h"

#include "core/ock-core.h"

#include "core/feat/ock-dummy.h"

/*
 * GObject definitions
 */

struct _OckDummy {
	/* Parent instance structure */
	OckFeature parent_instance;
};

G_DEFINE_TYPE(OckDummy, ock_dummy, OCK_TYPE_FEATURE)

/*
 * Helpers
 */

/*
 * Signal handlers & callbacks
 */

static GSignalHandler player_handlers[] = {
	{ NULL, NULL }
};

/*
 * Feature methods
 */

static void
ock_dummy_disable(OckFeature *feature)
{
	OckPlayer *player = ock_core_player;

	/* Signal handlers */
	g_signal_handlers_disconnect_by_data(player, feature);

	// TODO

	/* Chain up */
	OCK_FEATURE_CHAINUP_DISABLE(ock_dummy, feature);
}

static void
ock_dummy_enable(OckFeature *feature)
{
	OckPlayer *player = ock_core_player;

	/* Chain up */
	OCK_FEATURE_CHAINUP_ENABLE(ock_dummy, feature);

	// TODO

	/* Signal handlers */
	g_signal_handlers_connect(player, player_handlers, feature);
}

/*
 * GObject methods
 */

static void
ock_dummy_init(OckDummy *self)
{
	TRACE("%p", self);
}

static void
ock_dummy_class_init(OckDummyClass *class)
{
	OckFeatureClass *feature_class = OCK_FEATURE_CLASS(class);

	TRACE("%p", class);

	/* Override OckFeature methods */
	feature_class->enable  = ock_dummy_enable;
	feature_class->disable = ock_dummy_disable;
}

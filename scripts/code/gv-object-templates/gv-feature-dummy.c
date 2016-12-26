#include <glib.h>
#include <glib-object.h>

#include "framework/log.h"
#include "framework/gv-feature.h"

#include "core/gv-core.h"

#include "core/feat/gv-dummy.h"

/*
 * GObject definitions
 */

struct _GvDummy {
	/* Parent instance structure */
	GvFeature parent_instance;
};

G_DEFINE_TYPE(GvDummy, gv_dummy, GV_TYPE_FEATURE)

/*
 * Helpers
 */

/*
 * Feature methods
 */

static void
gv_dummy_disable(GvFeature *feature)
{
	GvPlayer *player = gv_core_player;

	/* Signal handlers */
	g_signal_handlers_disconnect_by_data(player, feature);

	// FILL THAT

	/* Chain up */
	GV_FEATURE_CHAINUP_DISABLE(gv_dummy, feature);
}

static void
gv_dummy_enable(GvFeature *feature)
{
	GvPlayer *player = gv_core_player;

	/* Chain up */
	GV_FEATURE_CHAINUP_ENABLE(gv_dummy, feature);

	// FILL THAT

	/* Signal handlers */
	// g_signal_connect(player, "blabla", on_blabla, feature);
}

/*
 * GObject methods
 */

static void
gv_dummy_init(GvDummy *self)
{
	TRACE("%p", self);
}

static void
gv_dummy_class_init(GvDummyClass *class)
{
	GvFeatureClass *feature_class = GV_FEATURE_CLASS(class);

	TRACE("%p", class);

	/* Override GvFeature methods */
	feature_class->enable  = gv_dummy_enable;
	feature_class->disable = gv_dummy_disable;
}

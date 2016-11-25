/*
 * Overcooked Radio Player
 *
 * Copyright (C) 2015-2016 Arnaud Rebillout
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <glib-object.h>

#include "additions/glib.h"
#include "additions/glib-object.h"

#include "libgszn/gszn.h"

#include "framework/log.h"
#include "framework/ock-feature.h"
#include "framework/ock-framework.h"
#include "framework/ock-framework-enum-types.h"
#include "framework/ock-list.h"
#include "framework/ock-param-specs.h"

/*
 * Properties
 */

enum {
	/* Reserved */
	PROP_0,
	/* Properties */
	PROP_STATE,
	PROP_ENABLED,
	/* Number of properties */
	PROP_N
};

static GParamSpec *properties[PROP_N];

/*
 * GObject definitions
 */

struct _OckFeaturePrivate {
	/* Properties */
	OckFeatureState state;
	gboolean        enabled;
	/* Pending enable/disable task */
	guint           when_idle_id;
};

typedef struct _OckFeaturePrivate OckFeaturePrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(OckFeature, ock_feature, G_TYPE_OBJECT)

/*
 * Private methods
 */

static void ock_feature_set_state(OckFeature *self, OckFeatureState state);

static void
ock_feature_disable(OckFeature *self)
{
	OckFeatureClass *feature_class = OCK_FEATURE_GET_CLASS(self);

	INFO("Disabling feature '%s'...", G_OBJECT_TYPE_NAME(self));

	/* Disable */
	if (feature_class->disable)
		feature_class->disable(self);

	/* Set state property */
	ock_feature_set_state(self, OCK_FEATURE_STATE_DISABLED);
}

static void
ock_feature_enable(OckFeature *self)
{
	OckFeatureClass *feature_class = OCK_FEATURE_GET_CLASS(self);

	INFO("Enabling feature '%s'...", G_OBJECT_TYPE_NAME(self));

	/* Enable */
	if (feature_class->enable)
		feature_class->enable(self);

	/* Set state property */
	ock_feature_set_state(self, OCK_FEATURE_STATE_ENABLED);
}

/*
 * Callbacks
 */

static gboolean
when_idle_enable_feature(gpointer user_data)
{
	OckFeature *self = OCK_FEATURE(user_data);
	OckFeaturePrivate *priv = ock_feature_get_instance_private(self);

	g_assert(priv->state == OCK_FEATURE_STATE_ENABLING);

	ock_feature_enable(self);

	priv->when_idle_id = 0;

	return G_SOURCE_REMOVE;
}

static gboolean
when_idle_disable_feature(gpointer user_data)
{
	OckFeature *self = OCK_FEATURE(user_data);
	OckFeaturePrivate *priv = ock_feature_get_instance_private(self);

	g_assert(priv->state == OCK_FEATURE_STATE_DISABLING);

	ock_feature_disable(self);

	priv->when_idle_id = 0;

	return G_SOURCE_REMOVE;
}

/*
 * Property accessors
 */

OckFeatureState
ock_feature_get_state(OckFeature *self)
{
	OckFeaturePrivate *priv = ock_feature_get_instance_private(self);

	return priv->state;
}

static void
ock_feature_set_state(OckFeature *self, OckFeatureState state)
{
	OckFeaturePrivate *priv = ock_feature_get_instance_private(self);

	if (priv->state == state)
		return;

	priv->state = state;
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_STATE]);
}

gboolean
ock_feature_get_enabled(OckFeature *self)
{
	OckFeaturePrivate *priv = ock_feature_get_instance_private(self);

	return priv->enabled;
}

void
ock_feature_set_enabled(OckFeature *self, gboolean enabled)
{
	OckFeaturePrivate *priv = ock_feature_get_instance_private(self);
	OckFeatureState new_state;

	/* Bail out if needed */
	if (priv->enabled == enabled)
		return;

	/* Check if we really need to do something.
	 *
	 * Enabling and disabling the feature is not done immediately,
	 * but scheduled for a later idle moment. There are several
	 * reasons for that.
	 *
	 * At first, features might be enabled very early at init time:
	 * - for features that are enabled by default, this code will be
	 *   run just after the object is created.
	 * - for features that are enabled by the used, this code will be
	 *   run when the configuration is applied.
	 * In both cases, it's too early to run the feature enable code.
	 *
	 * Moreover, if a feature is enabled by default, and disabled in the
	 * configuration, this code will be invoked twice successively, first
	 * to enable, then to disable. Thanks to the delay, in this particular
	 * case the two calls cancel each other, and no action is performed.
	 *
	 * So, enabling/disabling is a delayed operation, and therefore we
	 * have two properties: one for the setting, one for the state.
	 *
	 * In here, we just set the setting, and schedule a callback if needed.
	 *
	 * Notice that if the state is *ING, it just means that an operation
	 * is pending, but nothing was done yet. In such case, it would be an
	 * error to schedule a callback. The only thing to do is to remove the
	 * pending callback, and to change the state.
	 */

	new_state = priv->state;

	if (enabled == TRUE) {
		switch (priv->state) {
		case OCK_FEATURE_STATE_DISABLING:
			g_source_remove(priv->when_idle_id);
			priv->when_idle_id = 0;
			new_state = OCK_FEATURE_STATE_ENABLED;
			break;

		case OCK_FEATURE_STATE_DISABLED:
			priv->when_idle_id = g_idle_add(when_idle_enable_feature, self);
			new_state = OCK_FEATURE_STATE_ENABLING;
			break;

		default:
			/* We shouldn't pass here */
			WARNING("Unexpected state value %d", priv->state);
			break;
		}

	} else {
		switch (priv->state) {
		case OCK_FEATURE_STATE_ENABLING:
			g_source_remove(priv->when_idle_id);
			priv->when_idle_id = 0;
			new_state = OCK_FEATURE_STATE_DISABLED;
			break;

		case OCK_FEATURE_STATE_ENABLED:
			priv->when_idle_id = g_idle_add(when_idle_disable_feature, self);
			new_state = OCK_FEATURE_STATE_DISABLING;
			break;

		default:
			/* We shouldn't pass here */
			WARNING("Unexpected state value %d", priv->state);
			break;
		}
	}

	/* Save and notify */
	priv->enabled = enabled;
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_ENABLED]);

	ock_feature_set_state(self, new_state);
}

static void
ock_feature_get_property(GObject    *object,
                         guint       property_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
	OckFeature *self = OCK_FEATURE(object);

	TRACE_GET_PROPERTY(object, property_id, value, pspec);

	switch (property_id) {
	case PROP_STATE:
		g_value_set_enum(value, ock_feature_get_state(self));
		break;
	case PROP_ENABLED:
		g_value_set_boolean(value, ock_feature_get_enabled(self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void
ock_feature_set_property(GObject      *object,
                         guint         property_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
	OckFeature *self = OCK_FEATURE(object);

	TRACE_SET_PROPERTY(object, property_id, value, pspec);

	switch (property_id) {
	case PROP_ENABLED:
		ock_feature_set_enabled(self, g_value_get_boolean(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

/*
 * Public functions
 */

OckFeature *
ock_feature_new(GType object_type, gboolean enabled)
{
	return g_object_new(object_type,
	                    "enabled", enabled,
	                    NULL);
}

/*
 * GObject methods
 */

static void
ock_feature_finalize(GObject *object)
{
	OckFeature *self = OCK_FEATURE(object);
	OckFeaturePrivate *priv = ock_feature_get_instance_private(self);

	TRACE("%p", object);

	/* Unload */
	if (priv->enabled)
		ock_feature_set_enabled(self, FALSE);

	/* There might be some task pending */
	if (priv->when_idle_id > 0) {
		if (priv->state == OCK_FEATURE_STATE_DISABLING)
			when_idle_disable_feature(self);
		else
			g_source_remove(priv->when_idle_id);
	}

	/* Chain up */
	G_OBJECT_CHAINUP_FINALIZE(ock_feature, object);
}

static void
ock_feature_constructed(GObject *object)
{
	TRACE("%p", object);

	/* Chain up */
	G_OBJECT_CHAINUP_CONSTRUCTED(ock_feature, object);
}

static void
ock_feature_init(OckFeature *self)
{
	TRACE("%p", self);
}

static void
ock_feature_class_init(OckFeatureClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize = ock_feature_finalize;
	object_class->constructed = ock_feature_constructed;

	/* Properties */
	object_class->get_property = ock_feature_get_property;
	object_class->set_property = ock_feature_set_property;

	properties[PROP_STATE] =
	        g_param_spec_enum("state", "Feature State", NULL,
	                          OCK_FEATURE_STATE_ENUM_TYPE,
	                          OCK_FEATURE_STATE_DISABLED,
	                          OCK_PARAM_DEFAULT_FLAGS | G_PARAM_READABLE);

	properties[PROP_ENABLED] =
	        g_param_spec_boolean("enabled", "Enabled", NULL,
	                             FALSE,
	                             OCK_PARAM_DEFAULT_FLAGS | GSZN_PARAM_SERIALIZE |
	                             G_PARAM_READWRITE);

	g_object_class_install_properties(object_class, PROP_N, properties);
}

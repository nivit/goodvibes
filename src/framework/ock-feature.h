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

#ifndef __OVERCOOKED_FRAMEWORK_OCK_FEATURE_H__
#define __OVERCOOKED_FRAMEWORK_OCK_FEATURE_H__

#include <glib-object.h>

#include "additions/glib-object.h"

/* GObject declarations */

#define OCK_TYPE_FEATURE ock_feature_get_type()

G_DECLARE_DERIVABLE_TYPE(OckFeature, ock_feature, OCK, FEATURE, GObject)

/* Chain up macros */

#define OCK_FEATURE_CHAINUP_ENABLE(type_name, obj)        \
	do { \
		if (OCK_FEATURE_CLASS(type_name##_parent_class)->enable) \
			OCK_FEATURE_CLASS(type_name##_parent_class)->enable(obj); \
	} while (0)

#define OCK_FEATURE_CHAINUP_DISABLE(type_name, obj)       \
	do { \
		if (OCK_FEATURE_CLASS(type_name##_parent_class)->disable) \
			OCK_FEATURE_CLASS(type_name##_parent_class)->disable(obj); \
	} while (0)

/* Data types */

typedef enum {
	OCK_FEATURE_STATE_DISABLED,
	OCK_FEATURE_STATE_ENABLED,
	OCK_FEATURE_STATE_DISABLING,
	OCK_FEATURE_STATE_ENABLING
} OckFeatureState;

struct _OckFeatureClass {
	/* Parent class */
	GObjectClass parent_class;
	/* Virtual methods */
	void (*enable)  (OckFeature *);
	void (*disable) (OckFeature *);
};

/* Public methods */

OckFeature     *ock_feature_new(GType object_type, gboolean enabled);

/* Property accessors */

OckFeatureState ock_feature_get_state  (OckFeature *self);
gboolean        ock_feature_get_enabled(OckFeature *self);
void            ock_feature_set_enabled(OckFeature *self, gboolean enabled);

#endif /* __OVERCOOKED_FRAMEWORK_OCK_FEATURE_H__ */

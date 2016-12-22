/*
 * Libcaphe
 *
 * Copyright (C) 2016 Arnaud Rebillout
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

#include <stdlib.h>

#include <glib-object.h>
#include <gio/gio.h>

#include "caphe-trace.h"
#include "caphe-dbus-invokator.h"

/*
 * Properties
 */

enum {
	/* Reserved */
	PROP_0,
	/* Construct-only properties */
	PROP_PROXY,
	/* Properties */
	PROP_INHIBITED,
	/* Total number of properties */
	LAST_PROP
};

static GParamSpec *properties[LAST_PROP];

/*
 * GObject definitions
 */

struct _CapheDbusInvokatorPrivate {
	/* Properties */
	GDBusProxy *proxy;
	gboolean    inhibited;
};

typedef struct _CapheDbusInvokatorPrivate CapheDbusInvokatorPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(CapheDbusInvokator, caphe_dbus_invokator, G_TYPE_OBJECT)

/*
 * Property accessors
 */

GDBusProxy *
caphe_dbus_invokator_get_proxy(CapheDbusInvokator *self)
{
	CapheDbusInvokatorPrivate *priv = caphe_dbus_invokator_get_instance_private(self);

	return priv->proxy;
}

static void
caphe_dbus_invokator_set_proxy(CapheDbusInvokator *self, GDBusProxy *proxy)
{
	CapheDbusInvokatorPrivate *priv = caphe_dbus_invokator_get_instance_private(self);

	/* Construct-only property */
	g_assert(priv->proxy == NULL);
	g_assert(proxy != NULL);

	priv->proxy = g_object_ref(proxy);
}

gboolean
caphe_dbus_invokator_get_inhibited(CapheDbusInvokator *self)
{
	g_return_val_if_fail(CAPHE_IS_DBUS_INVOKATOR(self), FALSE);

	return CAPHE_DBUS_INVOKATOR_GET_CLASS(self)->is_inhibited(self);
}

static void
caphe_dbus_invokator_get_property(GObject    *object,
                                  guint       property_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
	CapheDbusInvokator *self = CAPHE_DBUS_INVOKATOR(object);

	switch (property_id) {
	case PROP_PROXY:
		g_value_set_object(value, caphe_dbus_invokator_get_proxy(self));
		break;
	case PROP_INHIBITED:
		g_value_set_boolean(value, caphe_dbus_invokator_get_inhibited(self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void
caphe_dbus_invokator_set_property(GObject      *object,
                                  guint         property_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
	CapheDbusInvokator *self = CAPHE_DBUS_INVOKATOR(object);

	switch (property_id) {
	case PROP_PROXY:
		caphe_dbus_invokator_set_proxy(self, g_value_get_object(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

/*
 * Public methods
 */

void
caphe_dbus_invokator_uninhibit(CapheDbusInvokator *self)
{
	g_return_if_fail(CAPHE_IS_DBUS_INVOKATOR(self));

	return CAPHE_DBUS_INVOKATOR_GET_CLASS(self)->uninhibit(self);
}

void
caphe_dbus_invokator_inhibit(CapheDbusInvokator *self, const gchar *application,
                             const gchar *reason)
{
	g_return_if_fail(CAPHE_IS_DBUS_INVOKATOR(self));

	return CAPHE_DBUS_INVOKATOR_GET_CLASS(self)->inhibit(self, application, reason);
}

CapheDbusInvokator *
caphe_dbus_invokator_new(GType invokator_type, GDBusProxy *proxy)
{
	g_return_val_if_fail(g_type_is_a(invokator_type, CAPHE_TYPE_DBUS_INVOKATOR), NULL);

	return g_object_new(invokator_type, "proxy", proxy, NULL);
}

/*
 * GObject methods
 */

static void
caphe_dbus_invokator_finalize(GObject *object)
{
	CapheDbusInvokator *self = CAPHE_DBUS_INVOKATOR(object);
	CapheDbusInvokatorPrivate *priv = caphe_dbus_invokator_get_instance_private(self);

	TRACE("%p", self);

	/* Free resources */
	g_object_unref(priv->proxy);

	/* Chain up */
	if (G_OBJECT_CLASS(caphe_dbus_invokator_parent_class)->finalize)
		G_OBJECT_CLASS(caphe_dbus_invokator_parent_class)->finalize(object);
}

static void
caphe_dbus_invokator_init(CapheDbusInvokator *self)
{
	CapheDbusInvokatorPrivate *priv = caphe_dbus_invokator_get_instance_private(self);

	TRACE("%p", self);

	/* Construct-only properties, initialized later on */
	priv->proxy = NULL;
}

static void
caphe_dbus_invokator_class_init(CapheDbusInvokatorClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize = caphe_dbus_invokator_finalize;

	/* Install properties */
	object_class->get_property = caphe_dbus_invokator_get_property;
	object_class->set_property = caphe_dbus_invokator_set_property;

	properties[PROP_PROXY] =
	        g_param_spec_object("proxy", "Proxy", NULL,
	                            G_TYPE_DBUS_PROXY,
	                            G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE |
	                            G_PARAM_CONSTRUCT_ONLY);

	properties[PROP_INHIBITED] =
	        g_param_spec_boolean("inhibited", "Inhibited", NULL, FALSE,
	                             G_PARAM_STATIC_STRINGS | G_PARAM_READABLE);

	g_object_class_install_properties(object_class, LAST_PROP, properties);
}

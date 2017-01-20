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
#include "caphe-inhibitor.h"
#include "caphe-dbus-inhibitor.h"
#include "caphe-main.h"

/* This whole code is a mess, these asynchronous problematics killed
 * me all along. Good luck to the guy reading these lines.
 */

/* Inhibitors we know of. This list is ordered by priority. */

enum {
	CAPHE_GNOME_SM_IDX,
	CAPHE_XFCE_SM_IDX,
	CAPHE_POWER_MANAGEMENT_IDX,
	CAPHE_LOGIN1_IDX,
	CAPHE_N_INHIBITORS,
};

static const gchar *caphe_inhibitor_ids[] = {
	"Gnome",  /* gnome session manager */
	"Xfce", /* xfce session manager */
	"PowerManagement", /* freedesktop power management */
	"Login1", /* systemd */
	NULL
};

/*
 * Properties
 */

enum {
	/* Reserved */
	PROP_0,
	/* Construct-only properties */
	PROP_APPLICATION_NAME,
	/* Properties */
	PROP_INHIBITED,
	PROP_INHIBITOR_ID,
	/* Total number of properties */
	LAST_PROP
};

static GParamSpec *properties[LAST_PROP];

/*
 * Signals
 */

enum {
	SIGNAL_INHIBIT_FINISHED,
	SIGNAL_UNINHIBIT_FINISHED,
	/* Number of signals */
	SIGNAL_N
};

static guint signals[SIGNAL_N];

/*
 * GObject definitions
 */

typedef struct {
	/* If NULL, means an unhibibit request.
	 * If non NULL, means an inhibit request.
	 */
	gchar *reason;
} CapheRequest;

struct _CapheMainPrivate {
	/* Application name */
	gchar          *application_name;
	/* List of inhibitors */
	CapheInhibitor *inhibitors[CAPHE_N_INHIBITORS];

	/* Current state */
	CapheInhibitor *current_inhibitor;
	gchar          *current_reason;
	/* Requests */
	CapheRequest   *started_request;
	CapheRequest   *pending_request;
	/* Pending processing */
	guint           when_idle_process_id;
	/* For init-time */
	gboolean        inhibitors_ready[CAPHE_N_INHIBITORS];
	gboolean        not_ready_yet;
};

typedef struct _CapheMainPrivate CapheMainPrivate;

struct _CapheMain {
	/* Parent instance structure */
	GObject           parent_instance;
	/* Private data */
	CapheMainPrivate *priv;
};

G_DEFINE_TYPE_WITH_PRIVATE(CapheMain, caphe_main, G_TYPE_OBJECT)

/*
 * Helpers
 */

CapheRequest *
caphe_request_new(const gchar *reason)
{
	CapheRequest *self;

	self = g_new0(CapheRequest, 1);
	self->reason = g_strdup(reason);

	return self;
}

void
caphe_request_free(CapheRequest *self)
{
	if (self == NULL)
		return;

	g_free(self->reason);
	g_free(self);
}

static gint
get_inhibitor_index(CapheInhibitor *inhibitors[], CapheInhibitor *inhibitor)
{
	gint i;

	for (i = 0; i < CAPHE_N_INHIBITORS; i++) {
		if (inhibitors[i] == inhibitor)
			return i;
	}

	return -1;
}

const gchar *
get_inhibitor_id(CapheInhibitor *inhibitors[], CapheInhibitor *inhibitor)
{
	gint i;

	for (i = 0; i < CAPHE_N_INHIBITORS; i++) {
		if (inhibitors[i] == inhibitor)
			return caphe_inhibitor_ids[i];
	}

	return "unknown";
}

/*
 * Private methods
 */

static gboolean
uninhibit_with(CapheInhibitor *inhibitor)
{
	if (inhibitor == NULL)
		return FALSE;

	caphe_inhibitor_uninhibit(inhibitor);
	return TRUE;
}

static gboolean
inhibit_after(CapheInhibitor *inhibitors[], CapheInhibitor *inhibitor,
              const gchar *application_name, const gchar *reason)
{
	gint inhibitor_index, i;

	/* Get index of inhibitor in the list. Passing NULL for inhibitor is allowed,
	 * it makes us start from from the first inhibitor.
	 */
	inhibitor_index = get_inhibitor_index(inhibitors, inhibitor);

	for (i = inhibitor_index + 1; i < CAPHE_N_INHIBITORS; i++) {
		inhibitor = inhibitors[i];

		if (caphe_inhibitor_get_available(inhibitor) == FALSE)
			continue;

		caphe_inhibitor_inhibit(inhibitor, application_name, reason);
		return TRUE;
	}

	return FALSE;
}

static void
caphe_main_process(CapheMain *self)
{
	CapheMainPrivate *priv = self->priv;

	/* As long as we're not ready, we can't do anything */
	if (priv->not_ready_yet)
		return;

	/* If a request is currently being processed, do nothing
	 * (we're waiting for this request to be completed)
	 */
	if (priv->started_request)
		return;

	/* If there's nothing pending, do nothing */
	if (priv->pending_request == NULL)
		return;

#if 0
	/* Let's sanity-check the pending request, it's still time to discard it.
	 * Basically, if the pending reason is the same at the existing reason,
	 * we discard.
	 */
	if (!g_strcmp0(priv->pending_request->reason, priv->current_reason)) {
		g_debug("Discarding request of identical reason '%s'", priv->current_reason);
		caphe_request_free(priv->pending_request);
		priv->pending_request = NULL;
		return;
	}
#endif

	/* We can start processing the pending request.
	 * In case we're trying to inhibit while already inhibited
	 * (it's possible to change the reason), we must slip an unhibit
	 * request in between.
	 */
	if (priv->current_reason &&
	    priv->pending_request->reason) {
		g_debug("Slipping uninhibit request between two inhibit requests");
		priv->started_request = caphe_request_new(NULL);
	} else {
		priv->started_request = priv->pending_request;
		priv->pending_request = NULL;
	}

	if (priv->started_request->reason) {
		gboolean success;

		g_debug("Inhibition: processing request");

		success = inhibit_after(priv->inhibitors, NULL, priv->application_name,
		                        priv->started_request->reason);
		if (success == FALSE) {
			/* Consume request */
			caphe_request_free(priv->started_request);
			priv->started_request = NULL;

			// Failure, we should report that
		}
	} else {
		gboolean success;

		g_debug("Uninhibition: processing request");

		success = uninhibit_with(priv->current_inhibitor);

		if (success == FALSE) {
			/* Consume request */
			caphe_request_free(priv->started_request);
			priv->started_request = NULL;

			// Failure, we should report that
		}
	}
}

/*
 * Signal handlers
 */

static void
on_inhibitor_notify_available(CapheInhibitor *inhibitor,
                              GParamSpec *pspec G_GNUC_UNUSED,
                              CapheMain *self)
{
	CapheMainPrivate *priv = self->priv;
	gboolean available = caphe_inhibitor_get_available(inhibitor);

	TRACE("%s, 'available=%s', %p", get_inhibitor_id(priv->inhibitors, inhibitor),
	      available ? "true" : "false", self);

	/*
	 * Init-time
	 */

	/* Receiving an 'available' signal from an inhibitor means that this
	 * inhibitor is ready. And as long as we're in this early stage, that's
	 * all we care about, the value of the available property doesn't matter.
	 */

	if (priv->not_ready_yet) {
		gint index;

		/* Update ready list */
		index = get_inhibitor_index(priv->inhibitors, inhibitor);
		priv->inhibitors_ready[index] = TRUE;

		/* Check if every inhibitors are ready */
		for (index = 0; index < CAPHE_N_INHIBITORS; index++) {
			if (priv->inhibitors_ready[index] == FALSE)
				return;
		}

		/* We're ready */
		g_debug("All inhibitors ready");
		priv->not_ready_yet = FALSE;

		/* Let's check if there's a pending request */
		caphe_main_process(self);

		/* In any case, we've done enough */
		return;
	}

	/*
	 * Run-time
	 */

	/* Receiving a change in 'available' is quite unlikely.
	 * In case it happens, we don't try to be smart, we just re-init.
	 */

	/* If we're currently processing a request, it's best to do nothing.
	 * The request might fail, and this will be handled in the request
	 * callback.
	 */
	if (priv->started_request)
		return;

	/* If we're not inhibited, nothing to do */
	if (priv->current_inhibitor == NULL)
		return;

	/* If we're inhibited, and it's the current inhibitor which became
	 * unavailable, that's a problem. We must re-inhibit.
	 */
	if (inhibitor == priv->current_inhibitor) {
		/* We expect the inhibitor to have gone unavailable */
		if (available == FALSE) {
			if (priv->pending_request == NULL)
				priv->pending_request = caphe_request_new(priv->current_reason);

			priv->current_inhibitor = NULL;
			g_free(priv->current_reason);
			priv->current_reason = NULL;

			caphe_main_process(self);
		}

		/* It seems impossible that the inhibitor reports available */
		if (available == TRUE)
			g_warning("Current inhibitor already set (reported 'available=true'");

		return;
	}

	/* For any other inhibitor, let's just cleanup the situation */
	if (priv->pending_request == NULL)
		priv->pending_request = caphe_request_new(priv->current_reason);

	caphe_main_process(self);
}

static void
on_inhibitor_notify_inhibited(CapheInhibitor *inhibitor,
                              GParamSpec *pspec G_GNUC_UNUSED,
                              CapheMain *self)
{
	CapheMainPrivate *priv = self->priv;
	gboolean inhibited = caphe_inhibitor_get_inhibited(inhibitor);
	guint signal_name = 0;
	gboolean signal_arg = FALSE;

	TRACE("%s, 'inhibited=%s', %p",  get_inhibitor_id(priv->inhibitors, inhibitor),
	      inhibited ? "true" : "false", self);

	/* Ignore signals during the pre-init stage */
	if (priv->not_ready_yet)
		return;

	/* Ignore unexpected signals */
	if (priv->started_request == NULL) {
		g_warning("Unexpected 'inhibited' notify (no request started)");
		goto process;
	}

	/* Handle case where inhibit request was in progress */
	if (priv->started_request->reason) {
		/* Check if inhibition succeeded */
		if (inhibited == TRUE) {
			g_debug("Inhibition: request succeeded");

			if (priv->current_inhibitor)
				g_warning("Current inhibitor already set "
				          "(received 'inhibited=true')");

			priv->current_inhibitor = inhibitor;

			if (priv->current_reason)
				g_warning("Current reason already set "
				          "(received 'inhibited=true')");

			g_free(priv->current_reason);
			priv->current_reason = g_strdup(priv->started_request->reason);

			signal_name = SIGNAL_INHIBIT_FINISHED;
			signal_arg = TRUE;
		}

		/* Check if inhibition failed */
		if (inhibited == FALSE) {
			gboolean success;

			g_debug("Inhibition: request failed with inhibitor '%s'",
			        get_inhibitor_id(priv->inhibitors, inhibitor));

			/* Try inhibiting with the next inhibitor */
			success = inhibit_after(priv->inhibitors, inhibitor,
			                        priv->application_name,
			                        priv->started_request->reason);
			if (success) {
				g_debug("Inhibition: retrying with nex inhibitor");
				return;
			} else {
				g_debug("Inhibition: no more inhibitors available");
				signal_name = SIGNAL_INHIBIT_FINISHED;
				signal_arg = FALSE;
			}
		}
	}

	/* Handle case where uninhibit request was in progress */
	if (priv->started_request->reason == NULL) {
		/* Check if uninhibition succeeded */
		if (inhibited == FALSE) {
			g_debug("Uninhibition: request succeeded");

			if (priv->current_inhibitor == NULL)
				g_warning("Current inhibitor not set "
				          "(received 'inhibited=false')");

			priv->current_inhibitor = NULL;

			if (priv->current_reason == NULL)
				g_warning("Current reason not set "
				          "(received 'inhibited=false')");

			g_free(priv->current_reason);
			priv->current_reason = NULL;

			signal_name = SIGNAL_UNINHIBIT_FINISHED;
			signal_arg = TRUE;
		}

		/* Check if uninhibition failed */
		if (inhibited == TRUE) {
			g_warning("Inhibition: request failed with inhibitor '%s'",
			          get_inhibitor_id(priv->inhibitors, inhibitor));

			signal_name = SIGNAL_UNINHIBIT_FINISHED;
			signal_arg = FALSE;
		}
	}

	/* Clean request */
	caphe_request_free(priv->started_request);
	priv->started_request = NULL;

	/* Send signal now */
	g_signal_emit(self, signals[signal_name], 0, signal_arg);

	/* In case of success, we notify like crazy */
	if (signal_arg == TRUE) {
		g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_INHIBITED]);
		g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_INHIBITOR_ID]);
	}

process:
	/* Keep on processing (there might be some operations pending) */
	caphe_main_process(self);
}

/*
 * Public
 */

static gboolean
when_idle_process(CapheMain *self)
{
	CapheMainPrivate *priv = self->priv;

	caphe_main_process(self);

	priv->when_idle_process_id = 0;

	return G_SOURCE_REMOVE;
}

void
caphe_main_uninhibit(CapheMain *self)
{
	CapheMainPrivate *priv = self->priv;

	/* We might ignore the request in the following case:
	 * - we're already uninhibited
	 * - we're sure there's nothing happening at the moment
	 */
	if (priv->current_inhibitor == NULL &&
	    priv->started_request == NULL &&
	    priv->pending_request == NULL)
		return;

	/* There's already a pending uninhibit request */
	if (priv->pending_request &&
	    priv->pending_request->reason == NULL)
		return;

	/* There's already a started uninhibit request */
	if (priv->started_request &&
	    priv->started_request->reason == NULL) {
		caphe_request_free(priv->pending_request);
		priv->pending_request = NULL;
		return;
	}

	/* Otherwise, create a request */
	caphe_request_free(priv->pending_request);
	priv->pending_request = caphe_request_new(NULL);
	g_debug("Uninhibition: request created and pending");

	/* Process */
	if (priv->when_idle_process_id == 0)
		priv->when_idle_process_id = g_idle_add((GSourceFunc) when_idle_process, self);
}

void
caphe_main_inhibit(CapheMain *self, const gchar *reason)
{
	CapheMainPrivate *priv = self->priv;

	g_return_if_fail(reason != NULL);

	/* We might ignore the request in the following case:
	 * - we're already inhibited
	 * - the reason is the same
	 * - we're sure there's nothing happening at the moment
	 * ...
	 */
	if (priv->current_inhibitor &&
	    priv->started_request == NULL &&
	    priv->pending_request == NULL &&
	    !g_strcmp0(reason, priv->current_reason))
		return;

	/* If there's already a pending request with the same reason */
	if (priv->pending_request &&
	    !g_strcmp0(priv->pending_request->reason, reason))
		return;

	/* If there's already a current request with the same reason */
	if (priv->started_request &&
	    !g_strcmp0(priv->started_request->reason, reason)) {
		caphe_request_free(priv->pending_request);
		priv->pending_request = NULL;
		return;
	}

	/* Otherwise, create a request */
	caphe_request_free(priv->pending_request);
	priv->pending_request = caphe_request_new(reason);
	g_debug("Inhibition: request created and pending");

	/* Process */
	if (priv->when_idle_process_id == 0)
		priv->when_idle_process_id = g_idle_add((GSourceFunc) when_idle_process, self);
}

CapheMain *
caphe_main_new(void)
{
	return g_object_new(CAPHE_TYPE_MAIN, NULL);
}

/*
 * Property accessors
 */

gboolean
caphe_main_get_inhibited(CapheMain *self)
{
	CapheMainPrivate *priv = self->priv;

	return priv->current_inhibitor != NULL;
}

const gchar *
caphe_main_get_inhibitor_id(CapheMain *self)
{
	CapheMainPrivate *priv = self->priv;
	gint index = get_inhibitor_index(priv->inhibitors, priv->current_inhibitor);

	if (index == -1)
		return NULL;

	return caphe_inhibitor_ids[index];
}

static void
caphe_main_set_application_name(CapheMain *self, const gchar *application_name)
{
	CapheMainPrivate *priv = self->priv;

	/* Construct-only property */
	g_assert_null(priv->application_name);
	g_assert_nonnull(application_name);

	priv->application_name = g_strdup(application_name);
}

static void
caphe_main_get_property(GObject    *object,
                        guint       property_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
	CapheMain *self = CAPHE_MAIN(object);

	switch (property_id) {
	case PROP_INHIBITED:
		g_value_set_boolean(value, caphe_main_get_inhibited(self));
		break;
	case PROP_INHIBITOR_ID:
		g_value_set_string(value, caphe_main_get_inhibitor_id(self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void
caphe_main_set_property(GObject      *object,
                        guint         property_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
	CapheMain *self = CAPHE_MAIN(object);

	switch (property_id) {
	case PROP_APPLICATION_NAME:
		caphe_main_set_application_name(self, g_value_get_string(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

/*
 * GObject methods
 */

static void
caphe_main_finalize(GObject *object)
{
	CapheMain *self = CAPHE_MAIN(object);
	CapheMainPrivate *priv = self->priv;
	guint i;

	TRACE("%p", self);

	if (priv->when_idle_process_id)
		g_source_remove(priv->when_idle_process_id);

	caphe_request_free(priv->pending_request);
	caphe_request_free(priv->started_request);

	g_free(priv->current_reason);

	for (i = 0; i < CAPHE_N_INHIBITORS; i++)
		g_object_unref(priv->inhibitors[i]);

	g_free(priv->application_name);

	/* Chain up */
	if (G_OBJECT_CLASS(caphe_main_parent_class)->finalize)
		G_OBJECT_CLASS(caphe_main_parent_class)->finalize(object);
}

static void
caphe_main_constructed(GObject *object)
{
	CapheMain *self = CAPHE_MAIN(object);
	CapheMainPrivate *priv = self->priv;
	guint i;

	TRACE("%p", self);

	/* Create inhibitors */
	for (i = 0; i < CAPHE_N_INHIBITORS; i++) {
		priv->inhibitors[i] = caphe_dbus_inhibitor_new(caphe_inhibitor_ids[i]);

		g_signal_connect(priv->inhibitors[i], "notify::available",
		                 (GCallback) on_inhibitor_notify_available, self);
		g_signal_connect(priv->inhibitors[i], "notify::inhibited",
		                 (GCallback) on_inhibitor_notify_inhibited, self);
	}

	/* Chain up */
	if (G_OBJECT_CLASS(caphe_main_parent_class)->constructed)
		G_OBJECT_CLASS(caphe_main_parent_class)->constructed(object);
}

static void
caphe_main_init(CapheMain *self)
{
	CapheMainPrivate *priv = caphe_main_get_instance_private(self);

	TRACE("%p", self);

	/* Init some stuff */
	priv->not_ready_yet = TRUE;

	/* Set private pointer */
	self->priv = priv;
}

static void
caphe_main_class_init(CapheMainClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	TRACE("%p", class);

	/* Override GObject methods */
	object_class->finalize    = caphe_main_finalize;
	object_class->constructed = caphe_main_constructed;

	/* Install properties */
	object_class->get_property = caphe_main_get_property;
	object_class->set_property = caphe_main_set_property;

	properties[PROP_APPLICATION_NAME] =
	        g_param_spec_string("application-name", "Application Name", NULL,
	                            NULL,
	                            G_PARAM_STATIC_STRINGS | G_PARAM_WRITABLE |
	                            G_PARAM_CONSTRUCT_ONLY);

	properties[PROP_INHIBITED] =
	        g_param_spec_boolean("inhibited", "Inhibited", NULL,
	                             FALSE,
	                             G_PARAM_STATIC_STRINGS | G_PARAM_READABLE);

	properties[PROP_INHIBITOR_ID] =
	        g_param_spec_string("inhibitor-id", "Inhibitor ID", NULL,
	                            NULL,
	                            G_PARAM_STATIC_STRINGS | G_PARAM_READABLE);

	g_object_class_install_properties(object_class, LAST_PROP, properties);

	/* Signals */
	signals[SIGNAL_INHIBIT_FINISHED] =
	        g_signal_new("inhibit-finished", G_TYPE_FROM_CLASS(class),
	                     G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
	                     0, NULL, NULL, g_cclosure_marshal_generic, G_TYPE_NONE,
	                     1, G_TYPE_BOOLEAN);

	signals[SIGNAL_UNINHIBIT_FINISHED] =
	        g_signal_new("uninhibit-finished", G_TYPE_FROM_CLASS(class),
	                     G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
	                     0, NULL, NULL, g_cclosure_marshal_generic, G_TYPE_NONE,
	                     1, G_TYPE_BOOLEAN);
}

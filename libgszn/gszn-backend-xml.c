/*
 * Libgszn
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

#include <string.h>
#include <glib.h>
#include <glib-object.h>

#include <libxml/parser.h>

#include "gszn-error.h"
#include "gszn-backend.h"
#include "gszn-backend-xml.h"

/*
 * GObject definitions
 */

struct _GsznBackendXmlPrivate {
	xmlDoc *doc;
	xmlNode *root;
};

typedef struct _GsznBackendXmlPrivate GsznBackendXmlPrivate;

struct _GsznBackendXml {
	/* Parent instance structure */
	GObject                parent_instance;
	/* Private data */
	GsznBackendXmlPrivate *priv;
};

static void gszn_backend_xml_interface_init(GsznBackendInterface *iface);

G_DEFINE_TYPE_WITH_CODE(GsznBackendXml, gszn_backend_xml, G_TYPE_OBJECT,
                        G_ADD_PRIVATE(GsznBackendXml)
                        G_IMPLEMENT_INTERFACE(GSZN_TYPE_BACKEND,
                                        gszn_backend_xml_interface_init))

/*
 * Helpers
 */

static guint
count_object_properties(xmlNode *object_node)
{
	return xmlChildElementCount(object_node);
}

static xmlNode *
find_object_node(xmlNode *root, const gchar *object_name, const gchar *object_uid)
{
	xmlNode *object_node;

	for (object_node = root->children; object_node; object_node = object_node->next) {
		/* Match object name */
		if (g_strcmp0((const gchar *) object_node->name, object_name))
			continue;

		/* Match object uid */
		if (object_uid) {
			xmlChar *uid;
			gboolean match;

			uid = xmlGetProp(object_node, (xmlChar *) "uid");
			if (!uid)
				continue;

			match = !g_strcmp0((const gchar *) uid, object_uid);

			xmlFree(uid);

			if (match == FALSE)
				continue;
		}

		/* Gotcha ! */
		return object_node;
	}

	return NULL;
}

static xmlNode *
create_property_node(xmlNode *object_node, const gchar *prop_name)
{
	xmlNode *prop_node;

	prop_node = xmlNewChild(object_node, NULL, (const xmlChar *) prop_name, NULL);

	return prop_node;
}

static xmlNode *
create_object_node(xmlNode *root, const gchar *object_name, const gchar *object_uid)
{
	xmlNode *object_node;

	object_node = xmlNewChild(root, NULL, (const xmlChar *) object_name, NULL);
	if (object_uid)
		xmlSetProp(object_node, (const xmlChar *) "uid", (const xmlChar *) object_uid);

	return object_node;
}

/*
 * GsznBackend interface methods
 */

struct _GsznBackendXmlIter {
	xmlNode *cur;
	gchar *object_uid;
};

typedef struct _GsznBackendXmlIter GsznBackendXmlIter;

#define GSZN_BACKEND_XML_ITER(i) ((GsznBackendXmlIter *)(i))

static GsznBackendIter *
gszn_backend_xml_get_uninitialized_iter(GsznBackend *backend G_GNUC_UNUSED)
{
	GsznBackendXmlIter *iter;

	iter = g_new0(GsznBackendXmlIter, 1);
	iter->cur = NULL;

	return GSZN_BACKEND_ITER(iter);
}

static void
gszn_backend_xml_free_iter(GsznBackend *backend G_GNUC_UNUSED, GsznBackendIter *backend_iter)
{
	GsznBackendXmlIter *iter = GSZN_BACKEND_XML_ITER(backend_iter);

	g_free(iter->object_uid);
	g_free(iter);
}

static gboolean
gszn_backend_xml_loop(GsznBackend *backend, GsznBackendIter *backend_iter,
                      const gchar **object_name)
{
	GsznBackendXml *self = GSZN_BACKEND_XML(backend);
	GsznBackendXmlPrivate *priv = self->priv;
	GsznBackendXmlIter *iter = GSZN_BACKEND_XML_ITER(backend_iter);

	g_free(iter->object_uid);
	iter->object_uid = NULL;

	if (iter->cur == NULL)
		iter->cur = priv->root->children;
	else
		iter->cur = iter->cur->next;

	if (object_name)
		*object_name = iter->cur ? (const gchar *) iter->cur->name : NULL;

	return iter->cur ? TRUE : FALSE;
}

static const gchar *
gszn_backend_xml_get_object_uid(GsznBackend *backend G_GNUC_UNUSED, GsznBackendIter *backend_iter)
{
	GsznBackendXmlIter *iter = GSZN_BACKEND_XML_ITER(backend_iter);
	xmlNode *object_node;
	xmlChar *content;

	object_node = iter->cur;
	if (object_node == NULL)
		return NULL;

	if (iter->object_uid)
		return iter->object_uid;

	content = xmlGetProp(object_node, (const xmlChar *) "uid");
	if (content == NULL)
		return NULL;

	iter->object_uid = g_strdup((gchar *) content);
	xmlFree(content);

	return iter->object_uid;
}

static GsznParameter *
gszn_backend_xml_get_properties(GsznBackend *backend G_GNUC_UNUSED, GsznBackendIter *backend_iter,
                                guint *n_params)
{
	GsznBackendXmlIter *iter = GSZN_BACKEND_XML_ITER(backend_iter);
	GsznParameter *params;
	xmlNode *object_node;
	xmlNode *prop_node;
	guint i, n;

	g_return_val_if_fail(n_params != NULL, NULL);
	*n_params = 0;

	object_node = iter->cur;
	if (object_node == NULL)
		return NULL;

	n = count_object_properties(object_node);

	params = g_new0(GsznParameter, n);

	for (prop_node = object_node->children, i = 0; prop_node; prop_node = prop_node->next) {
		GsznParameter *param = &params[i];
		xmlChar *content;

		content = xmlNodeGetContent(prop_node);
		if (content) {
			param->name = g_strdup((gchar *) prop_node->name);
			param->string = g_strdup((gchar *) content);

			xmlFree(content);
			++i;
		}
	}

	*n_params = i;
	return params;
}

static void
gszn_backend_xml_add_properties(GsznBackend *backend G_GNUC_UNUSED, GsznBackendIter *backend_iter,
                                GsznParameter *params, guint n_params)
{
	GsznBackendXmlIter *iter = GSZN_BACKEND_XML_ITER(backend_iter);
	xmlNode *object_node;
	guint i;

	object_node = iter->cur;
	if (object_node == NULL)
		return;

	for (i = 0; i < n_params; i++) {
		GsznParameter *param = &params[i];
		xmlNode *prop_node;

		prop_node = create_property_node(object_node, param->name);

		xmlNodeSetContent(prop_node, (xmlChar *) param->string);

		g_free(param->name);
		g_free(param->string);
	}

	g_free(params);
}

static GsznBackendIter *
gszn_backend_xml_get_object(GsznBackend *backend, const gchar *object_name,
                            const gchar *object_uid)
{
	GsznBackendXml *self = GSZN_BACKEND_XML(backend);
	GsznBackendXmlPrivate *priv = self->priv;
	GsznBackendXmlIter *iter;
	xmlNode *object_node;

	object_node = find_object_node(priv->root, object_name, object_uid);

	if (object_node == NULL)
		return NULL;

	iter = g_new0(GsznBackendXmlIter, 1);
	iter->cur = object_node;

	return GSZN_BACKEND_ITER(iter);
}

static GsznBackendIter *
gszn_backend_xml_add_object(GsznBackend *backend, const gchar *object_name,
                            const gchar *object_uid)
{
	GsznBackendXml *self = GSZN_BACKEND_XML(backend);
	GsznBackendXmlPrivate *priv = self->priv;
	GsznBackendXmlIter *iter;
	xmlNode *object_node;

	object_node = create_object_node(priv->root, object_name, object_uid);

	iter = g_new0(GsznBackendXmlIter, 1);
	iter->cur = object_node;

	return GSZN_BACKEND_ITER(iter);
}

static gchar *
gszn_backend_xml_print(GsznBackend *backend)
{
	GsznBackendXml *self = GSZN_BACKEND_XML(backend);
	GsznBackendXmlPrivate *priv = self->priv;
	xmlDoc *doc = priv->doc;
	xmlChar *xmlbody;
	gchar *text;
	int len;

	if (doc == NULL)
		return NULL;

	xmlDocDumpFormatMemory(doc, &xmlbody, &len, 1);
	text = g_strndup((gchar *) xmlbody, len);
	xmlFree(xmlbody);

	return text;
}

static gboolean
gszn_backend_xml_parse(GsznBackend *backend, const gchar *text, GError **err)
{
	GsznBackendXml *self = GSZN_BACKEND_XML(backend);
	GsznBackendXmlPrivate *priv = self->priv;
	xmlDoc *doc;
	xmlNode *root;

	g_return_val_if_fail(text != NULL, FALSE);
	g_return_val_if_fail(err == NULL || *err == NULL, FALSE);

	doc = xmlReadMemory(text, strlen(text), NULL, NULL, XML_PARSE_NOBLANKS);
//	doc = xmlParseMemory(text, strlen(text));
	if (doc == NULL) {
		xmlError *xml_err;
		const gchar *message;

		/* I think this is useless... */
		xml_err = xmlGetLastError();
		if (xml_err)
			message = xml_err->message;
		else
			message = "XML parse error";

		g_set_error_literal(err, GSZN_ERROR, GSZN_ERROR_PARSE,
		                    message);
		return FALSE;
	}

	root = xmlDocGetRootElement(doc);
	if (root == NULL) {
		g_set_error_literal(err, GSZN_ERROR, GSZN_ERROR_PARSE,
		                    "Failed to get root element");
		return FALSE;
	}

	if (priv->doc) {
		xmlFreeDoc(priv->doc);
	}

	priv->doc = doc;
	priv->root = root;

	return TRUE;
}

static void
gszn_backend_xml_interface_init(GsznBackendInterface *iface)
{
	iface->print = gszn_backend_xml_print;
	iface->parse = gszn_backend_xml_parse;

	iface->free_iter              = gszn_backend_xml_free_iter;
	iface->get_uninitialized_iter = gszn_backend_xml_get_uninitialized_iter;
	iface->loop                   = gszn_backend_xml_loop;

	iface->get_object      = gszn_backend_xml_get_object;
	iface->add_object      = gszn_backend_xml_add_object;
	iface->get_object_uid  = gszn_backend_xml_get_object_uid;
	iface->get_properties  = gszn_backend_xml_get_properties;
	iface->add_properties  = gszn_backend_xml_add_properties;
}

/*
 * GObject methods
 */

static void
gszn_backend_xml_finalize(GObject *object)
{
	GsznBackendXml *self = GSZN_BACKEND_XML(object);
	GsznBackendXmlPrivate *priv = self->priv;

	/* Free resources */
	if (priv->doc)
		xmlFreeDoc(priv->doc);

	/* Chain up */
	G_OBJECT_CLASS(gszn_backend_xml_parent_class)->finalize(object);
}

static void
gszn_backend_xml_constructed(GObject *object)
{
	GsznBackendXml *self = GSZN_BACKEND_XML(object);
	GsznBackendXmlPrivate *priv = self->priv;

	/* Create document */
	priv->doc = xmlNewDoc((const xmlChar *) "1.0");
	priv->root = xmlNewDocNode(priv->doc, NULL, (const xmlChar *) PACKAGE_CAMEL_NAME, NULL);
	xmlDocSetRootElement(priv->doc, priv->root);

	/* Chain up */
	if (G_OBJECT_CLASS(gszn_backend_xml_parent_class)->constructed)
		G_OBJECT_CLASS(gszn_backend_xml_parent_class)->constructed(object);
}

static void
gszn_backend_xml_init(GsznBackendXml *self)
{
	/* Initialize private pointer */
	self->priv = gszn_backend_xml_get_instance_private(self);
}

static void
gszn_backend_xml_class_init(GsznBackendXmlClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	/* Override GObject methods */
	object_class->finalize = gszn_backend_xml_finalize;
	object_class->constructed = gszn_backend_xml_constructed;
}

/*
 * Global methods
 */

void
gszn_backend_xml_cleanup(void)
{
	xmlCleanupParser();
}

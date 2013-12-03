/*
 * ovirt-vm-pool.c: oVirt virtual machine pool
 *
 * Copyright (C) 2013 Iordan Iordanov <i@iiordanov.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include <stdlib.h>
#include <rest/rest-xml-node.h>
#include <rest/rest-xml-parser.h>

#include "govirt.h"
#include "govirt-private.h"


static gboolean ovirt_vm_pool_refresh_from_xml(OvirtVmPool *vm_pool, RestXmlNode *node);
#define OVIRT_VM_POOL_GET_PRIVATE(obj)                         \
        (G_TYPE_INSTANCE_GET_PRIVATE((obj), OVIRT_TYPE_VM_POOL, OvirtVmPoolPrivate))

struct _OvirtVmPoolPrivate {
        guint prestarted_vms;
        guint max_user_vms;
        guint size;
};

G_DEFINE_TYPE(OvirtVmPool, ovirt_vm_pool, OVIRT_TYPE_RESOURCE);

enum OvirtResponseStatus {
    OVIRT_RESPONSE_UNKNOWN,
    OVIRT_RESPONSE_FAILED,
    OVIRT_RESPONSE_PENDING,
    OVIRT_RESPONSE_IN_PROGRESS,
    OVIRT_RESPONSE_COMPLETE
};

enum {
    PROP_0,
    PROP_SIZE,
    PROP_PRESTARTED_VMS,
    PROP_MAX_USER_VMS
};

static void ovirt_vm_pool_get_property(GObject *object,
                                       guint prop_id,
                                       GValue *value,
                                       GParamSpec *pspec)
{
    OvirtVmPool *vm_pool = OVIRT_VM_POOL(object);

    switch (prop_id) {
    case PROP_SIZE:
        g_value_set_uint(value, vm_pool->priv->size);
        break;
    case PROP_PRESTARTED_VMS:
        g_value_set_uint(value, vm_pool->priv->prestarted_vms);
        break;
    case PROP_MAX_USER_VMS:
        g_value_set_uint(value, vm_pool->priv->max_user_vms);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void ovirt_vm_pool_set_property(GObject *object,
                                       guint prop_id,
                                       const GValue *value,
                                       GParamSpec *pspec)
{
    OvirtVmPool *vm_pool = OVIRT_VM_POOL(object);

    switch (prop_id) {
    case PROP_SIZE:
        vm_pool->priv->size = g_value_get_uint(value);
        break;
    case PROP_PRESTARTED_VMS:
        vm_pool->priv->prestarted_vms = g_value_get_uint(value);
        break;
    case PROP_MAX_USER_VMS:
        vm_pool->priv->max_user_vms = g_value_get_uint(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void ovirt_vm_pool_dispose(GObject *object)
{
    G_OBJECT_CLASS(ovirt_vm_pool_parent_class)->dispose(object);
}

static gboolean ovirt_vm_pool_init_from_xml(OvirtResource *resource,
                                            RestXmlNode *node,
                                            GError **error)
{
    gboolean parsed_ok;
    OvirtResourceClass *parent_class;

    parsed_ok = ovirt_vm_pool_refresh_from_xml(OVIRT_VM_POOL(resource), node);
    if (!parsed_ok) {
        return FALSE;
    }
    parent_class = OVIRT_RESOURCE_CLASS(ovirt_vm_pool_parent_class);

    return parent_class->init_from_xml(resource, node, error);
}

static void ovirt_vm_pool_class_init(OvirtVmPoolClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    OvirtResourceClass *resource_class = OVIRT_RESOURCE_CLASS(klass);

    g_type_class_add_private(klass, sizeof(OvirtVmPoolPrivate));

    resource_class->init_from_xml = ovirt_vm_pool_init_from_xml;
    object_class->dispose = ovirt_vm_pool_dispose;
    object_class->get_property = ovirt_vm_pool_get_property;
    object_class->set_property = ovirt_vm_pool_set_property;

    g_object_class_install_property(object_class,
                                    PROP_SIZE,
                                    g_param_spec_uint("size",
                                                      "Size of pool",
                                                      "The number of VMs in the pool",
                                                      0,
                                                      G_MAXUINT,
                                                      0,
                                                      G_PARAM_READWRITE));
    g_object_class_install_property(object_class,
                                    PROP_PRESTARTED_VMS,
                                    g_param_spec_uint("prestarted_vms",
                                                      "Prestarted VMs",
                                                      "The number of VMs prestarted in the pool",
                                                      0,
                                                      G_MAXUINT,
                                                      0,
                                                      G_PARAM_READWRITE));
    g_object_class_install_property(object_class,
                                    PROP_MAX_USER_VMS,
                                    g_param_spec_uint("max_user_vms",
                                                      "Max VMs per user",
                                                      "The number of VMs a user can allocate from the pool",
                                                      0,
                                                      G_MAXUINT,
                                                      0,
                                                      G_PARAM_READWRITE));
}

static void ovirt_vm_pool_init(G_GNUC_UNUSED OvirtVmPool *vm_pool)
{
    vm_pool->priv = OVIRT_VM_POOL_GET_PRIVATE(vm_pool);
}

OvirtVmPool *ovirt_vm_pool_new(void)
{
    return OVIRT_VM_POOL(g_initable_new(OVIRT_TYPE_VM_POOL, NULL, NULL, NULL));
}


gboolean ovirt_vm_pool_allocate_vm(OvirtVmPool *vm_pool, OvirtProxy *proxy, GError **error)
{
    return ovirt_resource_action(OVIRT_RESOURCE(vm_pool), proxy, "allocatevm", NULL, error);
}


static gboolean vm_pool_set_size_from_xml(OvirtVmPool *vm_pool, RestXmlNode *node)
{
    RestXmlNode *size_node;
    size_node = rest_xml_node_find(node, "size");
    if (size_node != NULL) {
        guint size;
        g_return_val_if_fail(size_node->content != NULL, FALSE);
        if (!ovirt_utils_guint_from_string(size_node->content, &size)) {
            return FALSE;
        }
        g_object_set(G_OBJECT(vm_pool), "size", size, NULL);
        return TRUE;
    }
    return FALSE;
}


static gboolean vm_pool_set_prestarted_vms_from_xml(OvirtVmPool *vm_pool, RestXmlNode *node)
{
    RestXmlNode *prestarted_vms_node;
    prestarted_vms_node = rest_xml_node_find(node, "prestarted_vms");
    if (prestarted_vms_node != NULL) {
        guint prestarted_vms;
        g_return_val_if_fail(prestarted_vms_node->content != NULL, FALSE);
        if (!ovirt_utils_guint_from_string(prestarted_vms_node->content, &prestarted_vms)) {
            return FALSE;
        }
        g_object_set(G_OBJECT(vm_pool), "prestarted_vms", prestarted_vms, NULL);
        return TRUE;
    }
    return FALSE;
}


static gboolean vm_pool_set_max_user_vms_from_xml(OvirtVmPool *vm_pool, RestXmlNode *node)
{
    RestXmlNode *max_user_vms_node;
    max_user_vms_node = rest_xml_node_find(node, "max_user_vms");
    if (max_user_vms_node != NULL) {
        guint max_user_vms;
        g_return_val_if_fail(max_user_vms_node->content != NULL, FALSE);
        if (!ovirt_utils_guint_from_string(max_user_vms_node->content, &max_user_vms)) {
            return FALSE;
        }
        g_object_set(G_OBJECT(vm_pool), "max_user_vms", max_user_vms, NULL);
        return TRUE;
    }
    return FALSE;
}


static gboolean ovirt_vm_pool_refresh_from_xml(OvirtVmPool *vm_pool, RestXmlNode *node)
{
    vm_pool_set_size_from_xml(vm_pool, node);
    vm_pool_set_prestarted_vms_from_xml(vm_pool, node);
    vm_pool_set_max_user_vms_from_xml(vm_pool, node);
    return TRUE;
}
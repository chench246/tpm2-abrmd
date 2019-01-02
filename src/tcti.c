/* SPDX-License-Identifier: BSD-2 */
/*
 * Copyright (c) 2017 - 2018, Intel Corporation
 * All rights reserved.
 */
#include <dlfcn.h>
#include <inttypes.h>

#include "tcti.h"
#include "util.h"

G_DEFINE_TYPE (Tcti, tcti, G_TYPE_OBJECT);

enum {
    PROP_0,
    PROP_CONTEXT,
    PROP_DL_HANDLE,
    N_PROPERTIES
};
static GParamSpec *obj_properties [N_PROPERTIES] = { NULL };
/*
 * GObject property setter.
 */
static void
tcti_set_property (GObject        *object,
                   guint           property_id,
                   GValue const   *value,
                   GParamSpec     *pspec)
{
    Tcti *self = TCTI (object);

    g_debug ("%s", __func__);
    switch (property_id) {
    case PROP_CONTEXT:
        self->tcti_context = (TSS2_TCTI_CONTEXT*)g_value_get_pointer (value);
        break;
    case PROP_DL_HANDLE:
        self->tcti_dl_handle = g_value_get_pointer (value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}
/*
 * GObject property getter.
 */
static void
tcti_get_property (GObject     *object,
                   guint        property_id,
                   GValue      *value,
                   GParamSpec  *pspec)
{
    Tcti *self = TCTI (object);

    g_debug ("%s: %p", __func__, objid (self));
    switch (property_id) {
    case PROP_CONTEXT:
        g_value_set_pointer (value, self->tcti_context);
        break;
    case PROP_DL_HANDLE:
        g_value_set_pointer (value, self->tcti_dl_handle);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
tcti_dispose (GObject *object)
{
    Tcti *self = TCTI (object);

    g_debug ("%s: Tcti %p", __func__, objid (self));
    if (self->tcti_context) {
        Tss2_Tcti_Finalize (self->tcti_context);
    }
    G_OBJECT_CLASS (tcti_parent_class)->dispose (object);
}
static void
tcti_finalize (GObject *object)
{
    Tcti *self = TCTI (object);

    g_debug ("%s: Tcti %p", __func__, objid (self));
#if !defined (DISABLE_DLCLOSE)
    g_clear_pointer (&self->tcti_dl_handle, dlclose);
#endif
    g_clear_pointer (&self->tcti_context, g_free);
    G_OBJECT_CLASS (tcti_parent_class)->finalize (object);
}

static void
tcti_init (Tcti *tcti)
{
    UNUSED_PARAM (tcti);
}
static void
tcti_class_init (TctiClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    if (tcti_parent_class == NULL)
        tcti_parent_class = g_type_class_peek_parent (klass);
    object_class->dispose = tcti_dispose;
    object_class->finalize = tcti_finalize;
    object_class->get_property = tcti_get_property;
    object_class->set_property = tcti_set_property;

    obj_properties [PROP_CONTEXT] =
        g_param_spec_pointer ("tcti-context",
                              "TSS2_TCTI_CONTEXT",
                              "TSS2_TCTI_CONTEXT",
                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
    obj_properties [PROP_DL_HANDLE] =
        g_param_spec_pointer ("tcti-dl-handle",
                              "dl handle",
                              "dl handle",
                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
    g_object_class_install_properties (object_class,
                                       N_PROPERTIES,
                                       obj_properties);
}
Tcti*
tcti_new (TSS2_TCTI_CONTEXT *tcti_context,
          void *tcti_dl_handle)
{
    return TCTI (g_object_new (TYPE_TCTI,
                               "tcti-context", tcti_context,
                               "tcti-dl-handle", tcti_dl_handle,
                               NULL));
}

TSS2_TCTI_CONTEXT*
tcti_peek_context (Tcti *self)
{
    g_debug ("%s: %p", __func__, objid (self));
    return self->tcti_context;
}
/**
 * The rest of these functions are just wrappers around the macros provided
 * by the TSS for calling the TCTI functions. There's no implementation for
 * 'getPollHandles' yet since I've got no need for it yet.
 */
TSS2_RC
tcti_transmit (Tcti      *self,
               size_t     size,
               uint8_t   *command)
{
    return Tss2_Tcti_Transmit (self->tcti_context,
                               size,
                               command);
}
TSS2_RC
tcti_receive (Tcti      *self,
              size_t    *size,
              uint8_t   *response,
              int32_t    timeout)
{
    return Tss2_Tcti_Receive (self->tcti_context,
                              size,
                              response,
                              timeout);
}

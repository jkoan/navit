/*
 * Navit, a modular navigation system.
 * Copyright (C) 2005-2009 Navit Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

/** @file headup.c
 * @defgroup headup-plugins headup plugins
 * @ingroup plugins
 * @brief Generic components of the headup object.
 *
 * This file implements the generic headup interface, i.e. everything which is
 * not specific to a single data source.
 *
 * @author Navit Team
 * @date 2005-2014
 */

#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <time.h>
#include <math.h> /* for sqrt from coord.h */
#include "config.h"
#include "debug.h"
#include "coord.h"
#include "item.h"
#include "xmlconfig.h"
#include "log.h"
#include "plugin.h"
#include "transform.h"
#include "util.h"
#include "event.h"
#include "coord.h"
#include "transform.h"
#include "projection.h"
#include "point.h"
#include "graphics.h"
#include "callback.h"
#include "color.h"
#include "layout.h"
#include "headup.h"
#include "navit_nls.h"
#include "navit.h"

struct headup {
    NAVIT_OBJECT
    struct headup_methods meth;
    struct headup_priv *priv;
    struct callback_list *cbl;
};

struct object_func headup_func;

/**
 * @brief Creates a new headup
 *
 * @param parent
 * @param attrs Points to a null-terminated array of pointers to the attributes
 * for the new headup type.
 *
 * @return The newly created headup object
 */
struct headup*
headup_new(struct attr *parent, struct attr **attrs) {
    struct headup *this_;
    struct attr *type_attr;
    struct headup_priv* (*headuptype_new)(struct navit *navit, struct headup_methods *meth, struct callback_list *cbl,
                                          struct attr **attrs);
    char *type;

    dbg(lvl_debug, "enter");
    type_attr = attr_search(attrs, attr_type);
    if (!type_attr) {
        dbg(lvl_error, "incomplete headup definition: missing attribute 'type_attr'");
        return NULL;
    }

    type = g_strdup(type_attr->u.str);
    dbg(lvl_debug, "type='%s'", type_attr->u.str);

    headuptype_new = plugin_get_category_headup(type);
    if (!headuptype_new) {
        dbg(lvl_error, "invalid type_attr '%s'. Did you enable the plugin with \"cmake -Dplugin/%s=TRUE\"?", type_attr->u.str,
            type);
        g_free(type);
        return NULL;
    }
    //g_free(type);

    this_ = g_new0(struct headup, 1);
    this_->func = &headup_func;
    navit_object_ref((struct navit_object*) this_);
    this_->cbl = callback_list_new();
    this_->priv = headuptype_new(parent->u.navit, &this_->meth, this_->cbl, attrs);
    if (!this_->priv) {
        dbg(lvl_error, "headuptype_new failed");
        callback_list_destroy(this_->cbl);
        g_free(this_);
        return NULL;
    }
    this_->attrs = attr_list_dup(attrs);

    if(!strcmp(type, "obd2")) {
        navit_set_obd2(parent->u.navit, this_);
    }
    if(!strcmp(type, "btheadup")) {
        navit_set_btheadup(parent->u.navit, this_);
    }
    if(!strcmp(type, "googleglass")) {
        navit_set_googleglass(parent->u.navit, this_);
    }
    if(!strcmp(type, "tpms")) {
        navit_set_tpms(parent->u.navit, this_);
    }
    g_free(type);

    return this_;
}

/**
 * @brief Destroys a headup
 *
 * @param this_ The headup to destroy
 */
void headup_destroy(struct headup *this_) {
#pragma unused(this_)
//    dbg(lvl_debug, "enter");
//    this_->meth.destroy(this_->priv);
//    callback_list_destroy(this_->cbl);
//    attr_list_free(this_->attrs);
//    g_free(this_);
}

/**
 * Creates an attribute iterator to be used with headups
 */
struct attr_iter*
headup_attr_iter_new(void *unused) {
#pragma unused(unused)
    return (struct attr_iter*) g_new0(void*, 1);
}

/**
 * Destroys a headup attribute iterator
 *
 * @param iter a headup attr_iter
 */
void headup_attr_iter_destroy(struct attr_iter *iter) {
    g_free(iter);
}

/**
 * Generic get function
 *
 * @param this_ Pointer to a headup structure
 * @param type The attribute type to look for
 * @param attr Pointer to a {@code struct attr} to store the attribute
 * @param iter A headup attr_iter. This is only used for generic attributes; for attributes specific to the headup object it is ignored.
 * @return True for success, false for failure
 */
int headup_get_attr(struct headup *this_, enum attr_type type, struct attr *attr, struct attr_iter *iter) {
    int ret;

    if (this_->meth.get_attr) {
        ret=this_->meth.get_attr(this_->priv, type, attr);
        if (ret)
            return ret;
    }

    return attr_generic_get_attr(this_->attrs, NULL, type, attr, iter);
}

/**
 * Generic set function
 *
 * @param this_ A headup
 * @param attr The attribute to set
 * @return False on success, true on failure
 */
int headup_set_attr(struct headup *this_, struct attr *attr) {
#pragma unused(this_, attr)
    int ret = 1;
//    if (attr->type == attr_log_gpx_desc) {
//        g_free(this_->gpx_desc);
//        this_->gpx_desc = g_strdup(attr->u.str);
//    } else if (this_->meth.set_attr)
//        ret=this_->meth.set_attr(this_->priv, attr);
//    /* attr_profilename probably is never used by headup itself but it's used to control the
//      routing engine. So any headup should allow to set and read it. */
//    if(attr->type == attr_profilename)
//        ret=1;
//    if (ret == 1 && attr->type != attr_navit && attr->type != attr_pdl_gps_update)
//        this_->attrs=attr_generic_set_attr(this_->attrs, attr);
    return ret != 0;
}

/**
 * Generic add function
 *
 * @param this_ A headup
 * @param attr The attribute to add
 *
 * @return true if the attribute was added, false if not.
 */
int headup_add_attr(struct headup *this_, struct attr *attr) {
    int ret = 1;
    switch (attr->type) {
    case attr_callback:
        callback_list_add(this_->cbl, attr->u.callback);
        break;
    default:
        break;
    }
    if (ret)
        this_->attrs = attr_generic_add_attr(this_->attrs, attr);
    return ret;
}

/**
 * @brief Generic remove function.
 *
 * Used to remove a callback from the headup.
 * @param this_ A headup
 * @param attr
 */
int headup_remove_attr(struct headup *this_, struct attr *attr) {
    switch (attr->type) {
    case attr_callback:
        callback_list_remove(this_->cbl, attr->u.callback);
        break;
//    case attr_log:
//        cb=g_hash_table_lookup(this_->log_to_cb, attr->u.log);
//        if (!cb)
//            return 0;
//        g_hash_table_remove(this_->log_to_cb, attr->u.log);
//        callback_list_remove(this_->cbl, cb);
//        break;
    default:
        this_->attrs = attr_generic_remove_attr(this_->attrs, attr);
        return 0;
    }
    return 1;
}

struct object_func headup_func = {
    attr_headup,
    (object_func_new) headup_new,
    (object_func_get_attr) headup_get_attr,
    (object_func_iter_new) headup_attr_iter_new,
    (object_func_iter_destroy) headup_attr_iter_destroy,
    (object_func_set_attr) headup_set_attr,
    (object_func_add_attr) headup_add_attr,
    (object_func_remove_attr) headup_remove_attr,
    (object_func_init) NULL,
    (object_func_destroy) headup_destroy,
    (object_func_dup) NULL,
    (object_func_ref) navit_object_ref,
    (object_func_unref) navit_object_unref,
};

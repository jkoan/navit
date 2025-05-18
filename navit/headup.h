/**
 * Navit, a modular navigation system.
 * Copyright (C) 2005-2009 Navit Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#ifndef NAVIT_HEADUP_H
#define NAVIT_HEADUP_H

#ifdef __cplusplus
extern "C" {
#endif

struct headup_priv;
enum attr_type;

struct headup_methods {
    void (*destroy)(struct headup_priv *priv);
    int (*set_attr)(struct headup_priv *priv, struct attr *attr);
    int (*get_attr)(struct headup_priv *priv, enum attr_type type, struct attr *attr);
};

/* prototypes */
struct attr;
struct attr_iter;
struct headup;
struct point;
struct cursor;
struct graphics;
struct headup *headup_new(struct attr *parent, struct attr **attrs);
void headup_destroy(struct headup *this_);
struct attr_iter *headup_attr_iter_new(void * unused);
void headup_attr_iter_destroy(struct attr_iter *iter);
int headup_get_attr(struct headup *this_, enum attr_type type, struct attr *attr, struct attr_iter *iter);
int headup_set_attr(struct headup *this_, struct attr *attr);
int headup_add_attr(struct headup *this_, struct attr *attr);
int headup_remove_attr(struct headup *this_, struct attr *attr);
void headup_set_cursor(struct headup *this_, struct cursor *cursor, int overwrite);
void headup_draw(struct headup *this_, struct graphics *gra, struct point *pnt, int angle, int speed);
int headup_get_cursor_data(struct headup *this_, struct point *pnt, int *angle, int *speed);
void headup_log_gpx_add_tag(char *tag, char **logstr);
struct headup * headup_ref(struct headup *this_);
void headup_unref(struct headup *this_);
/* end of prototypes */

#ifdef __cplusplus
}
#endif

#endif


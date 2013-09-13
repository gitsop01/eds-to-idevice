/*
 * Copyright (C) 2011 Christophe Fergeau <cfergeau@gmail.com>
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
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef ETI_PLIST_H
#define ETI_PLIST_H

#include <glib.h>
#include <plist/plist.h>

void eti_plist_set_debug(gboolean enable_debug);
void eti_plist_dump(plist_t plist);
void eti_plist_dict_set_date(plist_t dict, const char *key, GDateTime *date);
GDateTime *eti_plist_dict_get_date(plist_t node, const char *key);
void eti_plist_dict_set_string(plist_t dict,
                               const char *key,
                               const char *value);
char *eti_plist_dict_get_string(plist_t node, const char *key);
void eti_plist_dict_set_data(plist_t dict, const char *key,
                             const guchar *data, gsize len);
guchar *eti_plist_dict_get_data(plist_t node, const char *key, guint64 *len);

#endif

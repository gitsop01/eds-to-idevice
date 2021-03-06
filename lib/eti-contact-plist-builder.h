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
#ifndef ETI_CONTACT_PLIST_BUILDER_H
#define ETI_CONTACT_PLIST_BUILDER_H

#include <glib-2.0/glib.h>
#include <plist/plist.h>

GList *eti_contact_plist_builder_build(GHashTable *contacts);
plist_t eti_contact_plist_builder_build_main(GHashTable *contacts);
GList *eti_contact_plist_builder_build_others(GHashTable *contacts,
                                              plist_t remapped_uids);

#endif


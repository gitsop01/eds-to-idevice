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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1335  USA
 */
#ifndef ETI_CONTACT_PLIST_PARSER_H
#define ETI_CONTACT_PLIST_PARSER_H

#include <glib-2.0/glib.h>
#include <plist/plist.h>

typedef struct _EtiContactPlistParser EtiContactPlistParser;

EtiContactPlistParser *eti_contact_plist_parser_new(void);
gboolean eti_contact_plist_parser_parse(EtiContactPlistParser *parser,
                                        plist_t entities, GError **error);
GHashTable *eti_contact_plist_parser_get_contacts(EtiContactPlistParser *parser);
void eti_contact_plist_parser_free(EtiContactPlistParser *parser,
                                   gboolean free_contacts);
void eti_contact_plist_parser_dump(EtiContactPlistParser *parser);
#endif

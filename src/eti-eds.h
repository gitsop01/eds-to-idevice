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
#ifndef ETI_EDS_H
#define ETI_EDS_H

#include <glib-2.0/glib.h>
#include <evolution-data-server/libebook/libebook.h>
#include "eti-contact.h"

#define ETI_EBOOK_ERROR eti_ebook_error_quark()

typedef enum {
    ETI_EBOOK_ERROR_FAILED,
    ETI_EBOOK_ERROR_ADDRESSBOOK,
    ETI_EBOOK_ERROR_QUERY
} EtiEbookError;

GQuark eti_ebook_error_quark(void);
GSList *eti_eds_get_contacts(EBookClient *client,
                            const gchar *query_str,
                            GError **error);
EBookClient *eti_eds_open_addressbook(void);
char *eti_eds_get_econtact_uid(EContact *econtact);
EtiContact *eti_contact_from_econtact(EContact *econtact);
void eti_eds_dump_addressbooks(void);

#endif

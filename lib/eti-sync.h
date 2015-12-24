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
#ifndef ETI_SYNC_H
#define ETI_SYNC_H

#include <glib-2.0/glib.h>

#define ETI_SYNC_ERROR eti_sync_error_quark()

typedef enum {
    ETI_SYNC_ERROR_FAILED,
    ETI_SYNC_ERROR_IDEVICE_COMMUNICATION,
    ETI_SYNC_ERROR_READING,
    ETI_SYNC_ERROR_WRITING,
    ETI_SYNC_ERROR_SYNCING
} EtiSyncError;

typedef struct _EtiSync EtiSync;

GQuark eti_sync_error_quark(void);
EtiSync *eti_sync_new(const char *uuid, GError **error);
gboolean eti_sync_start_sync(EtiSync *sync, GError **error);
GHashTable *eti_sync_get_contacts(EtiSync *sync, GError **error);
void eti_sync_wipe_all_contacts(EtiSync *sync, GError **error);
void eti_sync_send_contacts(EtiSync *sync, GHashTable *contacts,
                            GError **error);
void eti_sync_stop_sync(EtiSync *sync, GError **error);
void eti_sync_free(EtiSync *sync);
#endif

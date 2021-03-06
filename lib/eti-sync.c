/*
 * Copyright (C) 2011 Christophe Fergeau <cfergeau@gmail.com>
 * Copyright (C) 2016-2020 Timothy Ward <gtwa001@gmail.com>
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
#include "eti-contact.h"
#include "eti-contact-plist-builder.h"
#include "eti-contact-plist-parser.h"
#include "eti-plist.h"
#include "eti-sync.h"

#include <glib-2.0/glib.h>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/mobilesync.h>
#include <libimobiledevice/lockdown.h>

static const uint64_t EDI_CLASS_STORAGE_VERSION = 106;

GQuark eti_sync_error_quark(void)
{
    return g_quark_from_static_string("eti-sync-error-quark");
}

struct _EtiSync {
    idevice_t idevice;
    mobilesync_client_t msync;
};

EtiSync *eti_sync_new(const char *uuid, GError **error)
{
    EtiSync *sync;
    lockdownd_client_t lockdownd = NULL;
   /* uint16_t port = 0; */
    idevice_error_t i_status;
    lockdownd_error_t l_status;
    mobilesync_error_t m_status;
	lockdownd_service_descriptor_t service = NULL; /* FIXME */
	
	
	
	/* I think we are missing the uuid of the device here TW 09-04-16 */

    sync = g_new0(EtiSync, 2);
    i_status = idevice_new(&sync->idevice, uuid);
    if (IDEVICE_E_SUCCESS != i_status) {
        g_set_error(error, ETI_SYNC_ERROR,
                    ETI_SYNC_ERROR_IDEVICE_COMMUNICATION,
                    "failed to connect to device\n");
        goto error;
    }

    l_status = lockdownd_client_new_with_handshake(sync->idevice, &lockdownd,
                                                   "eds-to-idevice");
    if (LOCKDOWN_E_SUCCESS != l_status) {
        g_set_error(error, ETI_SYNC_ERROR,
                    ETI_SYNC_ERROR_IDEVICE_COMMUNICATION,
                    "failed to create lockdownd client\n");
        goto error;
    }

	/* FIXME passing argument 3 of ‘lockdownd_start_service’ from incompatible pointer type */
	/* [-Werror=incompatible-pointer-types]  &port);  */
	/* note: expected ‘struct lockdownd_service_descriptor **’ but argument is of type */
    /* ‘uint16_t  {aka short unsigned int }’ */
	/*  lockdownd_error_t lockdownd_start_service(lockdownd_client_t client, const char */ 
	/* identifier, lockdownd_service_descriptor_t service); TW 10-1-2016 */

    l_status = lockdownd_start_service(lockdownd, "com.apple.mobilesync", &service);
    if (LOCKDOWN_E_SUCCESS != l_status) {
        g_set_error(error, ETI_SYNC_ERROR,
                    ETI_SYNC_ERROR_IDEVICE_COMMUNICATION,
                    "failed to start com.apple.mobilesync service\n");
        goto error;
    }

	/* FIXME passing argument 2 of ‘mobilesync_client_new’ makes pointer from integer without */
	/* a cast [-Werror=int-conversion] */
    /* m_status = mobilesync_client_new(sync->idevice, FIXME port, &sync->msync);  */
	/* note: expected ‘lockdownd_service_descriptor_t {aka struct lockdownd_service_descriptor }’ */
	/*  but argument is of type ‘uint16_t {aka short unsigned int}’ */
	/*  mobilesync_error_t mobilesync_client_new(idevice_t device, lockdownd_service_descriptor_t */ 		/* service, mobilesync_client_t  client); TW 10-1-2016 */



    m_status = mobilesync_client_new(sync->idevice, service, &sync->msync);
    if (MOBILESYNC_E_SUCCESS != m_status) {
        g_set_error(error, ETI_SYNC_ERROR,
                    ETI_SYNC_ERROR_IDEVICE_COMMUNICATION,
                    "failed to create mobilesync client\n");
        goto error;
    }

    lockdownd_client_free(lockdownd);

    return sync;

error:
    mobilesync_client_free(sync->msync);
    lockdownd_client_free(lockdownd);
    idevice_free(sync->idevice);
    g_free(sync);
    return NULL;
}

gboolean eti_sync_start_sync(EtiSync *sync, GError **error)
{
    struct GDateTime;
    gchar *cur_time_str;
    gchar *host_anchor;
    mobilesync_sync_type_t sync_type;
    uint64_t device_data_class_version;
    mobilesync_anchors_t anchors;
    mobilesync_error_t m_status;
	char *ERRor = NULL;

    g_get_real_time();
    cur_time_str = g_date_time_format_iso8601(*GDateTime);
    host_anchor = g_strdup_printf("eti-%s", cur_time_str);
    anchors = mobilesync_anchors_new(NULL, host_anchor);
    g_free(host_anchor);
    g_free(cur_time_str);

	/* FIXME too few arguments to function ‘mobilesync_start’ */
    /*  m_status = mobilesync_start(sync->msync, "com.apple.Contacts", anchors, */
	/* EDI_CLASS_STORAGE_VERSION, &sync_type, &device_data_class_version); */
	/* note: declared here */
	/* mobilesync_error_t mobilesync_start(mobilesync_client_t client, const char *data_class, */
	/* mobilesync_anchors_t anchors, uint64_t computer_data_class_version, mobilesync_sync_type_t */ 		
	/* *sync_type, uint64_t device_data_class_version, FIXME char error_description); TW 10-01-2016 */

	
    m_status = mobilesync_start ( sync->msync, "com.apple.Contacts", anchors,
                                EDI_CLASS_STORAGE_VERSION,
                                &sync_type, &device_data_class_version, &ERRor);
    if (MOBILESYNC_E_INVALID_ARG == m_status){
	g_print("mobilesync-start-arg is invalid\n");
	}
	mobilesync_anchors_free(anchors); 
    if (MOBILESYNC_E_SUCCESS != m_status) {
        g_set_error(error, ETI_SYNC_ERROR,
                    ETI_SYNC_ERROR_SYNCING,
                    "failed to start synchronization\n");
        return FALSE;
    }

#if 0
    g_print("Sync type is ");
    switch (sync_type) {
        case MOBILESYNC_SYNC_TYPE_SLOW:
            g_print("slow\n");
            break;
        case MOBILESYNC_SYNC_TYPE_FAST:
            g_print("fast\n");
            break;
        case MOBILESYNC_SYNC_TYPE_RESET:
            g_print("reset\n");
            break;
    }

    g_print("Device data class version is %"G_GUINT64_FORMAT"\n", device_data_class_version);
#endif

    return TRUE;
}

GHashTable *eti_sync_get_contacts(EtiSync *sync, GError **error)
{
    mobilesync_error_t m_status;
    plist_t entities;
    uint8_t is_last;
    EtiContactPlistParser *parser;
    GHashTable *contacts;

    parser = eti_contact_plist_parser_new();
    if (NULL == parser) {
        g_set_error(error, ETI_SYNC_ERROR,
                    ETI_SYNC_ERROR_READING,
                    "failed to create contacts parser\n");
        return NULL;
    }

    m_status = mobilesync_get_all_records_from_device(sync->msync);
    if (MOBILESYNC_E_SUCCESS != m_status) {
        g_set_error(error, ETI_SYNC_ERROR,
                    ETI_SYNC_ERROR_READING,
                    "failed to ask device for contacts\n");
        eti_contact_plist_parser_free(parser, TRUE);
        return NULL;
    }

    do {
        m_status = mobilesync_receive_changes(sync->msync, &entities,
                                              &is_last, NULL);
        if (MOBILESYNC_E_SUCCESS != m_status) {
            g_set_error(error, ETI_SYNC_ERROR,
                        ETI_SYNC_ERROR_READING,
                        "failed to read contacts from device\n");
            eti_contact_plist_parser_free(parser, TRUE);
            return NULL;
        }

        m_status = mobilesync_acknowledge_changes_from_device(sync->msync);
        if (MOBILESYNC_E_SUCCESS != m_status) {
            g_set_error(error, ETI_SYNC_ERROR,
                        ETI_SYNC_ERROR_READING,
                        "failed to acknowledge receiving contacts\n");
            break;
        }

        eti_plist_dump(entities);

        if (!eti_contact_plist_parser_parse(parser, entities, error))
            break;
        g_assert((error == NULL) || (*error == NULL));

        plist_free(entities);
        entities = NULL;
    } while (!is_last);

    if (entities != NULL) {
        plist_free(entities);
        entities = NULL;
    }
    contacts = eti_contact_plist_parser_get_contacts(parser);
    eti_contact_plist_parser_free(parser, FALSE);
    return contacts;
}

static plist_t send_one(EtiSync *sync, plist_t entities,
                        gboolean is_last, GError **error)
{
    plist_t remapped_identifiers;
    mobilesync_error_t m_status;

    m_status = mobilesync_send_changes(sync->msync, entities, is_last, NULL);
    if (MOBILESYNC_E_SUCCESS != m_status) {
        g_set_error(error, ETI_SYNC_ERROR,
                    ETI_SYNC_ERROR_WRITING,
                    "failed to send contacts to device\n");
        return NULL;
    }

    m_status = mobilesync_remap_identifiers(sync->msync, &remapped_identifiers);
    if (MOBILESYNC_E_SUCCESS != m_status) {
    /*    g_set_error(error, ETI_SYNC_ERROR,
                    ETI_SYNC_ERROR_WRITING,
                    "failed to receive remapped identifiers from device\n"); */
		g_print("Failed to recieve remapped identifiers from device\n");
		if (MOBILESYNC_E_INVALID_ARG == m_status){
			g_print("One of the parameters is invalid\n");
		}
		if (MOBILESYNC_E_PLIST_ERROR == m_status){
			g_print("The current recieved plist is not of valid form\n");
		}
		if (MOBILESYNC_E_WRONG_DIRECTION == m_status){
			g_print("The current sysnc session direction does not permit this call\n");
		}
		if (MOBILESYNC_E_CANCELLED == m_status){	
			g_print("Device explicitly cancelled the session\n");
		}
        return NULL;
    }

    return remapped_identifiers;
}

void eti_sync_send_contacts(EtiSync *sync, GHashTable *contacts,
                            GError **error)
{
    plist_t main_entities;
    plist_t remapped_uids;
    GList *plists;
    GList *it;
    mobilesync_error_t m_status;

    m_status = mobilesync_ready_to_send_changes_from_computer(sync->msync);
    if (MOBILESYNC_E_SUCCESS != m_status) {
        g_set_error(error, ETI_SYNC_ERROR,
                    ETI_SYNC_ERROR_WRITING,
                    "device is not ready to receive new contacts");
        return;
    }

    main_entities = eti_contact_plist_builder_build_main(contacts);
    if (main_entities == NULL) {
        g_set_error(error, ETI_SYNC_ERROR,
                    ETI_SYNC_ERROR_SYNCING,
                    "failed to read contacts from device");
        return;
    }
    remapped_uids = send_one(sync, main_entities, FALSE, error);
    if ((error != NULL) && (*error != NULL)) {
        g_assert(remapped_uids == NULL);
        return;
    }
    plists = eti_contact_plist_builder_build_others(contacts, remapped_uids);
    plist_free(main_entities);
    plist_free(remapped_uids);

    for (it = plists; it != NULL; it = it->next) {
        remapped_uids = send_one(sync, it->data, (it->next == NULL), error);
        if ((error != NULL) && (*error != NULL)) {
            g_assert(remapped_uids == NULL);
            break;
        }
        if (remapped_uids != NULL) {
            plist_free(remapped_uids);
        }
    }

    g_list_foreach(plists, (GFunc)plist_free, NULL);
    g_list_free(plists);
}

void eti_sync_wipe_all_contacts(EtiSync *sync, GError **error)
{
    mobilesync_error_t m_status;
    m_status = mobilesync_clear_all_records_on_device(sync->msync);
    if (MOBILESYNC_E_SUCCESS != m_status) {
        g_set_error(error, ETI_SYNC_ERROR,
                    ETI_SYNC_ERROR_SYNCING,
                    "failed to clear all contacts from device");
    }

    return;
}

void eti_sync_stop_sync(EtiSync *sync, GError **error)
{
    mobilesync_finish(sync->msync);
    mobilesync_client_free(sync->msync);
    sync->msync = NULL;
    idevice_free(sync->idevice);
    sync->idevice = NULL;
}

void eti_sync_free(EtiSync *sync)
{
    if (NULL != sync->msync)
        eti_sync_stop_sync(sync, NULL);

    idevice_free(sync->idevice);
    g_free(sync);
}

/*
 *  Copyright (C) 2011 Christophe Fergeau <cfergeau@gmail.com>
 *  Copyright (C) 2015-2016 Timothy Ward gtwa001@gmail.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.
 *
 */
#include "eti-contact.h"
#include "eti-eds.h"
#include "eti-plist.h"
#include "eti-sync.h"
#include <glib-2.0/glib.h>
#include <gtk/gtk.h>

/* NOTE */
/* One note that isn't well-documented is the program needs to be running */
/* a GLib main loop for the ESourceRegistry/EBookClient methods to work, as */
/* they rely on GLib to invoke D-Bus methods and collect results. */




struct _EtiOptions {
    gboolean transfer;
    gboolean debug;
    gboolean save_photos;
    gboolean wipe_contacts;
    gboolean list_addressbooks;
    gchar *idevice_uuid;
    gchar *addressbook_uri;
};
typedef struct _EtiOptions EtiOptions;

static void eti_options_free(EtiOptions *options)
{
    g_free(options->idevice_uuid);
    g_free(options->addressbook_uri);
    g_free(options);
}


static EtiOptions *parse_command_line(int argc, char **argv, GError **error)
{
    GOptionContext *context;
    gboolean parsing_ok;

    EtiOptions *options = g_new0(EtiOptions, 1);
    GOptionEntry entries[] =
      {
          { "transfer", 't', 0, G_OPTION_ARG_NONE, &options->transfer, "Transfer contacts to the device [default: false]", NULL },
          { "uuid", 'u', 0, G_OPTION_ARG_STRING, &options->idevice_uuid, "uuid of the device to use [default: autodetected]", "M" },
 /*         { "uri", 'f', 0, G_OPTION_ARG_STRING, &options->addressbook_uri, "uri of the addressbook to use [default: system default]", "uri" }, */
          { "list-addressbooks", 'l', 0, G_OPTION_ARG_NONE, &options->list_addressbooks, "list the name and URIs if all available addressbooks", NULL},
          { "save-photos", 'p', 0, G_OPTION_ARG_NONE, &options->save_photos, NULL },
          { "delete-all-contacts", 0, 0, G_OPTION_ARG_NONE, &options->wipe_contacts, "Delete all contacts on the device (DESTRUCTIVE!!) [default: off]", NULL },
          { "debug", 'd', 0, G_OPTION_ARG_NONE, &options->debug, "Dump all XML transfers between the host and the device [default: off]", NULL },
          { NULL }
      };

    context = g_option_context_new ("Transfer evolution-data-server contacts to an iOS device");
    g_option_context_add_main_entries(context, entries, NULL);
    parsing_ok = g_option_context_parse(context, &argc, &argv, error);
    g_option_context_free(context);
    if (!parsing_ok) {
        eti_options_free(options);
        return NULL;
    }

    return options;
}

static EtiContact *create_test_contact(void)
{
    EtiContact *contact = eti_contact_new_person("John", "Doe");
    GDateTime *birthday;

    eti_contact_add_phone_number(contact, ETI_CONTACT_PHONE_NUMBER_TYPE_MOBILE,
                                 NULL, "+666666666");
    eti_contact_add_email(contact, ETI_CONTACT_FIELD_TYPE_HOME,
                          NULL, "nobody@nowhere.com");
    birthday = g_date_time_new_utc(2010, 9, 9, 10, 9, 0);
    eti_contact_set_birthday(contact, birthday);
    g_date_time_unref(birthday);

    return contact;
}

static GHashTable *eds_to_eti_contacts(GList *e_contacts)
{
    GList *it;
    GHashTable *contacts;

    contacts = g_hash_table_new_full(g_str_hash, g_str_equal,
                                     g_free,
                                     (GDestroyNotify)eti_contact_free);
    for (it = e_contacts; it != NULL; it = it->next) {
        EContact *e_contact;
        EtiContact *contact;

        e_contact = (EContact *)it->data;
        contact = eti_contact_from_econtact(e_contact);
        if (contact != NULL) {
            gchar *uid;

            uid = eti_eds_get_econtact_uid(e_contact);
            if (uid == NULL) {
                g_warning("EContact UID was NULL, fallback needed");
                continue;
            }
            g_hash_table_insert(contacts, uid, contact);
        }
    }

    return contacts;
}

static void save_photos(GHashTable *table)
{
    GList *values;
    GList *it;

    values = g_hash_table_get_values(table);
    for (it = values; it != NULL; it = it->next) {
        EtiContact *contact = (EtiContact *)it->data;
        GString *filename;
        const guchar *image_data;
        gsize data_length;

        eti_contact_get_photo(contact, &image_data, &data_length);
        if ((image_data == NULL) || (data_length == 0))
            continue;

        filename = g_string_new(NULL);
        if (eti_contact_get_company_name(contact) != NULL)
            g_string_append(filename, eti_contact_get_company_name(contact));
        else {
            if (eti_contact_get_first_name(contact))
                g_string_append(filename, eti_contact_get_first_name(contact));
            if (eti_contact_get_last_name(contact))
                g_string_append(filename, eti_contact_get_last_name(contact));
        }
        g_string_append(filename, ".jpg");

        g_file_set_contents(filename->str, (const char *)image_data,
                            data_length, NULL);

        g_string_free(filename, TRUE);
    }

    g_list_free(values);
}

static gboolean transfer_eds_contacts(EtiSync *sync,
                                      const char *addressbook_uri,
                                      GError **error)
{
    EBook *addressbook = NULL;
    GList *e_contacts = NULL;
    GHashTable *contacts = NULL;
    gboolean success = FALSE;
	EClient *client; 
/*	EBookClient *client1; */

	/* FIXME addressbook_uri has been deprecated TW 20/12/15 */
	/* addressbook = eti_eds_open_addressbook(addressbook_uri, error); original code */
	/* Original code used addressbook uri to access address books */
	/* Have to return source or EBookClient or ????? for addressbooks here maybe */

	 client = eti_eds_open_addressbook();
    if ((addressbook == NULL) || ((error != NULL) && (*error != NULL))) {
        g_prefix_error(error, "Couldn't open addressbook: ");
        goto out;
    }

    e_contacts = eti_eds_get_contacts((EBookClient *) client, NULL, error);
    if ((error != NULL) && (*error != NULL)) {
        g_prefix_error(error,
                       "Error retrieving contacts from evolution addressbook: ");
        goto out;
    }
    if (e_contacts == NULL) {
        g_prefix_error(error, "No contacts in evolution addressbook");
        goto out;
    }

    contacts = eds_to_eti_contacts(e_contacts);

    eti_sync_send_contacts(sync, contacts, error);
    if ((NULL != error) && (*error != NULL))
        goto out;

    success = TRUE;

out:
    if (e_contacts != NULL) {
        g_list_foreach(e_contacts, (GFunc)g_object_unref, NULL);
        g_list_free(e_contacts);
    }
    if (contacts != NULL)
        g_hash_table_destroy(contacts);

 /*   if (client != NULL)
        g_object_unref(G_OBJECT(addressbook)); */

    return success;
}

int main(int argc, char **argv)
{
    EtiSync *sync;
    GError *error = { 0, };
    GHashTable *contacts;
    EtiOptions *command_line_options;
	
	/* FIXME START GTK-INSPECTOR interactive debugging */
	 gtk_window_set_interactive_debugging(TRUE); 
	
	 gtk_main(void);

    /*g_type_init(); This has been deprecated TW 29-01-16 */

	/* Create and Start the g_main_loop so that DBus can process messages TW */
	
   /* GMainLoop *loop = NULL; */
  /*  GMainContext *context = NULL;  This sets the default context to be used. */
  /*  GSource *source = NULL; */
    		
   
    /* create a context */
  /*  context = g_main_context_new(); */

    /* attach source to context */

  /*  g_source_attach(source,context); */
 
    /* create a main loop with context */
  /*  loop = g_main_loop_new(context,FALSE); */
	
	/* Main Loop run */
  /*  g_main_loop_run (loop); */

    command_line_options = parse_command_line(argc, argv, &error);
    if ((command_line_options == NULL) || (error != NULL)) {
        g_print("Failed to parse command line options: %s\n", error->message);
        goto error;
    }

    eti_plist_set_debug(command_line_options->debug);

    if (command_line_options->list_addressbooks) {
        eti_eds_dump_addressbooks();
        eti_options_free(command_line_options);
        return 0;
    }

    sync = eti_sync_new(command_line_options->idevice_uuid, &error);

    if (NULL != error) {
        g_print("failed to create sync object: %s\n", error->message);
        goto error;
    }
    g_assert(sync != NULL);

    eti_sync_start_sync(sync, &error);
    if (NULL != error) {
        g_print("failed to start synchronization: %s\n", error->message);
        goto error;
    }

    if (command_line_options->wipe_contacts) {
        g_print("All contacts will be deleted from your device in 5 seconds\n");
        g_print("Press Ctrl+C to interrupt now\n");
        g_usleep(5*G_USEC_PER_SEC);
        eti_sync_wipe_all_contacts(sync, &error);
        if (NULL != error) {
            g_print("failed to delete all contacts: %s\n", error->message);
            goto error;
        }
    }

    contacts = eti_sync_get_contacts(sync, &error);
    if (command_line_options->save_photos)
        save_photos(contacts);

    g_hash_table_destroy(contacts);
    contacts = NULL;
    if (NULL != error) {
        g_print("failed to get contacts: %s\n", error->message);
        goto error;
    }

    if (0) {
        contacts = g_hash_table_new_full(g_str_hash, g_str_equal,
                                         g_free, (GDestroyNotify)eti_contact_free);
        g_hash_table_insert(contacts, g_strdup("6"), create_test_contact());
    }

    if (command_line_options->transfer) {
        gboolean transfer_successful;
        transfer_successful = transfer_eds_contacts(sync,
                                                    command_line_options->addressbook_uri,
                                                    &error);
        if (!transfer_successful) {
            g_print("failed to transfer contacts: %s\n", error->message);
            goto error;
        }
    }

    eti_sync_stop_sync(sync, &error);
    eti_sync_free(sync);
    eti_options_free(command_line_options);
    sync = NULL;

    return 0;
error:
    if (error != NULL)
        g_clear_error(&error);
    if (contacts != NULL)
        g_hash_table_destroy(contacts);
    if (command_line_options != NULL)
        eti_options_free(command_line_options);

    return -1;
}

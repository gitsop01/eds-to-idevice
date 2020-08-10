/*
 *  Copyright (C) 2011 Christophe Fergeau <cfergeau@gmail.com>
 *  Copyright (C) 2015-2020 Timothy Ward gtwa001@gmail.com
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
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#include "eti-eds.h"
#include <evolution-data-server/libebook/libebook.h>
#include <evolution-data-server/libedataserver/libedataserver.h>
#include <glib-2.0/glib.h>


ESourceRegistry *source_registry = NULL;

GQuark eti_ebook_error_quark(void)
{
    return g_quark_from_static_string("eti-ebook-error-quark");
}



/* Code reused from
 * http://code.eikke.com/ProjectSoylent/evo-addressbooks-test2.c
 * Copyright (C) 2005 Nicolas "Ikke" Trangez (eikke eikke commercial)
 */
 /* Function return declaration changed to GSList TW 09/04/16 */

GSList *eti_eds_get_contacts(EBookClient *client, const gchar *query_str, GError **error)
{
EBookQuery *query;
/*  GList *contacts; */
GSList *out_contacts, *l;
gboolean query_succeeded;
gchar *query_string;

    /* Create a query */
    if (query_str == NULL) {
        /* Here we use something like "ls *" ;-) */
        query = e_book_query_any_field_contains("");
    }
    else {
        /* Create an EBookQuery from a querystring */
        query = e_book_query_from_string (query_str);
    }

    if(query == NULL) {
        g_set_error(error, ETI_EBOOK_ERROR,
                    ETI_EBOOK_ERROR_QUERY,
                    "Failed to build addressbook query");
        return NULL;
    }

    /* Perform the query, filling "contacts"
     * Returns a gboolean which tells us whether the query 
     * succeeded or not
     */
	
	/* FIXME e_book_get_contacts has been deprecated TW 21/12/15 */
 /*     query_succeeded = e_book_get_contacts(book, query, &contacts, error); */
	query_string = e_book_query_to_string( query);
	query_succeeded =e_book_client_get_contacts_sync(client, query_string, &out_contacts, NULL, error);

    /* We don't need the query object anymore */
  /*  e_book_query_unref(query); */
    if(!query_succeeded) {
        g_set_error(error, ETI_EBOOK_ERROR,
                    ETI_EBOOK_ERROR_QUERY,
                    "Failed to run addressbook query");
	
        return FALSE;
    }
	guint number = g_slist_length(out_contacts);
	g_print("\nNumber of EWS-Address-Books-contacts are: %i\n\n", number);
	 
	/* Loop through the list of EContacts - out_contacts */
		g_print("List of Default Addressbook Contacts are:\n");
 
    for(l = out_contacts; l!= NULL; l = l->next) {
		 EContact *name = E_CONTACT ( l->data); 
      	 g_print("%s\n", (char *) e_contact_get(name,E_CONTACT_FULL_NAME ));

	}
	return out_contacts;
}

	/* FIXME e_book_new_from_uri has been deprecated TW 21/12/15 */
	/* FIXME e_book_new_default_addressbook has been deprecated TW 21/12/15 */

EBookClient *eti_eds_open_addressbook(void)
{

ESource *source;
EBookClient *book_client;
EClient *client = NULL;
GError *error = NULL;



	source_registry = e_source_registry_new_sync( NULL, &error);	
	
	if (source_registry == NULL)
	if (error != NULL) {
		g_print("source_registry error = %s\n", error->message);
        g_warning ("%s: %s\n", G_STRFUNC, error->message);
      return FALSE;
    }

	source = e_source_registry_ref_builtin_address_book( source_registry);


	client = e_book_client_connect_sync( source, 10, NULL, &error);

	if (client == NULL){
		printf("eti_eds_open_addressbook--e_book_client_connect_sync failed");
		return NULL;
	}
	
    if (error != NULL){
        g_warning ("%s: %s", G_STRFUNC, error->message);
      return NULL;
    }
    g_object_unref(source);

  
  /* return EBookClient */ 
  	book_client = E_BOOK_CLIENT (client);

	return book_client; 
}

void eti_eds_dump_addressbooks(void)
{

ESourceRegistry *source_registry = NULL;
GError *error = NULL; 
guint number;
GList *list = NULL, *l = NULL;
const gchar *uid;	
	
	source_registry = e_source_registry_new_sync( NULL, &error);
	
	if (source_registry == NULL)
	if (error != NULL) {
       g_warning ("%s: %s", G_STRFUNC, error->message);
       return;
	}
	/* Get the list of enabled EWS sources */

    list = e_source_registry_list_enabled( source_registry, E_SOURCE_EXTENSION_ADDRESS_BOOK); 
	
	/* Print error if list is not available and return */
	if(list == NULL) {
       g_print("No E_Source_Extension_Address_Book Registry List found\n");
       return; 
    }
	
	/* Print-out enabled EWS source list names and their UID's */
	
	number = g_list_length(list);
	g_print("\nNumber of enabled EWS-Address-Books-Sources is: %i\n\n", number);
	g_print("Enabled EWS-Address-Book-Source-Names and UID are:\n\n");
	
	/* Loop through the list of sources */
	for(l = list; l!= NULL; l = l->next) { 
		ESource *source = E_SOURCE (l->data);
		uid = e_source_get_uid(source);
        g_print("%s, UID: %s\n", e_source_get_display_name( source), (char *) uid);
	} 
	 g_print("\n");

    /* Clean up */
       g_list_free_full(list, (GDestroyNotify) g_object_unref );

}


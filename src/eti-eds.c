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
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#include "eti-eds.h"
#include <evolution-data-server/libebook/libebook.h>
#include <evolution-data-server/libedataserver/libedataserver.h>

ESourceRegistry *source_registry = NULL;



GQuark eti_ebook_error_quark(void)
{
    return g_quark_from_static_string("eti-ebook-error-quark");
}



/* Code reused from
 * http://code.eikke.com/ProjectSoylent/evo-addressbooks-test2.c
 * Copyright (C) 2005 Nicolas "Ikke" Trangez (eikke eikke commercial)
 */
GList *eti_eds_get_contacts(EBookClient *client, const gchar *query_str, GError **error)
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

	 /* Loop through the list of out_contacts */
    for(l = out_contacts; l!= NULL; l = l->next) {
      	 printf("List of out_contacts are: %s", (char *)l->data);
	 /*   return (char *)l->data;  FIXME return value required */
	}
	return FALSE;
}

	/* FIXME e_book_new_from_uri has been deprecated TW 21/12/15 */
	/* FIXME e_book_new_default_addressbook has been deprecated TW 21/12/15 */

EClient *eti_eds_open_addressbook(void)
{

ESource *source;
EClient *client = NULL;
GError *error = NULL;
/* const gchar *source_uid; */


 /*   EBook *ebook;  */

 /*   if (addressbook_uri != NULL) */
 /*      ebook = e_book_new_from_uri(addressbook_uri, error); */
 /*   else */
 /*       ebook = e_book_new_default_addressbook (error); */

/*    if ((error != NULL) && (*error != NULL)) { */
/*        g_prefix_error (error, "Couldn't open default address book: "); */
/*        return NULL; */
/*    } */

	/* FIXME e_book_open has been deprecated TW 24/12/15 */

   /* e_book_open (ebook, TRUE, error); */

	source_registry = e_source_registry_new_sync( NULL, &error);	
	
	if (source_registry == NULL)
	if (error != NULL) {
        g_warning ("%s: %s", G_STRFUNC, error->message);
      return FALSE;
    }

	source = e_source_registry_ref_builtin_address_book( source_registry);

	/* source_uid = e_source_get_uid( source); */
	
	

	client = e_book_client_connect_sync( source, 10, NULL, &error);

	if (client == NULL){
		printf("eti_eds_open_addressbook--e_book_client_connect_sync failed");
	}
	
    if (error != NULL){
        g_warning ("%s: %s", G_STRFUNC, error->message);
      return FALSE;
    }
    g_object_unref(source);

  /*  return ebook; */
  /* we need to return EBookClient not EClient */ 
  /* or add  e_book_client_connect() to get an EBookClient object. */
	
     return client; 
}

void eti_eds_dump_addressbooks(void)
{

ESourceRegistry *source_registry = NULL;
GError *error = NULL;
/* char *p; */

GList *list = NULL, *l = NULL;  /* *data */

	  
	 /*	const gchar uid; */

	/* FIXME e_book_get_addressbooks has been deprecated TW 21/12/15 */

    /* Query all addressbooks available on the system */
  
	/*  if(!e_book_get_addressbooks(&books, &err)) { */
    /*    return; */
    /* } */

	/* FIXME e_source_list_peek_group() has been deprecated TW 21/12/15 */

    /* Get a real list of the books */

	/* list = e_source_list_peek_groups(books); */
	
	source_registry = e_source_registry_new_sync( NULL, &error);
	
	if (source_registry == NULL)
	if (error != NULL) {
        g_warning ("%s: %s", G_STRFUNC, error->message);
      return;
	}
	list = e_source_registry_list_sources( source_registry, E_SOURCE_EXTENSION_ADDRESS_BOOK);

	if(list == NULL) {
       printf("No E_Source_Extension_Address_Book Registry List found");
        return;
    }

    /* Loop through the list of sources */
    for(l = list; l!= NULL; l = l->next) {
       
	/*	GSList *groups, *s; */

	 printf("List of EWS-Address-books-sources are: %s", (char *)l->data);
	
	
	/* FIXME e_source_list_peek_sources() has been deprecated TW 21/12/15 */

       /* groups = e_source_group_peek_sources(l->data); */

      /*  for(s = groups; s != NULL; s = s->next) { */
      /*     ESource *source = E_SOURCE(s->data);	*/
            

			/* FIXME e_source_get_uri() has been deprecated TW 21/12/15 */
			/* FIXME e_source_peek_name() has been deprecated TW 21/12/15 */

			 /*   gchar *uid;	*/
   	        
			 /*uid = e_source_get_uid(source);	*/

   	 /*		 g_print("Source name: %s, UID: %s\n\n", e_source_get_display_name( source), &uid); */
    		
	 /*       g_free(uri);	*/
     /*   }  */
   }		

    /* Clean up */
    /* g_object_unref(books);	*/

    g_list_free_full(list, (GDestroyNotify) g_object_unref );
}


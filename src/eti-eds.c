/*
 *  Copyright (C) 2011 Christophe Fergeau <cfergeau@gmail.com>
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
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA.
 *
 */
#include "eti-eds.h"

GQuark eti_ebook_error_quark(void)
{
    return g_quark_from_static_string("eti-ebook-error-quark");
}

/* Code reused from
 * http://code.eikke.com/ProjectSoylent/evo-addressbooks-test2.c
 * Copyright (C) 2005 Nicolas "Ikke" Trangez (eikke eikke commercial)
 */
GList *eti_eds_get_contacts(EBook *book, const gchar *query_str, GError **error)
{
    EBookQuery *query;
    GList *contacts;
    gboolean query_succeeded;

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
    query_succeeded = e_book_get_contacts(book, query, &contacts, error);
    /* We don't need the query object anymore */
    e_book_query_unref(query);
    if(!query_succeeded) {
        g_set_error(error, ETI_EBOOK_ERROR,
                    ETI_EBOOK_ERROR_QUERY,
                    "Failed to run addressbook query");
        return NULL;
    }

    return contacts;
}

EBook *eti_eds_open_addressbook (const char *addressbook_uri, GError **error)
{
    EBook *ebook;

    if (addressbook_uri != NULL)
        ebook = e_book_new_from_uri(addressbook_uri, error);
    else
        ebook = e_book_new_default_addressbook (error);

    if ((error != NULL) && (*error != NULL)) {
        g_prefix_error (error, "Couldn't open default address book: ");
        return NULL;
    }

    e_book_open (ebook, TRUE, error);
    if ((error != NULL) && (*error != NULL)) {
        g_prefix_error (error, "Couldn't open default address book: ");
        g_object_unref (ebook);
        return NULL;
    }

    return ebook;
}

void eti_eds_dump_addressbooks(GError **error)
{
    ESourceList *books = NULL;
    GSList *list = NULL, *l = NULL;
    GError *err;

    /* Query all addressbooks available on the system */
    if(!e_book_get_addressbooks(&books, &err)) {
        return;
    }

    /* Get a real list of the books */
    list = e_source_list_peek_groups(books);
    if(list == NULL) {
        g_set_error(error, ETI_EBOOK_ERROR,
                    ETI_EBOOK_ERROR_ADDRESSBOOK,
                    "No addressbooks found");
        return;
    }

    /* Loop through the list of books */
    for(l = list; l!= NULL; l = l->next) {
        GSList *groups, *s;

        groups = e_source_group_peek_sources(l->data);

        for(s = groups; s != NULL; s = s->next) {
            ESource *source = E_SOURCE(s->data);
            char *uri;

            uri = e_source_get_uri(source);

            g_print("Source name: %s, URI: %s\n\n",
                    e_source_peek_name(source), uri);
            g_free(uri);
        }
    }

    /* Clean up */
    g_object_unref(books);
    g_slist_free(list);
}


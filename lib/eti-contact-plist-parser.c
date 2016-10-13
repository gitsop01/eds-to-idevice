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
#include "eti-contact.h"
#include "eti-contact-plist-parser.h"
#include "eti-plist.h"

#include <glib-2.0/glib.h>
#include <stdlib.h>
#include <string.h>

struct _EtiContactPlistParser {
    GHashTable *contacts;
};

#define PARSE_FIELD(fieldstring, fieldname)                     \
    do {                                                        \
        value = eti_plist_dict_get_string(entity, fieldstring); \
        if (value != NULL) {                                    \
            eti_contact_set_##fieldname(contact, value);        \
            free(value);                                        \
        }                                                       \
    } while (0)

static gboolean parse_contact_main_info(EtiContact *contact, plist_t entity,
                                        GError **error)
{
    char *value;
    GDateTime *date;
    guchar *image_data;
    guint64 data_length;

    PARSE_FIELD("first name", first_name);
    PARSE_FIELD("first name yomi", first_name_yomi);
    PARSE_FIELD("middle name", middle_name);
    PARSE_FIELD("last name", last_name);
    PARSE_FIELD("last name yomi", last_name_yomi);
    PARSE_FIELD("nickname", nickname);
    PARSE_FIELD("title", title);
    PARSE_FIELD("suffix", name_suffix);
    PARSE_FIELD("notes", notes);
    PARSE_FIELD("company name", company_name);
    PARSE_FIELD("department", department);
    PARSE_FIELD("job title", job_title);

    date = eti_plist_dict_get_date(entity, "birthday");
    if (date != NULL) {
        eti_contact_set_birthday(contact, date);
        g_date_time_unref(date);
    }

    image_data = eti_plist_dict_get_data(entity, "image", &data_length);
    eti_contact_set_photo_from_data(contact, image_data, data_length);
    free(image_data);

    return TRUE;
}

typedef void (*ContactAttributeSetter)(EtiContact *contact,
                                       const char *type,
                                       const char *label,
                                       const char *value);
static gboolean parse_contact_generic(EtiContact *contact,
                                      ContactAttributeSetter setter,
                                      plist_t entity,
                                      GError **error)
{
    char *type;
    char *label;
    char *value;

    type = eti_plist_dict_get_string(entity, "type");
    if (type == NULL) {
        g_set_error(error, ETI_CONTACT_ERROR,
                    ETI_CONTACT_ERROR_PARSING,
                    "missing 'type' field in entity");
        return FALSE;
    }
    value = eti_plist_dict_get_string(entity, "value");
    if (value == NULL) {
        g_set_error(error, ETI_CONTACT_ERROR,
                    ETI_CONTACT_ERROR_PARSING,
                    "missing 'value' field in entity");
        free(type);
        return FALSE;
    }
    /* 'label' is only set for custom categories, it's NULL for the
     *  predefined categories. For custom categories, 'type' will be set to
     * 'other'
     */
    label = eti_plist_dict_get_string(entity, "label");
    setter(contact, type, label, value);
    free(type);
    free(value);
    free(label);

    return TRUE;
}

static gboolean parse_contact_phone_number(EtiContact *contact, plist_t entity,
                                           GError **error)
{
    return parse_contact_generic(contact, eti_contact_add_phone_number,
                                 entity, error);
}

static gboolean parse_contact_address(EtiContact *contact, plist_t entity,
                                      GError **error)
{
    char *type;
    char *label;
    char *postal_code;
    char *city;
    char *country;
    char *country_code;
    char *street;

    type = eti_plist_dict_get_string(entity, "type");
    if (type == NULL) {
        g_set_error(error, ETI_CONTACT_ERROR,
                    ETI_CONTACT_ERROR_PARSING,
                    "missing 'type' field in address entity");
        return FALSE;
    }
    label = eti_plist_dict_get_string(entity, "label");
    street = eti_plist_dict_get_string(entity, "street");
    postal_code = eti_plist_dict_get_string(entity, "postal code");
    city = eti_plist_dict_get_string(entity, "city");
    country = eti_plist_dict_get_string(entity, "country");
    country_code = eti_plist_dict_get_string(entity, "country code");

    eti_contact_add_address(contact, type, label, street, postal_code,
                            city, country, country_code);

    free(type);
    free(label);
    free(street);
    free(postal_code);
    free(city);
    free(country);
    free(country_code);

    return TRUE;
}

static gboolean parse_contact_im(EtiContact *contact, plist_t entity,
                                 GError **error)
{
    char *type;
    char *label;
    char *service;
    char *user_id;

    type = eti_plist_dict_get_string(entity, "type");
    if (type == NULL) {
        g_set_error(error, ETI_CONTACT_ERROR,
                    ETI_CONTACT_ERROR_PARSING,
                    "missing 'type' field in IM entity");
        return FALSE;
    }
    label = eti_plist_dict_get_string(entity, "label");
    service = eti_plist_dict_get_string(entity, "service");
    user_id = eti_plist_dict_get_string(entity, "user");

    eti_contact_add_im_user_id(contact, type, label, service, user_id);

    free(type);
    free(label);
    free(service);
    free(user_id);

    return TRUE;
}

static gboolean parse_contact_email(EtiContact *contact, plist_t entity,
                                    GError **error)
{
    return parse_contact_generic(contact, eti_contact_add_email,
                                 entity, error);
}

static gboolean parse_contact_url(EtiContact *contact, plist_t entity,
                                  GError **error)
{
    return parse_contact_generic(contact, eti_contact_add_url,
                                 entity, error);
}

static gboolean parse_contact_date(EtiContact *contact, plist_t entity,
                                   GError **error)
{
    char *type;
    char *label;
    GDateTime *date;

    type = eti_plist_dict_get_string(entity, "type");
    if (type == NULL) {
        g_set_error(error, ETI_CONTACT_ERROR,
                    ETI_CONTACT_ERROR_PARSING,
                    "missing 'type' field in date entity");
        return FALSE;
    }

    date = eti_plist_dict_get_date(entity, "value");
    if (date == NULL) {
        g_set_error(error, ETI_CONTACT_ERROR,
                    ETI_CONTACT_ERROR_PARSING,
                    "missing 'value' field in date entity");
        free(type);
        return FALSE;
    }
    label = eti_plist_dict_get_string(entity, "label");

    eti_contact_add_date(contact, type, label, date);

    g_date_time_unref(date);
    free(type);
    free(label);

    return TRUE;
}

static EtiContact *create_new_contact(const char *id, plist_t entity,
                                      GHashTable *contacts, GError **error)
{
    char *display_as;
    EtiContact *contact;

    display_as = eti_plist_dict_get_string(entity, "display as company");
    if (display_as == NULL) {
        g_set_error(error, ETI_CONTACT_ERROR,
                    ETI_CONTACT_ERROR_PARSING,
                    "missing 'display as company' field in main entity");
        return NULL;
    }

    if (strcmp(display_as, "person") == 0)
        contact = eti_contact_new_person(NULL, NULL);
    else if (strcmp(display_as, "company") == 0)
        contact = eti_contact_new_company(NULL);
    else {
        g_set_error(error, ETI_CONTACT_ERROR,
                    ETI_CONTACT_ERROR_PARSING,
                    "invalid 'display as company' value: %s", display_as);
        contact = NULL;
    }

    free(display_as);
    if (contact == NULL)
        return NULL;

    if (!parse_contact_main_info(contact, entity, error)) {
        eti_contact_free(contact);
        g_assert((error == NULL) || (*error != NULL));
        return NULL;
    }
    g_hash_table_insert(contacts, g_strdup(id), contact);

    return contact;
}

static EtiContact *get_existing_contact(plist_t entity, GHashTable *contacts,
                                        GError **error)
{
    plist_t contact_ids;
    plist_t contact_id;
    char *id;
    EtiContact *contact;

    contact_ids = plist_dict_get_item(entity, "contact");
    if (contact_ids == NULL) {
        g_set_error(error, ETI_CONTACT_ERROR,
                    ETI_CONTACT_ERROR_PARSING,
                    "missing 'contact' field in entity");
        return NULL;
    }
    if (plist_get_node_type(contact_ids) != PLIST_ARRAY) {
        g_set_error(error, ETI_CONTACT_ERROR,
                    ETI_CONTACT_ERROR_PARSING,
                    "invalid type for the 'contact' field (%d)",
                    plist_get_node_type(contact_ids));
        return NULL;
    }
    if (plist_array_get_size(contact_ids) != 1) {
        g_set_error(error, ETI_CONTACT_ERROR,
                    ETI_CONTACT_ERROR_PARSING,
                    "invalid 'contact' array length: %d",
                    plist_array_get_size(contact_ids));
        return NULL;
    }
    contact_id = plist_array_get_item(contact_ids, 0);
    if (plist_get_node_type(contact_id) != PLIST_STRING) {
        g_set_error(error, ETI_CONTACT_ERROR,
                    ETI_CONTACT_ERROR_PARSING,
                    "invalid type for 'contact' array content (%d)",
                    plist_get_node_type(contact_id));
        return NULL;
    }
    plist_get_string_val(contact_id, &id);

    contact = g_hash_table_lookup(contacts, id);
    free(id);

    return contact;
}

static gboolean parse_contact_info(const char *key, plist_t entity,
                                   GHashTable *contacts, GError **error)
{
    char *entity_name;
    const char *info_type;
    EtiContact *contact;
    gboolean parse_ok;

    parse_ok = FALSE;
    entity_name = eti_plist_dict_get_string(entity,
                                            "com.apple.syncservices.RecordEntityName");
    if (entity_name == NULL) {
        g_set_error(error, ETI_CONTACT_ERROR,
                    ETI_CONTACT_ERROR_PARSING,
                    "missing entity name (com.apple.syncservices.RecordEntityName field)");
        goto out;
    }

    if (!g_str_has_prefix(entity_name, "com.apple.contacts.")) {
        g_set_error(error, ETI_CONTACT_ERROR,
                    ETI_CONTACT_ERROR_PARSING,
                    "Entity name doesn't have com.apple.contacts. prefix (%s)",
                    entity_name);
        goto out;
    }

    info_type = entity_name + strlen("com.apple.contacts.");
    if (strcmp(info_type, "Contact") == 0) {
        contact = create_new_contact(key, entity, contacts, error);
        parse_ok = (contact != NULL);
        goto out;
    } else {
        contact = get_existing_contact(entity, contacts, error);
        if (contact == NULL)
            goto out;
    }

    if (strcmp(info_type, "Email Address") == 0) {
        parse_ok = parse_contact_email(contact, entity, error);
    } else if (strcmp(info_type, "Phone Number") == 0) {
        parse_ok = parse_contact_phone_number(contact, entity, error);
    } else if (strcmp(info_type, "Street Address") == 0) {
        parse_ok = parse_contact_address(contact, entity, error);
    } else if (strcmp(info_type, "IM") == 0) {
        parse_ok = parse_contact_im(contact, entity, error);
    } else if (strcmp(info_type, "URL") == 0) {
        parse_ok = parse_contact_url(contact, entity, error);
    } else if (strcmp(info_type, "Date") == 0) {
        parse_ok = parse_contact_date(contact, entity, error);
    } else {
        g_set_error(error, ETI_CONTACT_ERROR,
                    ETI_CONTACT_ERROR_PARSING,
                    "Unknown entity name: %s", info_type);
    }

out:
    free(entity_name);
    g_assert(parse_ok || (error == NULL) || (*error != NULL));
    return parse_ok;
}

gboolean eti_contact_plist_parser_parse(EtiContactPlistParser *parser,
                                        plist_t entities, GError **error)
{
    plist_dict_iter iter;


    if (!entities) {
        g_set_error(error, ETI_CONTACT_ERROR,
                    ETI_CONTACT_ERROR_PARSING,
                    "empty entities plist");
        return FALSE;
    }

    if (plist_get_node_type(entities) != PLIST_DICT) {
        g_set_error(error, ETI_CONTACT_ERROR,
                    ETI_CONTACT_ERROR_PARSING,
                    "unexpected type for entities plist (%d)",
                    plist_get_node_type(entities));
        return FALSE;
    }

    if (plist_dict_get_size(entities) == 0)
        /* silently ignore empty plists */
        return TRUE;

    iter = NULL;
    plist_dict_new_iter(entities, &iter);
    if (iter) {
        char *key;
        plist_t node;
        node  = NULL;
        key = NULL;
        plist_dict_next_item(entities, iter, &key, &node);
        while (node) {
            parse_contact_info(key, node, parser->contacts, error);
            free(key);
            plist_dict_next_item(entities, iter, &key, &node);
        }
        free(iter);
    }

    return TRUE;
}

EtiContactPlistParser *eti_contact_plist_parser_new(void)
{
    EtiContactPlistParser *parser;

    parser = g_new0(EtiContactPlistParser, 1);
    parser->contacts = g_hash_table_new_full(g_str_hash, g_str_equal,
                                             g_free,
                                             (GDestroyNotify)eti_contact_free);

    return parser;
}

GHashTable *eti_contact_plist_parser_get_contacts(EtiContactPlistParser *parser)
{
    return parser->contacts;
}

void eti_contact_plist_parser_free(EtiContactPlistParser *parser,
                                   gboolean free_contacts)
{
    if (free_contacts)
        g_hash_table_destroy(parser->contacts);
    g_free(parser);
}

static void dump_one(gpointer key, gpointer value, gpointer user_data)
{
    char *id = (char *)key;
    EtiContact *contact = (EtiContact *)value;
    g_print("Contact ID: %s\n", id);
    eti_contact_dump(contact);
    g_print("--\n");
}

void eti_contact_plist_parser_dump(EtiContactPlistParser *parser)
{
    g_hash_table_foreach(parser->contacts, dump_one, NULL);
}

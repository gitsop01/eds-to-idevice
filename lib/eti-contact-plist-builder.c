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
#include "eti-contact-plist-builder.h"
#include "eti-contact.h"
#include "eti-plist.h"
#include <plist/plist.h>

static void plist_add_uid_link(plist_t node, const char *uid)
{
  plist_t array;

  array = plist_new_array();
  plist_array_append_item(array, plist_new_string(uid));
  plist_dict_set_item(node, "contact", array);
}

struct IterBuilderContext {
    plist_t dict;
    plist_t remapped_uids;
    const char *main_uid;
    unsigned int count;
    const char *entity_name;
    unsigned int category_id;
};

static plist_t create_dict(const char *entity_name, const char *main_uid,
                           const char *type, const char *label)
{
    plist_t field_info;

    field_info = plist_new_dict();

    plist_dict_set_item(field_info,
                           "com.apple.syncservices.RecordEntityName",
                           plist_new_string(entity_name));
    eti_plist_dict_set_string(field_info, "type", type);
    eti_plist_dict_set_string(field_info, "label", label);
    plist_add_uid_link(field_info, main_uid);

    return field_info;
}

static gchar *get_uid(struct IterBuilderContext *context)
{
    char *uid;
    char *remapped_uid;

    remapped_uid = eti_plist_dict_get_string(context->remapped_uids,
                                             context->main_uid);
    if (NULL != remapped_uid)
        uid = g_strdup_printf("%d/%s/%d", context->category_id,
                              remapped_uid, context->count);
    else
        uid = g_strdup_printf("%d/%s/%d", context->category_id,
                              context->main_uid, context->count);

    g_free(remapped_uid);

    return uid;
}

static void add_one_generic(EtiContact *contact, const char *type,
                            const char *label, const char *value,
                            gpointer user_data)
{
    plist_t field_info;
    struct IterBuilderContext *context;
    gchar *local_uid;

    context = (struct IterBuilderContext *)user_data;
    field_info = create_dict(context->entity_name, context->main_uid,
                             type, label);
    eti_plist_dict_set_string(field_info, "value", value);
    local_uid = get_uid(context);
    context->count++;
    plist_dict_set_item(context->dict, local_uid, field_info);
    g_free(local_uid);
}

static void add_one_date(EtiContact *contact, const char *type,
                         const char *label, GDateTime *date,
                         gpointer user_data)
{
    plist_t field_info;
    struct IterBuilderContext *context;
    gchar *local_uid;

    context = (struct IterBuilderContext *)user_data;
    field_info = create_dict(context->entity_name, context->main_uid,
                             type, label);
    eti_plist_dict_set_date(field_info, "value", date);
    local_uid = get_uid(context);
    context->count++;
    plist_dict_set_item(context->dict, local_uid, field_info);
    g_free(local_uid);
}

static void add_one_im_user_id(EtiContact *contact, const char *type,
                               const char *label, const char *service,
                               const char *user_id, gpointer user_data)
{
    plist_t field_info;
    struct IterBuilderContext *context;
    gchar *local_uid;

    context = (struct IterBuilderContext *)user_data;
    field_info = create_dict(context->entity_name, context->main_uid,
                             type, label);
    eti_plist_dict_set_string(field_info, "service", service);
    eti_plist_dict_set_string(field_info, "user", user_id);
    local_uid = get_uid(context);
    context->count++;
	plist_dict_set_item(context->dict, local_uid, field_info);
    g_free(local_uid);
}

static void add_one_address(EtiContact *contact, const char *type,
                            const char *label, const char *street,
                            const char *postal_code, const char *city,
                            const char *country, const char *country_code,
                            gpointer user_data)
{
    plist_t field_info;
    struct IterBuilderContext *context;
    gchar *local_uid;

    context = (struct IterBuilderContext *)user_data;
    field_info = create_dict(context->entity_name, context->main_uid,
                             type, label);
    eti_plist_dict_set_string(field_info, "street", street);
    eti_plist_dict_set_string(field_info, "postal code", postal_code);
    eti_plist_dict_set_string(field_info, "city", city);
    eti_plist_dict_set_string(field_info, "country", country);
    eti_plist_dict_set_string(field_info, "country code", country_code);
    local_uid = get_uid(context);
    context->count++;
	plist_dict_set_item(context->dict, local_uid, field_info);
    g_free(local_uid);
}

typedef void (*MultiFieldForeach)(EtiContact *contact, gpointer user_data);

static plist_t build_multi_field_plist(GHashTable *contacts,
                                       MultiFieldForeach field_foreach,
                                       plist_t remapped_uids)
{
    GHashTableIter iter;
    gpointer key;
    gpointer value;
    plist_t dict;

    dict = plist_new_dict();
    if (dict == NULL)
        return NULL;

    g_hash_table_iter_init(&iter, contacts);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        char *uid = (char *)key;
        EtiContact *contact = (EtiContact *)value;
        struct IterBuilderContext context = {
            .dict = dict,
            .remapped_uids = remapped_uids,
            .main_uid = uid,
            .count = 0,
            .entity_name = NULL,
            .category_id = 0
        };

        field_foreach(contact, &context);
    }

    eti_plist_dump(dict);

    return dict;
}

static void address_foreach(EtiContact *contact, gpointer user_data)
{
    struct IterBuilderContext *context;

    context = (struct IterBuilderContext *)user_data;
    context->entity_name = "com.apple.contacts.Street Address";
    context->category_id = 5;
    eti_contact_foreach_address(contact, add_one_address, context);
}

static plist_t build_addresses_plist(GHashTable *contacts,
                                     plist_t remapped_uids)
{
    return build_multi_field_plist(contacts, address_foreach, remapped_uids);
}

static void phone_number_foreach(EtiContact *contact, gpointer user_data)
{
    struct IterBuilderContext *context;

    context = (struct IterBuilderContext *)user_data;
    context->entity_name = "com.apple.contacts.Phone Number";
    context->category_id = 3;
    eti_contact_foreach_phone_number(contact, add_one_generic, context);
}

static plist_t build_phone_numbers_plist(GHashTable *contacts,
                                         plist_t remapped_uids)
{
    return build_multi_field_plist(contacts, phone_number_foreach, remapped_uids);
}

static void email_foreach(EtiContact *contact, gpointer user_data)
{
    struct IterBuilderContext *context;

    context = (struct IterBuilderContext *)user_data;
    context->entity_name = "com.apple.contacts.Email Address";
    context->category_id = 4;
    eti_contact_foreach_email(contact, add_one_generic, context);
}

static plist_t build_emails_plist(GHashTable *contacts, plist_t remapped_uids)
{
    return build_multi_field_plist(contacts, email_foreach, remapped_uids);
}

static void im_user_id_foreach(EtiContact *contact, gpointer user_data)
{
    struct IterBuilderContext *context;

    context = (struct IterBuilderContext *)user_data;
    context->entity_name = "com.apple.contacts.IM";
    context->category_id = 13;
    eti_contact_foreach_im_user_id(contact, add_one_im_user_id, context);
}

static plist_t build_im_user_ids_plist(GHashTable *contacts,
                                       plist_t remapped_uids)
{
    return build_multi_field_plist(contacts, im_user_id_foreach, remapped_uids);
}

static void url_foreach(EtiContact *contact, gpointer user_data)
{
    struct IterBuilderContext *context;

    context = (struct IterBuilderContext *)user_data;
    context->entity_name = "com.apple.contacts.URL";
    context->category_id = 22;
    eti_contact_foreach_url(contact, add_one_generic, context);
}

static plist_t build_urls_plist(GHashTable *contacts, plist_t remapped_uids)
{
    return build_multi_field_plist(contacts, url_foreach, remapped_uids);
}

static void date_foreach(EtiContact *contact, gpointer user_data)
{
    struct IterBuilderContext *context;

    context = (struct IterBuilderContext *)user_data;
    context->entity_name = "com.apple.contacts.Date";
    context->category_id = 12;
    eti_contact_foreach_date(contact, add_one_date, context);
}

static plist_t build_dates_plist(GHashTable *contacts, plist_t remapped_uids)
{
    return build_multi_field_plist(contacts, date_foreach, remapped_uids);
}

plist_t
eti_contact_plist_builder_build_main(GHashTable *contacts)
{
    GHashTableIter iter;
    gpointer key, value;
    plist_t main_plist;

    main_plist = plist_new_dict();
    if (main_plist == NULL)
        return NULL;

    g_hash_table_iter_init(&iter, contacts);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        char *uid = (char *)key;
        EtiContact *contact = (EtiContact *)value;
        plist_t main_info;
        GDateTime *birthday;
        const guchar *image_data;
        size_t data_length;

        main_info = plist_new_dict();
        if (main_info == NULL) {
            g_warning("couldn't create plist for %s", uid);
            continue;
        }

	    plist_dict_set_item(main_info,
                               "com.apple.syncservices.RecordEntityName",
                               plist_new_string("com.apple.contacts.Contact"));
        eti_plist_dict_set_string(main_info, "first name",
                                  eti_contact_get_first_name(contact));
        eti_plist_dict_set_string(main_info, "first name yomi",
                                  eti_contact_get_first_name_yomi(contact));
        eti_plist_dict_set_string(main_info, "middle name",
                                  eti_contact_get_middle_name(contact));
        eti_plist_dict_set_string(main_info, "last name",
                                  eti_contact_get_last_name(contact));
        eti_plist_dict_set_string(main_info, "last name yomi",
                                  eti_contact_get_last_name_yomi(contact));
        eti_plist_dict_set_string(main_info, "nickname",
                                  eti_contact_get_nickname(contact));
        eti_plist_dict_set_string(main_info, "title",
                                  eti_contact_get_title(contact));
        eti_plist_dict_set_string(main_info, "suffix",
                                  eti_contact_get_name_suffix(contact));
        eti_plist_dict_set_string(main_info, "notes",
                                  eti_contact_get_notes(contact));
        eti_plist_dict_set_string(main_info, "company name",
                                  eti_contact_get_company_name(contact));
        eti_plist_dict_set_string(main_info, "department",
                                  eti_contact_get_department(contact));
        eti_plist_dict_set_string(main_info, "job title",
                                  eti_contact_get_job_title(contact));

        birthday = eti_contact_get_birthday(contact);
        if (birthday != NULL) {
            eti_plist_dict_set_date(main_info, "birthday", birthday);
            g_date_time_unref(birthday);
        }

        eti_contact_get_photo(contact, &image_data, &data_length);
        eti_plist_dict_set_data(main_info, "image", image_data, data_length);
	    plist_dict_set_item(main_plist, uid, main_info);
    }

    eti_plist_dump(main_plist);

    return main_plist;
}

GList *
eti_contact_plist_builder_build_others(GHashTable *contacts,
                                       plist_t remapped_uids)
{
    GList *plists = NULL;

    plists = g_list_prepend(plists, build_addresses_plist(contacts,
                                                          remapped_uids));
    plists = g_list_prepend(plists, build_phone_numbers_plist(contacts,
                                                              remapped_uids));
    plists = g_list_prepend(plists, build_emails_plist(contacts,
                                                       remapped_uids));
    plists = g_list_prepend(plists, build_im_user_ids_plist(contacts,
                                                            remapped_uids));
    plists = g_list_prepend(plists, build_urls_plist(contacts,
                                                     remapped_uids));
    plists = g_list_prepend(plists, build_dates_plist(contacts,
                                                      remapped_uids));

    return g_list_reverse(plists);
}


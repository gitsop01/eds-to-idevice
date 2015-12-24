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
#include "eti-contact.h"

#include <glib2.0/glib.h>

GQuark eti_contact_error_quark(void)
{
    return g_quark_from_static_string("eti-contact-error-quark");
}

enum _EtiContactType {
    ETI_CONTACT_TYPE_PERSON,
    ETI_CONTACT_TYPE_COMPANY
};
typedef enum _EtiContactType EtiContactType;

struct _EtiContactPhoto {
    guchar *image_data;
    gsize data_length;
};
typedef struct _EtiContactPhoto EtiContactPhoto;

struct _EtiContact {
    EtiContactType type;
    char *first_name;
    char *first_name_yomi;
    char *middle_name;
    char *last_name;
    char *last_name_yomi;
    char *nickname;
    char *title;
    char *name_suffix;
    char *notes;
    char *company_name;
    char *department;
    char *job_title;
    GDateTime *birthday;
    GList *addresses;
    GList *phone_numbers;
    GList *emails;
    GList *im_user_ids;
    GList *urls;
    GList *dates;
    EtiContactPhoto photo;
};

struct _EtiContactGenericMultifield {
    char *type;
    char *label;
    gpointer value;
};
typedef struct _EtiContactGenericMultifield EtiContactGenericMultifield;

struct _EtiContactAddress {
    char *street;
    char *postal_code;
    char *city;
    char *country;
    char *country_code;
};
typedef struct _EtiContactAddress EtiContactAddress;

static EtiContactAddress *eti_contact_address_new(const char *street,
                                                  const char *postal_code,
                                                  const char *city,
                                                  const char *country,
                                                  const char *country_code)
{
    EtiContactAddress *address;

    address = g_new0(EtiContactAddress, 1);
    if (street != NULL)
        address->street = g_strdup(street);
    if (postal_code != NULL)
        address->postal_code = g_strdup(postal_code);
    if (city != NULL)
        address->city = g_strdup(city);
    if (country != NULL)
        address->country = g_strdup(country);
    if (country_code != NULL)
        address->country_code = g_strdup(country_code);

    return address;
}

static void eti_contact_address_dump(EtiContactAddress *address,
                                     const char *prefix)
{
    if (address->street)
        g_print("%sStreet: %s\n", prefix, address->street);
    if (address->postal_code)
        g_print("%sPostal Code: %s\n", prefix, address->postal_code);
    if (address->city)
        g_print("%sCity: %s\n", prefix, address->city);
    if (address->country)
        g_print("%sCountry: %s\n", prefix, address->country);
    if (address->street)
        g_print("%sCountry Code: %s\n", prefix, address->country_code);
}

static void eti_contact_address_free(EtiContactAddress *address)
{
    g_free(address->street);
    g_free(address->postal_code);
    g_free(address->city);
    g_free(address->country);
    g_free(address->country_code);
    g_free(address);
}

struct _EtiContactImUserId {
    char *service;
    char *user_id;
};
typedef struct _EtiContactImUserId EtiContactImUserId;

static EtiContactImUserId *eti_contact_im_user_id_new(const char *service,
                                                      const char *user_id)
{
    EtiContactImUserId *im_user_id;

    im_user_id = g_new0(EtiContactImUserId, 1);
    if (service)
        im_user_id->service = g_strdup(service);
    if (user_id)
        im_user_id->user_id = g_strdup(user_id);

    return im_user_id;
}

static void eti_contact_im_user_id_dump(EtiContactImUserId *user_id,
                                        const char *prefix)
{
    if (user_id->service)
        g_print("%sService: %s\n", prefix, user_id->service);
    if (user_id->user_id)
        g_print("%sUser ID: %s\n", prefix, user_id->user_id);
}

static void eti_contact_im_user_id_free(EtiContactImUserId *user_id)
{
    g_free(user_id->service);
    g_free(user_id->user_id);
    g_free(user_id);
}


static EtiContact *eti_contact_new_empty(void)
{
    EtiContact *contact;

    contact = g_new0(EtiContact, 1);

    return contact;
}

EtiContact *eti_contact_new_person(const char *first_name,
                                   const char *last_name)
{
    EtiContact *contact;

    contact = eti_contact_new_empty();
    contact->type = ETI_CONTACT_TYPE_PERSON;
    if (NULL != first_name)
        contact->first_name = g_strdup(first_name);
    if (NULL != last_name)
        contact->last_name = g_strdup(last_name);

    return contact;
}

EtiContact *eti_contact_new_company(const char *company_name)
{
    EtiContact *contact;

    contact = eti_contact_new_empty();
    contact->type = ETI_CONTACT_TYPE_COMPANY;
    if (NULL != company_name)
        contact->company_name = g_strdup(company_name);

    return contact;
}

#define ETI_CONTACT_SETTER(fieldname) \
    void eti_contact_set_##fieldname(EtiContact *contact,       \
                                     const char *fieldname)     \
{                                                               \
    if (NULL == fieldname)                                      \
        return;                                                 \
    g_free(contact->fieldname);                                 \
    contact->fieldname = g_strdup(fieldname);                   \
}

ETI_CONTACT_SETTER(first_name);
ETI_CONTACT_SETTER(first_name_yomi);
ETI_CONTACT_SETTER(middle_name);
ETI_CONTACT_SETTER(last_name);
ETI_CONTACT_SETTER(last_name_yomi);
ETI_CONTACT_SETTER(nickname);
ETI_CONTACT_SETTER(title);
ETI_CONTACT_SETTER(name_suffix);
ETI_CONTACT_SETTER(notes);
ETI_CONTACT_SETTER(company_name);
ETI_CONTACT_SETTER(department);
ETI_CONTACT_SETTER(job_title);

void eti_contact_set_birthday(EtiContact *contact, GDateTime *birthday)
{
    if (contact->birthday != NULL)
        g_date_time_unref(contact->birthday);
    contact->birthday = g_date_time_ref(birthday);
}

void eti_contact_set_photo_from_data(EtiContact *contact,
                                     const guchar *data, gsize len)
{
    if (contact->photo.image_data != NULL)
        g_free(contact->photo.image_data);

    contact->photo.image_data = g_memdup(data, len);
    contact->photo.data_length = len;
}

gboolean eti_contact_set_photo_from_file(EtiContact *contact,
                                         const char *filename,
                                         GError **error)
{
    if (contact->photo.image_data != NULL)
        g_free(contact->photo.image_data);

    return g_file_get_contents(filename, (gchar **)&contact->photo.image_data,
                               &contact->photo.data_length, error);
}

#define ETI_CONTACT_GETTER(fieldname) \
    const char *eti_contact_get_##fieldname(EtiContact *contact)        \
{                                                                       \
    return contact->fieldname;                                          \
}

ETI_CONTACT_GETTER(first_name);
ETI_CONTACT_GETTER(first_name_yomi);
ETI_CONTACT_GETTER(middle_name);
ETI_CONTACT_GETTER(last_name);
ETI_CONTACT_GETTER(last_name_yomi);
ETI_CONTACT_GETTER(nickname);
ETI_CONTACT_GETTER(title);
ETI_CONTACT_GETTER(name_suffix);
ETI_CONTACT_GETTER(notes);
ETI_CONTACT_GETTER(company_name);
ETI_CONTACT_GETTER(department);
ETI_CONTACT_GETTER(job_title);

GDateTime *eti_contact_get_birthday(EtiContact *contact)
{
    if (contact->birthday == NULL)
        return NULL;

    return g_date_time_ref(contact->birthday);
}

void eti_contact_get_photo(EtiContact *contact,
                           const guchar **image_data,
                           gsize *data_length)
{
    *image_data = contact->photo.image_data;
    *data_length = contact->photo.data_length;
}

static GList *generic_field_append(GList *fields, const char *type,
                                   const char *label, gpointer data)
{
    EtiContactGenericMultifield *field;

    g_assert(type != NULL);
    field = g_new0(EtiContactGenericMultifield, 1);
    field->type = g_strdup(type);
    if (label)
        field->label = g_strdup(label);
    field->value = data;

    return g_list_prepend(fields, field);
}

void eti_contact_add_address(EtiContact *contact, const char *type,
                             const char *label, const char *street,
                             const char *postal_code, const char *city,
                             const char *country, const char *country_code)
{
    EtiContactAddress *address;

    address = eti_contact_address_new(street, postal_code, city,
                                      country, country_code);
    contact->addresses = generic_field_append(contact->addresses, type,
                                              label, address);
}

void eti_contact_add_phone_number(EtiContact *contact,
                                  const char *type,
                                  const char *label,
                                  const char *phone_number)
{
    if (phone_number == NULL)
        return;
    contact->phone_numbers = generic_field_append(contact->phone_numbers,
                                                  type, label,
                                                  g_strdup(phone_number));
}

void eti_contact_add_email(EtiContact *contact,
                           const char *type,
                           const char *label,
                           const char *email)
{
    if (email == NULL)
        return;
    contact->emails = generic_field_append(contact->emails, type,
                                           label, g_strdup(email));
}

void eti_contact_add_im_user_id(EtiContact *contact, const char *type,
                                const char *label,
                                const char *service, const char *user_id)
{
    EtiContactImUserId *im_user_id;

    if (user_id == NULL)
        return;
    im_user_id = eti_contact_im_user_id_new(service, user_id);
    contact->im_user_ids = generic_field_append(contact->im_user_ids, type,
                                                label, im_user_id);
}

void eti_contact_add_url(EtiContact *contact,
                         const char *type,
                         const char *label,
                         const char *url)
{
    if (url == NULL)
        return;
    contact->urls = generic_field_append(contact->urls, type, label,
                                         g_strdup(url));
}

void eti_contact_add_date(EtiContact *contact,
                          const char *type,
                          const char *label,
                          GDateTime *date)
{
    if (date == NULL)
        return;
    contact->dates = generic_field_append(contact->dates, type, label,
                                          g_date_time_ref(date));
}

void eti_contact_foreach_address(EtiContact *contact,
                                 EtiContactAddressIterator iter_func,
                                 gpointer user_data)
{
    GList *it;

    for (it = contact->addresses; it != NULL; it = it->next) {
        EtiContactGenericMultifield *field;
        EtiContactAddress *address;

        field = (EtiContactGenericMultifield *)it->data;
        address = (EtiContactAddress *)field->value;

        iter_func(contact, field->type, field->label,
                  address->street, address->postal_code,
                  address->city, address->country,
                  address->country_code,
                  user_data);
    }
}

void eti_contact_foreach_email(EtiContact *contact,
                               EtiContactGenericIterator iter_func,
                               gpointer user_data)
{
    GList *it;

    for (it = contact->emails; it != NULL; it = it->next) {
        EtiContactGenericMultifield *field;

        field = (EtiContactGenericMultifield *)it->data;

        iter_func(contact, field->type, field->label, field->value, user_data);
    }
}

void eti_contact_foreach_phone_number(EtiContact *contact,
                                      EtiContactGenericIterator iter_func,
                                      gpointer user_data)
{
    GList *it;

    for (it = contact->phone_numbers; it != NULL; it = it->next) {
        EtiContactGenericMultifield *field;

        field = (EtiContactGenericMultifield *)it->data;

        iter_func(contact, field->type, field->label, field->value, user_data);
    }
}

void eti_contact_foreach_url(EtiContact *contact,
                             EtiContactGenericIterator iter_func,
                             gpointer user_data)
{
  GList *it;

    for (it = contact->urls; it != NULL; it = it->next) {
        EtiContactGenericMultifield *field;

        field = (EtiContactGenericMultifield *)it->data;

        iter_func(contact, field->type, field->label, field->value, user_data);
    }
}

void eti_contact_foreach_date(EtiContact *contact,
                              EtiContactDateIterator iter_func,
                              gpointer user_data)
{
    GList *it;

    for (it = contact->dates; it != NULL; it = it->next) {
        EtiContactGenericMultifield *field;

        field = (EtiContactGenericMultifield *)it->data;

        iter_func(contact, field->type, field->label, field->value, user_data);
    }
}

void eti_contact_foreach_im_user_id(EtiContact *contact,
                                    EtiContactImUserIdIterator iter_func,
                                    gpointer user_data)
{
    GList *it;

    for (it = contact->im_user_ids; it != NULL; it = it->next) {
        EtiContactGenericMultifield *field;
        EtiContactImUserId *im_user_id;

        field = (EtiContactGenericMultifield *)it->data;
        im_user_id = (EtiContactImUserId *)field->value;

        iter_func(contact, field->type, field->label,
                  im_user_id->service, im_user_id->user_id,
                  user_data);
    }
}

static void generic_field_free(gpointer data, gpointer user_data)
{
    EtiContactGenericMultifield *field;
    GDestroyNotify free_func;

    field = (EtiContactGenericMultifield *)data;
    free_func = (GDestroyNotify)user_data;

    g_free(field->type);
    g_free(field->label);
    if (free_func)
        free_func(field->value);
    g_free(field);
}

void eti_contact_free(EtiContact *contact)
{
    g_list_foreach(contact->addresses, generic_field_free,
                   eti_contact_address_free);
    g_list_free(contact->addresses);
    g_list_foreach(contact->phone_numbers, generic_field_free, g_free);
    g_list_free(contact->phone_numbers);
    g_list_foreach(contact->emails, generic_field_free, g_free);
    g_list_free(contact->emails);
    g_list_foreach(contact->im_user_ids, generic_field_free,
                   eti_contact_im_user_id_free);
    g_list_free(contact->im_user_ids);
    g_list_foreach(contact->urls, generic_field_free, g_free);
    g_list_free(contact->urls);
    g_list_foreach(contact->dates, generic_field_free, g_date_time_unref);
    g_list_free(contact->dates);
    g_free(contact->first_name);
    g_free(contact->first_name_yomi);
    g_free(contact->middle_name);
    g_free(contact->last_name);
    g_free(contact->last_name_yomi);
    g_free(contact->nickname);
    g_free(contact->title);
    g_free(contact->name_suffix);
    g_free(contact->company_name);
    g_free(contact->department);
    g_free(contact->job_title);
    g_free(contact->notes);
    g_free(contact->photo.image_data);
    if (contact->birthday != NULL)
        g_date_time_unref(contact->birthday);
    g_free(contact);
}

static void dump_one_date(gpointer data, gpointer user_data)
{
    EtiContactGenericMultifield *field;

    field = (EtiContactGenericMultifield *)data;
    GDateTime *date = (GDateTime *)field->value;
    gchar *date_str;

    date_str = g_date_time_format(date, "%x");
    if (field->label)
        g_print("\t%s: %s\n", field->label, date_str);
    else
        g_print("\t%s: %s\n", field->type, date_str);
    g_free(date_str);
}

typedef void (*EtiContactFieldPrettyPrinter)(gpointer data, const char *prefix);

static void dump_one_generic_field(gpointer data, gpointer user_data)
{
    EtiContactGenericMultifield *field;
    EtiContactFieldPrettyPrinter pretty_printer;

    field = (EtiContactGenericMultifield *)data;
    pretty_printer = (EtiContactFieldPrettyPrinter)user_data;

    if (field->label)
        g_print("\t%s:", field->label);
    else
        g_print("\t%s:", field->type);
    if (pretty_printer == NULL)
        g_print(" %s\n", (char *)field->value);
    else {
        g_print("\n");
        pretty_printer(field->value, "\t\t");
    }
}

void eti_contact_dump(EtiContact *contact)
{
    if (contact->type == ETI_CONTACT_TYPE_PERSON)
        g_print("Contact Type: person\n");
    else
        g_print("Contact Type: company\n");
    if (contact->title)
        g_print("Title: %s\n", contact->title);
    if (contact->first_name)
        g_print("First Name: %s", contact->first_name);
    if (contact->first_name_yomi)
        g_print(" (%s)", contact->first_name_yomi);
    g_print("\n");
    if (contact->middle_name)
        g_print("Middle Name: %s\n", contact->middle_name);
    if (contact->last_name)
        g_print("Last Name: %s", contact->last_name);
    if (contact->last_name_yomi)
        g_print(" (%s)", contact->last_name_yomi);
    g_print("\n");
    if (contact->nickname)
        g_print("Nickname: %s\n", contact->nickname);
    if (contact->name_suffix)
        g_print("Name Suffix: %s\n", contact->name_suffix);
    if (contact->photo.image_data)
        g_print("Contact has photo\n");
    if (contact->birthday) {
        char *date;
        date = g_date_time_format(contact->birthday, "%x");
        g_print("Birthday: %s\n", date);
        g_free(date);
    }
    if (contact->company_name)
        g_print("Company Name: %s\n", contact->company_name);
    if (contact->department)
        g_print("Department: %s\n", contact->department);
    if (contact->job_title)
        g_print("Job Title: %s\n", contact->job_title);
    if (contact->notes)
        g_print("Notes: %s\n", contact->notes);
    if (contact->addresses != NULL) {
        g_print("Addresses:\n");
        g_list_foreach(contact->addresses, dump_one_generic_field,
                       eti_contact_address_dump);
    }
    if (contact->phone_numbers != NULL) {
        g_print("Phone Numbers:\n");
        g_list_foreach(contact->phone_numbers, dump_one_generic_field,
                       NULL);
    }
    if (contact->emails != NULL) {
        g_print("Emails:\n");
        g_list_foreach(contact->emails, dump_one_generic_field, NULL);
    }
    if (contact->im_user_ids != NULL) {
        g_print("IM User IDs:\n");
        g_list_foreach(contact->im_user_ids, dump_one_generic_field,
                       eti_contact_im_user_id_dump);
    }
    if (contact->urls != NULL) {
        g_print("URLS:\n");
        g_list_foreach(contact->urls, dump_one_generic_field, NULL);
    }
    if (contact->dates != NULL) {
        g_print("Dates:\n");
        g_list_foreach(contact->dates, dump_one_date, NULL);
    }
}

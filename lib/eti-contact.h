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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1335 USA
 */
#ifndef ETI_CONTACT_H
#define ETI_CONTACT_H

#include <glib-2.0/glib.h>


typedef struct _EtiContact EtiContact;

typedef enum {
    ETI_CONTACT_ERROR_FAILED,
    ETI_CONTACT_ERROR_PARSING
} EtiContactError;

#define ETI_CONTACT_ERROR eti_contact_error_quark()

#define ETI_CONTACT_FIELD_TYPE_HOME "home"
#define ETI_CONTACT_FIELD_TYPE_WORK "work"
#define ETI_CONTACT_FIELD_TYPE_OTHER "other"
#define ETI_CONTACT_URL_TYPE_HOMEPAGE "home page"
#define ETI_CONTACT_DATE_TYPE_ANNIVERSARY "anniversary"
#define ETI_CONTACT_PHONE_NUMBER_TYPE_MOBILE "mobile"

GQuark eti_contact_error_quark(void);
EtiContact *eti_contact_new_person(const char *first_name,
                                   const char *last_name);
EtiContact *eti_contact_new_company(const char *company_name);
void eti_contact_set_first_name(EtiContact *contact, const char *first_name);
void eti_contact_set_first_name_yomi(EtiContact *contact,
                                     const char *first_name_yomi);
void eti_contact_set_middle_name(EtiContact *contact, const char *middle_name);
void eti_contact_set_last_name(EtiContact *contact, const char *last_name);
void eti_contact_set_last_name_yomi(EtiContact *contact,
                                    const char *last_name_yomi);
void eti_contact_set_nickname(EtiContact *contact, const char *nickname);
void eti_contact_set_birthday(EtiContact *contact, GDateTime *date);
void eti_contact_set_notes(EtiContact *contact, const char *notes);
void eti_contact_set_company_name(EtiContact *contact,
                                  const char *company_name);
void eti_contact_set_department(EtiContact *contact, const char *department);
void eti_contact_set_job_title(EtiContact *contact, const char *job_title);
void eti_contact_set_title(EtiContact *contact, const char *title);
void eti_contact_set_name_suffix(EtiContact *contact, const char *name_suffix);
void eti_contact_set_photo_from_data(EtiContact *contact,
                                     const guchar *data, gsize len);
gboolean eti_contact_set_photo_from_file(EtiContact *contact,
                                         const char *filename,
                                         GError **error);

void eti_contact_add_address(EtiContact *contact, const char *type,
                             const char *label, const char *street,
                             const char *postal_code, const char *city,
                             const char *country, const char *country_code);
void eti_contact_add_phone_number(EtiContact *contact, const char *type,
                                  const char *label, const char *phone_number);
void eti_contact_add_email(EtiContact *contact, const char *type,
                           const char *label, const char *email);
void eti_contact_add_im_user_id(EtiContact *contact, const char *type,
                                const char *label,
                                const char *service, const char *user_id);
void eti_contact_add_url(EtiContact *contact, const char *type,
                         const char *label, const char *url);
void eti_contact_add_date(EtiContact *contact, const char *type,
                          const char *label, GDateTime *date);

const char *eti_contact_get_first_name(EtiContact *contact);
const char *eti_contact_get_first_name_yomi(EtiContact *contact);
const char *eti_contact_get_middle_name(EtiContact *contact);
const char *eti_contact_get_last_name(EtiContact *contact);
const char *eti_contact_get_last_name_yomi(EtiContact *contact);
const char *eti_contact_get_nickname(EtiContact *contact);
GDateTime *eti_contact_get_birthday(EtiContact *contact);
const char *eti_contact_get_notes(EtiContact *contact);
const char *eti_contact_get_company_name(EtiContact *contact);
const char *eti_contact_get_department(EtiContact *contact);
const char *eti_contact_get_job_title(EtiContact *contact);
const char *eti_contact_get_title(EtiContact *contact);
const char *eti_contact_get_name_suffix(EtiContact *contact);
void eti_contact_get_photo(EtiContact *contact,
                           const guchar **image_data,
                           gsize *data_length);

typedef void (*EtiContactGenericIterator)(EtiContact *contact,
                                        const char *type,
                                        const char *label,
                                        const char *value,
                                        gpointer user_data);
void eti_contact_foreach_email(EtiContact *contact,
                               EtiContactGenericIterator iter_func,
                               gpointer user_data);
void eti_contact_foreach_phone_number(EtiContact *contact,
                                      EtiContactGenericIterator iter_func,
                                      gpointer user_data);
void eti_contact_foreach_url(EtiContact *contact,
                             EtiContactGenericIterator iter_func,
                             gpointer user_data);
typedef void (*EtiContactAddressIterator)(EtiContact *contact,
                                          const char *type,
                                          const char *label,
                                          const char *street,
                                          const char *postal_code,
                                          const char *city,
                                          const char *country,
                                          const char *country_code,
                                          gpointer user_data);
void eti_contact_foreach_address(EtiContact *contact,
                                 EtiContactAddressIterator iter_func,
                                 gpointer user_data);
typedef void (*EtiContactDateIterator)(EtiContact *contact,
                                       const char *type,
                                       const char *label,
                                       GDateTime *date,
                                       gpointer user_data);
void eti_contact_foreach_date(EtiContact *contact,
                              EtiContactDateIterator iter_func,
                              gpointer user_data);
typedef void (*EtiContactImUserIdIterator)(EtiContact *contact,
                                           const char *type,
                                           const char *label,
                                           const char *service,
                                           const char *user_id,
                                           gpointer user_data);
void eti_contact_foreach_im_user_id(EtiContact *contact,
                                    EtiContactImUserIdIterator iter_func,
                                    gpointer user_data);

void eti_contact_free(EtiContact *contact);
void eti_contact_dump(EtiContact *contact);
#endif

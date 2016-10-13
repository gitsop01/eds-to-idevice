/*
 *  Copyright (C) 2011 Christophe Fergeau <cfergeau@gmail.com>
 *  Copyright (C) 2015 Timothy Ward gtwa001@gmail.com
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
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1335  USA
 *
 */
#include "eti-contact.h"
#include "eti-eds.h"
#include <evolution-data-server/libebook-contacts/libebook-contacts.h>

static gboolean is_empty(const char *str)
{
    return ((str == NULL) || (*str == '\0'));
}

	/* FIXME e_contact_get_string has been deprecated but is a built function */
	/* comflict with same name as library deprecated function TW 21/12/15 */
	/* Name changed to *E_contact_get_string() */

static gchar *E_contact_get_string(EContact *contact, EContactField field_id)
{
  gchar *value;
  value = e_contact_get(contact, field_id);
  if (is_empty(value)) {
    g_free(value);
    value = NULL;
  }

  return value;
}

static const char *get_string_not_empty(const char *str)
{
    if (is_empty(str))
        return NULL;

    return str;
}

static void eti_contact_add_econtact_address(EtiContact *contact,
                                             EContactAddress *address,
                                             const char *address_type)
{
    gchar *street;
    const gchar *postal_code;
    const gchar *city;
    const gchar *country;

    if (address == NULL)
        return;

    /* if all the fields we are interested in are empty, silently returns */
    if (is_empty(address->street)
        && is_empty(address->ext)
        && is_empty(address->code)
        && is_empty(address->locality)
        && is_empty(address->country))
        return;

    if (is_empty(address->street) && is_empty(address->ext))
        street = NULL;
    if (is_empty(address->street))
        street = g_strdup(address->ext);
    else if (is_empty(address->ext))
        street = g_strdup(address->street);
    else
        street = g_strconcat(address->street, "\n", address->ext, NULL);

    postal_code = get_string_not_empty(address->code);
    city = get_string_not_empty(address->locality);
    country = get_string_not_empty(address->country);

    eti_contact_add_address(contact, address_type, NULL,
                            street, postal_code, city,
                            country, NULL);
    g_free(street);
}

static void add_names(EtiContact *contact, EContact *econtact,
                     EContactName *name)
{
    gchar *nickname;

    if (name == NULL)
        return;

    eti_contact_set_first_name(contact, get_string_not_empty(name->given));
    eti_contact_set_last_name(contact, get_string_not_empty(name->family));
    eti_contact_set_middle_name(contact, get_string_not_empty(name->additional));
    eti_contact_set_name_suffix(contact, get_string_not_empty(name->suffixes));
    eti_contact_set_title(contact, get_string_not_empty(name->prefixes));

    nickname = E_contact_get_string(econtact, E_CONTACT_NICKNAME);
    eti_contact_set_nickname(contact, nickname);
    g_free(nickname);
}

static GDateTime *e_contact_date_to_g_date_time(EContactDate *edate)
{
  return g_date_time_new_local(edate->year, edate->month, edate->day, 0, 0, 0);
}

static void add_photo(EtiContact *contact, EContact *econtact)
{
    EContactPhoto *photo;
    photo = e_contact_get(econtact, E_CONTACT_PHOTO);
    if (photo == NULL) {
        photo = e_contact_get(econtact, E_CONTACT_LOGO);
    }

    if (photo != NULL) {
        /* various limitations here, E_CONTACT_PHOTO_TYPE_URI isn't
         * handled, we don't check at all what format the photo is in, ...
         */
        if (photo->type == E_CONTACT_PHOTO_TYPE_INLINED) {
            eti_contact_set_photo_from_data(contact, photo->data.inlined.data,
                                            photo->data.inlined.length);
        }

        e_contact_photo_free(photo);
    }
}

static void add_phone_number(EContact *econtact, EContactField field_id,
                             EtiContact *contact,
                             const char *type, const char *label)
{
    gchar *phone_number;

    phone_number = E_contact_get_string(econtact, field_id);
    eti_contact_add_phone_number(contact, type, label, phone_number);
    g_free(phone_number);
}

static void add_im_contact(EContact *econtact, EContactField field_id,
                           EtiContact *contact,
                           const char *service, const char *type)
{
    gchar *user_id;

    user_id = E_contact_get_string(econtact, field_id);
    eti_contact_add_im_user_id(contact, type, NULL, service, user_id);
    g_free(user_id);
}

static void convert_addresses(EContact *econtact, EtiContact *contact)
{
    EContactAddress *address;

    address = e_contact_get(econtact, E_CONTACT_ADDRESS_HOME);
    eti_contact_add_econtact_address(contact, address,
                                     ETI_CONTACT_FIELD_TYPE_HOME);
    e_contact_address_free(address);

    address = e_contact_get(econtact, E_CONTACT_ADDRESS_WORK);
    eti_contact_add_econtact_address(contact, address,
                                     ETI_CONTACT_FIELD_TYPE_WORK);
    e_contact_address_free(address);

    address = e_contact_get(econtact, E_CONTACT_ADDRESS_OTHER);
    eti_contact_add_econtact_address(contact, address,
                                     ETI_CONTACT_FIELD_TYPE_OTHER);
    e_contact_address_free(address);
}


static void convert_dates(EContact *econtact, EtiContact *contact)
{
    GDateTime *gdate;
    EContactDate *edate;

    edate = e_contact_get(econtact, E_CONTACT_BIRTH_DATE);
    if (edate != NULL) {
        gdate = e_contact_date_to_g_date_time(edate);
        e_contact_date_free(edate);
        eti_contact_set_birthday(contact, gdate);
        g_date_time_unref(gdate);
    }

    edate = e_contact_get(econtact, E_CONTACT_ANNIVERSARY);
    if (edate != NULL) {
        gdate = e_contact_date_to_g_date_time(edate);
        e_contact_date_free(edate);
        eti_contact_add_date(contact, ETI_CONTACT_DATE_TYPE_ANNIVERSARY,
                             NULL, gdate);
        g_date_time_unref(gdate);
    }
}


	/* FIXME e_contact_get_string has been deprecated but is a built function */
	/* conflicts with same name as library deprecated function TW 21/12/15 */

static void convert_emails(EContact *econtact, EtiContact *contact)
{
    gchar *email;

    email = E_contact_get_string(econtact, E_CONTACT_EMAIL_1);
    eti_contact_add_email(contact, ETI_CONTACT_FIELD_TYPE_OTHER, NULL, email);
    g_free(email);

    email = E_contact_get_string(econtact, E_CONTACT_EMAIL_2);
    eti_contact_add_email(contact, ETI_CONTACT_FIELD_TYPE_OTHER, NULL, email);
    g_free(email);

    email = E_contact_get_string(econtact, E_CONTACT_EMAIL_3);
    eti_contact_add_email(contact, ETI_CONTACT_FIELD_TYPE_OTHER, NULL, email);
    g_free(email);

    email = E_contact_get_string(econtact, E_CONTACT_EMAIL_4);
    eti_contact_add_email(contact, ETI_CONTACT_FIELD_TYPE_OTHER, NULL, email);
    g_free(email);
}


	/* FIXME e_contact_get_string has been deprecated but is a built function */
	/* comflict with same name as library deprecated function TW 21/12/15 */

static void convert_urls(EContact *econtact, EtiContact *contact)
{
    gchar *url;

    url = E_contact_get_string(econtact, E_CONTACT_HOMEPAGE_URL);
    eti_contact_add_url(contact, ETI_CONTACT_URL_TYPE_HOMEPAGE, NULL, url);
    g_free(url);

    url = E_contact_get_string(econtact, E_CONTACT_BLOG_URL);
    eti_contact_add_url(contact, ETI_CONTACT_FIELD_TYPE_OTHER, "blog", url);
    g_free(url);
}

static void convert_phone_numbers(EContact *econtact, EtiContact *contact)
{
    /* FIXME: not really sure what "primary" should be mapped to */
    add_phone_number(econtact, E_CONTACT_PHONE_PRIMARY,
                     contact, ETI_CONTACT_PHONE_NUMBER_TYPE_MOBILE, NULL);
    add_phone_number(econtact, E_CONTACT_PHONE_MOBILE,
                     contact, ETI_CONTACT_PHONE_NUMBER_TYPE_MOBILE, NULL);
    add_phone_number(econtact, E_CONTACT_PHONE_HOME,
                     contact, ETI_CONTACT_FIELD_TYPE_HOME, NULL);
    add_phone_number(econtact, E_CONTACT_PHONE_HOME_2,
                     contact, ETI_CONTACT_FIELD_TYPE_HOME, NULL);
    add_phone_number(econtact, E_CONTACT_PHONE_BUSINESS,
                     contact, ETI_CONTACT_FIELD_TYPE_WORK, NULL);
    add_phone_number(econtact, E_CONTACT_PHONE_BUSINESS_2,
                     contact, ETI_CONTACT_FIELD_TYPE_WORK, NULL);
    add_phone_number(econtact, E_CONTACT_PHONE_OTHER,
                     contact, ETI_CONTACT_FIELD_TYPE_OTHER, NULL);
    add_phone_number(econtact, E_CONTACT_PHONE_ASSISTANT,
                     contact, ETI_CONTACT_FIELD_TYPE_OTHER, "assistant");
    add_phone_number(econtact, E_CONTACT_PHONE_BUSINESS_FAX,
                     contact, ETI_CONTACT_FIELD_TYPE_OTHER, "work fax");
    add_phone_number(econtact, E_CONTACT_PHONE_CALLBACK,
                     contact, ETI_CONTACT_FIELD_TYPE_OTHER, "callback");
    add_phone_number(econtact, E_CONTACT_PHONE_CAR,
                     contact, ETI_CONTACT_FIELD_TYPE_OTHER, "car");
    add_phone_number(econtact, E_CONTACT_PHONE_COMPANY,
                     contact, ETI_CONTACT_FIELD_TYPE_OTHER, "company");
    add_phone_number(econtact, E_CONTACT_PHONE_HOME_FAX,
                     contact, ETI_CONTACT_FIELD_TYPE_OTHER, "home fax");
    add_phone_number(econtact, E_CONTACT_PHONE_ISDN,
                     contact, ETI_CONTACT_FIELD_TYPE_OTHER, "ISDN");
    add_phone_number(econtact, E_CONTACT_PHONE_OTHER_FAX,
                     contact, ETI_CONTACT_FIELD_TYPE_OTHER, "other fax");
    add_phone_number(econtact, E_CONTACT_PHONE_PAGER,
                     contact, ETI_CONTACT_FIELD_TYPE_OTHER, "pager");
    add_phone_number(econtact, E_CONTACT_PHONE_RADIO,
                     contact, ETI_CONTACT_FIELD_TYPE_OTHER, "radio");
    add_phone_number(econtact, E_CONTACT_PHONE_TELEX,
                     contact, ETI_CONTACT_FIELD_TYPE_OTHER, "telex");
    add_phone_number(econtact, E_CONTACT_PHONE_TTYTDD,
                     contact, ETI_CONTACT_FIELD_TYPE_OTHER, "TTY TDD");
}

static void convert_im_contacts(EContact *econtact, EtiContact *contact)
{
    add_im_contact(econtact, E_CONTACT_IM_AIM_HOME_1,
                   contact, "aim", ETI_CONTACT_FIELD_TYPE_HOME);
    add_im_contact(econtact, E_CONTACT_IM_AIM_HOME_2,
                   contact, "aim", ETI_CONTACT_FIELD_TYPE_HOME);
    add_im_contact(econtact, E_CONTACT_IM_AIM_HOME_3,
                   contact, "aim", ETI_CONTACT_FIELD_TYPE_HOME);
    add_im_contact(econtact, E_CONTACT_IM_AIM_WORK_1,
                   contact, "aim", ETI_CONTACT_FIELD_TYPE_WORK);
    add_im_contact(econtact, E_CONTACT_IM_AIM_WORK_2,
                   contact, "aim", ETI_CONTACT_FIELD_TYPE_WORK);
    add_im_contact(econtact, E_CONTACT_IM_AIM_WORK_3,
                   contact, "aim", ETI_CONTACT_FIELD_TYPE_WORK);

    add_im_contact(econtact, E_CONTACT_IM_GROUPWISE_HOME_1,
                   contact, "groupwise", ETI_CONTACT_FIELD_TYPE_HOME);
    add_im_contact(econtact, E_CONTACT_IM_GROUPWISE_HOME_2,
                   contact, "groupwise", ETI_CONTACT_FIELD_TYPE_HOME);
    add_im_contact(econtact, E_CONTACT_IM_GROUPWISE_HOME_3,
                   contact, "groupwise", ETI_CONTACT_FIELD_TYPE_HOME);
    add_im_contact(econtact, E_CONTACT_IM_GROUPWISE_WORK_1,
                   contact, "groupwise", ETI_CONTACT_FIELD_TYPE_WORK);
    add_im_contact(econtact, E_CONTACT_IM_GROUPWISE_WORK_2,
                   contact, "groupwise", ETI_CONTACT_FIELD_TYPE_WORK);
    add_im_contact(econtact, E_CONTACT_IM_GROUPWISE_WORK_3,
                   contact, "groupwise", ETI_CONTACT_FIELD_TYPE_WORK);

    add_im_contact(econtact, E_CONTACT_IM_JABBER_HOME_1,
                   contact, "jabber", ETI_CONTACT_FIELD_TYPE_HOME);
    add_im_contact(econtact, E_CONTACT_IM_JABBER_HOME_2,
                   contact, "jabber", ETI_CONTACT_FIELD_TYPE_HOME);
    add_im_contact(econtact, E_CONTACT_IM_JABBER_HOME_3,
                   contact, "jabber", ETI_CONTACT_FIELD_TYPE_HOME);
    add_im_contact(econtact, E_CONTACT_IM_JABBER_WORK_1,
                   contact, "jabber", ETI_CONTACT_FIELD_TYPE_WORK);
    add_im_contact(econtact, E_CONTACT_IM_JABBER_WORK_2,
                   contact, "jabber", ETI_CONTACT_FIELD_TYPE_WORK);
    add_im_contact(econtact, E_CONTACT_IM_JABBER_WORK_3,
                   contact, "jabber", ETI_CONTACT_FIELD_TYPE_WORK);

    add_im_contact(econtact, E_CONTACT_IM_YAHOO_HOME_1,
                   contact, "yahoo", ETI_CONTACT_FIELD_TYPE_HOME);
    add_im_contact(econtact, E_CONTACT_IM_YAHOO_HOME_2,
                   contact, "yahoo", ETI_CONTACT_FIELD_TYPE_HOME);
    add_im_contact(econtact, E_CONTACT_IM_YAHOO_HOME_3,
                   contact, "yahoo", ETI_CONTACT_FIELD_TYPE_HOME);
    add_im_contact(econtact, E_CONTACT_IM_YAHOO_WORK_1,
                   contact, "yahoo", ETI_CONTACT_FIELD_TYPE_WORK);
    add_im_contact(econtact, E_CONTACT_IM_YAHOO_WORK_2,
                   contact, "yahoo", ETI_CONTACT_FIELD_TYPE_WORK);
    add_im_contact(econtact, E_CONTACT_IM_YAHOO_WORK_3,
                   contact, "yahoo", ETI_CONTACT_FIELD_TYPE_WORK);

    add_im_contact(econtact, E_CONTACT_IM_MSN_HOME_1,
                   contact, "msn", ETI_CONTACT_FIELD_TYPE_HOME);
    add_im_contact(econtact, E_CONTACT_IM_MSN_HOME_2,
                   contact, "msn", ETI_CONTACT_FIELD_TYPE_HOME);
    add_im_contact(econtact, E_CONTACT_IM_MSN_HOME_3,
                   contact, "msn", ETI_CONTACT_FIELD_TYPE_HOME);
    add_im_contact(econtact, E_CONTACT_IM_MSN_WORK_1,
                   contact, "msn", ETI_CONTACT_FIELD_TYPE_WORK);
    add_im_contact(econtact, E_CONTACT_IM_MSN_WORK_2,
                   contact, "msn", ETI_CONTACT_FIELD_TYPE_WORK);
    add_im_contact(econtact, E_CONTACT_IM_MSN_WORK_3,
                   contact, "msn", ETI_CONTACT_FIELD_TYPE_WORK);

    add_im_contact(econtact, E_CONTACT_IM_ICQ_HOME_1,
                   contact, "icq", ETI_CONTACT_FIELD_TYPE_HOME);
    add_im_contact(econtact, E_CONTACT_IM_ICQ_HOME_2,
                   contact, "icq", ETI_CONTACT_FIELD_TYPE_HOME);
    add_im_contact(econtact, E_CONTACT_IM_ICQ_HOME_3,
                   contact, "icq", ETI_CONTACT_FIELD_TYPE_HOME);
    add_im_contact(econtact, E_CONTACT_IM_ICQ_WORK_1,
                   contact, "icq", ETI_CONTACT_FIELD_TYPE_WORK);
    add_im_contact(econtact, E_CONTACT_IM_ICQ_WORK_2,
                   contact, "icq", ETI_CONTACT_FIELD_TYPE_WORK);
    add_im_contact(econtact, E_CONTACT_IM_ICQ_WORK_3,
                   contact, "icq", ETI_CONTACT_FIELD_TYPE_WORK);

    add_im_contact(econtact, E_CONTACT_IM_GADUGADU_HOME_1,
                   contact, "gadugadu", ETI_CONTACT_FIELD_TYPE_HOME);
    add_im_contact(econtact, E_CONTACT_IM_GADUGADU_HOME_2,
                   contact, "gadugadu", ETI_CONTACT_FIELD_TYPE_HOME);
    add_im_contact(econtact, E_CONTACT_IM_GADUGADU_HOME_3,
                   contact, "gadugadu", ETI_CONTACT_FIELD_TYPE_HOME);
    add_im_contact(econtact, E_CONTACT_IM_GADUGADU_WORK_1,
                   contact, "gadugadu", ETI_CONTACT_FIELD_TYPE_WORK);
    add_im_contact(econtact, E_CONTACT_IM_GADUGADU_WORK_2,
                   contact, "gadugadu", ETI_CONTACT_FIELD_TYPE_WORK);
    add_im_contact(econtact, E_CONTACT_IM_GADUGADU_WORK_3,
                   contact, "gadugadu", ETI_CONTACT_FIELD_TYPE_WORK);

    add_im_contact(econtact, E_CONTACT_IM_SKYPE_HOME_1,
                   contact, "skype", ETI_CONTACT_FIELD_TYPE_HOME);
    add_im_contact(econtact, E_CONTACT_IM_SKYPE_HOME_2,
                   contact, "skype", ETI_CONTACT_FIELD_TYPE_HOME);
    add_im_contact(econtact, E_CONTACT_IM_SKYPE_HOME_3,
                   contact, "skype", ETI_CONTACT_FIELD_TYPE_HOME);
    add_im_contact(econtact, E_CONTACT_IM_SKYPE_WORK_1,
                   contact, "skype", ETI_CONTACT_FIELD_TYPE_WORK);
    add_im_contact(econtact, E_CONTACT_IM_SKYPE_WORK_2,
                   contact, "skype", ETI_CONTACT_FIELD_TYPE_WORK);
    add_im_contact(econtact, E_CONTACT_IM_SKYPE_WORK_3,
                   contact, "skype", ETI_CONTACT_FIELD_TYPE_WORK);
}

EtiContact *eti_contact_from_econtact(EContact *econtact)
{
    EtiContact *contact;
    EContactName *name;
    gchar *company_name;
    gchar *job_title;
    gchar *department;
    gchar *notes;

	/* FIXME e_contact_get_string has been deprecated but is a built function */
	/* conflict with same name as library deprecated function TW 21/12/15 */

	/* FIXME  error: implicit declaration of function ‘e_contact’ */
	/* [-Werror=implicit-function-declaration] name = e_contact(econtact, E_CONTACT_NAME); */


    name = e_contact_get(econtact, E_CONTACT_NAME);
    company_name = E_contact_get_string(econtact, E_CONTACT_ORG);

    if ((name == NULL) && (company_name != NULL)) {
        contact = eti_contact_new_company(company_name);
    } else {
        contact = eti_contact_new_person(NULL, NULL);
        add_names(contact, econtact, name);
        e_contact_name_free(name);
    }

    /* Organizational fields */

	/* FIXME e_contact_get_string has been deprecated but is a built function */
	/* conflict with same name as library deprecated function TW 21/12/15 */


    eti_contact_set_company_name(contact, company_name);
    g_free(company_name);
    department = E_contact_get_string(econtact, E_CONTACT_ORG_UNIT);
    eti_contact_set_department(contact, department);
    g_free(department);
    job_title = E_contact_get_string(econtact, E_CONTACT_TITLE);
    eti_contact_set_job_title(contact, job_title);
    g_free(job_title);

    /* misc fields */

	/* FIXME e_contact_get_string has been deprecated TW 21/12/15 */
	/* conflict with same name as library deprecated function TW 21/12/15 */

    notes = E_contact_get_string(econtact, E_CONTACT_NOTE);
    eti_contact_set_notes(contact, notes);
    g_free(notes);
    add_photo(contact, econtact);

    convert_addresses(econtact, contact);
    convert_dates(econtact, contact);
    convert_emails(econtact, contact);
    convert_urls(econtact, contact);
    convert_phone_numbers(econtact, contact);
    convert_im_contacts(econtact, contact);

    return contact;
}

char *eti_eds_get_econtact_uid(EContact *econtact)
{
  return e_contact_get(econtact, E_CONTACT_UID);
}

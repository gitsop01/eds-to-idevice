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
#include "eti-plist.h"

static gboolean eti_enable_dump_xml = FALSE;

void eti_plist_set_debug(gboolean enable_debug)
{
    eti_enable_dump_xml = enable_debug;
}

void eti_plist_dump(plist_t plist)
{
    uint32_t len;
    char *xml = NULL;
    if (eti_enable_dump_xml) {
        plist_to_xml(plist, &xml, &len);
        if (len > 0) {
            g_print("%s\n", xml);
            g_free(xml);
        }
    }
}

void eti_plist_dict_set_date(plist_t dict, const char *key, GDateTime *date)
{
    GDateTime *epoch;
    GTimeSpan timespan;

    if (date == NULL)
        return;

    epoch = g_date_time_new_utc(2001, 1, 1, 0, 0, 0);
    timespan = g_date_time_difference(date, epoch);
	plist_dict_set_item(dict, plist_new_date(timespan/G_TIME_SPAN_SECOND,
                                          timespan%G_TIME_SPAN_SECOND));
    g_date_time_unref(epoch);
}

GDateTime *eti_plist_dict_get_date(plist_t node, const char *key)
{
    plist_t value;
    int32_t secs;
    int32_t usecs;
    GDateTime *date;
    GDateTime *tmp_date;

    if (plist_get_node_type(node) != PLIST_DICT)
        return NULL;
    value = plist_dict_get_item(node, key);
    if (value == NULL)
        return NULL;
    if (plist_get_node_type(value) != PLIST_DATE)
        return NULL;

    plist_get_date_val(value, &secs, &usecs);
    tmp_date = g_date_time_new_utc(2001, 1, 1, 0, 0, 0);
    date = g_date_time_add_seconds(tmp_date, secs);
    g_date_time_unref(tmp_date);
    tmp_date = date;
    date = g_date_time_add_seconds(tmp_date, (double)usecs/1000000.0);
    g_date_time_unref(tmp_date);

    return date;
}

void eti_plist_dict_set_string(plist_t dict, const char *key, const char *value)
{
    if (value == NULL)
        return;
	plist_dict_set_item(dict, key, plist_new_string(value));
}

char *eti_plist_dict_get_string(plist_t node, const char *key)
{
    plist_t value;
    char *value_str;

    if (plist_get_node_type(node) != PLIST_DICT)
        return NULL;
    value = plist_dict_get_item(node, key);
    if (value == NULL)
        return NULL;
    if (plist_get_node_type(value) != PLIST_STRING)
        return NULL;

    plist_get_string_val(value, &value_str);

    return value_str;
}

void eti_plist_dict_set_data(plist_t dict, const char *key,
                             const guchar *data, gsize len)
{
    if (data == NULL)
        return;
	    plist_dict_set_item(dict, key, plist_new_data((const gchar *)data, len));
}

guchar *eti_plist_dict_get_data(plist_t node, const char *key, guint64 *len)
{
    plist_t value;
    guchar *data;

    if (plist_get_node_type(node) != PLIST_DICT)
        return NULL;
    value = plist_dict_get_item(node, key);
    if (value == NULL)
        return NULL;
    if (plist_get_node_type(value) != PLIST_DATA)
        return NULL;

    plist_get_data_val(value, (char **)&data, len);

    return data;
}

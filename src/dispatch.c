/*
 * transmission-remote-gtk - A GTK RPC client to Transmission
 * Copyright (C) 2011  Alan Fitton

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <glib.h>
#include <glib/gprintf.h>
#include <json-glib/json-glib.h>
#include <curl/curl.h>

#include "config.h"
#include "dispatch.h"
#include "json.h"
#include "protocol-constants.h"

static void dispatch_async_threadfunc(struct DispatchAsyncData *task,
                                      TrgClient * client);

JsonObject *dispatch(TrgClient * client, JsonNode * req, int *status)
{
    gchar *serialized;
    trg_http_response *response;
    JsonObject *deserialized;
    JsonNode *result;
    GError *decode_error = NULL;

    serialized = trg_serialize(req);
    json_node_free(req);
#ifdef DEBUG
    if (g_getenv("TRG_SHOW_OUTGOING"))
        g_debug("=>(outgoing)=>\n%s", serialized);
#endif
    response = trg_http_perform(client, serialized);
    g_free(serialized);

    if (status)
        *status = response->status;

    if (response->status != CURLE_OK) {
        http_response_free(response);
        return NULL;
    }

    deserialized = trg_deserialize(response, &decode_error);
    http_response_free(response);

    if (decode_error) {
        g_error("JSON decoding error: %s", decode_error->message);
        g_error_free(decode_error);
        if (status)
            *status = FAIL_JSON_DECODE;
        return NULL;
    }

    result = json_object_get_member(deserialized, FIELD_RESULT);
    if (status
        && (!result || g_strcmp0(json_node_get_string(result), FIELD_SUCCESS)))
        *status = FAIL_RESPONSE_UNSUCCESSFUL;

    return deserialized;
}

static void dispatch_async_threadfunc(struct DispatchAsyncData *task,
                                      TrgClient * client)
{
    int status;
    JsonObject *result = dispatch(client, task->req, &status);
    if (task->callback)
        task->callback(result, status, task->data);
    g_free(task);
}

GThreadPool *dispatch_init_pool(TrgClient * client)
{
    return g_thread_pool_new((GFunc) dispatch_async_threadfunc, client,
                             DISPATCH_POOL_SIZE, TRUE, NULL);
}

gboolean dispatch_async(TrgClient * client, JsonNode * req,
                        void (*callback) (JsonObject *, int, gpointer),
                        gpointer data)
{
    GError *error = NULL;
    struct DispatchAsyncData *args = g_new(struct DispatchAsyncData, 1);

    args->callback = callback;
    args->data = data;
    args->req = req;

    trg_client_thread_pool_push(client, args, &error);
    if (error) {
        g_error("thread creation error: %s\n", error->message);
        g_error_free(error);
        g_free(args);
        return FALSE;
    } else {
        return TRUE;
    }
}

#include <curl/curl.h>
#include <json.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "curl_utils.h"
#include "json_parser_utils.h"

json_value *invoke_request(char *token, char *method, int n_params, ...) {
  CURL *curl = curl_easy_init();
  if (!curl) {
    fprintf(stderr, "Failed to init curl\n");
    exit(1);
  }

  CURLU *url = curl_url();

  char *part = get_url(token, method);
  check_curlu(curl_url_set(url, CURLUPART_URL, part, 0));
  curl_free(part);

  va_list list;
  va_start(list, n_params);

  for (int i = 0; i < n_params; i++) {
    check_curlu(curl_url_set(url, CURLUPART_QUERY, va_arg(list, char *),
                             CURLU_APPENDQUERY | CURLU_URLDECODE));
  }

  va_end(list);

  char *out;
  curl_url_get(url, CURLUPART_URL, &out, 0);

  curl_easy_setopt(curl, CURLOPT_CURLU, url);
  Response response = {0};
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_function);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

  check_curl(curl_easy_perform(curl));

  curl_url_cleanup(url);
  curl_easy_cleanup(curl);

  return json_parse(response.content, response.size);
}

int main() {
  char *token = getenv("BOT_TOKEN");
  if (token == NULL) {
    fprintf(stderr, "BOT_TOKEN is not set\n");
    exit(1);
  }

  json_value_free(
      invoke_request(token, "deleteWebhook", 1, "drop_pending_updates=true"));

  int64_t offset = 0;

  while (1) {
    char *offset_param = curl_maprintf("offset=%d", offset + 1);
    json_value *response =
        invoke_request(token, "getUpdates", 2, "timeout=30", offset_param);

    curl_free(offset_param);

    json_value *ok = get_key("ok", json_boolean, response);

    if (ok) {
      json_value *result = get_key("result", json_array, response);
      if (result != NULL) {
        for (int i = 0; i < result->u.array.length; i++) {
          if (result->u.array.values[i]->type == json_object) {
            json_value *message =
                get_key("message", json_object, result->u.array.values[i]);
            json_value *update_id =
                get_key("update_id", json_integer, result->u.array.values[i]);

            if (message != NULL) {
              json_value *chat = get_key("chat", json_object, message);

              if (chat != NULL) {
                json_value *chat_id = get_key("id", json_integer, chat);

                if (chat_id != NULL) {
                  json_value *text = get_key("text", json_string, message);

                  if (text != NULL) {
                    char *chat_id_param =
                        curl_maprintf("chat_id=%d", chat_id->u.integer);
                    char *text_param =
                        curl_maprintf("text=%s", text->u.string.ptr);

                    json_value_free(invoke_request(token, "sendMessage", 2,
                                                   chat_id_param, text_param));
                    curl_free(chat_id_param);
                    curl_free(text_param);
                  }
                }
              }
            }

            offset = update_id->u.integer;
          }
        }
      }
    }

    json_value_free(response);
  }
  return 0;
}

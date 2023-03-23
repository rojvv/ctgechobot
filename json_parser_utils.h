#pragma once

#include <json.h>
#include <string.h>

json_value *get_key(char *key, json_type expected_type, json_value *value) {
  if (value->type == json_object) {
    for (int i = 0; i < value->u.object.length; i++) {
      if (!strcmp(key, value->u.object.values[i].name) &&
          value->u.object.values[i].value->type == expected_type) {
        return value->u.object.values[i].value;
      }
    }
  }
  return NULL;
}

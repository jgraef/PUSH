/* unserialize.c - Unserialize PUSH objects
 * NOTE: uses the expat library
 *
 * Copyright (c) 2012 Janosch Gräf <janosch.graef@gmx.net>
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <string.h>
#include <glib.h>

#include "push.h"



#define push_unserialize_bool(s) (g_ascii_strcasecmp((s), "true") == 0)


struct push_unserialize_args {
  push_t *push;

  push_stack_t *current_stack;
  push_name_t current_binding;
  push_name_t current_config;
  GList *val_stack;
};



static push_stack_t *push_unserialize_name2stack(push_t *push, const char *name) {
  if (strcmp(name, "boolean") == 0) {
    return push->boolean;
  }
  else if (strcmp(name, "code") == 0) {
    return push->code;
  }
  else if (strcmp(name, "exec") == 0) {
    return push->exec;
  }
  else if (strcmp(name, "integer") == 0) {
    return push->integer;
  }
  else if (strcmp(name, "name") == 0) {
    return push->name;
  }
  else if (strcmp(name, "real") == 0) {
    return push->real;
  }
  else {
    return NULL;
  }
}


static const char *push_unserialize_get_value(const char **attribute_names, const char **attribute_values, const char *key) {
  int i;

  for (i = 0; attribute_names[i] != NULL && attribute_values[i] != NULL; i++) {
    if (strcmp(attribute_names[i], key) == 0) {
      return attribute_values[i];
    }
  }

  return NULL;
}


static void push_unserialize_add_value(struct push_unserialize_args *args, push_val_t *val) {
  push_val_t *val2;

  if (args->val_stack != NULL) {
    val2 = (push_val_t*)args->val_stack->data;
    push_code_append(val2->code, val);
  }
  else if (args->current_stack != NULL) {
    g_queue_push_tail(args->current_stack, val);
  }
  else if (args->current_binding != NULL) {
    push_define(args->push, args->current_binding, val);
  }
  else if (args->current_config != NULL) {
    push_config_set(args->push, args->current_config, val);
  }
  else {
    // NOTE: should not happen
    g_warning("Found unused value");
  }
}


static void push_unserialize_start_tag(GMarkupParseContext *ctx, const char *element_name, const char **attribute_names, const char **attribute_values, void *userdata, GError **error) {
  struct push_unserialize_args *args = (struct push_unserialize_args*)userdata;
  int type;
  push_val_t *val;
  push_instr_t *instr;
  const char *val_str;

  if (strcmp(element_name, "stack") == 0) {
    val_str = push_unserialize_get_value(attribute_names, attribute_values, "name");
    if (val_str != NULL) {
      args->current_stack = push_unserialize_name2stack(args->push, val_str);
    }
  }
  else if (strcmp(element_name, "binding") == 0) {
    val_str = push_unserialize_get_value(attribute_names, attribute_values, "name");
    if (val_str != NULL) {
      args->current_binding = push_intern_name(args->push, val_str);
    }
  }
  else if (strcmp(element_name, "config") == 0) {
    val_str = push_unserialize_get_value(attribute_names, attribute_values, "name");
    if (val_str != NULL) {
      args->current_config = push_intern_name(args->push, val_str);
    }
  }
  else if (strcmp(element_name, "bool") == 0) {
    val_str = push_unserialize_get_value(attribute_names, attribute_values, "value");
    if (val_str != NULL) {
      val = push_val_new(args->push, PUSH_TYPE_BOOL, push_unserialize_bool(val_str));
      push_unserialize_add_value(args, val);
    }
  }
  else if (strcmp(element_name, "code") == 0) {
    val = push_val_new(args->push, PUSH_TYPE_CODE, NULL);
    args->val_stack = g_list_prepend(args->val_stack, val);
  }
  else if (strcmp(element_name, "int") == 0) {
    val_str = push_unserialize_get_value(attribute_names, attribute_values, "value");
    if (val_str != NULL) {
      val = push_val_new(args->push, PUSH_TYPE_INT, atoi(val_str));
      push_unserialize_add_value(args, val);
    }
  }
  else if (strcmp(element_name, "instr") == 0) {
    val_str = push_unserialize_get_value(attribute_names, attribute_values, "name");
    if (val_str != NULL) {
      instr = push_instr_lookup(args->push, val_str);
      if (instr != NULL) {
        val = push_val_new(args->push, PUSH_TYPE_INSTR, instr);
        push_unserialize_add_value(args, val);
      }
      else {
        g_warning("Unknown instruction: %s", val_str);
      }
    }
  }
  else if (strcmp(element_name, "name") == 0) {
    val_str = push_unserialize_get_value(attribute_names, attribute_values, "value");
    if (val_str != NULL) {
      val = push_val_new(args->push, PUSH_TYPE_NAME, push_intern_name(args->push, val_str));
      push_unserialize_add_value(args, val);
    }
  }
  else if (strcmp(element_name, "real") == 0) {
    val_str = push_unserialize_get_value(attribute_names, attribute_values, "value");
    if (val_str != NULL) {
      val = push_val_new(args->push, PUSH_TYPE_REAL, strtod(val_str, NULL));
      push_unserialize_add_value(args, val);
    }
  }
}


static void push_unserialize_end_tag(GMarkupParseContext *ctx, const char *element_name, void *userdata, GError **error) {
  struct push_unserialize_args *args = (struct push_unserialize_args*)userdata;
  push_val_t *val;

  if (strcmp(element_name, "stack") == 0) {
    args->current_stack = NULL;
  }
  else if (strcmp(element_name, "binding") == 0) {
    args->current_binding = NULL;
  }
  else if (strcmp(element_name, "config") == 0) {
    args->current_config = NULL;
  }
  else if (strcmp(element_name, "code") == 0 && args->val_stack != NULL) {
    // pop top-most (code) value
    val = (push_val_t*)args->val_stack->data;
    args->val_stack = g_list_delete_link(args->val_stack, args->val_stack);

    // add value
    push_unserialize_add_value(args, val);
  }
}


static void push_unserialize_error(GMarkupParseContext *ctx, GError *error, void *userdata) {

}


void push_unserialize_parse(push_t *push, const char *xml_data, GError **error) {
  const GMarkupParser parser = {
    .start_element = push_unserialize_start_tag,
    .end_element = push_unserialize_end_tag,
    .error = push_unserialize_error
  };
  struct push_unserialize_args args;
  GMarkupParseContext *ctx;

  args.push = push;
  args.current_binding = NULL;
  args.current_config = NULL;
  args.current_stack = NULL;
  args.val_stack = NULL;

  ctx = g_markup_parse_context_new(&parser, 0, &args, NULL);
  g_markup_parse_context_parse(ctx, xml_data, strlen(xml_data), error);
  g_markup_parse_context_end_parse(ctx, error);
  g_markup_parse_context_free(ctx);
}



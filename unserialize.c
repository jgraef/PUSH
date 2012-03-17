/* unserialize.c - Unserialize XML representation
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

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <expat.h>

#include "push.h"

#include <stdio.h>



#define push_unserialize_bool(s) (g_ascii_strcasecmp((s), "true") == 0)


struct push_unserialize_parser {
  XML_Parser xml_parser;
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


static const char *push_unserialize_get_value(const char **attrs, const char *key) {
  int i;

  for (i = 0; attrs[i] != NULL; i += 2) {
    if (strcmp(attrs[i], key) == 0) {
      return attrs[i + 1];
    }
  }

  return NULL;
}


static void push_unserialize_add_value(struct push_unserialize_parser *parser, push_val_t *val) {
  push_val_t *val2;

  if (parser->val_stack != NULL) {
    val2 = (push_val_t*)parser->val_stack->data;
    g_queue_push_tail(val2->code, val);
  }
  else if (parser->current_stack != NULL) {
    g_queue_push_tail(parser->current_stack, val);
  }
  else if (parser->current_binding != NULL) {
    push_define(parser->push, parser->current_binding, val);
  }
  else if (parser->current_config != NULL) {
    push_config_set(parser->push, parser->current_config, val);
  }
  else {
    // NOTE: should not happen; else free value
    g_warning("Found unused value");
    push_val_destroy(val);
  }
}


static void push_unserialize_start_tag(struct push_unserialize_parser *parser, const char *name, const char **attrs) {
  int type;
  push_val_t *val;
  push_instr_t *instr;
  const char *val_str;

  if (strcmp(name, "stack") == 0) {
    val_str = push_unserialize_get_value(attrs, "name");
    if (val_str != NULL) {
      parser->current_stack = push_unserialize_name2stack(parser->push, val_str);
    }
  }
  else if (strcmp(name, "binding") == 0) {
    val_str = push_unserialize_get_value(attrs, "name");
    if (val_str != NULL) {
      parser->current_binding = push_intern_name(parser->push, val_str);
    }
  }
  else if (strcmp(name, "config") == 0) {
    val_str = push_unserialize_get_value(attrs, "name");
    if (val_str != NULL) {
      parser->current_config = push_intern_name(parser->push, val_str);
    }
  }
  else if (strcmp(name, "none") == 0) {
    val = push_val_new(parser->push, PUSH_TYPE_NONE);
    push_unserialize_add_value(parser, val);
  }
  else if (strcmp(name, "bool") == 0) {
    val_str = push_unserialize_get_value(attrs, "value");
    if (val_str != NULL) {
      val = push_val_new(parser->push, PUSH_TYPE_BOOL, push_unserialize_bool(val_str));
      push_unserialize_add_value(parser, val);
    }
  }
  else if (strcmp(name, "code") == 0) {
    val = push_val_new(parser->push, PUSH_TYPE_CODE, NULL);
    parser->val_stack = g_list_prepend(parser->val_stack, val);
  }
  else if (strcmp(name, "int") == 0) {
    val_str = push_unserialize_get_value(attrs, "value");
    if (val_str != NULL) {
      val = push_val_new(parser->push, PUSH_TYPE_INT, atoi(val_str));
      push_unserialize_add_value(parser, val);
    }
  }
  else if (strcmp(name, "instr") == 0) {
    val_str = push_unserialize_get_value(attrs, "name");
    if (val_str != NULL) {
      instr = push_instr_lookup(parser->push, val_str);
      if (instr != NULL) {
        val = push_val_new(parser->push, PUSH_TYPE_INSTR, instr);
        push_unserialize_add_value(parser, val);
      }
      else {
        g_warning("Unknown instruction: %s", val_str);
      }
    }
  }
  else if (strcmp(name, "name") == 0) {
    val_str = push_unserialize_get_value(attrs, "value");
    if (val_str != NULL) {
      val = push_val_new(parser->push, PUSH_TYPE_NAME, push_intern_name(parser->push, val_str));
      push_unserialize_add_value(parser, val);
    }
  }
  else if (strcmp(name, "real") == 0) {
    val_str = push_unserialize_get_value(attrs, "value");
    if (val_str != NULL) {
      val = push_val_new(parser->push, PUSH_TYPE_REAL, strtod(val_str, NULL));
      push_unserialize_add_value(parser, val);
    }
  }
}


static void push_unserialize_end_tag(struct push_unserialize_parser *parser, const char *name, const char *attrs) {
  push_val_t *val;

  if (strcmp(name, "stack") == 0) {
    parser->current_stack = NULL;
  }
  else if (strcmp(name, "binding") == 0) {
    parser->current_binding = NULL;
  }
  else if (strcmp(name, "config") == 0) {
    parser->current_config = NULL;
  }
  else if (strcmp(name, "code") == 0 && parser->val_stack != NULL) {
    // pop top-most (code) value
    val = (push_val_t*)parser->val_stack->data;
    parser->val_stack = g_list_delete_link(parser->val_stack, parser->val_stack);

    // add value
    push_unserialize_add_value(parser, val);
  }
}


void push_unserialize_parse(push_t *push, const char *xml_data) {
  struct push_unserialize_parser parser;

  parser.xml_parser = XML_ParserCreate("UTF-8");
  parser.push = push;
  parser.current_binding = NULL;
  parser.current_config = NULL;
  parser.current_stack = NULL;
  parser.val_stack = NULL;

  XML_SetUserData(parser.xml_parser, &parser);
  XML_SetElementHandler(parser.xml_parser, (XML_StartElementHandler)push_unserialize_start_tag, (XML_EndElementHandler)push_unserialize_end_tag);

  XML_Parse(parser.xml_parser, xml_data, strlen(xml_data), 1);

  XML_ParserFree(parser.xml_parser);
}



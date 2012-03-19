/* serialize.c - Serializing PUSH objects
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


#define make_ident(n) g_strnfill(n, ' ')
#define free_ident(i) g_free(i)
#define str_bool(b)   ((b) ? "true" : "false")


void push_serialize_val(GString *xml, int ident_count, push_val_t *val) {
  char *ident;

  g_return_if_null(val);

  ident = make_ident(ident_count);
  ident_count++;

  switch (val->type) {
    case PUSH_TYPE_BOOL:
      g_string_append_printf(xml, "%s<bool value=\"%s\" />\n", ident, str_bool(val->boolean));
      break;
    case PUSH_TYPE_CODE:
      g_string_append_printf(xml, "%s<code>\n", ident);
      push_serialize_code(xml, ident_count, val->code);
      g_string_append_printf(xml, "%s</code>\n", ident);
      break;
    case PUSH_TYPE_INT:
      g_string_append_printf(xml, "%s<int value=\"%d\" />\n", ident, val->integer);
      break;
    case PUSH_TYPE_INSTR:
      g_string_append_printf(xml, "%s<instr name=\"%s\" />\n", ident, val->instr->name);
      break;
    case PUSH_TYPE_NAME:
      g_string_append_printf(xml, "%s<name value=\"%s\" />\n", ident, val->name);
      break;
    case PUSH_TYPE_REAL:
      g_string_append_printf(xml, "%s<real value=\"%f\" />\n", ident, val->real);
      break;
  }

  free_ident(ident);
}


void push_serialize_code(GString *xml, int ident_count, push_code_t *code) {
  GList *link;

  g_return_if_null(code);

  for (link = code->head; link != NULL; link = link->next) {
    push_serialize_val(xml, ident_count, link->data);
  }
}


void push_serialize_stack(GString *xml, int ident_count, const char *name, push_stack_t *stack) {
  GList *link;
  char *ident;

  g_return_if_null(stack);

  ident = make_ident(ident_count);
  ident_count++;

  g_string_append_printf(xml, "%s<stack name=\"%s\">\n", ident, name);

  for (link = stack->head; link != NULL; link = link->next) {
    push_serialize_val(xml, ident_count, (push_val_t*)link->data);
  }

  g_string_append_printf(xml, "%s</stack>\n", ident);

  free_ident(ident);

}


static void push_serialize_dict(GString *xml, int ident_count, const char *item_name, GHashTable *dict) {
  char *ident;
  GHashTableIter iter;
  push_name_t name;
  push_val_t *val;

  ident = make_ident(ident_count);
  ident_count++;

  g_hash_table_iter_init(&iter, dict);
  while (g_hash_table_iter_next(&iter, (void*)&name, (void*)&val)) {
    if (val->type != PUSH_TYPE_INSTR) {
      g_string_append_printf(xml, "%s<%s name=\"%s\">\n", ident, item_name, name);
      push_serialize_val(xml, ident_count, val);
      g_string_append_printf(xml, "%s</%s>\n", ident, item_name);
    }
  }

  free_ident(ident);
}



void push_serialize(GString *xml, int ident_count, push_t *push) {
  char *ident;

  g_return_if_null(push);

  ident = make_ident(ident_count);
  ident_count++;

  g_string_append_printf(xml, "%s<state>\n", ident);

  push_serialize_dict(xml, ident_count, "config", push->config);
  push_serialize_dict(xml, ident_count, "binding", push->bindings);

  push_serialize_stack(xml, ident_count, "boolean", push->boolean);
  push_serialize_stack(xml, ident_count, "code", push->code);
  push_serialize_stack(xml, ident_count, "exec", push->exec);
  push_serialize_stack(xml, ident_count, "integer", push->integer);
  push_serialize_stack(xml, ident_count, "name", push->name);
  push_serialize_stack(xml, ident_count, "real", push->real);

  g_string_append_printf(xml, "%s</state>\n", ident);

  free_ident(ident);
}


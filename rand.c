/* rand.c - Generating random PUSH values
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

#include <glib.h>

#include "push.h"



static void g_hash_table_random(GRand *rand, GHashTable *hash_table, void *key, void *value) {
  int i, j, size;
  GHashTableIter iter;

  size = g_hash_table_size(hash_table);
  if (size > 0) {
    i = g_rand_int_range(rand, 0, size);
    g_hash_table_iter_init(&iter, hash_table);

    for (j = 0; j <= i && g_hash_table_iter_next(&iter, key, value); j++);
  }
}


void push_rand_set_seed(push_t *push, int seed) {
  g_return_if_null(push);

  g_rand_set_seed(push->rand, (guint32)seed);
}


push_bool_t push_rand_bool(push_t *push) {
  g_return_val_if_null(push, PUSH_FALSE);

  return g_rand_boolean(push->rand);
}


push_code_t *push_rand_code(push_t *push, push_int_t *size, push_bool_t force_size) {
  push_code_t *code;
  push_int_t this_size;
  push_val_t *val;

  g_return_val_if_null(push, NULL);
  g_return_val_if_null(size, NULL);

  code = push_code_new();
  this_size = force_size || *size == 0 ? *size : g_rand_int_range(push->rand, 0, *size);

  *size -= this_size;

  while (this_size > 0) {
    val = push_rand_val(push, PUSH_TYPE_NONE, &this_size, PUSH_FALSE);

    push_code_append(code, val);
  }

  return code;
}


push_int_t push_rand_int(push_t *push) {
  push_val_t *val1, *val2;

  g_return_val_if_null(push, 0);

  val1 = push_config_get(push, "MIN-RANDOM-INT");
  val2 = push_config_get(push, "MAX-RANDOM-INT");
  g_return_val_if_fail(val1 != NULL && push_check_int(val1), 0);
  g_return_val_if_fail(val2 != NULL && push_check_int(val2), 0);

  return (push_int_t)g_rand_int_range(push->rand, (gint32)val1->integer, (gint32)val2->integer);
}


push_instr_t *push_rand_instr(push_t *push) {
  push_instr_t *instr;

  g_return_val_if_null(push, NULL);

  g_hash_table_random(push->rand, push->instructions, NULL, (void*)&instr);

  g_warn_if_fail(instr != NULL);

  return instr;
}


push_name_t push_rand_name(push_t *push) {
  char *buf;
  push_name_t name;
  push_val_t *val1, *val2;
  int i, length;

  val1 = push_config_get(push, "MIN-RANDOM-NAME-LENGTH");
  val2 = push_config_get(push, "MAX-RANDOM-NAME-LENGTH");
  g_return_val_if_fail(val1 != NULL && push_check_int(val1), 0);
  g_return_val_if_fail(val2 != NULL && push_check_int(val2), 0);

  /* get random name length and allocate name buffer */
  length = g_rand_int_range(push->rand, val1->integer, val2->integer);
  buf = g_malloc(length + 1);

  /* generate random uppercase letter sequence */
  for (i = 0; i < length; i++) {
    buf[i] = g_rand_int_range(push->rand, 'A', 'Z' + 1);
  }
  /* 0-terminator */
  buf[i] = 0;

  /* intern name, free buf and return interned name */
  name = push_intern_name(push, buf);
  g_free(buf);
  return name;
}


push_name_t push_rand_bound_name(push_t *push) {
  push_name_t name = NULL;

  g_hash_table_random(push->rand, push->bindings, (void*)&name, NULL);
  if (name == NULL) {
    /* no names defined */
    name = push_rand_name(push);
  }

  return name;
}


push_real_t push_rand_real(push_t *push) {
  push_val_t *val1, *val2;

  g_return_val_if_null(push, 0.0);

  val1 = push_config_get(push, "MIN-RANDOM-REAL");
  val2 = push_config_get(push, "MAX-RANDOM-REAL");
  g_return_val_if_fail(val1 != NULL && push_check_real(val1), 0.0);
  g_return_val_if_fail(val2 != NULL && push_check_real(val2), 0.0);

  return (push_real_t)g_rand_double_range(push->rand, (gint32)val1->real, (gint32)val2->real);
}


push_val_t *push_rand_val(push_t *push, int type, push_int_t *size, push_bool_t force_size) {
  push_val_t *val, *p;
  push_int_t size_dummy = 1;

  g_return_val_if_null(push, NULL);

  /* if no size pointer supplied, use a dummy (1) */
  if (size == NULL) {
    size = &size_dummy;
  }
  if (*size < 1) {
    *size = 1;
  }

  /* create new value, either with given type or random type */
  val = push_val_new(push, PUSH_TYPE_NONE);
  val->type = type == PUSH_TYPE_NONE ? g_rand_int_range(push->rand, PUSH_TYPE_BOOL, PUSH_TYPE_REAL + 1) : type;
  *size -= 1;

  /* set random value */
  switch (val->type) {
    case PUSH_TYPE_BOOL:
      val->boolean = push_rand_bool(push);
      break;

    case PUSH_TYPE_CODE:
      val->code = push_rand_code(push, size, force_size);
      break;

    case PUSH_TYPE_INT:
      val->integer = push_rand_int(push);
      break;

    case PUSH_TYPE_INSTR:
      val->instr = push_rand_instr(push);
      break;

    case PUSH_TYPE_NAME:
      p = push_config_get(push, "NEW-ERC-NAME-PROBABILITY");
      if (p != NULL && push_check_real(p)) {
        val->name = g_rand_double(push->rand) < p->real ? push_rand_name(push) : push_rand_bound_name(push);
      }
      else {
        g_warning("Configuration value 'NEW-ERC-NAME-PROBABILITY' is not a real number");
        val->name = push_rand_name(push);
      }
      break;

    case PUSH_TYPE_REAL:
      val->real = push_rand_real(push);
      break;

    default:
      g_warning("Unknown value type: %d", val->type);
      break;
  }

  return val;
}


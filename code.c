/* code.c - Implementation of the code data type
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



push_code_t *push_code_new(void) {
  return g_queue_new();
}

void push_code_destroy(push_code_t *code) {
  g_queue_free(code);
}

void push_code_append(push_code_t *code, push_val_t *val) {
  g_return_if_null(val);

  g_queue_push_tail(code, val);
}

void push_code_prepend(push_code_t *code, push_val_t *val) {
  g_return_if_null(val);

  g_queue_push_head(code, val);
}

void push_code_insert(push_code_t *code, int n, push_val_t *val) {
  g_return_if_null(val);

  g_queue_push_nth(code, val, n);
}

push_val_t *push_code_pop(push_code_t *code) {
  return (push_val_t*)g_queue_pop_head(code);
}

push_val_t *push_code_pop_nth(push_code_t *code, int n) {
  return (push_val_t*)g_queue_pop_nth(code, n);
}

push_val_t *push_code_peek(push_code_t *code) {
  return (push_val_t*)g_queue_peek_head(code);
}

push_val_t *push_code_peek_nth(push_code_t *code, int n) {
  return (push_val_t*)g_queue_peek_nth(code, n);
}

int push_code_length(push_code_t *code) {
  return code->length;
}

void push_code_flush(push_code_t *code) {
  g_queue_clear(code);
}

void push_code_foreach(push_code_t *code, GFunc func, void *userdata) {
  g_queue_foreach(code, func, userdata);
}


/* duplicate code */
push_code_t *push_code_dup_ext(push_code_t *code, GList *first_link, GList *last_link) {
  push_code_t *new_code;
  GList *link;

  g_return_val_if_null(code, NULL);

  new_code = push_code_new();

  if (first_link == NULL) {
    first_link = code->head;
  }

  for (link = first_link; link != NULL; link = link->next) {
    push_code_append(new_code, (push_val_t*)link->data);

    if (link == last_link) {
      break;
    }
  }

  return new_code;
}


push_code_t *push_code_dup(push_code_t *code) {
  return push_code_dup_ext(code, NULL, NULL);
}


push_bool_t push_code_equal(push_code_t *code1, push_code_t *code2) {
  GList *link1, *link2;

  g_return_val_if_null(code1, PUSH_FALSE);
  g_return_val_if_null(code2, PUSH_FALSE);

  if (code1->length != code2->length) {
    return PUSH_FALSE;
  }

  link1 = code1->head;
  link2 = code2->head;

  while (link1 != NULL && link2 != NULL) {
    if (!push_val_equal(link1->data, link2->data)) {
      return PUSH_FALSE;
    }

    link1 = link1->next;
    link2 = link2->next;
  }

  return PUSH_TRUE;

}


/* Concat code1 and code2 */
push_code_t *push_code_concat(push_code_t *_code1, push_code_t *_code2) {
  push_code_t *code1, *code2;

  g_return_val_if_null(_code1, NULL);
  g_return_val_if_null(_code2, NULL);

  code1 = push_code_dup(_code1);
  code2 = push_code_dup(_code2);

  if (code2->head != NULL && code2->tail != NULL) {
    if (code1->head == NULL && code1->tail == NULL) {
      push_code_destroy(code1);
      return code2;
    }
    else {
      code1->tail->next = code2->head;
      code2->head->prev = code1->tail;
      code1->tail = code2->tail;
      code1->length += code2->length;
      code2->length = 0;
      code2->head = NULL;
      code2->tail = NULL;
    }
  }

  push_code_destroy(code2);
  return code1;
}


/* Returns the sub-code in haystack which contains the needle
 * NOTE: Definition from http://hampshire.edu/lspector/push3-description.html
 *       CODE.CONTAINER: Pushes the "container" of the second CODE stack item
 *       within the first CODE stack item onto the CODE stack. If second item
 *       contains the first anywhere (i.e. in any nested list) then the
 *       container is the smallest sub-list that contains but is not equal to
 *       the first instance. For example, if the top piece of code is "( B ( C
 *       ( A ) ) ( D ( A ) ) )" and the second piece of code is "( A )" then
 *       this pushes ( C ( A ) ). Pushes an empty list if there is no such
 *       container. 
 */
struct push_code_container_args {
  push_val_t *needle;
  push_code_t *finger;
};
static push_code_t *push_code_container_finger(push_code_t *haystack, push_val_t *needle);

static int push_code_container_find(push_val_t *val, struct push_code_container_args *args) {
  push_val_t *needle = args->needle;
  push_code_t *finger;

  g_return_val_if_null(val, 1);

  if (push_val_equal(val, args->needle)) {
    /* found needle, stop search so push_code_container_finger can return the finger */
    return 0;
  }
  else if (push_check_code(val)) {
    // this is a sub-code, search it
    finger = push_code_container(val->code, needle);
    if (finger != NULL) {
      /* recursive call found container, pass finger and stop search */
      args->finger = finger;
      return 0;
    }
  }

  /* nothing found, go to next element */
  return 1;
}

push_code_t *push_code_container(push_code_t *haystack, push_val_t *needle) {
  struct push_code_container_args args = {
    .needle = needle,
    .finger = haystack
  };

  if (g_queue_find_custom(haystack, &args, (GCompareFunc)push_code_container_find) != NULL) {
    return args.finger;
  }
  else {
    return NULL;
  }
}


/* Calculate discrepancy of both lists */
int push_code_discrepancy(push_code_t *code1, push_code_t *code2) {
  // TODO
  return 0;
}


/* removes element indexed by point from code and returns it */
struct push_code_extract_args {
  int point;
  push_val_t *val;
};

static int push_code_extract_find(push_val_t *val, struct push_code_extract_args *args) {
  g_return_val_if_null(val, 0);

  if (args->point == 0) {
    /* found element, return it */
    args->val = val;
    return 0;
  }
  else if (push_check_code(val)) {
    args->point--;

    /* descend on sub-code */
    if (g_queue_find_custom(val->code, args, (GCompareFunc)push_code_extract_find) != NULL) {
      /* found element in sub-code, return it */
      return 0;
    }
  }
  else {
    args->point--;
  }

  return 1;
}

push_val_t *push_code_extract(push_code_t *code, push_int_t point) {
  struct push_code_extract_args args = {
    .point = point,
    .val = NULL
  };

  g_return_val_if_fail(point >= 0, NULL);

  g_queue_find_custom(code, &args, (GCompareFunc)push_code_extract_find);

  return args.val;
}


/* returns index of value in code */
struct push_code_index_args {
  push_val_t *needle;
  int index;
};

static int push_code_index_find(push_val_t *val, struct push_code_index_args *args) {
  if (push_val_equal(val, args->needle)) {
    return 0;
  }
  else {
    args->index++;
    return 1;
  }
}

int push_code_index(push_code_t *haystack, push_val_t *needle) {
  struct push_code_index_args args = {
    .needle = needle,
    .index = 0
  };

  if (g_queue_find_custom(haystack, &args, (GCompareFunc)push_code_index_find) != NULL) {
    return args.index;
  }
  else {
    return -1;
  }
}


/* returns size ("number of points") */
static void push_code_size_iter(push_val_t *val, int *size) {
  g_return_if_null(val);

  *size += 1;
  if (push_check_code(val)) {
    g_queue_foreach(val->code, (GFunc)push_code_size_iter, size);
  }
}

int push_code_size(push_code_t *code) {
  int size = 0;

  g_queue_foreach(code, (GFunc)push_code_size_iter, &size);

  return size;
}


/* insert element */
push_code_t *push_code_replace(push_code_t *_code, push_int_t i, push_val_t *val) {
  push_code_t *code;

  code = push_code_dup(_code);

  // TODO

  return code;
}


/* push each element onto stack */
void push_code_push_elements(push_code_t *code, push_stack_t *stack) {
  GList *link;

  for (link = code->tail; link != NULL; link = link->prev) {
    g_warn_if_fail(link->data != NULL);
    push_stack_push(stack, (push_val_t*)link->data);
  }
}


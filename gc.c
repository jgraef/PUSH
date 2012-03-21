/* gc.c - A multi-threaded mark-and-sweep garbage collector for PUSH values
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


/* Messages to communicate with GC asynchronously */
#define PUSH_GC_MSG_ADD_INTERPRETER    1
#define PUSH_GC_MSG_REMOVE_INTERPRETER 2
#define PUSH_GC_MSG_ADD_VAL            3
#define PUSH_GC_MSG_REMOVE_VAL         4
#define PUSH_GC_MSG_QUIT               5
struct push_gc_msg {
  int type;
  union {
    push_t *push;
    push_val_t *val;
    void *_data;
  };
};



static void push_gc_mark_val(push_val_t *val, push_int_t *mark) {
  val->gc.mark = *mark;

  if (push_check_code(val)) {
    g_queue_foreach(val->code, (GFunc)push_gc_mark_val, mark);
  }
}


static void push_gc_mark_hash_table(GHashTable *hash_table, push_int_t *mark) {
  GHashTableIter iter;
  push_val_t *val;

  g_hash_table_iter_init(&iter, hash_table);

  while (g_hash_table_iter_next(&iter, NULL, (void*)&val)) {
    push_gc_mark_val(val, mark);
  }
}


static void push_gc_mark_stack(push_stack_t *stack, push_int_t *mark) {
  g_queue_foreach(stack, (GFunc)push_gc_mark_val, mark);
}


static void push_gc_mark_interpreter(push_t *push, push_int_t *mark) {
  g_static_mutex_lock(&push->mutex);

  /* mark stacks */
  push_gc_mark_stack(push->boolean, mark);
  push_gc_mark_stack(push->code, mark);
  push_gc_mark_stack(push->exec, mark);
  push_gc_mark_stack(push->integer, mark);
  push_gc_mark_stack(push->name, mark);
  push_gc_mark_stack(push->real, mark);

  /* mark bindings */
  push_gc_mark_hash_table(push->bindings, mark);

  /* mark config */
  push_gc_mark_hash_table(push->config, mark);

  g_static_mutex_unlock(&push->mutex);
}


static GList *push_gc_sweep(GList *values, push_int_t mark) {
  GList *link, *next_link;
  push_val_t *val;

  for (link = values; link != NULL; link = next_link) {
    next_link = link->next;
    val = (push_val_t*)link->data;
    if (val->gc.untrack) {
      values = g_list_delete_link(values, link);
      val->gc.untrack = FALSE;
    }
    else if (val->gc.mark != mark) {
      values = g_list_delete_link(values, link);
      push_val_destroy(val);
    }
  }

  return values;
}


static void *push_gc_main(GAsyncQueue *queue) {
  GList *interpreters = NULL;
  GList *values = NULL;
  push_int_t mark;
  struct push_gc_msg *msg;
  GTimeVal end_time;
  push_bool_t alive = TRUE;

  /* initialize */
  g_async_queue_ref(queue);

  /* mark & sweep until quit */
  for (mark = 0; alive; mark++) {
    /* read message queue */
    do {
      g_get_current_time(&end_time);
      g_time_val_add(&end_time, PUSH_GC_WAIT_USEC);
      msg = g_async_queue_timed_pop(queue, &end_time);

      if (msg != NULL) {
        switch (msg->type) {
          case PUSH_GC_MSG_ADD_INTERPRETER:
            interpreters = g_list_prepend(interpreters, msg->push);
            break;
          case PUSH_GC_MSG_REMOVE_INTERPRETER:
            interpreters = g_list_remove(interpreters, msg->push);
            break;
          case PUSH_GC_MSG_ADD_VAL:
            msg->val->gc.mark = mark;
            msg->val->gc.untrack = FALSE;
            values = g_list_prepend(values, msg->val);
            break;
          case PUSH_GC_MSG_REMOVE_VAL:
            msg->val->gc.untrack = TRUE;
            break;
          case PUSH_GC_MSG_QUIT:
            alive = FALSE;
            break;
          default:
            g_warning("%s: Unknown message type: %d", __func__, msg->type);
            break;

          g_slice_free(struct push_gc_msg, msg);
        }
      }
    } while (msg != NULL && alive);

    /* mark interpreters */
    g_list_foreach(interpreters, (GFunc)push_gc_mark_interpreter, &mark);

    /* sweep values */
    push_gc_sweep(values, mark);

    g_thread_yield();
  }

  /* clean up */
  g_list_free_full(values, (GDestroyNotify)push_val_destroy);
  g_list_free(interpreters);
  g_async_queue_unref(queue);

  return NULL;
}


static void push_gp_send(push_gc_t *gc, int type, void *data) {
  struct push_gc_msg *msg;

  msg = g_slice_new(struct push_gc_msg);
  msg->type = type;
  msg->_data = data;
  g_async_queue_push(gc->queue, msg);
}



push_gc_t *push_gc_new(void) {
  push_gc_t *gc;

  /* initialize threading (if not yet initialized) */
  g_thread_init(NULL);

  gc = g_slice_new(push_gc_t);
  gc->queue = g_async_queue_new();
  gc->thread = g_thread_create((GThreadFunc)push_gc_main, gc->queue, TRUE, NULL);

  return gc;
}


void push_gc_destroy(push_gc_t *gc) {
  /* stop thread */
  push_gp_send(gc, PUSH_GC_MSG_QUIT, NULL);
  g_thread_join(gc->thread);

  g_async_queue_unref(gc->queue);
}


void push_gc_add_interpreter(push_gc_t *gc, push_t *push) {
  push_gp_send(gc, PUSH_GC_MSG_ADD_INTERPRETER, push);
}


void push_gc_remove_interpreter(push_gc_t *gc, push_t *push) {
  push_gp_send(gc, PUSH_GC_MSG_REMOVE_INTERPRETER, push);
}


void push_gc_add_val(push_gc_t *gc, push_val_t *val, push_bool_t recursive) {
  GList *link;

  push_gp_send(gc, PUSH_GC_MSG_ADD_VAL, val);

  if (recursive && push_check_code(val)) {
    for (link = val->code->head; link != NULL; link = link->next) {
      push_gc_add_val(gc, val, TRUE);
    }
  }
}


void push_gc_remove_val(push_gc_t *gc, push_val_t *val, push_bool_t recursive) {
  GList *link;

  push_gp_send(gc, PUSH_GC_MSG_REMOVE_VAL, val);

  if (recursive && push_check_code(val)) {
    for (link = val->code->head; link != NULL; link = link->next) {
      push_gc_remove_val(gc, val, TRUE);
    }
  }
}



static push_gc_t *_global_gc = NULL;


static void push_gc_global_destroy(void) {
  if (_global_gc != NULL) {
    push_gc_destroy(_global_gc);
    _global_gc = NULL;
  }
}


push_gc_t *push_gc_global(void) {
  if (_global_gc == NULL) {
    _global_gc = push_gc_new();
#ifdef GC_USE_ATEXIT
    g_atexit(push_gc_global_destroy);
#endif
  }

  return _global_gc;
}


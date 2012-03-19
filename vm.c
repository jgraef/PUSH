/* vm.c - Multi-threaded execution of PUSH interpreters
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



static void push_vm_run_process(push_t *push, push_vm_t *vm) {
  if (vm->max_steps > 0) {
    push_steps(push, vm->max_steps);
  }
  else {
    push_run(push);
  }

  g_static_mutex_lock(&vm->mutex);
  vm->processes = g_list_remove(vm->processes, push);  
  g_static_mutex_unlock(&vm->mutex);

  if (vm->done_callback != NULL) {
    vm->done_callback(vm, push);
  }
}


static void push_vm_destroy_process(push_t *push, push_vm_t *vm) {
  push_destroy(push);
}


push_vm_t *push_vm_new(push_int_t num_threads, push_int_t max_steps, push_vm_done_callback_t done_callback) {
  push_vm_t *vm;

  /* initialize threading (if not yet initialized) */
  g_thread_init(NULL);

  vm = g_slice_new(push_vm_t);

  vm->processes = NULL;
  vm->max_steps = max_steps;
  vm->done_callback = done_callback;
  g_static_mutex_init(&vm->mutex);

  /* initialize thread pool */
  vm->threads = g_thread_pool_new((GFunc)push_vm_run_process, vm, num_threads, TRUE, NULL);
  if (vm->threads == NULL) {
    g_slice_free(push_vm_t, vm);
    return NULL;
  }

  return vm;
}


void push_vm_destroy(push_vm_t *vm, push_bool_t kill_all) {
  g_return_if_null(vm);
  g_return_if_fail(g_static_mutex_trylock(&vm->mutex));

  if (kill_all) {
    push_vm_kill_all(vm);
  }

  g_thread_pool_free(vm->threads, kill_all, TRUE);

  g_slice_free(push_vm_t, vm);
}


void push_vm_run(push_vm_t *vm, push_t *push) {
  g_return_if_null(vm);

  g_static_mutex_lock(&vm->mutex);
  vm->processes = g_list_prepend(vm->processes, push);
  g_thread_pool_push(vm->threads, push, NULL);
  g_static_mutex_unlock(&vm->mutex);
}


push_int_t push_vm_num_processes(push_vm_t *vm) {
  push_int_t num;

  g_return_val_if_null(vm, 0);

  g_static_mutex_lock(&vm->mutex);
  num = g_list_length(vm->processes);
  g_static_mutex_unlock(&vm->mutex);

  return num;
}


push_int_t push_vm_num_queued(push_vm_t *vm) {
  push_int_t num;

  g_return_val_if_null(vm, 0);

  g_static_mutex_lock(&vm->mutex);
  num = g_thread_pool_unprocessed(vm->threads);
  g_static_mutex_unlock(&vm->mutex);

  return num;
}


void push_vm_interrupt_all(push_vm_t *vm, push_int_t interrupt_flag) {
  GList *link;

  g_return_if_null(vm);

  g_static_mutex_lock(&vm->mutex);
  for (link = vm->processes; link != NULL; link = link->next) {
    push_interrupt((push_t*)link->data, interrupt_flag);
  }
  g_static_mutex_unlock(&vm->mutex);
}


void push_vm_kill_all(push_vm_t *vm) {
  push_vm_interrupt_all(vm, PUSH_VM_INTERRUPT_KILL);
}



/* rand.h - Generating random PUSH values
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

#ifndef _PUSH_RAND_H_
#define _PUSH_RAND_H_


#include "push/types.h"
#include "push/interpreter.h"
#include "push/val.h"


void push_rand_set_seed(push_t *push, int seed);
push_bool_t push_rand_bool(push_t *push);
push_code_t *push_rand_code(push_t *push, push_int_t *size, push_bool_t force_size);
push_int_t push_rand_int(push_t *push);
push_instr_t *push_rand_instr(push_t *push);
push_name_t push_rand_name(push_t *push);
push_name_t push_rand_bound_name(push_t *push);
push_real_t push_rand_real(push_t *push);
push_val_t *push_rand_val(push_t *push, int type, push_int_t *size, push_bool_t force_size);


#endif /* _PUSH_RAND_H_ */


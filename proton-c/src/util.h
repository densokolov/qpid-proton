#ifndef _PROTON_SRC_UTIL_H
#define _PROTON_SRC_UTIL_H 1

/*
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 */

#include <errno.h>
#ifndef __cplusplus
#include <stdbool.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <proton/types.h>

#define PN_LEVEL_ERROR 1
#define PN_LEVEL_WARN  2
#define PN_LEVEL_INFO  3
#define PN_LEVEL_DEBUG 4
#define PN_LEVEL_TRACE 5

#define PN_ERRORF(...) pn_trace_do(PN_LEVEL_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define PN_DEBUGF(...) pn_trace_do(PN_LEVEL_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define PN_TRACEF(...) pn_trace_do(PN_LEVEL_TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__)
void pn_trace_do(int level, const char *file, int line, const char *func, const char *fmt, ...);

#define PN_OBJID_BASE pn_objid_t the_objid_base
#define PN_OBJID(x) ((x) ? ((pn_objid_t*)(x))->id : "(null " #x ")")
#define PN_OBJID_INIT(x, base) pn_objid_init(&(x)->the_objid_base, base)
#define PN_OBJID_INIT2(x, y, base) pn_objid_init2(&(x)->the_objid_base, PN_OBJID(y), base)
#define PN_OBJID_MAX 100

typedef struct pn_objid_t {
  char id[PN_OBJID_MAX];
} pn_objid_t;
void pn_objid_init(pn_objid_t *objid, const char *base);
void pn_objid_init2(pn_objid_t *objid, const char *y, const char *base);

PN_EXTERN ssize_t pn_quote_data(char *dst, size_t capacity, const char *src, size_t size);
PN_EXTERN void pn_fprint_data(FILE *stream, const char *bytes, size_t size);
PN_EXTERN void pn_print_data(const char *bytes, size_t size);
bool pn_env_bool(const char *name);
pn_timestamp_t pn_timestamp_min(pn_timestamp_t a, pn_timestamp_t b);

#define DIE_IFR(EXPR, STRERR)                                           \
  do {                                                                  \
    int __code__ = (EXPR);                                              \
    if (__code__) {                                                     \
      fprintf(stderr, "%s:%d: %s: %s (%d)\n", __FILE__, __LINE__,       \
              #EXPR, STRERR(__code__), __code__);                       \
      exit(-1);                                                         \
    }                                                                   \
  } while (0)

#define DIE_IFE(EXPR)                                                   \
  do {                                                                  \
    if ((EXPR) == -1) {                                                 \
      int __code__ = errno;                                             \
      fprintf(stderr, "%s:%d: %s: %s (%d)\n", __FILE__, __LINE__,       \
              #EXPR, strerror(__code__), __code__);                     \
      exit(-1);                                                         \
    }                                                                   \
  } while (0)


#define LL_HEAD(ROOT, LIST) ((ROOT)-> LIST ## _head)
#define LL_TAIL(ROOT, LIST) ((ROOT)-> LIST ## _tail)
#define LL_ADD(ROOT, LIST, NODE)                              \
  {                                                           \
    (NODE)-> LIST ## _next = NULL;                            \
    (NODE)-> LIST ## _prev = (ROOT)-> LIST ## _tail;          \
    if (LL_TAIL(ROOT, LIST))                                  \
      LL_TAIL(ROOT, LIST)-> LIST ## _next = (NODE);           \
    LL_TAIL(ROOT, LIST) = (NODE);                             \
    if (!LL_HEAD(ROOT, LIST)) LL_HEAD(ROOT, LIST) = (NODE);   \
  }

#define LL_POP(ROOT, LIST)                                       \
  {                                                              \
    if (LL_HEAD(ROOT, LIST)) {                                   \
      void *_head = LL_HEAD(ROOT, LIST);                         \
      LL_HEAD(ROOT, LIST) = LL_HEAD(ROOT, LIST)-> LIST ## _next; \
      if (_head == LL_TAIL(ROOT, LIST))                          \
        LL_TAIL(ROOT, LIST) = NULL;                              \
    }                                                            \
  }

#define LL_REMOVE(ROOT, LIST, NODE)                                    \
  {                                                                    \
    if ((NODE)-> LIST ## _prev)                                        \
      (NODE)-> LIST ## _prev-> LIST ## _next = (NODE)-> LIST ## _next; \
    if ((NODE)-> LIST ## _next)                                        \
      (NODE)-> LIST ## _next-> LIST ## _prev = (NODE)-> LIST ## _prev; \
    if ((NODE) == LL_HEAD(ROOT, LIST))                                 \
      LL_HEAD(ROOT, LIST) = (NODE)-> LIST ## _next;                    \
    if ((NODE) == LL_TAIL(ROOT, LIST))                                 \
      LL_TAIL(ROOT, LIST) = (NODE)-> LIST ## _prev;                    \
  }

char *pn_strdup(const char *src);
char *pn_strndup(const char *src, size_t n);

#define pn_min(X,Y) ((X) > (Y) ? (Y) : (X))
#define pn_max(X,Y) ((X) < (Y) ? (Y) : (X))

#define PN_ENSURE(ARRAY, CAPACITY, COUNT, TYPE)                 \
  while ((CAPACITY) < (COUNT)) {                                \
    (CAPACITY) = (CAPACITY) ? 2 * (CAPACITY) : 16;              \
    (ARRAY) = (TYPE *) realloc((ARRAY), (CAPACITY) * sizeof (TYPE));    \
  }                                                             \

#define PN_ENSUREZ(ARRAY, CAPACITY, COUNT, TYPE)           \
  {                                                        \
    size_t _old_capacity = (CAPACITY);                     \
    PN_ENSURE(ARRAY, CAPACITY, COUNT, TYPE);               \
    memset((ARRAY) + _old_capacity, 0,                     \
           sizeof(TYPE)*((CAPACITY) - _old_capacity));     \
  }

#endif /* util.h */

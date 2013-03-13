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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#ifndef __cplusplus
#include <stdint.h>
#include <stdbool.h>
#else
#include <proton/type_compat.h>
#endif
#include <ctype.h>
#include <string.h>
#include <proton/error.h>
#include <proton/util.h>
#include <proton/types.h>
#include "util.h"

ssize_t pn_quote_data(char *dst, size_t capacity, const char *src, size_t size)
{
  int idx = 0;
  for (unsigned i = 0; i < size; i++)
  {
    uint8_t c = src[i];
    if (isprint(c)) {
      if (idx < (int) (capacity - 1)) {
        dst[idx++] = c;
      } else {
        dst[idx - 1] = '\0';
        return PN_OVERFLOW;
      }
    } else {
      if (idx < (int) (capacity - 4)) {
        idx += sprintf(dst + idx, "\\x%.2x", c);
      } else {
        dst[idx - 1] = '\0';
        return PN_OVERFLOW;
      }
    }
  }

  dst[idx] = '\0';
  return idx;
}

void pn_fprint_data(FILE *stream, const char *bytes, size_t size)
{
  char buf[256];
  ssize_t n = pn_quote_data(buf, 256, bytes, size);
  if (n >= 0) {
    fputs(buf, stream);
  } else {
    if (n == PN_OVERFLOW) {
      fputs(buf, stream);
      fputs("... (truncated)", stream);
    }
    else
      fprintf(stderr, "pn_quote_data: %s\n", pn_code(n));
  }
}

void pn_print_data(const char *bytes, size_t size)
{
  pn_fprint_data(stdout, bytes, size);
}

void parse_url(char *url, char **scheme, char **user, char **pass, char **host, char **port, char **path)
{
  if (url) {
    char *scheme_end = strstr(url, "://");
    if (scheme_end) {
      *scheme_end = '\0';
      *scheme = url;
      url = scheme_end + 3;
    }

    char *at = strchr(url, '@');
    if (at) {
      *at = '\0';
      char *up = url;
      *user = up;
      url = at + 1;
      char *colon = strchr(up, ':');
      if (colon) {
        *colon = '\0';
        *pass = colon + 1;
      }
    }

    char *slash = strchr(url, '/');
    if (slash) {
      *slash = '\0';
      *host = url;
      url = slash + 1;
      *path = url;
    } else {
      *host = url;
    }

    char *colon = strchr(*host, ':');
    if (colon) {
      *colon = '\0';
      *port = colon + 1;
    }
  }
}

void pn_vfatal(const char *fmt, va_list ap)
{
  vfprintf(stderr, fmt, ap);
  abort();
}

void pn_fatal(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  pn_vfatal(fmt, ap);
  va_end(ap);
}

static bool pn_i_eq_nocase(const char *a, const char *b)
{
    while (*b) {
        if (tolower(*a++) != tolower(*b++))
            return false;
    }
    return !(*a);
}

bool pn_env_bool(const char *name)
{
  char *v = getenv(name);
  return v && (pn_i_eq_nocase(v, "true") || pn_i_eq_nocase(v, "1") ||
               pn_i_eq_nocase(v, "yes") || pn_i_eq_nocase(v, "on"));
}

int pn_env_debug_level(const char *name)
{
  char *v = getenv(name);
  if ( v ) {
    if (pn_i_eq_nocase(v, "error"))
      return PN_LEVEL_DEBUG;
    if (pn_i_eq_nocase(v, "warn") || pn_i_eq_nocase(v, "warning"))
      return PN_LEVEL_WARN;
    if (pn_i_eq_nocase(v, "info"))
      return PN_LEVEL_INFO;
    if (pn_i_eq_nocase(v, "debug"))
      return PN_LEVEL_DEBUG;
    if (pn_i_eq_nocase(v, "trace") || pn_i_eq_nocase(v, "full"))
      return PN_LEVEL_TRACE;
  }
  return PN_LEVEL_ERROR;
}

const char *pn_level_name(int level, char *level_buf) {
  const char *level_name;
  switch(level) {
    case PN_LEVEL_ERROR: level_name = "ERROR"; break;
    case PN_LEVEL_WARN:  level_name = "WARN";  break;
    case PN_LEVEL_INFO:  level_name = "INFO";  break;
    case PN_LEVEL_DEBUG: level_name = "DEBUG"; break;
    case PN_LEVEL_TRACE: level_name = "TRACE"; break;
    default:
      sprintf(level_buf, "<?-%d-?>", level);
      level_name = level_buf;
      break;
  }
  return level_name;
}

char *pn_strdup(const char *src)
{
  if (src) {
    char *dest = (char *) malloc((strlen(src)+1)*sizeof(char));
    if (!dest) return NULL;
    return strcpy(dest, src);
  } else {
    return NULL;
  }
}

char *pn_strndup(const char *src, size_t n)
{
  if (src) {
    unsigned size = 0;
    for (const char *c = src; size < n && *c; c++) {
      size++;
    }

    char *dest = (char *) malloc(size + 1);
    if (!dest) return NULL;
    strncpy(dest, src, n);
    dest[size] = '\0';
    return dest;
  } else {
    return NULL;
  }
}

// which timestamp will expire next, or zero if none set
pn_timestamp_t pn_timestamp_min( pn_timestamp_t a, pn_timestamp_t b )
{
  if (a && b) return pn_min(a, b);
  if (a) return a;
  return b;
}


void pn_objid_init(pn_objid_t *objid, const char *base)
{
  static int seq = 0;
  memset(objid, 0, sizeof(*objid));
  sprintf(objid->id, "%s-%d", base, seq);
  seq++;
}

void pn_objid_init2(pn_objid_t *objid, const char *y, const char *base)
{
  pn_objid_t buf;
  sprintf(buf.id, "%s-%s", y, base);
  pn_objid_init(objid, buf.id);
}

static void pn_trace_default_writer(void *stream, const char *buf, size_t buflen) {
  fprintf(stderr, "%.*s\n", (int)buflen, buf);
}

static void *pn_trace_writer_stream;
static pn_trace_writer_t pn_trace_writer_func = pn_trace_default_writer;

void pn_trace_set_writer(pn_trace_writer_t writer, void *stream) {
  if ( writer ) {
    pn_trace_writer_func = writer;
    pn_trace_writer_stream = stream;
  } else {
    pn_trace_writer_func = pn_trace_default_writer;
    pn_trace_writer_stream = 0;
  }
}

#define PN_TRACE_BUFLEN 10000

void pn_trace_do(int level, const char *file, int line, const char *func, const char *fmt, ...)
{
  static bool log_init = false;
  static int min_level = PN_LEVEL_ERROR;
  const size_t buflen = PN_TRACE_BUFLEN;
  char fmtbuf[PN_TRACE_BUFLEN+1+2];
  char level_buf[100];
  if ( !log_init ) {
    log_init = true;
    min_level = pn_env_debug_level("PN_TRACE_ENABLED");
    sprintf(fmtbuf, "Logging at %s", pn_level_name(min_level, level_buf));
    pn_trace_writer_func(pn_trace_writer_stream, fmtbuf, strlen(fmtbuf));
  }
  if ( level > min_level )
    return;
  const char *level_name = pn_level_name(level, level_buf);
  va_list ap;
  va_start(ap, fmt);
  snprintf(fmtbuf, buflen, "### %s:%d %s %s ### ",
           strrchr(file,'/')+1, line, func, level_name);
  vsnprintf(fmtbuf+strlen(fmtbuf), buflen-strlen(fmtbuf), fmt, ap);
  pn_trace_writer_func(pn_trace_writer_stream, fmtbuf, strlen(fmtbuf));
  va_end(ap);
}

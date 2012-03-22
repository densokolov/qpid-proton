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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

// XXX: should centralize error codes
#define PN_OVERFLOW (-1)

ssize_t pn_quote_data(char *dst, size_t capacity, const char *src, size_t size)
{
  int idx = 0;
  for (int i = 0; i < size; i++)
  {
    uint8_t c = src[i];
    if (isprint(c)) {
      if (idx < capacity - 1) {
        dst[idx++] = c;
      } else {
        return PN_OVERFLOW;
      }
    } else {
      if (idx < capacity - 5) {
        sprintf(dst + idx, "\\x%.2x", c);
        idx += 4;
      } else {
        return PN_OVERFLOW;
      }
    }
  }

  dst[idx] = '\0';
  return idx;
}

void pn_fprint_data(FILE *stream, const char *bytes, size_t size)
{
  size_t capacity = 4*size + 1;
  char buf[capacity];
  pn_quote_data(buf, capacity, bytes, size);
  fputs(buf, stream);
}

void pn_print_data(const char *bytes, size_t size)
{
  pn_fprint_data(stdout, bytes, size);
}

void parse_url(char *url, char **user, char **pass, char **host, char **port)
{
  if (url) {
    char *at = index(url, '@');
    char *hp;
    if (at) {
      *at = '\0';
      *user = url;
      char *colon = index(url, ':');
      if (colon) {
        *colon = '\0';
        *pass = colon + 1;
      }
      hp = at + 1;
    } else {
      hp = url;
    }

    *host = hp;

    char *colon = index(hp, ':');
    if (colon) {
      *colon = '\0';
      *port = colon + 1;
    }
  }
}

void pn_vfatal(char *fmt, va_list ap)
{
  vfprintf(stderr, fmt, ap);
  exit(EXIT_FAILURE);
}

void pn_fatal(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  pn_vfatal(fmt, ap);
  va_end(ap);
}

bool pn_env_bool(const char *name)
{
  char *v = getenv(name);
  return v && (!strcasecmp(v, "true") || !strcasecmp(v, "1") ||
               !strcasecmp(v, "yes"));
}
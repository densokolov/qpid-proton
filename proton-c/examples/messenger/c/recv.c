/*
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

#include "proton/message.h"
#include "proton/messenger.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "timestamp.h"

#define check(messenger)                                         \
  {                                                              \
    if(pn_messenger_errno(messenger))                            \
    {                                                            \
      die(__FILE__, __LINE__, pn_messenger_error(messenger));    \
    }                                                            \
  }                                                              \

void die(const char *file, int line, const char *message)
{
  fprintf(stderr, "%s:%i: %s\n", file, line, message);
  exit(1);
}

int timeoutdefault = 0;
int recvdefault = 1024;

void usage()
{
  printf("Usage: recv [options] <addr>\n");
  printf("-c    \tPath to the certificate file.\n");
  printf("-k    \tPath to the private key file.\n");
  printf("-p    \tPassword for the private key.\n");
  printf("-T     \tmessenger timeout [%dms]\n", timeoutdefault);
  printf("-R     \tnumber of messages to recv per one put/send [%d]\n", recvdefault);
  printf("<addr>\tAn address.\n");
  exit(0);
}

int main(int argc, char** argv)
{
  char* certificate = NULL;
  char* privatekey = NULL;
  char* password = NULL;
  char* address = "amqp://~0.0.0.0";
  int c;
  long timeout_amt = timeoutdefault;
  long recv_amt = recvdefault;
  char *interr;
  char name[100];
  snprintf(name, sizeof(name), "recv-messenger-%d", getpid());

  opterr = 0;

  while((c = getopt(argc, argv, "hc:k:p:T:R:")) != -1)
  {
    switch(c)
    {
    case 'h':
      usage();
      break;

    case 'c': certificate = optarg; break;
    case 'k': privatekey = optarg; break;
    case 'p': password = optarg; break;
    case 'T':
      timeout_amt = strtol(optarg, &interr, 10);
      if (*interr)
        usage();
      break;
    case 'R':
      recv_amt = strtol(optarg, &interr, 10);
      if (*interr)
        usage();
      break;
    case '?':
      if(optopt == 'c' ||
         optopt == 'k' ||
         optopt == 'p')
      {
        fprintf(stderr, "Option -%c requires an argument.\n", optopt);
      }
      else if(isprint(optopt))
      {
        fprintf(stderr, "Unknown option `-%c'.\n", optopt);
      }
      else
      {
        fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
      }
      return 1;
    default:
      abort();
    }
  }

  if((argc - optind) > 0)
  {
    address = argv[argc - optind];
  }

  pn_message_t * message;
  pn_messenger_t * messenger;

  message = pn_message();
  messenger = pn_messenger("recv-messenger");

  /* load the various command line options if they're set */
  if(certificate)
  {
    pn_messenger_set_certificate(messenger, certificate);
  }

  if(privatekey)
  {
    pn_messenger_set_private_key(messenger, privatekey);
  }

  if(password)
  {
    pn_messenger_set_password(messenger, password);
  }

  pn_messenger_start(messenger);
  check(messenger);

  pn_messenger_subscribe(messenger, address);
  if ( timeout_amt )
    pn_messenger_set_timeout(messenger, timeout_amt);
  check(messenger);

  for(;;)
  {
    pn_messenger_recv(messenger, recv_amt);
    check(messenger);
    printf("."); fflush(stdout);

    while(pn_messenger_incoming(messenger))
    {
      pn_messenger_get(messenger, message);
      check(messenger);

      size_t buffsize = 1024;
      char buffer[buffsize];
      char timebuf[100];
      pn_data_t *body = pn_message_body(message);
      pn_data_format(body, buffer, &buffsize);
      printf("\n%s\n", tv_now(timebuf));
      printf("    Address: %s\n", pn_message_get_address(message));
      const char* subject = pn_message_get_subject(message);
      printf("    Subject: %s\n", subject ? subject : "(no subject)");
      printf("    Content: %s\n", buffer);
      pn_messenger_send(messenger);
    }
  }

  return 0;
}

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
#include <string.h>
#include <ctype.h>
#include <time.h>

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

char subjdefault[100];
int sleepdefault = 0;
int waitdefault = 0;
int timeoutdefault = 0;
int recvdefault = 1024;

void usage()
{
  printf("Usage: send [options] [message] [message] ...\n");
  printf("-a     \tThe target address [amqp[s]://domain[/name]]\n");
  printf("-s     \tsubject [%s]\n", subjdefault);
  printf("-S     \ttime to wait between messages [%ds]\n", sleepdefault);
  printf("-W     \ttime to wait before ending [%ds]\n", waitdefault);
  printf("-T     \tmessenger timeout [%dms]\n", timeoutdefault);
  printf("-R     \tnumber of messages to recv per one put/send [%d]\n", recvdefault);
  printf("message\tA text string to send.\n");
  exit(0);
}

int main(int argc, char** argv)
{
  int c;
  opterr = 0;
  char * address = "amqp://0.0.0.0";
  static char *msgdefault[] = {"Hello World!", "Hello World, part 2!",
                               "Oh no! More Hello World!", 0};
  char **msgtext = msgdefault;

  char *subject = subjdefault;
  snprintf(subjdefault, sizeof(subjdefault), "Greetings from send %d", getpid());
  long sleep_amt = sleepdefault;
  long wait_amt = waitdefault;
  long timeout_amt = timeoutdefault;
  long recv_amt = recvdefault;
  char *interr;
  char name[100];
  snprintf(name, sizeof(name), "send-messenger-%d", getpid());

  while((c = getopt(argc, argv, "ha:b:c:s:S:W:T:R:")) != -1)
  {
    switch(c)
    {
    case 'a': address = optarg; break;
    case 's': subject = optarg; break;
    case 'S':
      sleep_amt = strtol(optarg, &interr, 10);
      if (*interr)
        usage();
      break;
    case 'W':
      wait_amt = strtol(optarg, &interr, 10);
      if (*interr)
        usage();
      break;
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
    case 'h': usage(); break;

    case '?':
      if(optopt == 'a')
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

  if((argc - optind) > 0) msgtext = argv + optind;

  pn_message_t * message;
  pn_messenger_t * messenger;
  char timebuf[100];

  message = pn_message();
  messenger = pn_messenger(name);

  pn_messenger_start(messenger);
  if ( timeout_amt )
    pn_messenger_set_timeout(messenger, timeout_amt);
  while ( *msgtext ) {
    pn_message_set_address(message, address);
    pn_message_set_subject(message, subject);
    pn_data_t *body = pn_message_body(message);
    pn_data_put_string(body, pn_bytes(strlen(*msgtext), *msgtext));
    pn_messenger_put(messenger, message);
    check(messenger);
    printf("\n%s Sending %d %s\n", tv_now(timebuf), getpid(), *msgtext);
    int end = time(0) + sleep_amt;
    do {
      pn_messenger_send(messenger);
      check(messenger);
      if ( recv_amt )
        pn_messenger_recv(messenger, recv_amt);
      check(messenger);
    } while ( end > time(0) );
    ++msgtext;
  }

  if ( wait_amt )
    sleep(wait_amt);
  
  pn_messenger_stop(messenger);
  pn_messenger_free(messenger);
  pn_message_free(message);

  return 0;
}

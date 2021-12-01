/*
  Copyright (c) 2021, Zack Phillips
  Copyright (c) 2018, Zachary Phillips (UC Berkeley)
  All rights reserved.

  BSD 3-Clause License

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
      Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
      Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
      Neither the name of the UC Berkley nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL ZACHARY PHILLIPS (UC BERKELEY) BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA , OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROUTING_H
#define ROUTING_H

#include "illuminate.h"
#include <inttypes.h>
#include <stdio.h>

#define DELIMETER '.'

// Need to declare command router since it is used by the command_item struct;
class CommandRouter;

typedef struct command_item {
  const char *name;
  const char *description;
  const char *syntax;
  int (*func)(CommandRouter *cmd, int argc, const char **argv);
} command_item_t;

class CommandRouter {
public:
  int init(command_item_t *commands, int buffer_size, int argv_max);
  int init_no_malloc(command_item_t *commands, int buffer_size,
                     char *serial_buffer, int argv_max,
                     const char **argv_buffer);
  int help(const char *command_name = nullptr);
  int processSerialStream();
  void cleanup();
  ~CommandRouter();

  char *buffer = nullptr; // Allow for terminating null byte
  int buffer_size = 0;

private:
  int route(int argc, const char **argv);

  const char **argv;
  int argv_max = 0;

  bool malloc_used = false;
  command_item_t *command_list;
};

extern CommandRouter cmd;

int command_help_func(CommandRouter *cmd, int argc, const char **argv);

#endif

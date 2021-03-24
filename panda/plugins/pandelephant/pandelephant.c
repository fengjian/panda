/* PANDABEGINCOMMENT
 * 
 * Authors:
 *  Andrew Fasano               andrew.fasano@ll.mit.edu
 * 
 * This work is licensed under the terms of the GNU GPL, version 2. 
 * See the COPYING file in the top-level directory. 
 * 
PANDAENDCOMMENT */

#include "panda/plugin.h"

bool init_plugin(void *);
void uninit_plugin(void *);

bool init_plugin(void *self) {
    // Required plugins for PANDelephant
    panda_require("asidstory");
    panda_require("syscalls_logger");

    // Configurable plugins - load is specified by plugin options
    // (None for now)
    return true;
}

void uninit_plugin(void *self) { }

/* PANDABEGINCOMMENT
 * 
 * Authors:
 *  Andrew Fasano          fasano@mit.edu
 * 
 * This work is licensed under the terms of the GNU GPL, version 2. 
 * See the COPYING file in the top-level directory. 
 * 
PANDAENDCOMMENT */
// This needs to be defined before anything is included in order to get
// the PRIx64 macro
#define __STDC_FORMAT_MACROS

#include "panda/plugin.h"
#include "panda/tcg-utils.h"

// These need to be extern "C" so that the ABI is compatible with
// QEMU/PANDA, which is written in C
extern "C" {
bool init_plugin(void *);
void uninit_plugin(void *);
#include "forcedexec_int_fns.h"
}

// Handle to self
void* self = NULL;
panda_cb tcg_cb;


//void after_block_exec(CPUState *env, TranslationBlock *tb, uint8_t exitCode) { }

void tcg_parse(CPUState *env, TranslationBlock *tb) {
  // After we generated a TCG block but before it's lowered to host ISA
  // Check if last insn is a conditional branch
  // If so - flip it (TODO: only do this once, or sometimes)
  

  if (panda_in_kernel(env) || tb->pc > 0xc0000000) {
    return;
  }
  // Iterate oi until it's last instruction in block
  TCGOp *op;
  int oi;
  TCGContext *s = &tcg_ctx; // global tcg context

  // For each branch in the block - consider flipping it
  for (oi = s->gen_op_buf[0].next; oi != 0; oi = op->next) {
        op = &s->gen_op_buf[oi];

        TCGOpcode c = op->opc;
        const TCGOpDef *def = &tcg_op_defs[c];
        TCGArg * args = &s->gen_opparam_buf[op->args];

        // Flip 1/10 branches
        if ((rand() % 10) < 8) {
          return;
        }

        switch(c) {
            case INDEX_op_brcond_i32:
            case INDEX_op_brcond_i64:
            case INDEX_op_brcond2_i32:
                printf("Flip conditional branch in block at 0x%x: %s\n", tb->pc, def->name);
                // All the brcond cases in tcg/ppc/tcg-target-inc.c show args[2] contains arg
                // same with tcg/arm/tcg-target.inc.c
                args[2] = tcg_invert_cond((TCGCond) args[2]);
                break;

            default:
                break;
        }
  };
}

void enable_forcedexec() {
    assert(self != NULL);
    panda_enable_callback(self, PANDA_CB_BEFORE_TCG_CODEGEN, tcg_cb);
}

void disable_forcedexec() {
    assert(self != NULL);
    panda_disable_callback(self, PANDA_CB_BEFORE_TCG_CODEGEN, tcg_cb);
}


bool init_plugin(void *_self) {
    self = _self;
    tcg_cb.before_tcg_codegen = tcg_parse;
    panda_register_callback(self, PANDA_CB_BEFORE_TCG_CODEGEN, tcg_cb);
    disable_forcedexec();

    return true;
}

void uninit_plugin(void *self) { }

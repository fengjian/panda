/* PANDABEGINCOMMENT
 * 
 * Authors:
 *  Andrew Fasano               fasano@mit.edu
 * 
 * This work is licensed under the terms of the GNU GPL, version 2. 
 * See the COPYING file in the top-level directory. 
 * 
PANDAENDCOMMENT */
// This needs to be defined before anything is included in order to get
// the PRIx64 macro
#define __STDC_FORMAT_MACROS

#include "panda/plugin.h"

#ifdef TARGET_I386
#ifdef TARGET_X86_64
static const char * const regnames[] = {
  "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
};
#else
static const char * const regnames[] = {
  "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi",
  "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d"
};
#endif //i386
#endif


// These need to be extern "C" so that the ABI is compatible with
// QEMU/PANDA, which is written in C
extern "C" {
  bool init_plugin(void *);
  void uninit_plugin(void *);
  void mem_write_callback(CPUState *env, target_ulong pc, target_ulong addr, size_t size, uint8_t *buf);

  void mem_read_callback(CPUState *env, target_ulong pc, target_ulong addr, size_t size, uint8_t *buf);
  int exec_callback(CPUState *cpu, target_ulong pc);
}

target_ulong *lastregs;
target_ulong lastpc = 0;

FILE *result_log;


bool translate_callback(CPUState* cpu, target_ulong pc){
    // always call exec_callback
  return true;
}

int exec_callback(CPUState *cpu, target_ulong pc) {
    // Report register delta
    if (panda_current_pc(cpu) != lastpc) {
        if (lastpc) fprintf(result_log, "\n");
        fprintf(result_log, "0x" TARGET_FMT_lx ",0x%x,", panda_current_asid(cpu), panda_in_kernel(cpu));
        lastpc = panda_current_pc(cpu);
    }
    
#ifdef TARGET_I386
    CPUArchState *env = (CPUArchState*)cpu->env_ptr;

    for (int reg_idx=0; reg_idx < CPU_NB_REGS; reg_idx++) {
        if (lastregs[reg_idx] != env->regs[reg_idx]) {
            // Report delta and update
            lastregs[reg_idx] = env->regs[reg_idx];
            fprintf(result_log, "%s=0x" TARGET_FMT_lx ",", regnames[reg_idx], lastregs[reg_idx]);
        }
    }
#endif
    return 0;
}

static void mem_callback(CPUState *cpu, target_ulong pc, target_ulong addr,
                         size_t size, uint8_t *buf,
                         bool is_write) {
    if (panda_current_pc(cpu) != lastpc) {
        if (lastpc) fprintf(result_log, "\n");
        lastpc = panda_current_pc(cpu);
        fprintf(result_log, "0x" TARGET_FMT_lx ",0x%x,", panda_current_asid(cpu), panda_in_kernel(cpu));
    }
    if (is_write) {
        fprintf(result_log, "mw=0x" TARGET_FMT_lx ":0x", addr);
    }else{
        fprintf(result_log, "mr=0x" TARGET_FMT_lx ":0x", addr);
    }

    for (unsigned int i = 0; i < size; i++) {
        uint8_t val = ((uint8_t *)buf)[i];
        fprintf(result_log, "%x", val);
    }
    fprintf(result_log, ",");

    return;
}
void mem_read_callback(CPUState *cpu, target_ulong pc, target_ulong addr,
                       size_t size, uint8_t *buf) {
    mem_callback(cpu, pc, addr, size, buf, false);
    return;
}

void mem_write_callback(CPUState *cpu, target_ulong pc, target_ulong addr,
                        size_t size, uint8_t *buf) {
    mem_callback(cpu, pc, addr, size, buf, true);
    return;
}

bool init_plugin(void *self) {
    panda_cb pcb;
    const char* filename;
    panda_arg_list *args = panda_get_args("trace");
    filename = panda_parse_string_opt(args, "log", "trace.txt", "filename of the trace");
    result_log = fopen(filename, "w");

    if (!result_log) {
        printf("Couldn't open result_log\n");
        perror("fopen");
        return false;
    }


    pcb.insn_translate = translate_callback;
    panda_register_callback(self, PANDA_CB_INSN_TRANSLATE, pcb);

    pcb.insn_exec = exec_callback;
    panda_register_callback(self, PANDA_CB_INSN_EXEC, pcb);

    pcb.virt_mem_after_read = mem_read_callback;
    panda_register_callback(self, PANDA_CB_VIRT_MEM_AFTER_READ, pcb);

    pcb.virt_mem_after_write = mem_write_callback;
    panda_register_callback(self, PANDA_CB_VIRT_MEM_AFTER_WRITE, pcb);

    // Need this to get precise PC within basic blocks
    panda_enable_precise_pc();
    // Enable memory result_logging
    panda_enable_memcb();

    // Allocate last regs obj
#ifdef TARGET_I386
    // x86 or x86_64
    lastregs = (target_ulong*)malloc(CPU_NB_REGS*sizeof(target_ulong));
#else
    printf("Unsupported architecture\n");
    return false;
#endif
    return true;
}

void uninit_plugin(void *self) {
    fclose(result_log);
}

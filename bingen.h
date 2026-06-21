#ifndef BINGEN_H_
#define BINGEN_H_

#define UNIMPL(name, fail)                                              \
  do {                                                                  \
    printf("%s:%d: " name " is unimplemented!\n", __FILE__, __LINE__);  \
    return fail;                                                        \
  } while (0)

#include "registers.h"

#include <stddef.h>
#include <elf.h>

typedef enum {
  PATCH_TYPE_ADDRESS = 0,
  PATCH_TYPE_VIRTUAL,
  PATCH_TYPES,
} PatchType;

typedef struct {
  PatchType type;
  size_t *patch;
  size_t offset;
  size_t size;
  size_t index;
} Patch;

typedef struct {
  Patch *items;
  size_t count;
  size_t capacity;
} Patches;

typedef struct {
  size_t fni;
  size_t index;
  size_t addr;
} Address;

typedef struct {
  Address *items;
  size_t count;
  size_t capacity;
} Addresses;

typedef struct {
  Longs *longs; // Numbers too big to be passed directly to registers.
  Longs *longs_locations;
  struct {
    size_t r; // current read size in bytes
    size_t mask;
    size_t shift;
  };
  size_t tape_size;
  size_t arg_tape_size;
  size_t ret_tape_size;
  size_t global_var_tape_size;
  size_t local_var_tape_size;
  Tokenizer *t;
  Function *fn;
  Operation *op;
  Functions *fns;
  size_t fni; // current function index
  Vars *globals;
  String *bytes;
  Patches *patches;

  size_t tape_start;
  size_t args_start;
  size_t rets_start;
  size_t globals_start;
  size_t locals_start;

  size_t tape_head_location;
  size_t args_head_location;
  size_t rets_head_location;
  size_t locals_head_location;

  size_t true_location;

  Longs *fn_addrs;
  Addresses *addrs;
} BinaryGenerator;

TARE_DEF bool gen_elf(const char *output, BinaryGenerator *b);

// ************************ Headers in ELF file ************************
TARE_DEF Elf64_Ehdr gen_elf_header(void);
TARE_DEF Elf64_Phdr gen_elf_tapes_and_heads_program_header(size_t total_size);
TARE_DEF Elf64_Phdr gen_elf_entry_program_header(void);
TARE_DEF Elf64_Phdr gen_elf_constants_program_header(void);
TARE_DEF void patch_tapes_and_heads_program_header(Elf64_Phdr *entry, Elf64_Phdr *tapes_and_heads, Elf64_Phdr *constants, size_t byte_count, size_t longs_count);

// *************************** Instructions ****************************
TARE_DEF bool gen_mov_r64_imm32(BinaryGenerator *b, Reg64 r64, size_t imm32);
TARE_DEF bool gen_mov_r64_imm16(BinaryGenerator *b, Reg64 r64, size_t imm16);
TARE_DEF bool gen_mov_r64_imm8(BinaryGenerator *b, Reg64 r64, size_t imm8);

TARE_DEF bool gen_mov_rbp_r64(BinaryGenerator *b, Reg64 r64);
TARE_DEF bool gen_mov_rsp_r64(BinaryGenerator *b, Reg64 r64);

TARE_DEF bool gen_mov_cl_r8(BinaryGenerator *b, Reg8 r8);

TARE_DEF bool gen_cmovl_r64_qword_deref_m64_or_append_patch(BinaryGenerator *b, Reg64 r64, size_t *m64);
TARE_DEF bool gen_cmovle_r64_qword_deref_m64_or_append_patch(BinaryGenerator *b, Reg64 r64, size_t *m64);
TARE_DEF bool gen_cmovg_r64_qword_deref_m64_or_append_patch(BinaryGenerator *b, Reg64 r64, size_t *m64);
TARE_DEF bool gen_cmovge_r64_qword_deref_m64_or_append_patch(BinaryGenerator *b, Reg64 r64, size_t *m64);
TARE_DEF bool gen_cmove_r64_qword_deref_m64_or_append_patch(BinaryGenerator *b, Reg64 r64, size_t *m64);
TARE_DEF bool gen_cmovne_r64_qword_deref_m64_or_append_patch(BinaryGenerator *b, Reg64 r64, size_t *m64);

TARE_DEF bool gen_mov_rax_r64(BinaryGenerator *b, Reg64 r64);
TARE_DEF bool gen_mov_rcx_r64(BinaryGenerator *b, Reg64 r64);

TARE_DEF bool gen_mov_r64_qword_deref_rax(BinaryGenerator *b, Reg64 r64);
TARE_DEF bool gen_mov_r64_qword_deref_m64_or_append_patch(BinaryGenerator *b, Reg64 r64, size_t *m64);
TARE_DEF bool gen_mov_r64_m64_or_append_patch(BinaryGenerator *b, Reg64 r64, size_t *m64);
TARE_DEF bool gen_mov_qword_deref_m64_m64_or_append_patches(BinaryGenerator *b, size_t *m64_1, size_t *m64_2);

TARE_DEF bool gen_mov_qword_deref_rax_r64(BinaryGenerator *b, Reg64 r64);
TARE_DEF bool gen_mov_dword_deref_rax_r32(BinaryGenerator *b, Reg32 r32);
TARE_DEF bool gen_mov_word_deref_rax_r16(BinaryGenerator *b, Reg16 r16);
TARE_DEF bool gen_mov_byte_deref_rax_r8(BinaryGenerator *b, Reg8 r8);

TARE_DEF bool gen_mov_rbx_qword_deref_r64(BinaryGenerator *b, Reg64 r64);
TARE_DEF bool gen_mov_ebx_dword_deref_r64(BinaryGenerator *b, Reg64 r64);
TARE_DEF bool gen_mov_bx_word_deref_r64(BinaryGenerator *b, Reg64 r64);
TARE_DEF bool gen_mov_bl_byte_deref_r64(BinaryGenerator *b, Reg64 r64);

TARE_DEF bool gen_mov_qword_deref_r64_imm32(BinaryGenerator *b, Reg64 r64, size_t imm32);
TARE_DEF bool gen_mov_dword_deref_r64_imm32(BinaryGenerator *b, Reg64 r64, size_t imm32);
TARE_DEF bool gen_mov_word_deref_r64_imm16(BinaryGenerator *b, Reg64 r64, size_t imm16);
TARE_DEF bool gen_mov_byte_deref_r64_imm8(BinaryGenerator *b, Reg64 r64, size_t imm8);

TARE_DEF bool gen_add_qword_deref_m64_r64_or_append_patch(BinaryGenerator *b, size_t *m64, Reg64 r64);
TARE_DEF bool gen_sub_qword_deref_m64_r64_or_append_patch(BinaryGenerator *b, size_t *m64, Reg64 r64);

TARE_DEF bool gen_add_qword_deref_m64_imm32_or_append_patch(BinaryGenerator *b, size_t *m64, size_t imm32);
TARE_DEF bool gen_sub_qword_deref_m64_imm32_or_append_patch(BinaryGenerator *b, size_t *m64, size_t imm32);

TARE_DEF bool gen_add_rax_r64(BinaryGenerator *b, Reg64 r64);
TARE_DEF bool gen_sub_rax_r64(BinaryGenerator *b, Reg64 r64);
TARE_DEF bool gen_mul_r64(BinaryGenerator *b, Reg64 r64);
TARE_DEF bool gen_div_r64(BinaryGenerator *b, Reg64 r64);

TARE_DEF bool gen_add_r64_imm32(BinaryGenerator *b, Reg64 r64, size_t imm32);
TARE_DEF bool gen_sub_r64_imm32(BinaryGenerator *b, Reg64 r64, size_t imm32);

TARE_DEF bool gen_sub_r64_m64_or_append_patch(BinaryGenerator *b, Reg64 r64, size_t *m64);

TARE_DEF bool gen_add_qword_deref_rbx_r64(BinaryGenerator *b, Reg64 r64);
TARE_DEF bool gen_add_dword_deref_rbx_r32(BinaryGenerator *b, Reg32 r32);
TARE_DEF bool gen_add_word_deref_rbx_r16(BinaryGenerator *b, Reg16 r16);
TARE_DEF bool gen_add_byte_deref_rbx_r8(BinaryGenerator *b, Reg8 r8);

TARE_DEF bool gen_sub_qword_deref_rbx_r64(BinaryGenerator *b, Reg64 r64);
TARE_DEF bool gen_sub_dword_deref_rbx_r32(BinaryGenerator *b, Reg32 r32);
TARE_DEF bool gen_sub_word_deref_rbx_r16(BinaryGenerator *b, Reg16 r16);
TARE_DEF bool gen_sub_byte_deref_rbx_r8(BinaryGenerator *b, Reg8 r8);

TARE_DEF bool gen_shl_r64_imm8(BinaryGenerator *b, Reg64 r64, size_t imm8);
TARE_DEF bool gen_shl_r64_1(BinaryGenerator *b, Reg64 r64);

TARE_DEF bool gen_shl_r64_cl(BinaryGenerator *b, Reg64 r64);
TARE_DEF bool gen_shr_r64_cl(BinaryGenerator *b, Reg64 r64);

TARE_DEF bool gen_and_rax_r64(BinaryGenerator *b, Reg64 r64);
TARE_DEF bool gen_or_rax_r64(BinaryGenerator *b, Reg64 r64);

TARE_DEF bool gen_or_r64_imm32(BinaryGenerator *b, Reg64 r64, size_t imm32);

TARE_DEF bool gen_xor_r64_self(BinaryGenerator *b, Reg64 r64);

TARE_DEF bool gen_pop_r64(BinaryGenerator *b, Reg64 r64);
TARE_DEF bool gen_push_r64(BinaryGenerator *b, Reg64 r64);
TARE_DEF bool gen_push_qword_deref_r64(BinaryGenerator *b, Reg64 r64);

TARE_DEF bool gen_cmp_rax_r64(BinaryGenerator *b, Reg64 r64);
TARE_DEF bool gen_cmp_rbx_r64(BinaryGenerator *b, Reg64 r64);

TARE_DEF bool gen_cmp_r64_imm32(BinaryGenerator *b, Reg64 r64, size_t imm32);
TARE_DEF bool gen_cmp_r32_imm32(BinaryGenerator *b, Reg32 r32, size_t imm32);
TARE_DEF bool gen_cmp_r16_imm16(BinaryGenerator *b, Reg16 r16, size_t imm16);
TARE_DEF bool gen_cmp_r8_imm8(BinaryGenerator *b, Reg8 r8, size_t imm8);

TARE_DEF void gen_syscall(BinaryGenerator *b);

TARE_DEF void gen_ret(BinaryGenerator *b);

TARE_DEF bool gen_jmp_or_append_patch(BinaryGenerator *b, size_t *addr);
TARE_DEF bool gen_jz_or_append_patch(BinaryGenerator *b, size_t *addr);

TARE_DEF bool gen_call_or_append_patch(BinaryGenerator *b, size_t *func);

// **************************** Operations *****************************
TARE_DEF bool gen_funcall_bin(BinaryGenerator *b, size_t fid);
TARE_DEF bool gen_func_binary(BinaryGenerator *b);
TARE_DEF bool gen_op_binary(BinaryGenerator *b);

TARE_DEF bool gen_ptr_add_op_bin(BinaryGenerator *b);
TARE_DEF bool gen_ptr_sub_op_bin(BinaryGenerator *b);
TARE_DEF bool gen_elem_add_op_bin(BinaryGenerator *b);
TARE_DEF bool gen_elem_sub_op_bin(BinaryGenerator *b);

TARE_DEF bool gen_read_size_op_bin(BinaryGenerator *b, size_t r);
TARE_DEF bool gen_conditional_op_bin(BinaryGenerator *b);
TARE_DEF bool gen_goto_op_bin(BinaryGenerator *b);
TARE_DEF bool gen_funcall_op_bin(BinaryGenerator *b);
TARE_DEF bool gen_ret_op_bin(BinaryGenerator *b);
TARE_DEF bool gen_write_op_bin(BinaryGenerator *b);
TARE_DEF bool gen_read_op_bin(BinaryGenerator *b);
TARE_DEF bool gen_syscall_op_bin(BinaryGenerator *b);
  
TARE_DEF bool gen_tape_op_bin(BinaryGenerator *b);
TARE_DEF bool gen_head_op_bin(BinaryGenerator *b);
TARE_DEF bool gen_base_op_bin(BinaryGenerator *b);
TARE_DEF bool gen_index_op_bin(BinaryGenerator *b);
TARE_DEF bool gen_length_op_bin(BinaryGenerator *b);
    
TARE_DEF bool gen_push_op_bin(BinaryGenerator *b);
TARE_DEF bool gen_pop_op_bin(BinaryGenerator *b);

TARE_DEF bool gen_add_op_bin(BinaryGenerator *b);
TARE_DEF bool gen_sub_op_bin(BinaryGenerator *b);
TARE_DEF bool gen_mul_op_bin(BinaryGenerator *b);
TARE_DEF bool gen_div_op_bin(BinaryGenerator *b);
TARE_DEF bool gen_mod_op_bin(BinaryGenerator *b);
TARE_DEF bool gen_shl_op_bin(BinaryGenerator *b);
TARE_DEF bool gen_shr_op_bin(BinaryGenerator *b);
TARE_DEF bool gen_not_op_bin(BinaryGenerator *b);
    
TARE_DEF bool gen_comparison_op_bin(BinaryGenerator *b);
TARE_DEF bool gen_bitwise_bin(BinaryGenerator *b);
TARE_DEF bool gen_logical_bin(BinaryGenerator *b);

TARE_DEF bool gen_deref_op_bin(BinaryGenerator *b);

TARE_DEF bool gen_num_op_bin(BinaryGenerator *b);

TARE_DEF bool gen_op_pop_from_ops_bin(BinaryGenerator *b);

TARE_DEF bool gen_assign_op_bin(BinaryGenerator *b);

TARE_DEF bool gen_gvid_op_bin(BinaryGenerator *b);
TARE_DEF bool gen_lvid_op_bin(BinaryGenerator *b);
TARE_DEF bool gen_rvid_op_bin(BinaryGenerator *b);
TARE_DEF bool gen_avid_op_bin(BinaryGenerator *b);

TARE_DEF void append_address(BinaryGenerator *b, size_t fni, size_t index);

TARE_DEF size_t *find_long_address(BinaryGenerator *b, size_t op);
TARE_DEF size_t *find_addr(BinaryGenerator *b, size_t fni, size_t index);

// ***************************** Patching ******************************
TARE_DEF void patch_addr(BinaryGenerator *b);
TARE_DEF void patch_binary(BinaryGenerator *b);

#endif // BINGEN_H_
#ifdef BINGEN_IMPLEMENTATION

TARE_DEF bool gen_elf(const char *output, BinaryGenerator *b) {
  if (output == NULL || b == NULL) return false;

  FILE *f = fopen(output, "wb");
  Elf64_Ehdr elfh = gen_elf_header();

  // entry.p_filesz
  // entry.p_memsz
  // both need to be patched
  Elf64_Phdr entry = gen_elf_entry_program_header();

  // p_offset
  // p_vaddr
  // p_paddr
  // all five need to be patched
  size_t total_size = b->tape_size + b->arg_tape_size
    + b->ret_tape_size + b->global_var_tape_size + b->local_var_tape_size
    + 4 * 8; // 4 haeds
  Elf64_Phdr tapes_and_heads =
    gen_elf_tapes_and_heads_program_header(total_size);

  // p_offset
  // p_vaddr
  // p_paddr
  // p_filesz - 8 by default, for the size of `true`
  // p_memsz  - 8 by default, for the size of `true`
  // all five need to be patched
  Elf64_Phdr constants = gen_elf_constants_program_header();

  for (size_t i = 0; i < b->fns->count; i++) da_append(b->fn_addrs, 0);

  for (size_t i = 0; i < b->fns->count; i++) {
    Function *fn = b->fns->items + i;
    for (size_t j = 0; j < fn->count; j++) {
      Operation *op = fn->items + j;
      if (op->type == OP_ADDRESS) {
        append_address(b, i, op->op);
      } else if (op->type == OP_NUM) {
        if (!(op->op < ((R32_MAX) >> 1))) {
          da_append(b->longs, op->op);
          da_append(b->longs_locations, 0);
        }
      } else continue;
    }
  }

  if (!gen_mov_qword_deref_m64_m64_or_append_patches(b, &b->tape_head_location, &b->tape_start)) return false;
  if (!gen_mov_qword_deref_m64_m64_or_append_patches(b, &b->args_head_location, &b->args_start)) return false;
  if (!gen_mov_qword_deref_m64_m64_or_append_patches(b, &b->rets_head_location, &b->rets_start)) return false;
  if (!gen_mov_qword_deref_m64_m64_or_append_patches(b, &b->locals_head_location, &b->locals_start)) return false;

  if (!gen_funcall_bin(b, 0)) return false;
  if (b->fns->items[0].rets.count == 0) {
    if (!gen_mov_r64_imm32(b, RDI, 0)) return false;
  } else {
    if (!gen_pop_r64(b, RDI)) return false;
  }
  if (!gen_mov_r64_imm32(b, RAX, 60)) return false;
  gen_syscall(b);

  b->r = 8;
  b->mask = R64_MAX;
  b->shift = 3;

  for (size_t i = 0; i < b->fns->count; i++) {
    b->fni = i;
    b->fn = b->fns->items + i;
    if (!gen_func_binary(b)) return false;
  }

  b->tape_start = b->bytes->count + 0x1000;
  b->args_start = b->tape_start + b->tape_size;
  b->rets_start = b->args_start + b->arg_tape_size;
  b->globals_start = b->rets_start + b->ret_tape_size;
  b->locals_start = b->globals_start + b->global_var_tape_size;
  b->tape_head_location = b->locals_start + b->local_var_tape_size;
  b->args_head_location = b->tape_head_location + 8;
  b->rets_head_location = b->args_head_location + 8;
  b->locals_head_location = b->rets_head_location + 8;

  size_t first_longs_location = b->locals_head_location + 0x1000 + 8;
  for (size_t i = 0; i < b->longs_locations->count; i++) {
    b->longs_locations->items[i] = first_longs_location + i * 8;
  }

  b->true_location = first_longs_location + b->longs_locations->count * 8;
  patch_binary(b);
  patch_tapes_and_heads_program_header(&entry, &tapes_and_heads, &constants, b->bytes->count, b->longs->count);

  size_t count = fwrite(&elfh, 1, sizeof(elfh), f);
  if (count != sizeof(elfh)) {
    perror("fwrite");
    return false;
  }

  count = fwrite(&entry, 1, sizeof(entry), f);
  if (count != sizeof(entry)) {
    perror("fwrite");
    return false;
  }

  count = fwrite(&tapes_and_heads, 1, sizeof(tapes_and_heads), f);
  if (count != sizeof(tapes_and_heads)) {
    perror("fwrite");
    return false;
  }

  count = fwrite(&constants, 1, sizeof(constants), f);
  if (count != sizeof(constants)) {
    perror("fwrite");
    return false;
  }

  count = fwrite(b->bytes->items, 1, b->bytes->count, f);
  if (count != b->bytes->count) {
    perror("fwrite");
    return false;
  }

  char *empty = calloc(tapes_and_heads.p_filesz, 1);
  count = fwrite(empty, 1, tapes_and_heads.p_filesz, f);
  if (count != tapes_and_heads.p_filesz) {
    if (empty) free(empty);
    perror("fwrite");
    return false;
  }
  if (empty) free(empty);

  for (size_t i = 0; i < b->longs->count; i++) {
    size_t constant = b->longs->items[i];
    count = fwrite(&constant, 1, 8, f);
    if (count != 8) {
      perror("fwrite");
      return false;
    }
  }

  size_t true_constant = 1;
  count = fwrite(&true_constant, 1, 8, f);
  if (count != 8) {
    perror("fwrite");
    return false;
  }

  if (f) fclose(f);

  printf("Successfully generated file %s\n", output);

  return true;
}

// ************************ Headers in ELF file ************************
TARE_DEF Elf64_Ehdr gen_elf_header(void) {
  return (Elf64_Ehdr) {
    .e_ident = {
      ELFMAG0,
      ELFMAG1,
      ELFMAG2,
      ELFMAG3,
      ELFCLASS64,
      ELFDATA2LSB,
      EV_CURRENT,
      ELFOSABI_LINUX,
      0, 0, 0, 0, 0, 0, 0, 0 // EI_NIDENT
    },                       /* Magic number and other info */
    .e_type = ET_EXEC,       /* Object file type */
    .e_machine = EM_X86_64,  /* Architecture */
    .e_version = EV_CURRENT, /* Object file version */
    .e_entry = 0x4000e8,     /* Entry point virtual address */
    .e_phoff = 64,           /* Program header table file offset */
    .e_shoff = 0,            /* Section header table file offset */
    .e_flags = 0,            /* Processor-specific flags */
    .e_ehsize = 64,          /* ELF header size in bytes */
    .e_phentsize = 56,       /* Program header table entry size */
    .e_phnum = 3,            /* Program header table entry count */
    .e_shentsize = 64,       /* Section header table entry size */
    .e_shnum = 0,            /* Section header table entry count */
    .e_shstrndx = SHN_UNDEF, /* Section header string table index */
  };
}

TARE_DEF Elf64_Phdr gen_elf_entry_program_header(void) {
  return (Elf64_Phdr) {
    .p_type = PT_LOAD,             /* Segment type */
    .p_offset = 0,                 /* Segment file offset */
    .p_vaddr = 0x400000,           /* Segment virtual address */
    .p_paddr = 0x400000,           /* Segment physical address */
    .p_filesz = 0x1bea,            /* Segment size in file */
    .p_memsz = 0x1bea,             /* Segment size in memory */
    .p_flags = PF_X | PF_W | PF_R, /* Segment flags */
    .p_align = 0x1000,             /* Segment alignment */
  };
}

TARE_DEF Elf64_Phdr gen_elf_tapes_and_heads_program_header(size_t total_size) {
  return (Elf64_Phdr) {
    .p_type = PT_LOAD,      /* Segment type */
    .p_offset = 0,          /* Segment file offset */
    .p_vaddr = 0x401000,    /* Segment virtual address */
    .p_paddr = 0x401000,    /* Segment physical address */
    .p_filesz = total_size, /* Segment size in file */
    .p_memsz = total_size,  /* Segment size in memory */
    .p_flags = PF_W | PF_R, /* Segment flags */
    .p_align = 0x1000,      /* Segment alignment */
  };
}

TARE_DEF Elf64_Phdr gen_elf_constants_program_header(void) {
  return (Elf64_Phdr) {
    // size is 8 because that's the minimum, the size of `true`
    .p_type = PT_LOAD,      /* Segment type */
    .p_offset = 0,          /* Segment file offset */
    .p_vaddr = 0x402000,    /* Segment virtual address */
    .p_paddr = 0x402000,    /* Segment physical address */
    .p_filesz = 0x08,       /* Segment size in file */
    .p_memsz = 0x08,        /* Segment size in memory */
    .p_flags = PF_R,        /* Segment flags */
    .p_align = 0x1000,      /* Segment alignment */
  };
}

TARE_DEF void patch_tapes_and_heads_program_header(Elf64_Phdr *entry, Elf64_Phdr *tapes_and_heads, Elf64_Phdr *constants, size_t byte_count, size_t longs_count) {
  // patch entry
  entry->p_filesz = sizeof(Elf64_Phdr) * 3 + sizeof(Elf64_Ehdr) + byte_count;
  entry->p_memsz = entry->p_filesz;

  // patch tapes_and_heads
  tapes_and_heads->p_vaddr += entry->p_filesz;
  tapes_and_heads->p_paddr = tapes_and_heads->p_vaddr;
  tapes_and_heads->p_offset = entry->p_filesz;

  // patch constants
  constants->p_offset =
    tapes_and_heads->p_offset + tapes_and_heads->p_filesz;
  constants->p_vaddr =
    tapes_and_heads->p_vaddr + tapes_and_heads->p_filesz + 0x1000;
  constants->p_paddr = constants->p_vaddr;
  constants->p_filesz += longs_count * 8;
  constants->p_memsz = constants->p_filesz;
}

// *************************** Instructions ****************************
TARE_DEF bool gen_mov_r64_imm32(BinaryGenerator *b, Reg64 r64, size_t imm32) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\xc7\xc0", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\xc7\xc3", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\xc7\xc1", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\xc7\xc2", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\xc7\xc6", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\xc7\xc7", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\xc7\xc5", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\xc7\xc4", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x49\xc7\xc0", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x49\xc7\xc1", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x49\xc7\xc2", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x49\xc7\xc3", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x49\xc7\xc4", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x49\xc7\xc5", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x49\xc7\xc6", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x49\xc7\xc7", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  if (!append_to_string(s, (const char *)&imm32, 4)) return false;
  return true;
}

TARE_DEF bool gen_mov_r64_imm16(BinaryGenerator *b, Reg64 r64, size_t imm16) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\xc7\xc0", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\xc7\xc3", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\xc7\xc1", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\xc7\xc2", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\xc7\xc6", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\xc7\xc7", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\xc7\xc5", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\xc7\xc4", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x49\xc7\xc0", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x49\xc7\xc1", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x49\xc7\xc2", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x49\xc7\xc3", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x49\xc7\xc4", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x49\xc7\xc5", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x49\xc7\xc6", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x49\xc7\xc7", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  if (!append_to_string(s, (const char *)&imm16, 4)) return false;
  return true;
}

TARE_DEF bool gen_mov_r64_imm8(BinaryGenerator *b, Reg64 r64, size_t imm8) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\xc7\xc0", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\xc7\xc3", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\xc7\xc1", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\xc7\xc2", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\xc7\xc6", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\xc7\xc7", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\xc7\xc5", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\xc7\xc4", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x49\xc7\xc0", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x49\xc7\xc1", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x49\xc7\xc2", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x49\xc7\xc3", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x49\xc7\xc4", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x49\xc7\xc5", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x49\xc7\xc6", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x49\xc7\xc7", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  if (!append_to_string(s, (const char *)&imm8, 4)) return false;
  return true;
}

TARE_DEF bool gen_mov_rbp_r64(BinaryGenerator *b, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x89\xc5", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x89\xdd", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x89\xcd", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x89\xd5", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x89\xf5", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x89\xfd", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x89\xed", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x89\xe5", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x4c\x89\xc5", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x4c\x89\xcd", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x4c\x89\xd5", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x4c\x89\xdd", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x4c\x89\xe5", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x4c\x89\xed", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x4c\x89\xf5", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x4c\x89\xfd", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_mov_rsp_r64(BinaryGenerator *b, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x89\xc4", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x89\xdc", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x89\xcc", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x89\xd4", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x89\xf4", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x89\xfc", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x89\xec", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x89\xe4", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x4c\x89\xc4", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x4c\x89\xcc", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x4c\x89\xd4", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x4c\x89\xdc", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x4c\x89\xe4", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x4c\x89\xec", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x4c\x89\xf4", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x4c\x89\xfc", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_mov_cl_r8(BinaryGenerator *b, Reg8 r8) {
  String *s = b->bytes;
  switch (r8) {
  case AL:   if (!append_to_string(s, "\x88\xc1", 2))     return false; break;
  case AH:   if (!append_to_string(s, "\x88\xe1", 2))     return false; break;
  case BL:   if (!append_to_string(s, "\x88\xd9", 2))     return false; break;
  case BH:   if (!append_to_string(s, "\x88\xf9", 2))     return false; break;
  case CL:   if (!append_to_string(s, "\x88\xc9", 2))     return false; break;
  case CH:   if (!append_to_string(s, "\x88\xe9", 2))     return false; break;
  case DL:   if (!append_to_string(s, "\x88\xd1", 2))     return false; break;
  case DH:   if (!append_to_string(s, "\x88\xf1", 2))     return false; break;
  case SIL:  if (!append_to_string(s, "\x40\x88\xf1", 3)) return false; break;
  case DIL:  if (!append_to_string(s, "\x40\x88\xf9", 3)) return false; break;
  case BPL:  if (!append_to_string(s, "\x40\x88\xe9", 3)) return false; break;
  case SPL:  if (!append_to_string(s, "\x40\x88\xe1", 3)) return false; break;
  case R8B:  if (!append_to_string(s, "\x44\x88\xc1", 3)) return false; break;
  case R9B:  if (!append_to_string(s, "\x44\x88\xc9", 3)) return false; break;
  case R10B: if (!append_to_string(s, "\x44\x88\xd1", 3)) return false; break;
  case R11B: if (!append_to_string(s, "\x44\x88\xd9", 3)) return false; break;
  case R12B: if (!append_to_string(s, "\x44\x88\xe1", 3)) return false; break;
  case R13B: if (!append_to_string(s, "\x44\x88\xe9", 3)) return false; break;
  case R14B: if (!append_to_string(s, "\x44\x88\xf1", 3)) return false; break;
  case R15B: if (!append_to_string(s, "\x44\x88\xf9", 3)) return false; break;
  case REG_8_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_cmovl_r64_qword_deref_m64_or_append_patch(BinaryGenerator *b, Reg64 r64, size_t *m64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x0f\x4c\x05", 4)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x0f\x4c\x1d", 4)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x0f\x4c\x0d", 4)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x0f\x4c\x15", 4)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x0f\x4c\x35", 4)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x0f\x4c\x3d", 4)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x0f\x4c\x2d", 4)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x0f\x4c\x25", 4)) return false; break;
  case R8:  if (!append_to_string(s, "\x4c\x0f\x4c\x05", 4)) return false; break;
  case R9:  if (!append_to_string(s, "\x4c\x0f\x4c\x0d", 4)) return false; break;
  case R10: if (!append_to_string(s, "\x4c\x0f\x4c\x15", 4)) return false; break;
  case R11: if (!append_to_string(s, "\x4c\x0f\x4c\x1d", 4)) return false; break;
  case R12: if (!append_to_string(s, "\x4c\x0f\x4c\x25", 4)) return false; break;
  case R13: if (!append_to_string(s, "\x4c\x0f\x4c\x2d", 4)) return false; break;
  case R14: if (!append_to_string(s, "\x4c\x0f\x4c\x35", 4)) return false; break;
  case R15: if (!append_to_string(s, "\x4c\x0f\x4c\x3d", 4)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }

  /* [m64] is translated as : m64 - byte count INCLUDING the instruction */
  Patch patch = {
    .index = s->count,
    .patch = m64,
    // the instruction is 8 bytes, 4 of which are already accounted for
    .offset   = s->count + 4,
    .size     = 4,
    .type     = PATCH_TYPE_ADDRESS,
  };
  if (*m64 == 0) da_append(b->patches, patch);
  if (!append_to_string(s, (const char *)m64, 4)) return false;
  return true;
}

TARE_DEF bool gen_cmovle_r64_qword_deref_m64_or_append_patch(BinaryGenerator *b, Reg64 r64, size_t *m64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x0f\x4e\x05", 4)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x0f\x4e\x1d", 4)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x0f\x4e\x0d", 4)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x0f\x4e\x15", 4)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x0f\x4e\x35", 4)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x0f\x4e\x3d", 4)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x0f\x4e\x2d", 4)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x0f\x4e\x25", 4)) return false; break;
  case R8:  if (!append_to_string(s, "\x4c\x0f\x4e\x05", 4)) return false; break;
  case R9:  if (!append_to_string(s, "\x4c\x0f\x4e\x0d", 4)) return false; break;
  case R10: if (!append_to_string(s, "\x4c\x0f\x4e\x15", 4)) return false; break;
  case R11: if (!append_to_string(s, "\x4c\x0f\x4e\x1d", 4)) return false; break;
  case R12: if (!append_to_string(s, "\x4c\x0f\x4e\x25", 4)) return false; break;
  case R13: if (!append_to_string(s, "\x4c\x0f\x4e\x2d", 4)) return false; break;
  case R14: if (!append_to_string(s, "\x4c\x0f\x4e\x35", 4)) return false; break;
  case R15: if (!append_to_string(s, "\x4c\x0f\x4e\x3d", 4)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }

  /* [m64] is translated as : m64 - byte count INCLUDING the instruction */
  Patch patch = {
    .index = s->count,
    .patch = m64,
    // the instruction is 8 bytes, 4 of which are already accounted for
    .offset   = s->count + 4,
    .size     = 4,
    .type     = PATCH_TYPE_ADDRESS,
  };
  if (*m64 == 0) da_append(b->patches, patch);
  if (!append_to_string(s, (const char *)m64, 4)) return false;
  return true;
}

TARE_DEF bool gen_cmovg_r64_qword_deref_m64_or_append_patch(BinaryGenerator *b, Reg64 r64, size_t *m64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x0f\x4f\x05", 4)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x0f\x4f\x1d", 4)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x0f\x4f\x0d", 4)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x0f\x4f\x15", 4)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x0f\x4f\x35", 4)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x0f\x4f\x3d", 4)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x0f\x4f\x2d", 4)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x0f\x4f\x25", 4)) return false; break;
  case R8:  if (!append_to_string(s, "\x4c\x0f\x4f\x05", 4)) return false; break;
  case R9:  if (!append_to_string(s, "\x4c\x0f\x4f\x0d", 4)) return false; break;
  case R10: if (!append_to_string(s, "\x4c\x0f\x4f\x15", 4)) return false; break;
  case R11: if (!append_to_string(s, "\x4c\x0f\x4f\x1d", 4)) return false; break;
  case R12: if (!append_to_string(s, "\x4c\x0f\x4f\x25", 4)) return false; break;
  case R13: if (!append_to_string(s, "\x4c\x0f\x4f\x2d", 4)) return false; break;
  case R14: if (!append_to_string(s, "\x4c\x0f\x4f\x35", 4)) return false; break;
  case R15: if (!append_to_string(s, "\x4c\x0f\x4f\x3d", 4)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }

  /* [m64] is translated as : m64 - byte count INCLUDING the instruction */
  Patch patch = {
    .index = s->count,
    .patch = m64,
    // the instruction is 8 bytes, 4 of which are already accounted for
    .offset   = s->count + 4,
    .size     = 4,
    .type     = PATCH_TYPE_ADDRESS,
  };
  if (*m64 == 0) da_append(b->patches, patch);
  if (!append_to_string(s, (const char *)m64, 4)) return false;
  return true;
}

TARE_DEF bool gen_cmovge_r64_qword_deref_m64_or_append_patch(BinaryGenerator *b, Reg64 r64, size_t *m64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x0f\x4d\x05", 4)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x0f\x4d\x1d", 4)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x0f\x4d\x0d", 4)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x0f\x4d\x15", 4)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x0f\x4d\x35", 4)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x0f\x4d\x3d", 4)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x0f\x4d\x2d", 4)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x0f\x4d\x25", 4)) return false; break;
  case R8:  if (!append_to_string(s, "\x4c\x0f\x4d\x05", 4)) return false; break;
  case R9:  if (!append_to_string(s, "\x4c\x0f\x4d\x0d", 4)) return false; break;
  case R10: if (!append_to_string(s, "\x4c\x0f\x4d\x15", 4)) return false; break;
  case R11: if (!append_to_string(s, "\x4c\x0f\x4d\x1d", 4)) return false; break;
  case R12: if (!append_to_string(s, "\x4c\x0f\x4d\x25", 4)) return false; break;
  case R13: if (!append_to_string(s, "\x4c\x0f\x4d\x2d", 4)) return false; break;
  case R14: if (!append_to_string(s, "\x4c\x0f\x4d\x35", 4)) return false; break;
  case R15: if (!append_to_string(s, "\x4c\x0f\x4d\x3d", 4)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }

  /* [m64] is translated as : m64 - byte count INCLUDING the instruction */
  Patch patch = {
    .index = s->count,
    .patch = m64,
    // the instruction is 8 bytes, 4 of which are already accounted for
    .offset   = s->count + 4,
    .size     = 4,
    .type     = PATCH_TYPE_ADDRESS,
  };
  if (*m64 == 0) da_append(b->patches, patch);
  if (!append_to_string(s, (const char *)m64, 4)) return false;
  return true;
}

TARE_DEF bool gen_cmove_r64_qword_deref_m64_or_append_patch(BinaryGenerator *b, Reg64 r64, size_t *m64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x0f\x44\x05", 4)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x0f\x44\x1d", 4)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x0f\x44\x0d", 4)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x0f\x44\x15", 4)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x0f\x44\x35", 4)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x0f\x44\x3d", 4)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x0f\x44\x2d", 4)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x0f\x44\x25", 4)) return false; break;
  case R8:  if (!append_to_string(s, "\x4c\x0f\x44\x05", 4)) return false; break;
  case R9:  if (!append_to_string(s, "\x4c\x0f\x44\x0d", 4)) return false; break;
  case R10: if (!append_to_string(s, "\x4c\x0f\x44\x15", 4)) return false; break;
  case R11: if (!append_to_string(s, "\x4c\x0f\x44\x1d", 4)) return false; break;
  case R12: if (!append_to_string(s, "\x4c\x0f\x44\x25", 4)) return false; break;
  case R13: if (!append_to_string(s, "\x4c\x0f\x44\x2d", 4)) return false; break;
  case R14: if (!append_to_string(s, "\x4c\x0f\x44\x35", 4)) return false; break;
  case R15: if (!append_to_string(s, "\x4c\x0f\x44\x3d", 4)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }

  /* [m64] is translated as : m64 - byte count INCLUDING the instruction */
  Patch patch = {
    .index = s->count,
    .patch = m64,
    // the instruction is 8 bytes, 4 of which are already accounted for
    .offset   = s->count + 4,
    .size     = 4,
    .type     = PATCH_TYPE_ADDRESS,
  };
  if (*m64 == 0) da_append(b->patches, patch);
  if (!append_to_string(s, (const char *)m64, 4)) return false;
  return true;
}

TARE_DEF bool gen_cmovne_r64_qword_deref_m64_or_append_patch(BinaryGenerator *b, Reg64 r64, size_t *m64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x0f\x45\x05", 4)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x0f\x45\x1d", 4)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x0f\x45\x0d", 4)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x0f\x45\x15", 4)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x0f\x45\x35", 4)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x0f\x45\x3d", 4)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x0f\x45\x2d", 4)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x0f\x45\x25", 4)) return false; break;
  case R8:  if (!append_to_string(s, "\x4c\x0f\x45\x05", 4)) return false; break;
  case R9:  if (!append_to_string(s, "\x4c\x0f\x45\x0d", 4)) return false; break;
  case R10: if (!append_to_string(s, "\x4c\x0f\x45\x15", 4)) return false; break;
  case R11: if (!append_to_string(s, "\x4c\x0f\x45\x1d", 4)) return false; break;
  case R12: if (!append_to_string(s, "\x4c\x0f\x45\x25", 4)) return false; break;
  case R13: if (!append_to_string(s, "\x4c\x0f\x45\x2d", 4)) return false; break;
  case R14: if (!append_to_string(s, "\x4c\x0f\x45\x35", 4)) return false; break;
  case R15: if (!append_to_string(s, "\x4c\x0f\x45\x3d", 4)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }

  /* [m64] is translated as : m64 - byte count INCLUDING the instruction */
  Patch patch = {
    .index = s->count,
    .patch = m64,
    // the instruction is 8 bytes, 4 of which are already accounted for
    .offset   = s->count + 4,
    .size     = 4,
    .type     = PATCH_TYPE_ADDRESS,
  };
  if (*m64 == 0) da_append(b->patches, patch);
  if (!append_to_string(s, (const char *)m64, 4)) return false;
  return true;
}

TARE_DEF bool gen_mov_rax_r64(BinaryGenerator *b, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x89\xc0", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x89\xd8", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x89\xc8", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x89\xd0", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x89\xf0", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x89\xf8", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x89\xe8", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x89\xe0", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x4c\x89\xc0", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x4c\x89\xc8", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x4c\x89\xd0", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x4c\x89\xd8", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x4c\x89\xe0", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x4c\x89\xe8", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x4c\x89\xf0", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x4c\x89\xf8", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_mov_rcx_r64(BinaryGenerator *b, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x89\xc1", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x89\xd9", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x89\xc9", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x89\xd1", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x89\xf1", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x89\xf9", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x89\xe9", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x89\xe1", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x4c\x89\xc1", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x4c\x89\xc9", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x4c\x89\xd1", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x4c\x89\xd9", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x4c\x89\xe1", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x4c\x89\xe9", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x4c\x89\xf1", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x4c\x89\xf9", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_mov_r64_qword_deref_rax(BinaryGenerator *b, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x8b\x00", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x8b\x18", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x8b\x08", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x8b\x10", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x8b\x30", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x8b\x38", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x8b\x28", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x8b\x20", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x4c\x8b\x00", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x4c\x8b\x08", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x4c\x8b\x10", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x4c\x8b\x18", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x4c\x8b\x20", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x4c\x8b\x28", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x4c\x8b\x30", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x4c\x8b\x38", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_mov_r64_qword_deref_m64_or_append_patch(BinaryGenerator *b, Reg64 r64, size_t *m64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x8b\x05", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x8b\x1d", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x8b\x0d", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x8b\x15", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x8b\x35", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x8b\x3d", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x8b\x2d", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x8b\x25", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x4c\x8b\x05", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x4c\x8b\x0d", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x4c\x8b\x15", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x4c\x8b\x1d", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x4c\x8b\x25", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x4c\x8b\x2d", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x4c\x8b\x35", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x4c\x8b\x3d", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }

  /* [m64] is translated as : m64 - byte count INCLUDING the instruction */
  Patch patch = {
    .index = s->count,
    .patch = m64,
    // the instruction is 7 bytes, 3 of which are already accounted for
    .offset   = s->count + 4,
    .size     = 4,
    .type     = PATCH_TYPE_ADDRESS,
  };
  if (*m64 == 0) da_append(b->patches, patch);
  if (!append_to_string(s, (const char *)m64, 4)) return false;
  return true;
}

TARE_DEF bool gen_mov_r64_m64_or_append_patch(BinaryGenerator *b, Reg64 r64, size_t *m64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\xc7\xc0", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\xc7\xc3", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\xc7\xc1", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\xc7\xc2", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\xc7\xc6", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\xc7\xc7", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\xc7\xc5", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\xc7\xc4", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x49\xc7\xc0", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x49\xc7\xc1", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x49\xc7\xc2", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x49\xc7\xc3", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x49\xc7\xc4", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x49\xc7\xc5", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x49\xc7\xc6", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x49\xc7\xc7", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }

  /* [m64] is translated as : m64 - byte count INCLUDING the instruction */
  Patch patch = {
    .index = s->count,
    .patch = m64,
    .size  = 4,
    .type  = PATCH_TYPE_VIRTUAL, // TODO: MAKE SURE THIS IS CORRECT
  };
  if (*m64 == 0) da_append(b->patches, patch);
  if (!append_to_string(s, (const char *)m64, 4)) return false;
  return true;
}

TARE_DEF bool gen_mov_qword_deref_m64_m64_or_append_patches(BinaryGenerator *b, size_t *m64_1, size_t *m64_2) {
  if (!append_to_string(b->bytes, "\x48\xc7\x05", 3)) return false;

  /* [m64] is translated as : m64 - byte count INCLUDING the instruction */
  Patch patch = {
    .index = b->bytes->count,
    .patch = m64_1,
    // the instruction is 11 bytes, 3 of which are already accounted for
    .offset   = b->bytes->count + 8,
    .size     = 4,
    .type     = PATCH_TYPE_ADDRESS,
  };
  if (*m64_1 == 0) da_append(b->patches, patch);
  if (!append_to_string(b->bytes, (const char *)m64_1, 4)) return false;

  if (*m64_2 == 0) {
    patch.index = b->bytes->count;
    patch.patch = m64_2;
    patch.offset = 0;
    patch.type  = PATCH_TYPE_VIRTUAL; // TODO: MAKE SURE THIS IS CORRECT

    da_append(b->patches, patch);
  }
  if (!append_to_string(b->bytes, (const char *)m64_2, 4)) return false;
  return true;
}

TARE_DEF bool gen_mov_qword_deref_rax_r64(BinaryGenerator *b, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x89\x00", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x89\x18", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x89\x08", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x89\x10", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x89\x30", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x89\x38", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x89\x28", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x89\x20", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x4c\x89\x00", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x4c\x89\x08", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x4c\x89\x10", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x4c\x89\x18", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x4c\x89\x20", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x4c\x89\x28", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x4c\x89\x30", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x4c\x89\x38", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_mov_dword_deref_rax_r32(BinaryGenerator *b, Reg32 r32) {
  String *s = b->bytes;
  switch (r32) {
  case EAX:  if (!append_to_string(s, "\x89\x00",     2)) return false; break;
  case EBX:  if (!append_to_string(s, "\x89\x18",     2)) return false; break;
  case ECX:  if (!append_to_string(s, "\x89\x08",     2)) return false; break;
  case EDX:  if (!append_to_string(s, "\x89\x10",     2)) return false; break;
  case ESI:  if (!append_to_string(s, "\x89\x30",     2)) return false; break;
  case EDI:  if (!append_to_string(s, "\x89\x38",     2)) return false; break;
  case EBP:  if (!append_to_string(s, "\x89\x28",     2)) return false; break;
  case ESP:  if (!append_to_string(s, "\x89\x20",     2)) return false; break;
  case R8D:  if (!append_to_string(s, "\x44\x89\x00", 3)) return false; break;
  case R9D:  if (!append_to_string(s, "\x44\x89\x08", 3)) return false; break;
  case R10D: if (!append_to_string(s, "\x44\x89\x10", 3)) return false; break;
  case R11D: if (!append_to_string(s, "\x44\x89\x18", 3)) return false; break;
  case R12D: if (!append_to_string(s, "\x44\x89\x20", 3)) return false; break;
  case R13D: if (!append_to_string(s, "\x44\x89\x28", 3)) return false; break;
  case R14D: if (!append_to_string(s, "\x44\x89\x30", 3)) return false; break;
  case R15D: if (!append_to_string(s, "\x44\x89\x38", 3)) return false; break;
  case REG_32_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_mov_word_deref_rax_r16(BinaryGenerator *b, Reg16 r16) {
  String *s = b->bytes;
  switch (r16) {
  case AX:   if (!append_to_string(s, "\x66\x89\x00", 3)) return false; break;
  case BX:   if (!append_to_string(s, "\x66\x89\x18", 3)) return false; break;
  case CX:   if (!append_to_string(s, "\x66\x89\x08", 3)) return false; break;
  case DX:   if (!append_to_string(s, "\x66\x89\x10", 3)) return false; break;
  case SI:   if (!append_to_string(s, "\x66\x89\x30", 3)) return false; break;
  case DI:   if (!append_to_string(s, "\x66\x89\x38", 3)) return false; break;
  case BP:   if (!append_to_string(s, "\x66\x89\x28", 3)) return false; break;
  case SP:   if (!append_to_string(s, "\x66\x89\x20", 3)) return false; break;
  case R8W:  if (!append_to_string(s, "\x66\x44\x89\x00", 4)) return false; break;
  case R9W:  if (!append_to_string(s, "\x66\x44\x89\x08", 4)) return false; break;
  case R10W: if (!append_to_string(s, "\x66\x44\x89\x10", 4)) return false; break;
  case R11W: if (!append_to_string(s, "\x66\x44\x89\x18", 4)) return false; break;
  case R12W: if (!append_to_string(s, "\x66\x44\x89\x20", 4)) return false; break;
  case R13W: if (!append_to_string(s, "\x66\x44\x89\x28", 4)) return false; break;
  case R14W: if (!append_to_string(s, "\x66\x44\x89\x30", 4)) return false; break;
  case R15W: if (!append_to_string(s, "\x66\x44\x89\x38", 4)) return false; break;
  case REG_16_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_mov_byte_deref_rax_r8(BinaryGenerator *b, Reg8 r8) {
  String *s = b->bytes;
  switch (r8) {
  case AL:   if (!append_to_string(s, "\x88\x00",     2)) return false; break;
  case AH:   if (!append_to_string(s, "\x88\x20",     2)) return false; break;
  case BL:   if (!append_to_string(s, "\x88\x18",     2)) return false; break;
  case BH:   if (!append_to_string(s, "\x88\x38",     2)) return false; break;
  case CL:   if (!append_to_string(s, "\x88\x08",     2)) return false; break;
  case CH:   if (!append_to_string(s, "\x88\x28",     2)) return false; break;
  case DL:   if (!append_to_string(s, "\x88\x10",     2)) return false; break;
  case DH:   if (!append_to_string(s, "\x88\x30",     2)) return false; break;
  case SIL:  if (!append_to_string(s, "\x40\x88\x30", 3)) return false; break;
  case DIL:  if (!append_to_string(s, "\x40\x88\x38", 3)) return false; break;
  case BPL:  if (!append_to_string(s, "\x40\x88\x28", 3)) return false; break;
  case SPL:  if (!append_to_string(s, "\x40\x88\x20", 3)) return false; break;
  case R8B:  if (!append_to_string(s, "\x44\x88\x00", 3)) return false; break;
  case R9B:  if (!append_to_string(s, "\x44\x88\x08", 3)) return false; break;
  case R10B: if (!append_to_string(s, "\x44\x88\x10", 3)) return false; break;
  case R11B: if (!append_to_string(s, "\x44\x88\x18", 3)) return false; break;
  case R12B: if (!append_to_string(s, "\x44\x88\x20", 3)) return false; break;
  case R13B: if (!append_to_string(s, "\x44\x88\x28", 3)) return false; break;
  case R14B: if (!append_to_string(s, "\x44\x88\x30", 3)) return false; break;
  case R15B: if (!append_to_string(s, "\x44\x88\x38", 3)) return false; break;
  case REG_8_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_mov_rbx_qword_deref_r64(BinaryGenerator *b, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x8b\x18", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x8b\x1b", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x8b\x19", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x8b\x1a", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x8b\x1e", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x8b\x1f", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x8b\x5d\x00", 4)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x8b\x1c\x24", 4)) return false; break;
  case R8:  if (!append_to_string(s, "\x49\x8b\x18", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x49\x8b\x19", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x49\x8b\x1a", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x49\x8b\x1b", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x49\x8b\x1c\x24", 4)) return false; break;
  case R13: if (!append_to_string(s, "\x49\x8b\x5d\x00", 4)) return false; break;
  case R14: if (!append_to_string(s, "\x49\x8b\x1e", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x49\x8b\x1f", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_mov_ebx_dword_deref_r64(BinaryGenerator *b, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x8b\x18", 2)) return false; break;
  case RBX: if (!append_to_string(s, "\x8b\x1b", 2)) return false; break;
  case RCX: if (!append_to_string(s, "\x8b\x19", 2)) return false; break;
  case RDX: if (!append_to_string(s, "\x8b\x1a", 2)) return false; break;
  case RSI: if (!append_to_string(s, "\x8b\x1e", 2)) return false; break;
  case RDI: if (!append_to_string(s, "\x8b\x1f", 2)) return false; break;
  case RBP: if (!append_to_string(s, "\x8b\x5d\x00", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x8b\x1c\x24", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x41\x8b\x18", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x41\x8b\x19", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x41\x8b\x1a", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x41\x8b\x1b", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x41\x8b\x1c\x24", 4)) return false; break;
  case R13: if (!append_to_string(s, "\x41\x8b\x5d\x00", 4)) return false; break;
  case R14: if (!append_to_string(s, "\x41\x8b\x1e", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x41\x8b\x1f", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_mov_bx_word_deref_r64(BinaryGenerator *b, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x66\x8b\x18", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x66\x8b\x1b", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x66\x8b\x19", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x66\x8b\x1a", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x66\x8b\x1e", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x66\x8b\x1f", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x66\x8b\x5d\x00", 4)) return false; break;
  case RSP: if (!append_to_string(s, "\x66\x8b\x1c\x24", 4)) return false; break;
  case R8:  if (!append_to_string(s, "\x66\x41\x8b\x18", 4)) return false; break;
  case R9:  if (!append_to_string(s, "\x66\x41\x8b\x19", 4)) return false; break;
  case R10: if (!append_to_string(s, "\x66\x41\x8b\x1a", 4)) return false; break;
  case R11: if (!append_to_string(s, "\x66\x41\x8b\x1b", 4)) return false; break;
  case R12: if (!append_to_string(s, "\x66\x41\x8b\x1c\x24", 5)) return false; break;
  case R13: if (!append_to_string(s, "\x66\x41\x8b\x5d\x00", 5)) return false; break;
  case R14: if (!append_to_string(s, "\x66\x41\x8b\x1e", 4)) return false; break;
  case R15: if (!append_to_string(s, "\x66\x41\x8b\x1f", 4)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_mov_bl_byte_deref_r64(BinaryGenerator *b, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x8a\x18", 2)) return false; break;
  case RBX: if (!append_to_string(s, "\x8a\x1b", 2)) return false; break;
  case RCX: if (!append_to_string(s, "\x8a\x19", 2)) return false; break;
  case RDX: if (!append_to_string(s, "\x8a\x1a", 2)) return false; break;
  case RSI: if (!append_to_string(s, "\x8a\x1e", 2)) return false; break;
  case RDI: if (!append_to_string(s, "\x8a\x1f", 2)) return false; break;
  case RBP: if (!append_to_string(s, "\x8a\x5d\x00", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x8a\x1c\x24", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x41\x8a\x18", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x41\x8a\x19", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x41\x8a\x1a", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x41\x8a\x1b", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x41\x8a\x1c\x24", 4)) return false; break;
  case R13: if (!append_to_string(s, "\x41\x8a\x5d\x00", 4)) return false; break;
  case R14: if (!append_to_string(s, "\x41\x8a\x1e", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x41\x8a\x1f", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_mov_qword_deref_r64_imm32(BinaryGenerator *b, Reg64 r64, size_t imm32) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\xc7\x00", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\xc7\x03", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\xc7\x01", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\xc7\x02", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\xc7\x06", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\xc7\x07", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\xc7\x45\x00", 4)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\xc7\x04\x24", 4)) return false; break;
  case R8:  if (!append_to_string(s, "\x49\xc7\x00", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x49\xc7\x01", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x49\xc7\x02", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x49\xc7\x03", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x49\xc7\x04\x24", 4)) return false; break;
  case R13: if (!append_to_string(s, "\x49\xc7\x45\x00", 4)) return false; break;
  case R14: if (!append_to_string(s, "\x49\xc7\x06", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x49\xc7\x07", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  if (!append_to_string(s, (const char *)&imm32, 4)) return false;
  return true;
}

TARE_DEF bool gen_mov_dword_deref_r64_imm32(BinaryGenerator *b, Reg64 r64, size_t imm32) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\xc7\x00", 2)) return false; break;
  case RBX: if (!append_to_string(s, "\xc7\x03", 2)) return false; break;
  case RCX: if (!append_to_string(s, "\xc7\x01", 2)) return false; break;
  case RDX: if (!append_to_string(s, "\xc7\x02", 2)) return false; break;
  case RSI: if (!append_to_string(s, "\xc7\x06", 2)) return false; break;
  case RDI: if (!append_to_string(s, "\xc7\x07", 2)) return false; break;
  case RBP: if (!append_to_string(s, "\xc7\x45\x00", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\xc7\x04\x24", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x41\xc7\x00", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x41\xc7\x01", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x41\xc7\x02", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x41\xc7\x03", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x41\xc7\x04\x24", 4)) return false; break;
  case R13: if (!append_to_string(s, "\x41\xc7\x45\x00", 4)) return false; break;
  case R14: if (!append_to_string(s, "\x41\xc7\x06", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x41\xc7\x07", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  if (!append_to_string(s, (const char *)&imm32, 4)) return false;
  return true;
}

TARE_DEF bool gen_mov_word_deref_r64_imm16(BinaryGenerator *b, Reg64 r64, size_t imm16) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x66\xc7\x00", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x66\xc7\x03", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x66\xc7\x01", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x66\xc7\x02", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x66\xc7\x06", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x66\xc7\x07", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x66\xc7\x45\x00", 4)) return false; break;
  case RSP: if (!append_to_string(s, "\x66\xc7\x04\x24", 4)) return false; break;
  case R8:  if (!append_to_string(s, "\x66\x41\xc7\x00", 4)) return false; break;
  case R9:  if (!append_to_string(s, "\x66\x41\xc7\x01", 4)) return false; break;
  case R10: if (!append_to_string(s, "\x66\x41\xc7\x02", 4)) return false; break;
  case R11: if (!append_to_string(s, "\x66\x41\xc7\x03", 4)) return false; break;
  case R12: if (!append_to_string(s, "\x66\x41\xc7\x04\x24", 5)) return false; break;
  case R13: if (!append_to_string(s, "\x66\x41\xc7\x45\x00", 5)) return false; break;
  case R14: if (!append_to_string(s, "\x66\x41\xc7\x06", 4)) return false; break;
  case R15: if (!append_to_string(s, "\x66\x41\xc7\x07", 4)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  if (!append_to_string(s, (const char *)&imm16, 2)) return false; 
  return true;
}

TARE_DEF bool gen_mov_byte_deref_r64_imm8(BinaryGenerator *b, Reg64 r64, size_t imm8) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\xc6\x00", 2)) return false; break;
  case RBX: if (!append_to_string(s, "\xc6\x03", 2)) return false; break;
  case RCX: if (!append_to_string(s, "\xc6\x01", 2)) return false; break;
  case RDX: if (!append_to_string(s, "\xc6\x02", 2)) return false; break;
  case RSI: if (!append_to_string(s, "\xc6\x06", 2)) return false; break;
  case RDI: if (!append_to_string(s, "\xc6\x07", 2)) return false; break;
  case RBP: if (!append_to_string(s, "\xc6\x45\x00", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\xc6\x04\x24", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x41\xc6\x00", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x41\xc6\x01", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x41\xc6\x02", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x41\xc6\x03", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x41\xc6\x04\x24", 4)) return false; break;
  case R13: if (!append_to_string(s, "\x41\xc6\x45\x00", 4)) return false; break;
  case R14: if (!append_to_string(s, "\x41\xc6\x06", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x41\xc6\x07", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  da_append(s, (char) imm8);
  return true;
}

TARE_DEF bool gen_add_qword_deref_m64_r64_or_append_patch(BinaryGenerator *b, size_t *m64, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x01\x05", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x01\x1d", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x01\x0d", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x01\x15", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x01\x35", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x01\x3d", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x01\x2d", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x01\x25", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x4c\x01\x05", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x4c\x01\x0d", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x4c\x01\x15", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x4c\x01\x1d", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x4c\x01\x25", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x4c\x01\x2d", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x4c\x01\x35", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x4c\x01\x3d", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }

  Patch patch = {
    .index = b->bytes->count,
    .patch = m64,
    // the instruction is 7 bytes, 3 of which are already accounted for
    .offset   = b->bytes->count + 4,
    .size     = 4,
    .type     = PATCH_TYPE_ADDRESS,
  };
  if (*m64 == 0) da_append(b->patches, patch);
  if (!append_to_string(b->bytes, (const char *)m64, 4)) return false;
  return true;
}

TARE_DEF bool gen_sub_qword_deref_m64_r64_or_append_patch(BinaryGenerator *b, size_t *m64, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x29\x05", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x29\x1d", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x29\x0d", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x29\x15", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x29\x35", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x29\x3d", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x29\x2d", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x29\x25", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x4c\x29\x05", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x4c\x29\x0d", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x4c\x29\x15", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x4c\x29\x1d", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x4c\x29\x25", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x4c\x29\x2d", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x4c\x29\x35", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x4c\x29\x3d", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }

  Patch patch = {
    .index = b->bytes->count,
    .patch = m64,
    // the instruction is 7 bytes, 3 of which are already accounted for
    .offset   = b->bytes->count + 4,
    .size     = 4,
    .type     = PATCH_TYPE_ADDRESS,
  };
  if (*m64 == 0) da_append(b->patches, patch);
  if (!append_to_string(b->bytes, (const char *)m64, 4)) return false;
  return true;
}

TARE_DEF bool gen_add_qword_deref_m64_imm32_or_append_patch(BinaryGenerator *b, size_t *m64, size_t imm32) {
  if (!append_to_string(b->bytes, "\x48\x81\x05", 3)) return false;

  /* [m64] is translated as : m64 - byte count INCLUDING the instruction */
  Patch patch = {
    .index = b->bytes->count,
    .patch = m64,
    // the instruction is 11 bytes, 3 of which are already accounted for
    .offset   = b->bytes->count + 8,
    .size     = 4,
    .type     = PATCH_TYPE_ADDRESS,
  };
  if (*m64 == 0) da_append(b->patches, patch);
  if (!append_to_string(b->bytes, (const char *)m64, 4)) return false;
  if (!append_to_string(b->bytes, (const char *)&imm32, 4)) return false;
  return true;
}

TARE_DEF bool gen_sub_qword_deref_m64_imm32_or_append_patch(BinaryGenerator *b, size_t *m64, size_t imm32) {
  if (!append_to_string(b->bytes, "\x48\x81\x2d", 3)) return false;

  /* [m64] is translated as : m64 - byte count INCLUDING the instruction */
  Patch patch = {
    .index = b->bytes->count,
    .patch = m64,
    // the instruction is 11 bytes, 3 of which are already accounted for
    .offset   = b->bytes->count + 8,
    .size     = 4,
    .type     = PATCH_TYPE_ADDRESS,
  };
  if (*m64 == 0) da_append(b->patches, patch);
  if (!append_to_string(b->bytes, (const char *)m64, 4)) return false;
  if (!append_to_string(b->bytes, (const char *)&imm32, 4)) return false;
  return true;
}

TARE_DEF bool gen_add_rax_r64(BinaryGenerator *b, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x01\xc0", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x01\xd8", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x01\xc8", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x01\xd0", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x01\xf0", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x01\xf8", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x01\xe8", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x01\xe0", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x4c\x01\xc0", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x4c\x01\xc8", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x4c\x01\xd0", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x4c\x01\xd8", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x4c\x01\xe0", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x4c\x01\xe8", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x4c\x01\xf0", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x4c\x01\xf8", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_sub_rax_r64(BinaryGenerator *b, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x29\xc0", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x29\xd8", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x29\xc8", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x29\xd0", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x29\xf0", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x29\xf8", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x29\xe8", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x29\xe0", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x4c\x29\xc0", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x4c\x29\xc8", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x4c\x29\xd0", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x4c\x29\xd8", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x4c\x29\xe0", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x4c\x29\xe8", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x4c\x29\xf0", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x4c\x29\xf8", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_mul_r64(BinaryGenerator *b, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\xf7\xe0", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\xf7\xe3", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\xf7\xe1", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\xf7\xe2", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\xf7\xe6", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\xf7\xe7", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\xf7\xe5", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\xf7\xe4", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x49\xf7\xe0", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x49\xf7\xe1", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x49\xf7\xe2", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x49\xf7\xe3", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x49\xf7\xe4", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x49\xf7\xe5", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x49\xf7\xe6", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x49\xf7\xe7", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_div_r64(BinaryGenerator *b, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\xf7\xf0", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\xf7\xf3", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\xf7\xf1", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\xf7\xf2", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\xf7\xf6", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\xf7\xf7", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\xf7\xf5", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\xf7\xf4", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x49\xf7\xf0", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x49\xf7\xf1", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x49\xf7\xf2", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x49\xf7\xf3", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x49\xf7\xf4", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x49\xf7\xf5", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x49\xf7\xf6", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x49\xf7\xf7", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_add_r64_imm32(BinaryGenerator *b, Reg64 r64, size_t imm32) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x05", 2)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x81\xc3", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x81\xc1", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x81\xc2", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x81\xc6", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x81\xc7", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x81\xc5", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x81\xc4", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x49\x81\xc0", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x49\x81\xc1", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x49\x81\xc2", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x49\x81\xc3", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x49\x81\xc4", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x49\x81\xc5", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x49\x81\xc6", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x49\x81\xc7", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  if (!append_to_string(s, (const char *)&imm32, 4)) return false;
  return true;
}

TARE_DEF bool gen_sub_r64_imm32(BinaryGenerator *b, Reg64 r64, size_t imm32) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x2d", 2)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x81\xeb", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x81\xe9", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x81\xea", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x81\xee", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x81\xef", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x81\xed", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x81\xec", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x49\x81\xe8", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x49\x81\xe9", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x49\x81\xea", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x49\x81\xeb", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x49\x81\xec", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x49\x81\xed", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x49\x81\xee", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x49\x81\xef", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  if (!append_to_string(s, (const char *)&imm32, 4)) return false;
  return true;
}

TARE_DEF bool gen_sub_r64_m64_or_append_patch(BinaryGenerator *b, Reg64 r64, size_t *m64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x2d", 2)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x81\xeb", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x81\xe9", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x81\xea", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x81\xee", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x81\xef", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x81\xed", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x81\xec", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x49\x81\xe8", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x49\x81\xe9", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x49\x81\xea", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x49\x81\xeb", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x49\x81\xec", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x49\x81\xed", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x49\x81\xee", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x49\x81\xef", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }

  Patch patch = {
    .index = b->bytes->count,
    .patch = m64,
    .size  = 4,
    .type  = PATCH_TYPE_VIRTUAL, // TODO: MAKE SURE THIS IS CORRECT
  };
  if (*m64 == 0) da_append(b->patches, patch);
  if (!append_to_string(b->bytes, (const char *)m64, 4)) return false;
  return true;
}

TARE_DEF bool gen_add_qword_deref_rbx_r64(BinaryGenerator *b, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x01\x03", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x01\x1b", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x01\x0b", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x01\x13", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x01\x33", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x01\x3b", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x01\x2b", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x01\x23", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x4c\x01\x03", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x4c\x01\x0b", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x4c\x01\x13", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x4c\x01\x1b", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x4c\x01\x23", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x4c\x01\x2b", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x4c\x01\x33", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x4c\x01\x3b", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_add_dword_deref_rbx_r32(BinaryGenerator *b, Reg32 r32) {
  String *s = b->bytes;
  switch (r32) {
  case EAX:  if (!append_to_string(s, "\x01\x03",     2)) return false; break;
  case EBX:  if (!append_to_string(s, "\x01\x1b",     2)) return false; break;
  case ECX:  if (!append_to_string(s, "\x01\x0b",     2)) return false; break;
  case EDX:  if (!append_to_string(s, "\x01\x13",     2)) return false; break;
  case ESI:  if (!append_to_string(s, "\x01\x33",     2)) return false; break;
  case EDI:  if (!append_to_string(s, "\x01\x3b",     2)) return false; break;
  case EBP:  if (!append_to_string(s, "\x01\x2b",     2)) return false; break;
  case ESP:  if (!append_to_string(s, "\x01\x23",     2)) return false; break;
  case R8D:  if (!append_to_string(s, "\x44\x01\x03", 3)) return false; break;
  case R9D:  if (!append_to_string(s, "\x44\x01\x0b", 3)) return false; break;
  case R10D: if (!append_to_string(s, "\x44\x01\x13", 3)) return false; break;
  case R11D: if (!append_to_string(s, "\x44\x01\x1b", 3)) return false; break;
  case R12D: if (!append_to_string(s, "\x44\x01\x23", 3)) return false; break;
  case R13D: if (!append_to_string(s, "\x44\x01\x2b", 3)) return false; break;
  case R14D: if (!append_to_string(s, "\x44\x01\x33", 3)) return false; break;
  case R15D: if (!append_to_string(s, "\x44\x01\x3b", 3)) return false; break;
  case REG_32_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_add_word_deref_rbx_r16(BinaryGenerator *b, Reg16 r16) {
  String *s = b->bytes;
  switch (r16) {
  case AX:   if (!append_to_string(s, "\x66\x01\x03", 3)) return false; break;
  case BX:   if (!append_to_string(s, "\x66\x01\x1b", 3)) return false; break;
  case CX:   if (!append_to_string(s, "\x66\x01\x0b", 3)) return false; break;
  case DX:   if (!append_to_string(s, "\x66\x01\x13", 3)) return false; break;
  case SI:   if (!append_to_string(s, "\x66\x01\x33", 3)) return false; break;
  case DI:   if (!append_to_string(s, "\x66\x01\x3b", 3)) return false; break;
  case BP:   if (!append_to_string(s, "\x66\x01\x2b", 3)) return false; break;
  case SP:   if (!append_to_string(s, "\x66\x01\x23", 3)) return false; break;
  case R8W:  if (!append_to_string(s, "\x66\x44\x01\x03", 4)) return false; break;
  case R9W:  if (!append_to_string(s, "\x66\x44\x01\x0b", 4)) return false; break;
  case R10W: if (!append_to_string(s, "\x66\x44\x01\x13", 4)) return false; break;
  case R11W: if (!append_to_string(s, "\x66\x44\x01\x1b", 4)) return false; break;
  case R12W: if (!append_to_string(s, "\x66\x44\x01\x23", 4)) return false; break;
  case R13W: if (!append_to_string(s, "\x66\x44\x01\x2b", 4)) return false; break;
  case R14W: if (!append_to_string(s, "\x66\x44\x01\x33", 4)) return false; break;
  case R15W: if (!append_to_string(s, "\x66\x44\x01\x3b", 4)) return false; break;
  case REG_16_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_add_byte_deref_rbx_r8(BinaryGenerator *b, Reg8 r8) {
  String *s = b->bytes;
  switch (r8) {
  case AL:   if (!append_to_string(s, "\x00\x03",     2)) return false; break;
  case AH:   if (!append_to_string(s, "\x00\x23",     2)) return false; break;
  case BL:   if (!append_to_string(s, "\x00\x1b",     2)) return false; break;
  case BH:   if (!append_to_string(s, "\x00\x3b",     2)) return false; break;
  case CL:   if (!append_to_string(s, "\x00\x0b",     2)) return false; break;
  case CH:   if (!append_to_string(s, "\x00\x2b",     2)) return false; break;
  case DL:   if (!append_to_string(s, "\x00\x13",     2)) return false; break;
  case DH:   if (!append_to_string(s, "\x00\x33",     2)) return false; break;
  case SIL:  if (!append_to_string(s, "\x40\x00\x33", 3)) return false; break;
  case DIL:  if (!append_to_string(s, "\x40\x00\x3b", 3)) return false; break;
  case BPL:  if (!append_to_string(s, "\x40\x00\x2b", 3)) return false; break;
  case SPL:  if (!append_to_string(s, "\x40\x00\x23", 3)) return false; break;
  case R8B:  if (!append_to_string(s, "\x44\x00\x03", 3)) return false; break;
  case R9B:  if (!append_to_string(s, "\x44\x00\x0b", 3)) return false; break;
  case R10B: if (!append_to_string(s, "\x44\x00\x13", 3)) return false; break;
  case R11B: if (!append_to_string(s, "\x44\x00\x1b", 3)) return false; break;
  case R12B: if (!append_to_string(s, "\x44\x00\x23", 3)) return false; break;
  case R13B: if (!append_to_string(s, "\x44\x00\x2b", 3)) return false; break;
  case R14B: if (!append_to_string(s, "\x44\x00\x33", 3)) return false; break;
  case R15B: if (!append_to_string(s, "\x44\x00\x3b", 3)) return false; break;
  case REG_8_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_sub_qword_deref_rbx_r64(BinaryGenerator *b, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x29\x03", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x29\x1b", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x29\x0b", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x29\x13", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x29\x33", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x29\x3b", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x29\x2b", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x29\x23", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x4c\x29\x03", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x4c\x29\x0b", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x4c\x29\x13", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x4c\x29\x1b", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x4c\x29\x23", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x4c\x29\x2b", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x4c\x29\x33", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x4c\x29\x3b", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_sub_dword_deref_rbx_r32(BinaryGenerator *b, Reg32 r32) {
  String *s = b->bytes;
  switch (r32) {
  case EAX:  if (!append_to_string(s, "\x29\x03",     2)) return false; break;
  case EBX:  if (!append_to_string(s, "\x29\x1b",     2)) return false; break;
  case ECX:  if (!append_to_string(s, "\x29\x0b",     2)) return false; break;
  case EDX:  if (!append_to_string(s, "\x29\x13",     2)) return false; break;
  case ESI:  if (!append_to_string(s, "\x29\x33",     2)) return false; break;
  case EDI:  if (!append_to_string(s, "\x29\x3b",     2)) return false; break;
  case EBP:  if (!append_to_string(s, "\x29\x2b",     2)) return false; break;
  case ESP:  if (!append_to_string(s, "\x29\x23",     2)) return false; break;
  case R8D:  if (!append_to_string(s, "\x44\x29\x03", 3)) return false; break;
  case R9D:  if (!append_to_string(s, "\x44\x29\x0b", 3)) return false; break;
  case R10D: if (!append_to_string(s, "\x44\x29\x13", 3)) return false; break;
  case R11D: if (!append_to_string(s, "\x44\x29\x1b", 3)) return false; break;
  case R12D: if (!append_to_string(s, "\x44\x29\x23", 3)) return false; break;
  case R13D: if (!append_to_string(s, "\x44\x29\x2b", 3)) return false; break;
  case R14D: if (!append_to_string(s, "\x44\x29\x33", 3)) return false; break;
  case R15D: if (!append_to_string(s, "\x44\x29\x3b", 3)) return false; break;
  case REG_32_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_sub_word_deref_rbx_r16(BinaryGenerator *b, Reg16 r16) {
  String *s = b->bytes;
  switch (r16) {
  case AX:   if (!append_to_string(s, "\x66\x29\x03", 3)) return false; break;
  case BX:   if (!append_to_string(s, "\x66\x29\x1b", 3)) return false; break;
  case CX:   if (!append_to_string(s, "\x66\x29\x0b", 3)) return false; break;
  case DX:   if (!append_to_string(s, "\x66\x29\x13", 3)) return false; break;
  case SI:   if (!append_to_string(s, "\x66\x29\x33", 3)) return false; break;
  case DI:   if (!append_to_string(s, "\x66\x29\x3b", 3)) return false; break;
  case BP:   if (!append_to_string(s, "\x66\x29\x2b", 3)) return false; break;
  case SP:   if (!append_to_string(s, "\x66\x29\x23", 3)) return false; break;
  case R8W:  if (!append_to_string(s, "\x66\x44\x29\x03", 4)) return false; break;
  case R9W:  if (!append_to_string(s, "\x66\x44\x29\x0b", 4)) return false; break;
  case R10W: if (!append_to_string(s, "\x66\x44\x29\x13", 4)) return false; break;
  case R11W: if (!append_to_string(s, "\x66\x44\x29\x1b", 4)) return false; break;
  case R12W: if (!append_to_string(s, "\x66\x44\x29\x23", 4)) return false; break;
  case R13W: if (!append_to_string(s, "\x66\x44\x29\x2b", 4)) return false; break;
  case R14W: if (!append_to_string(s, "\x66\x44\x29\x33", 4)) return false; break;
  case R15W: if (!append_to_string(s, "\x66\x44\x29\x3b", 4)) return false; break;
  case REG_16_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_sub_byte_deref_rbx_r8(BinaryGenerator *b, Reg8 r8) {
  String *s = b->bytes;
  switch (r8) {
  case AL:   if (!append_to_string(s, "\x28\x03",     2)) return false; break;
  case AH:   if (!append_to_string(s, "\x28\x23",     2)) return false; break;
  case BL:   if (!append_to_string(s, "\x28\x1b",     2)) return false; break;
  case BH:   if (!append_to_string(s, "\x28\x3b",     2)) return false; break;
  case CL:   if (!append_to_string(s, "\x28\x0b",     2)) return false; break;
  case CH:   if (!append_to_string(s, "\x28\x2b",     2)) return false; break;
  case DL:   if (!append_to_string(s, "\x28\x13",     2)) return false; break;
  case DH:   if (!append_to_string(s, "\x28\x33",     2)) return false; break;
  case SIL:  if (!append_to_string(s, "\x40\x28\x33", 3)) return false; break;
  case DIL:  if (!append_to_string(s, "\x40\x28\x3b", 3)) return false; break;
  case BPL:  if (!append_to_string(s, "\x40\x28\x2b", 3)) return false; break;
  case SPL:  if (!append_to_string(s, "\x40\x28\x23", 3)) return false; break;
  case R8B:  if (!append_to_string(s, "\x44\x28\x03", 3)) return false; break;
  case R9B:  if (!append_to_string(s, "\x44\x28\x0b", 3)) return false; break;
  case R10B: if (!append_to_string(s, "\x44\x28\x13", 3)) return false; break;
  case R11B: if (!append_to_string(s, "\x44\x28\x1b", 3)) return false; break;
  case R12B: if (!append_to_string(s, "\x44\x28\x23", 3)) return false; break;
  case R13B: if (!append_to_string(s, "\x44\x28\x2b", 3)) return false; break;
  case R14B: if (!append_to_string(s, "\x44\x28\x33", 3)) return false; break;
  case R15B: if (!append_to_string(s, "\x44\x28\x3b", 3)) return false; break;
  case REG_8_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_shl_r64_imm8(BinaryGenerator *b, Reg64 r64, size_t imm8) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\xc1\xe0", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\xc1\xe3", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\xc1\xe1", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\xc1\xe2", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\xc1\xe6", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\xc1\xe7", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\xc1\xe5", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\xc1\xe4", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x49\xc1\xe0", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x49\xc1\xe1", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x49\xc1\xe2", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x49\xc1\xe3", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x49\xc1\xe4", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x49\xc1\xe5", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x49\xc1\xe6", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x49\xc1\xe7", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  da_append(s, (char) imm8);
  return true;
}

TARE_DEF bool gen_shl_r64_1(BinaryGenerator *b, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\xd1\xe0", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\xd1\xe3", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\xd1\xe1", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\xd1\xe2", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\xd1\xe6", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\xd1\xe7", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\xd1\xe5", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\xd1\xe4", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x49\xd1\xe0", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x49\xd1\xe1", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x49\xd1\xe2", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x49\xd1\xe3", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x49\xd1\xe4", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x49\xd1\xe5", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x49\xd1\xe6", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x49\xd1\xe7", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_and_rax_r64(BinaryGenerator *b, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x21\xc0", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x21\xd8", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x21\xc8", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x21\xd0", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x21\xf0", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x21\xf8", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x21\xe8", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x21\xe0", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x4c\x21\xc0", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x4c\x21\xc8", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x4c\x21\xd0", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x4c\x21\xd8", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x4c\x21\xe0", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x4c\x21\xe8", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x4c\x21\xf0", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x4c\x21\xf8", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_or_rax_r64(BinaryGenerator *b, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x09\xc0", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x09\xd8", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x09\xc8", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x09\xd0", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x09\xf0", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x09\xf8", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x09\xe8", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x09\xe0", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x4c\x09\xc0", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x4c\x09\xc8", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x4c\x09\xd0", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x4c\x09\xd8", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x4c\x09\xe0", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x4c\x09\xe8", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x4c\x09\xf0", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x4c\x09\xf8", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_or_r64_imm32(BinaryGenerator *b, Reg64 r64, size_t imm32) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x0d", 2)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x81\xcb", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x81\xc9", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x81\xca", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x81\xce", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x81\xcf", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x81\xcd", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x81\xcc", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x49\x81\xc8", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x49\x81\xc9", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x49\x81\xca", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x49\x81\xcb", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x49\x81\xcc", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x49\x81\xcd", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x49\x81\xce", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x49\x81\xcf", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  if (!append_to_string(s, (const char *)&imm32, 4)) return false;
  return true;
}

TARE_DEF bool gen_xor_r64_self(BinaryGenerator *b, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x31\xc0", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x31\xdb", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x31\xc9", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x31\xd2", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x31\xf6", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x31\xff", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x31\xed", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x31\xe4", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x4d\x31\xc0", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x4d\x31\xc9", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x4d\x31\xd2", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x4d\x31\xdb", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x4d\x31\xe4", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x4d\x31\xed", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x4d\x31\xf6", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x4d\x31\xff", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_shl_r64_cl(BinaryGenerator *b, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\xd3\xe0", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\xd3\xe3", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\xd3\xe1", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\xd3\xe2", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\xd3\xe6", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\xd3\xe7", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\xd3\xe5", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\xd3\xe4", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x49\xd3\xe0", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x49\xd3\xe1", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x49\xd3\xe2", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x49\xd3\xe3", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x49\xd3\xe4", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x49\xd3\xe5", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x49\xd3\xe6", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x49\xd3\xe7", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_shr_r64_cl(BinaryGenerator *b, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\xd3\xe8", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\xd3\xeb", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\xd3\xe9", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\xd3\xea", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\xd3\xee", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\xd3\xef", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\xd3\xed", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\xd3\xec", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x49\xd3\xe8", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x49\xd3\xe9", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x49\xd3\xea", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x49\xd3\xeb", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x49\xd3\xec", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x49\xd3\xed", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x49\xd3\xee", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x49\xd3\xef", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_pop_r64(BinaryGenerator *b, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: da_append(s, (char) 0x58); break;
  case RBX: da_append(s, (char) 0x5b); break;
  case RCX: da_append(s, (char) 0x59); break;
  case RDX: da_append(s, (char) 0x5a); break;
  case RSI: da_append(s, (char) 0x5e); break;
  case RDI: da_append(s, (char) 0x5f); break;
  case RBP: da_append(s, (char) 0x5d); break;
  case RSP: da_append(s, (char) 0x5c); break;
  case R8:  da_append(s, (char) 0x41); da_append(s, (char) 0x58); break;
  case R9:  da_append(s, (char) 0x41); da_append(s, (char) 0x59); break;
  case R10: da_append(s, (char) 0x41); da_append(s, (char) 0x5a); break;
  case R11: da_append(s, (char) 0x41); da_append(s, (char) 0x5b); break;
  case R12: da_append(s, (char) 0x41); da_append(s, (char) 0x5c); break;
  case R13: da_append(s, (char) 0x41); da_append(s, (char) 0x5d); break;
  case R14: da_append(s, (char) 0x41); da_append(s, (char) 0x5e); break;
  case R15: da_append(s, (char) 0x41); da_append(s, (char) 0x5f); break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_push_r64(BinaryGenerator *b, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: da_append(s, (char) 0x50); break;
  case RBX: da_append(s, (char) 0x53); break;
  case RCX: da_append(s, (char) 0x51); break;
  case RDX: da_append(s, (char) 0x52); break;
  case RSI: da_append(s, (char) 0x56); break;
  case RDI: da_append(s, (char) 0x57); break;
  case RBP: da_append(s, (char) 0x55); break;
  case RSP: da_append(s, (char) 0x54); break;
  case R8:  da_append(s, (char) 0x41); da_append(s, (char) 0x50); break;
  case R9:  da_append(s, (char) 0x41); da_append(s, (char) 0x51); break;
  case R10: da_append(s, (char) 0x41); da_append(s, (char) 0x52); break;
  case R11: da_append(s, (char) 0x41); da_append(s, (char) 0x53); break;
  case R12: da_append(s, (char) 0x41); da_append(s, (char) 0x54); break;
  case R13: da_append(s, (char) 0x41); da_append(s, (char) 0x55); break;
  case R14: da_append(s, (char) 0x41); da_append(s, (char) 0x56); break;
  case R15: da_append(s, (char) 0x41); da_append(s, (char) 0x57); break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_push_qword_deref_r64(BinaryGenerator *b, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\xff\x30", 2)) return false; break;
  case RBX: if (!append_to_string(s, "\xff\x33", 2)) return false; break;
  case RCX: if (!append_to_string(s, "\xff\x31", 2)) return false; break;
  case RDX: if (!append_to_string(s, "\xff\x32", 2)) return false; break;
  case RSI: if (!append_to_string(s, "\xff\x36", 2)) return false; break;
  case RDI: if (!append_to_string(s, "\xff\x37", 2)) return false; break;
  case RBP: if (!append_to_string(s, "\xff\x75\x00", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\xff\x34\x24", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x41\xff\x30", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x41\xff\x31", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x41\xff\x32", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x41\xff\x33", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x41\xff\x34\x24", 4)) return false; break;
  case R13: if (!append_to_string(s, "\x41\xff\x75\x00", 4)) return false; break;
  case R14: if (!append_to_string(s, "\x41\xff\x36", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x41\xff\x37", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_cmp_rax_r64(BinaryGenerator *b, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x39\xc0", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x39\xd8", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x39\xc8", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x39\xd0", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x39\xf0", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x39\xf8", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x39\xe8", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x39\xe0", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x4c\x39\xc0", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x4c\x39\xc8", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x4c\x39\xd0", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x4c\x39\xd8", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x4c\x39\xe0", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x4c\x39\xe8", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x4c\x39\xf0", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x4c\x39\xf8", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_cmp_rbx_r64(BinaryGenerator *b, Reg64 r64) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x39\xc3", 3)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x39\xdb", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x39\xcb", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x39\xd3", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x39\xf3", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x39\xfb", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x39\xeb", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x39\xe3", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x4c\x39\xc3", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x4c\x39\xcb", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x4c\x39\xd3", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x4c\x39\xdb", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x4c\x39\xe3", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x4c\x39\xeb", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x4c\x39\xf3", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x4c\x39\xfb", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  return true;
}

TARE_DEF bool gen_cmp_r64_imm32(BinaryGenerator *b, Reg64 r64, size_t imm32) {
  String *s = b->bytes;
  switch (r64) {
  case RAX: if (!append_to_string(s, "\x48\x3d",     2)) return false; break;
  case RBX: if (!append_to_string(s, "\x48\x81\xfb", 3)) return false; break;
  case RCX: if (!append_to_string(s, "\x48\x81\xf9", 3)) return false; break;
  case RDX: if (!append_to_string(s, "\x48\x81\xfa", 3)) return false; break;
  case RSI: if (!append_to_string(s, "\x48\x81\xfe", 3)) return false; break;
  case RDI: if (!append_to_string(s, "\x48\x81\xff", 3)) return false; break;
  case RBP: if (!append_to_string(s, "\x48\x81\xfd", 3)) return false; break;
  case RSP: if (!append_to_string(s, "\x48\x81\xfc", 3)) return false; break;
  case R8:  if (!append_to_string(s, "\x49\x81\xf8", 3)) return false; break;
  case R9:  if (!append_to_string(s, "\x49\x81\xf9", 3)) return false; break;
  case R10: if (!append_to_string(s, "\x49\x81\xfa", 3)) return false; break;
  case R11: if (!append_to_string(s, "\x49\x81\xfb", 3)) return false; break;
  case R12: if (!append_to_string(s, "\x49\x81\xfc", 3)) return false; break;
  case R13: if (!append_to_string(s, "\x49\x81\xfd", 3)) return false; break;
  case R14: if (!append_to_string(s, "\x49\x81\xfe", 3)) return false; break;
  case R15: if (!append_to_string(s, "\x49\x81\xff", 3)) return false; break;
  case REG_64_COUNT: default: assert(false && "unreachable");
  }
  if (!append_to_string(s, (const char *)&imm32, 4)) return false;
  return true;
}

TARE_DEF bool gen_cmp_r32_imm32(BinaryGenerator *b, Reg32 r32, size_t imm32) {
  String *s = b->bytes;
  switch (r32) {
  case EAX:  da_append(s, (char) 0x3d); break;
  case EBX:  if (!append_to_string(s, "\x81\xfb", 2)) return false; break;
  case ECX:  if (!append_to_string(s, "\x81\xf9", 2)) return false; break;
  case EDX:  if (!append_to_string(s, "\x81\xfa", 2)) return false; break;
  case ESI:  if (!append_to_string(s, "\x81\xfe", 2)) return false; break;
  case EDI:  if (!append_to_string(s, "\x81\xff", 2)) return false; break;
  case EBP:  if (!append_to_string(s, "\x81\xfd", 2)) return false; break;
  case ESP:  if (!append_to_string(s, "\x81\xfc", 2)) return false; break;
  case R8D:  if (!append_to_string(s, "\x41\x81\xf8", 3)) return false; break;
  case R9D:  if (!append_to_string(s, "\x41\x81\xf9", 3)) return false; break;
  case R10D: if (!append_to_string(s, "\x41\x81\xfa", 3)) return false; break;
  case R11D: if (!append_to_string(s, "\x41\x81\xfb", 3)) return false; break;
  case R12D: if (!append_to_string(s, "\x41\x81\xfc", 3)) return false; break;
  case R13D: if (!append_to_string(s, "\x41\x81\xfd", 3)) return false; break;
  case R14D: if (!append_to_string(s, "\x41\x81\xfe", 3)) return false; break;
  case R15D: if (!append_to_string(s, "\x41\x81\xff", 3)) return false; break;
  case REG_32_COUNT: default: assert(false && "unreachable");
  }
  if (!append_to_string(s, (const char *)&imm32, 4)) return false;
  return true;
}

TARE_DEF bool gen_cmp_r16_imm16(BinaryGenerator *b, Reg16 r16, size_t imm16) {
  String *s = b->bytes;
  switch (r16) {
  case AX:   if (!append_to_string(s, "\x66\x3d",     2)) return false; break;
  case BX:   if (!append_to_string(s, "\x66\x81\xfb", 3)) return false; break;
  case CX:   if (!append_to_string(s, "\x66\x81\xf9", 3)) return false; break;
  case DX:   if (!append_to_string(s, "\x66\x81\xfa", 3)) return false; break;
  case SI:   if (!append_to_string(s, "\x66\x81\xfe", 3)) return false; break;
  case DI:   if (!append_to_string(s, "\x66\x81\xff", 3)) return false; break;
  case BP:   if (!append_to_string(s, "\x66\x81\xfd", 3)) return false; break;
  case SP:   if (!append_to_string(s, "\x66\x81\xfc", 3)) return false; break;
  case R8W:  if (!append_to_string(s, "\x66\x41\x81\xf8", 4)) return false; break;
  case R9W:  if (!append_to_string(s, "\x66\x41\x81\xf9", 4)) return false; break;
  case R10W: if (!append_to_string(s, "\x66\x41\x81\xfa", 4)) return false; break;
  case R11W: if (!append_to_string(s, "\x66\x41\x81\xfb", 4)) return false; break;
  case R12W: if (!append_to_string(s, "\x66\x41\x81\xfc", 4)) return false; break;
  case R13W: if (!append_to_string(s, "\x66\x41\x81\xfd", 4)) return false; break;
  case R14W: if (!append_to_string(s, "\x66\x41\x81\xfe", 4)) return false; break;
  case R15W: if (!append_to_string(s, "\x66\x41\x81\xff", 4)) return false; break;
  case REG_16_COUNT: default: assert(false && "unreachable");
  }
  if (!append_to_string(s, (const char *)&imm16, 2)) return false; 
  return true;
}

TARE_DEF bool gen_cmp_r8_imm8(BinaryGenerator *b, Reg8 r8, size_t imm8) {
  String *s = b->bytes;
  switch (r8) {
  case AL:   da_append(s, (char) 0x3c); break;
  case AH:   if (!append_to_string(s, "\x80\xfc", 2)) return false; break;
  case BL:   if (!append_to_string(s, "\x80\xfb", 2)) return false; break;
  case BH:   if (!append_to_string(s, "\x80\xff", 2)) return false; break;
  case CL:   if (!append_to_string(s, "\x80\xf9", 2)) return false; break;
  case CH:   if (!append_to_string(s, "\x80\xfd", 2)) return false; break;
  case DL:   if (!append_to_string(s, "\x80\xfa", 2)) return false; break;
  case DH:   if (!append_to_string(s, "\x80\xfe", 2)) return false; break;
  case SIL:  if (!append_to_string(s, "\x40\x80\xfe", 3)) return false; break;
  case DIL:  if (!append_to_string(s, "\x40\x80\xff", 3)) return false; break;
  case BPL:  if (!append_to_string(s, "\x40\x80\xfd", 3)) return false; break;
  case SPL:  if (!append_to_string(s, "\x40\x80\xfc", 3)) return false; break;
  case R8B:  if (!append_to_string(s, "\x41\x80\xf8", 3)) return false; break;
  case R9B:  if (!append_to_string(s, "\x41\x80\xf9", 3)) return false; break;
  case R10B: if (!append_to_string(s, "\x41\x80\xfa", 3)) return false; break;
  case R11B: if (!append_to_string(s, "\x41\x80\xfb", 3)) return false; break;
  case R12B: if (!append_to_string(s, "\x41\x80\xfc", 3)) return false; break;
  case R13B: if (!append_to_string(s, "\x41\x80\xfd", 3)) return false; break;
  case R14B: if (!append_to_string(s, "\x41\x80\xfe", 3)) return false; break;
  case R15B: if (!append_to_string(s, "\x41\x80\xff", 3)) return false; break;
  case REG_8_COUNT: default: assert(false && "unreachable");
  }
  da_append(s, (char) imm8);
  return true;
}

TARE_DEF bool gen_jmp_or_append_patch(BinaryGenerator *b, size_t *addr) {
  da_append(b->bytes, (char) 0xe9);
  if (*addr == 0) {
    Patch patch = {
      .index = b->bytes->count,
      .patch = addr,
      // the instruction is 5 bytes, one of which is already accounted for
      .offset   = b->bytes->count + 4,
      .size     = 4,
      .type     = PATCH_TYPE_ADDRESS,
    };
    da_append(b->patches, patch);
    if (!append_to_string(b->bytes, (const char *) addr, 4)) return false;
  } else {
    size_t address = *addr - b->bytes->count - 4;
    if (!append_to_string(b->bytes, (const char *) &address, 4)) return false;
  }
  return true;
}

TARE_DEF bool gen_jz_or_append_patch(BinaryGenerator *b, size_t *addr) {
  da_append(b->bytes, (char) 0x0f);
  da_append(b->bytes, (char) 0x84);
  if (*addr == 0) {
    Patch patch = {
      .index = b->bytes->count,
      .patch = addr,
      // the instruction is 6 bytes, 2 of which are already accounted for
      .offset   = b->bytes->count + 4,
      .size     = 4,
      .type     = PATCH_TYPE_ADDRESS,
    };
    da_append(b->patches, patch);
    if (!append_to_string(b->bytes, (const char *) addr, 4)) return false;
  } else {
    size_t address = *addr - b->bytes->count - 4;
    if (!append_to_string(b->bytes, (const char *) &address, 4)) return false;
  }
  return true;
}

TARE_DEF void gen_syscall(BinaryGenerator *b) {
  da_append(b->bytes, (char) 0x0f);
  da_append(b->bytes, (char) 0x05);
}

TARE_DEF void gen_ret(BinaryGenerator *b) {
  da_append(b->bytes, (char) 0xc3);
}

TARE_DEF bool gen_call_or_append_patch(BinaryGenerator *b, size_t *func) {
  da_append(b->bytes, (char) 0xe8); // TODO: hopefully this actually works
  if (*func == 0) {
    Patch patch = {
      .index = b->bytes->count,
      .patch = func,
      // the instruction is 5 bytes, 1 of which are already accounted for
      .offset   = b->bytes->count + 4,
      .size     = 4,
      .type     = PATCH_TYPE_ADDRESS,
    };
    da_append(b->patches, patch);
    if (!append_to_string(b->bytes, (const char *) func, 4)) return false;
  } else {
    size_t function = *func - b->bytes->count - 4;
    if (!append_to_string(b->bytes, (const char *) &function, 4)) return false;
  }
  return true;
}

// **************************** Operations *****************************
TARE_DEF bool gen_funcall_bin(BinaryGenerator *b, size_t fid) {
  Function *fn = b->fns->items + fid;

  size_t args_fn = get_args_size(fn);
  size_t rets_fn = get_rets_size(fn);
  size_t lvars_fn = get_lvars_size(fn);
  
  Function *fni = b->fns->items + b->fni;
  
  size_t args_size = get_args_size(fni);
  size_t rets_size = get_rets_size(fni);
  size_t lvars_size = get_lvars_size(fni);

  if (args_size > 0) {
    if (!gen_add_qword_deref_m64_imm32_or_append_patch(b, &b->args_head_location, args_size)) return false;
  }
  if (rets_size > 0) {
    if (!gen_add_qword_deref_m64_imm32_or_append_patch(b, &b->rets_head_location, rets_size)) return false;
  }
  if (lvars_size > 0) {
    if (!gen_add_qword_deref_m64_imm32_or_append_patch(b, &b->locals_head_location, lvars_size)) return false;
  }

  if (lvars_fn > 0) {
    if (!gen_mov_r64_qword_deref_m64_or_append_patch(b, RAX, &b->locals_head_location)) return false;
    if (!gen_add_r64_imm32(b, RAX, lvars_fn)) return false;
  }
  
  for (size_t i = 0; i < fn->lvars.count; i++) {
    Var var = fn->lvars.items[i];
    switch (var.tid) {
    case TYPE_U8:
      if (!gen_sub_r64_imm32(b, RAX, 1)) return false;
      if (!gen_mov_byte_deref_r64_imm8(b, RAX, 0)) return false;
      /* if (!gen_sub_r64_imm32(b, RAX, 7)) return false; */
      break;
    case TYPE_U16:
      if (!gen_sub_r64_imm32(b, RAX, 2)) return false;
      if (!gen_mov_word_deref_r64_imm16(b, RAX, 0)) return false;
      /* if (!gen_sub_r64_imm32(b, RAX, 6)) return false; */
      break;
    case TYPE_U32:
      if (!gen_sub_r64_imm32(b, RAX, 4)) return false;
      if (!gen_mov_dword_deref_r64_imm32(b, RAX, 0)) return false;
      /* if (!gen_sub_r64_imm32(b, RAX, 4)) return false; */
      break;
    case TYPE_U64:
      if (!gen_sub_r64_imm32(b, RAX, 8)) return false;
      if (!gen_mov_qword_deref_r64_imm32(b, RAX, 0)) return false;
      break;
    case TYPES_COUNT:
    default: assert(false && "unimplemented");
    }
  }
  
  if (args_fn > 0) {
    if (!gen_mov_r64_qword_deref_m64_or_append_patch(b, RAX, &b->args_head_location)) return false;
    if (!gen_add_r64_imm32(b, RAX, args_fn)) return false;
  }
  
  for (size_t i = 0; i < fn->args.count; i++) {
    if (!gen_pop_r64(b, RBX)) return false;
    Var arg = fn->args.items[i];
    switch (arg.tid) {
    case TYPE_U8:
      if (!gen_sub_r64_imm32(b, RAX, 1)) return false;
      if (!gen_mov_byte_deref_rax_r8(b, BL)) return false;
      /* if (!gen_sub_r64_imm32(b, RAX, 7)) return false; */
      break;
    case TYPE_U16:
      if (!gen_sub_r64_imm32(b, RAX, 2)) return false;
      if (!gen_mov_word_deref_rax_r16(b, BX)) return false;
      /* if (!gen_sub_r64_imm32(b, RAX, 6)) return false; */
      break;
    case TYPE_U32:
      if (!gen_sub_r64_imm32(b, RAX, 4)) return false;
      if (!gen_mov_dword_deref_rax_r32(b, EBX)) return false;
      /* if (!gen_sub_r64_imm32(b, RAX, 4)) return false; */
      break;
    case TYPE_U64:
      if (!gen_sub_r64_imm32(b, RAX, 8)) return false;
      if (!gen_mov_qword_deref_rax_r64(b, RBX)) return false;
      break;
    case TYPES_COUNT:
    default: assert(false && "unimplemented");
    }
  }

  size_t *func = b->fn_addrs->items + fid;
  if (!gen_call_or_append_patch(b, func)) return false;

  if (rets_fn > 0) {
    if (!gen_mov_r64_qword_deref_m64_or_append_patch(b, RAX, &b->rets_head_location)) return false;
    if (!gen_add_r64_imm32(b, RAX, rets_fn)) return false;
  }
  
  if (args_size > 0) {
    if (!gen_sub_qword_deref_m64_imm32_or_append_patch(b, &b->args_head_location, args_size)) return false;
  }
  if (rets_size > 0) {
    if (!gen_sub_qword_deref_m64_imm32_or_append_patch(b, &b->rets_head_location, rets_size)) return false;
  }
  if (lvars_size > 0) {
    if (!gen_sub_qword_deref_m64_imm32_or_append_patch(b, &b->locals_head_location, lvars_size)) return false;
  }

  for (size_t i = 0; i < fn->rets.count; i++) {
    if (!gen_xor_r64_self(b, RBX)) return false;
    Var ret = fn->rets.items[i];
    switch (ret.tid) {
    case TYPE_U8:
      if (!gen_sub_r64_imm32(b, RAX, 1)) return false;
      if (!gen_mov_bl_byte_deref_r64(b, RAX)) return false;
      /* if (!gen_sub_r64_imm32(b, RAX, 7)) return false; */
      break;
    case TYPE_U16:
      if (!gen_sub_r64_imm32(b, RAX, 2)) return false;
      if (!gen_mov_bx_word_deref_r64(b, RAX)) return false;
      /* if (!gen_sub_r64_imm32(b, RAX, 6)) return false; */
      break;
    case TYPE_U32:
      if (!gen_sub_r64_imm32(b, RAX, 4)) return false;
      if (!gen_mov_ebx_dword_deref_r64(b, RAX)) return false;
      /* if (!gen_sub_r64_imm32(b, RAX, 4)) return false; */
      break;
    case TYPE_U64:
      if (!gen_sub_r64_imm32(b, RAX, 8)) return false;
      if (!gen_mov_rbx_qword_deref_r64(b, RAX)) return false;
      break;
    case TYPES_COUNT:
    default: assert(false && "unimplemented");
    }
    if (!gen_push_r64(b, RBX)) return false;
  }

  return true;
}

TARE_DEF bool gen_func_binary(BinaryGenerator *b) {
  b->fn_addrs->items[b->fni] = b->bytes->count;
  Function *fn = b->fn;
  size_t offset = 0;
  if (!gen_mov_r64_qword_deref_m64_or_append_patch(b, RAX, &b->rets_head_location)) return false;
  for (size_t i = 0; i < fn->rets.count; i++) {
    Var ret = fn->rets.items[i];
    if (!gen_add_r64_imm32(b, RAX, offset)) return false;
    if (offset % 8 != 0) {
      if (!gen_add_r64_imm32(b, RAX, (8 - offset % 8))) return false;
    }
    if (!gen_mov_qword_deref_r64_imm32(b, RAX, ret.initial)) return false;
    switch (ret.tid) {
    case TYPE_U8:  offset = 1; break;
    case TYPE_U16: offset = 2; break;
    case TYPE_U32: offset = 4; break;
    case TYPE_U64: offset = 8; break;
    case TYPES_COUNT:
    default: assert(false && "unimplemented");
    }
  }

  if (!gen_push_r64(b, RBP)) return false;
  if (!gen_mov_rbp_r64(b, RSP)) return false;

  for (size_t i = 0; i < b->fn->count; i++) {
    Operation *op = b->fn->items + i;
    b->op = op;
    if (!gen_op_binary(b)) return false;
  }

  return true;
}

TARE_DEF bool gen_op_binary(BinaryGenerator *b) {
  Operation *op = b->op;

  switch (op->type) {
  case OP_PTR_ADD: if (!gen_ptr_add_op_bin(b)) return false; break;
  case OP_PTR_SUB: if (!gen_ptr_sub_op_bin(b)) return false; break;
  case OP_ELEM_ADD: if (!gen_elem_add_op_bin(b)) return false; break;
  case OP_ELEM_SUB: if (!gen_elem_sub_op_bin(b)) return false; break;
  case OP_READ_SIZE: if (!gen_read_size_op_bin(b, op->op)) return false; break;
  case OP_CONDITIONAL: if (!gen_conditional_op_bin(b)) return false; break;
  case OP_GOTO: if (!gen_goto_op_bin(b)) return false; break;
  case OP_ADDRESS: patch_addr(b); break;
  case OP_FUNCALL: if (!gen_funcall_op_bin(b)) return false; break;
  case OP_RET: if (!gen_ret_op_bin(b)) return false; break;
  case OP_WRITE: if (!gen_write_op_bin(b)) return false; break;
  case OP_READ: if (!gen_read_op_bin(b)) return false; break;
  case OP_SYSCALL: if (!gen_syscall_op_bin(b)) return false; break;
  
  case OP_TAPE: if (!gen_tape_op_bin(b)) return false; break;
  case OP_HEAD: if (!gen_head_op_bin(b)) return false; break;
  case OP_BASE: if (!gen_base_op_bin(b)) return false; break;
  case OP_INDEX: if (!gen_index_op_bin(b)) return false; break;
  case OP_LENGTH: if (!gen_length_op_bin(b)) return false; break;
    
  case OP_CONST: UNIMPL("OP_CONST", false); break;

    // TODO: FIX `push` AND `pop`
  case OP_PUSH: if (!gen_push_op_bin(b)) return false; break;
  case OP_POP: if (!gen_pop_op_bin(b)) return false; break;

  case OP_ADD: if (!gen_add_op_bin(b)) return false; break;
  case OP_SUB: if (!gen_sub_op_bin(b)) return false; break;
  case OP_MUL: if (!gen_mul_op_bin(b)) return false; break;
  case OP_DIV: if (!gen_div_op_bin(b)) return false; break;
  case OP_MOD: if (!gen_mod_op_bin(b)) return false; break;
  case OP_SHL: if (!gen_shl_op_bin(b)) return false; break;
  case OP_SHR: if (!gen_shr_op_bin(b)) return false; break;
  case OP_NOT: if (!gen_not_op_bin(b)) return false; break;

  case OP_LESS: case OP_LESS_EQUAL: case OP_GREATER: case OP_GREATER_EQUAL:
  case OP_EQUAL: case OP_NOT_EQUAL:
    if (!gen_comparison_op_bin(b)) return false;
    break;
  case OP_BITWISE_AND: case OP_BITWISE_OR:
    if (!gen_bitwise_bin(b)) return false;
    break;
  case OP_LOGICAL_AND: case OP_LOGICAL_OR:
    if (!gen_logical_bin(b)) return false;
    break;

  case OP_DEREF: if (!gen_deref_op_bin(b)) return false; break;

  case OP_NUM: if (!gen_num_op_bin(b)) return false; break;
  
  case OP_POP_FROM_OPS: if (!gen_op_pop_from_ops_bin(b)) return false; break;

  case OP_ASSIGN: if (!gen_assign_op_bin(b)) return false; break;

  case OP_GVID: if (!gen_gvid_op_bin(b)) return false; break;
  case OP_LVID: if (!gen_lvid_op_bin(b)) return false; break;
  case OP_RVID: if (!gen_rvid_op_bin(b)) return false; break;
  case OP_AVID: if (!gen_avid_op_bin(b)) return false; break;
    
  case OP_TYPES: default: assert(false && "unreachable");
  }
  
  return true;
}

TARE_DEF bool gen_ptr_add_op_bin(BinaryGenerator *b) {
  // First argument
  if (!gen_pop_r64(b, RAX)) return false;

  // TODO: get rid of this hack
  if ((b->op - 1)->type != OP_INDEX) {
    if (b->shift == 1) {
      if (!gen_shl_r64_1(b, RAX)) return false;
    } else {
      if (!gen_shl_r64_imm8(b, RAX, b->shift)) return false;
    }
  }
  if (!gen_add_qword_deref_m64_r64_or_append_patch(b, &b->tape_head_location, RAX)) return false;

  if (!gen_push_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_ptr_sub_op_bin(BinaryGenerator *b) {
  // First argument
  if (!gen_pop_r64(b, RAX)) return false;

  // TODO: get rid of this hack
  if ((b->op - 1)->type != OP_INDEX) {
    if (b->shift == 1) {
      if (!gen_shl_r64_1(b, RAX)) return false;
    } else {
      if (!gen_shl_r64_imm8(b, RAX, b->shift)) return false;
    }
  }
  if (!gen_sub_qword_deref_m64_r64_or_append_patch(b, &b->tape_head_location, RAX)) return false;

  if (!gen_push_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_elem_add_op_bin(BinaryGenerator *b) {
  // First argument
  if (!gen_pop_r64(b, RAX)) return false;

  if (!gen_mov_r64_qword_deref_m64_or_append_patch(b, RBX, &b->tape_head_location)) return false;

  switch (b->r) {
  case 1: if (!gen_add_byte_deref_rbx_r8(b, AL)) return false; break;
  case 2: if (!gen_add_word_deref_rbx_r16(b, AX)) return false; break;
  case 4: if (!gen_add_dword_deref_rbx_r32(b, EAX)) return false; break;
  case 8: if (!gen_add_qword_deref_rbx_r64(b, RAX)) return false; break;
  default: assert(false && "unreachable");
  }

  if (!gen_push_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_elem_sub_op_bin(BinaryGenerator *b) {
  // First argument
  if (!gen_pop_r64(b, RAX)) return false;

  if (!gen_mov_r64_qword_deref_m64_or_append_patch(b, RBX, &b->tape_head_location)) return false;

  switch (b->r) {
  case 1: if (!gen_sub_byte_deref_rbx_r8(b, AL)) return false; break;
  case 2: if (!gen_sub_word_deref_rbx_r16(b, AX)) return false; break;
  case 4: if (!gen_sub_dword_deref_rbx_r32(b, EAX)) return false; break;
  case 8: if (!gen_sub_qword_deref_rbx_r64(b, RAX)) return false; break;
  default: assert(false && "unreachable");
  }

  if (!gen_push_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_read_size_op_bin(BinaryGenerator *b, size_t r) {
  b->r = r;
  switch (r) {
  case 1: b->shift = 0; b->mask = R8_MAX; break;
  case 2: b->shift = 1; b->mask = R16_MAX; break;
  case 4: b->shift = 2; b->mask = R32_MAX; break;
  case 8: b->shift = 3; b->mask = R64_MAX; break;
  default: diag_err(b->t, b->t->t, "invalid read size!\n"); return false;
  }
  return true;
}

TARE_DEF bool gen_conditional_op_bin(BinaryGenerator *b) {
  // First argument
  if (!gen_pop_r64(b, RAX)) return false;

  switch (b->r) {
  case 1: if (!gen_cmp_r8_imm8(b,   AL,  0)) return false; break;
  case 2: if (!gen_cmp_r16_imm16(b, AX,  0)) return false; break;
  case 4: if (!gen_cmp_r32_imm32(b, EAX, 0)) return false; break;
  case 8: if (!gen_cmp_r64_imm32(b, RAX, 0)) return false; break;
  default: assert(false && "unreachable");
  }

  size_t *addr = find_addr(b, b->fni, b->op->op);
  if (!gen_jz_or_append_patch(b, addr)) return false;
  return true;
}

TARE_DEF bool gen_goto_op_bin(BinaryGenerator *b) {
  size_t *addr = find_addr(b, b->fni, b->op->op);
  if (!gen_jmp_or_append_patch(b, addr)) return false;
  return true;
}

TARE_DEF bool gen_funcall_op_bin(BinaryGenerator *b) {
  size_t fid = b->op->op;
  if (!gen_funcall_bin(b, fid)) return false;
  return true;
}

TARE_DEF bool gen_ret_op_bin(BinaryGenerator *b) {
  if (!gen_mov_rsp_r64(b, RBP)) return false;
  if (!gen_pop_r64(b, RBP)) return false;
  gen_ret(b);
  return true;
}

TARE_DEF bool gen_write_op_bin(BinaryGenerator *b) {
  // Second argument
  if (!gen_pop_r64(b, RDX)) return false;

  if (b->shift != 0) {
    if (b->shift == 1) {
      if (!gen_shl_r64_1(b, RDX)) return false;
    } else {
      if (!gen_shl_r64_imm8(b, RDX, b->shift)) return false;
    }
  }

  // First argument
  if (!gen_pop_r64(b, RSI)) return false;

  if (!gen_mov_r64_imm32(b, RAX, 1)) return false; // write => syscall 1
  if (!gen_mov_r64_imm32(b, RDI, 1)) return false; // stdout => fd 1

  gen_syscall(b);

  if (!gen_push_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_read_op_bin(BinaryGenerator *b) {
  // Second argument
  if (!gen_pop_r64(b, RDX)) return false;

  if (b->shift != 0) {
    if (b->shift == 1) {
      if (!gen_shl_r64_1(b, RDX)) return false;
    } else {
      if (!gen_shl_r64_imm8(b, RDX, b->shift)) return false;
    }
  }

  // First argument
  if (!gen_pop_r64(b, RSI)) return false;

  if (!gen_mov_r64_imm32(b, RAX, 0)) return false; // read => syscall 0
  if (!gen_mov_r64_imm32(b, RDI, 0)) return false; // stdin => fd 0

  gen_syscall(b);

  if (!gen_push_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_syscall_op_bin(BinaryGenerator *b) {
  Reg64 regs[7] = {RAX, RDI, RSI, RDX, R10, R8, R9};
  size_t pops = b->op->op;
  while (pops--) {
    if (!gen_pop_r64(b, regs[pops])) return false;
  }
  gen_syscall(b);
  if (!gen_push_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_tape_op_bin(BinaryGenerator *b) {
  if (!gen_mov_r64_qword_deref_m64_or_append_patch(b, RAX, &b->tape_head_location)) return false;
  if (!gen_mov_r64_qword_deref_rax(b, RAX)) return false;
  if (!gen_push_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_head_op_bin(BinaryGenerator *b) {
  if (!gen_mov_r64_qword_deref_m64_or_append_patch(b, RAX, &b->tape_head_location)) return false;
  if (!gen_push_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_base_op_bin(BinaryGenerator *b) {
  if (!gen_mov_r64_m64_or_append_patch(b, RAX, &b->tape_start)) return false;
  if (!gen_push_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_index_op_bin(BinaryGenerator *b) {
  if (!gen_mov_r64_qword_deref_m64_or_append_patch(b, RAX, &b->tape_head_location)) return false;
  if (!gen_sub_r64_m64_or_append_patch(b, RAX, &b->tape_start)) return false;
  if (!gen_push_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_length_op_bin(BinaryGenerator *b) {
  if (!gen_mov_r64_imm32(b, RAX, b->tape_size)) return false;
  if (!gen_push_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_push_op_bin(BinaryGenerator *b) {
  if (!gen_push_qword_deref_r64(b, RBX)) return false;
  return true;
}

TARE_DEF bool gen_pop_op_bin(BinaryGenerator *b) {
  if (!gen_pop_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_add_op_bin(BinaryGenerator *b) {
  // Second argument
  if (!gen_pop_r64(b, RBX)) return false;
  
  // First argument
  if (!gen_pop_r64(b, RAX)) return false;

  if (!gen_add_rax_r64(b, RBX)) return false;
  if (!gen_push_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_sub_op_bin(BinaryGenerator *b) {
  // Second argument
  if (!gen_pop_r64(b, RBX)) return false;
  
  // First argument
  if (!gen_pop_r64(b, RAX)) return false;

  if (!gen_sub_rax_r64(b, RBX)) return false;
  if (!gen_push_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_mul_op_bin(BinaryGenerator *b) {
  // Second argument
  if (!gen_pop_r64(b, RBX)) return false;

  // First argument
  if (!gen_pop_r64(b, RAX)) return false;

  if (!gen_mul_r64(b, RBX)) return false;
  if (!gen_push_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_div_op_bin(BinaryGenerator *b) {
  // Second argument
  if (!gen_pop_r64(b, RBX)) return false;

  // First argument
  if (!gen_pop_r64(b, RAX)) return false;
  
  if (!gen_xor_r64_self(b, RDX)) return false;
  if (!gen_div_r64(b, RBX)) return false;
  if (!gen_push_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_mod_op_bin(BinaryGenerator *b) {
  // Second argument
  if (!gen_pop_r64(b, RBX)) return false;

  // First argument
  if (!gen_pop_r64(b, RAX)) return false;
  
  if (!gen_xor_r64_self(b, RDX)) return false;
  if (!gen_div_r64(b, RBX)) return false;
  if (!gen_push_r64(b, RDX)) return false;
  return true;
}

TARE_DEF bool gen_shl_op_bin(BinaryGenerator *b) {
  // Second argument
  if (!gen_pop_r64(b, RAX)) return false;
  if (!gen_mov_cl_r8(b, AL)) return false;
  
  // First argument
  if (!gen_pop_r64(b, RAX)) return false;

  if (!gen_shl_r64_cl(b, RAX)) return false;
  if (!gen_push_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_shr_op_bin(BinaryGenerator *b) {
  // Second argument
  if (!gen_pop_r64(b, RAX)) return false;
  if (!gen_mov_cl_r8(b, AL)) return false;
  
  // First argument
  if (!gen_pop_r64(b, RAX)) return false;

  if (!gen_shr_r64_cl(b, RAX)) return false;
  if (!gen_push_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_not_op_bin(BinaryGenerator *b) {
  // Get argument
  if (!gen_pop_r64(b, RAX)) return false;

  if (!gen_xor_r64_self(b, RBX)) return false;
  if (!gen_cmp_rax_r64(b, RBX)) return false;
  if (!gen_cmove_r64_qword_deref_m64_or_append_patch(b, RBX, &b->true_location)) return false;
  if (!gen_mov_rax_r64(b, RBX)) return false;

  if (!gen_push_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_comparison_op_bin(BinaryGenerator *b) {
  // Second argument
  if (!gen_pop_r64(b, RBX)) return false;

  // First argument
  if (!gen_xor_r64_self(b, RCX)) return false;
  if (!gen_pop_r64(b, RAX)) return false;
  if (!gen_cmp_rax_r64(b, RBX)) return false;

  if (b->op->type == OP_LESS) {
    if (!gen_cmovl_r64_qword_deref_m64_or_append_patch(b, RCX, &b->true_location)) return false;
  } else if (b->op->type == OP_LESS_EQUAL) {
    if (!gen_cmovle_r64_qword_deref_m64_or_append_patch(b, RCX, &b->true_location)) return false;
  } else if (b->op->type == OP_GREATER) {
    if (!gen_cmovg_r64_qword_deref_m64_or_append_patch(b, RCX, &b->true_location)) return false;
  } else if (b->op->type == OP_GREATER_EQUAL) {
    if (!gen_cmovge_r64_qword_deref_m64_or_append_patch(b, RCX, &b->true_location)) return false;
  } else if (b->op->type == OP_EQUAL) {
    if (!gen_cmove_r64_qword_deref_m64_or_append_patch(b, RCX, &b->true_location)) return false;
  } else if (b->op->type == OP_NOT_EQUAL) {
    if (!gen_cmovne_r64_qword_deref_m64_or_append_patch(b, RCX, &b->true_location)) return false;
  } else assert(false && "unreachable");

  if (!gen_push_r64(b, RCX)) return false;
  return true;
}

TARE_DEF bool gen_bitwise_bin(BinaryGenerator *b) {
  // Second argument
  if (!gen_pop_r64(b, RBX)) return false;

  // First argument
  if (!gen_pop_r64(b, RAX)) return false;

  if (b->op->type == OP_BITWISE_AND) {
    if (!gen_and_rax_r64(b, RBX)) return false;
  } else if (b->op->type == OP_BITWISE_OR) {
    if (!gen_or_rax_r64(b, RBX)) return false;
  } else assert(false && "unreachable");

  if (!gen_push_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_logical_bin(BinaryGenerator *b) {
  // Second argument
  if (!gen_pop_r64(b, RAX)) return false;

  if (!gen_xor_r64_self(b, RBX)) return false;
  if (!gen_cmp_rax_r64(b, RBX)) return false;
  if (!gen_cmovne_r64_qword_deref_m64_or_append_patch(b, RBX, &b->true_location)) return false;
  if (!gen_mov_rax_r64(b, RBX)) return false;

  // First argument
  if (!gen_pop_r64(b, RCX)) return false;

  if (!gen_xor_r64_self(b, RBX)) return false;
  if (!gen_cmp_rbx_r64(b, RCX)) return false;
  if (!gen_cmovne_r64_qword_deref_m64_or_append_patch(b, RBX, &b->true_location)) return false;
  if (!gen_mov_rcx_r64(b, RBX)) return false;

  if (b->op->type == OP_LOGICAL_AND) {
    if (!gen_and_rax_r64(b, RCX)) return false;
  } else if (b->op->type == OP_LOGICAL_OR) {
    if (!gen_or_rax_r64(b, RCX)) return false;
  } else assert(false && "unreachable");

  if (!gen_push_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_deref_op_bin(BinaryGenerator *b) {
  // First argument
  if (!gen_pop_r64(b, RAX)) return false;
  if (!gen_push_qword_deref_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_num_op_bin(BinaryGenerator *b) {
  const Token *t = b->op->start;
  Operation *operation = b->op;
  size_t op = operation->op;

  if (b->r == 1 && op > R8_MAX) {
    diag_err(b->t, t, "warning: operand of 8 bits read tries to use more than 8 bits in its operand. Please fix this if this is an error.\n");
    op %= R8_MAX;
  } else if (b->r == 2 && op > R16_MAX) {
    diag_err(b->t, t, "warning: operand of 16 bits read tries to use more than 16 bits in its operand. Please fix this if this is an error.\n");
    op %= R16_MAX;
  } else if (b->r == 4 && op > R32_MAX) {
    diag_err(b->t, t, "warning: operand of 32 bits read tries to use more than 32 bits in its operand. Please fix this if this is an error.\n");
    op %= R32_MAX;
  } else if (b->r == 8 && op > R64_MAX) {
    diag_err(b->t, t, "warning: operand of 64 bits read tries to use more than 64 bits in its operand. Please fix this if this is an error.\n");
    op %= R64_MAX;
  }

  if (op < ((R32_MAX) >> 1)) {
    if (!gen_mov_r64_imm32(b, RAX, op)) return false;
  } else {
    size_t *m64 = find_long_address(b, op);
    if (!gen_mov_r64_qword_deref_m64_or_append_patch(b, RAX, m64)) return false;

    /* size_t op2 = op >> 32; */
    /* size_t op1 = op - (op2 << 32); */

    /* if (op2 < ((R32_MAX) >> 1)) { */
    /*   if (!gen_mov_r64_imm32(b, RAX, op2)) return false; */
    /* } else { */
    /*   size_t op4 = op2 >> 16; */
    /*   size_t op3 = op2 - (op4 << 16); */
    /*   if (!gen_mov_r64_imm32(b, RAX, op4)) return false; */
    /*   if (!gen_shl_r64_imm8(b, RAX, 16)) return false; */
    /*   if (!gen_or_r64_imm32(b, RAX, op3)) return false; */
    /* } */
    /* if (!gen_shl_r64_imm8(b, RAX, 32)) return false; */
    /* if (op1 < ((R32_MAX) >> 1)) { */
    /*   if (!gen_or_r64_imm32(b, RAX, op1)) return false; */
    /* } else { */
    /*   size_t op4 = op1 >> 16; */
    /*   size_t op3 = op1 - (op4 << 16); */
    /*   if (!gen_mov_r64_imm32(b, RBX, op4)) return false; */
    /*   if (!gen_shl_r64_imm8(b, RBX, 16)) return false; */
    /*   if (!gen_or_r64_imm32(b, RBX, op3)) return false; */

    /*   if (!gen_or_rax_r64(b, RBX)) return false; */
    /* } */
  }

  if (!gen_push_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_op_pop_from_ops_bin(BinaryGenerator *b) {
  if (!gen_pop_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_assign_op_bin(BinaryGenerator *b) {
  // Second argument
  if (!gen_pop_r64(b, RBX)) return false;

  // First argument
  if (!gen_pop_r64(b, RAX)) return false;

  if (!gen_mov_qword_deref_rax_r64(b, RBX)) return false;
  return true;
}

TARE_DEF bool gen_gvid_op_bin(BinaryGenerator *b) {
  size_t op = b->op->op;
  if (!gen_mov_r64_m64_or_append_patch(b, RAX, &b->globals_start)) return false;
  /* if (op != 0) { */
  /*   if (!gen_add_r64_imm32(b, RAX, op * 8)) return false; */
  /* } */
  size_t offset = get_var_offset(op, b->globals);
  if (offset != 0) {
    if (!gen_add_r64_imm32(b, RAX, offset)) return false;
  }
  if (!gen_push_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_lvid_op_bin(BinaryGenerator *b) {
  size_t op = b->op->op;
  if (!gen_mov_r64_qword_deref_m64_or_append_patch(b, RAX, &b->locals_head_location)) return false;
  /* if (op != 0) { */
  /*   if (!gen_add_r64_imm32(b, RAX, op * 8)) return false; */
  /* } */
  size_t offset = get_var_offset(op, &b->fn->lvars);
  if (offset != 0) {
    if (!gen_add_r64_imm32(b, RAX, offset)) return false;
  }
  if (!gen_push_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_rvid_op_bin(BinaryGenerator *b) {
  size_t op = b->op->op;
  if (!gen_mov_r64_qword_deref_m64_or_append_patch(b, RAX, &b->rets_head_location)) return false;
  /* if (op != 0) { */
  /*   if (!gen_add_r64_imm32(b, RAX, op * 8)) return false; */
  /* } */
  size_t offset = get_var_offset(op, &b->fn->rets);
  if (offset != 0) {
    if (!gen_add_r64_imm32(b, RAX, offset)) return false;
  }
  if (!gen_push_r64(b, RAX)) return false;
  return true;
}

TARE_DEF bool gen_avid_op_bin(BinaryGenerator *b) {
  size_t op = b->op->op;
  if (!gen_mov_r64_qword_deref_m64_or_append_patch(b, RAX, &b->args_head_location)) return false;
  /* if (op != 0) { */
  /*   if (!gen_add_r64_imm32(b, RAX, op * 8)) return false; */
  /* } */
  size_t offset = get_var_offset(op, &b->fn->args);
  if (offset != 0) {
    if (!gen_add_r64_imm32(b, RAX, offset)) return false;
  }
  if (!gen_push_r64(b, RAX)) return false;
  return true;
}

TARE_DEF void append_address(BinaryGenerator *b, size_t fni, size_t index) {
  Address addr = { .fni = fni, .index = index, };
  da_append(b->addrs, addr);
}

TARE_DEF size_t *find_long_address(BinaryGenerator *b, size_t op) {
  size_t index = find_long(b->longs, op);
  assert(index < b->longs_locations->count && "unreachable");
  return b->longs_locations->items + index;
}

TARE_DEF size_t *find_addr(BinaryGenerator *b, size_t fni, size_t index) {
  for (size_t i = 0; i < b->addrs->count; i++) {
    Address *addr = b->addrs->items + i;
    if (addr->fni != fni) continue;
    if (addr->index != index) continue;
    return &addr->addr;
  }
  assert(false && "unreachable");
  return NULL;
}

// ***************************** Patching ******************************
TARE_DEF void patch_addr(BinaryGenerator *b) {
  size_t *addr = find_addr(b, b->fni, b->op->op);
  *addr = b->bytes->count;
}

TARE_DEF void patch_binary(BinaryGenerator *b) {
  for (size_t i = 0; i < b->patches->count; i++) {
    Patch patch = b->patches->items[i];
    size_t value = *patch.patch - patch.offset;
    char *location = b->bytes->items + patch.index;
    switch (patch.type) {
    case PATCH_TYPE_ADDRESS: break;
    case PATCH_TYPE_VIRTUAL: value += 0x4000e8; break;
    case PATCH_TYPES: default: assert(false && "unreachable"); break;
    }
    
    switch (patch.size) {
    case 4:
      *(unsigned int *) location = (unsigned int) value;
      break;
    default:
      fprintf(stderr, "patch of size %zu is unimplemented\n", patch.size);
      assert(false && "unreachable");
      break;
    }
  }
}

#endif // BINGEN_IMPLEMENTATION


#ifndef GENERATOR_H_
#define GENERATOR_H_

#define TAPE_BASE "[rbp]"
#define TAPE_HEAD "[rbp-8]"
#define READ_BYTES "[rbp-16]"
#define READ_SHL "[rbp-24]"
#define READ_MASK "[rbp-32]"

#define ARGS_HEAD "[rbp-40]"
#define ARGS_BASE "[rbp-48]"
  
#define RETS_HEAD "[rbp-56]"
#define RETS_BASE "[rbp-64]"
  
#define VARS_HEAD "[rbp-72]"
#define VARS_BASE "[rbp-80]"

#define OPS_HEAD "[rbp-88]"
#define OPS_BASE "[rbp-96]"


/* #define ARGS_HEAD "[rbp-40]" */
/* #define RETS_HEAD "[rbp-48]" */
/* #define VARS_HEAD "[rbp-56]" */
/* #define OPS_HEAD "[rbp-64]" */


// TODO: Make tape sizes compile-time and runtime variables.
#define TAPE_SIZE 1024
#define ARG_STACK_SIZE 1024
#define RET_STACK_SIZE 1024
#define VAR_STACK_SIZE 1024
#define OPS_STACK_SIZE 1024
  
#define TAPE_NAME "tape"
#define ARGS_NAME "function_arguments_tape"
#define RETS_NAME "return_values_tape"
#define VARS_NAME "global_variables_tape"
#define OPS_NAME  "operations_tape"

typedef struct {
  Longs *longs; // Numbers too big to be passed directly to registers.
  Longs *gotos; // Current goto, e.g. in a loop.
  bool fail;
  size_t r; // current read size in bytes
  Tokenizer *t;
  String errors; // Doesn't really make much sense anymore.
  Longs error_indices; // Doesn't really make much sense anymore.
  Function *fn;
  Operation *op;
  Functions *fns;
  size_t fni; // current function index
  Funcs *funcs;
} Generator;

TARE_DEF bool gen_fasm(Parser *p, const char *output);

TARE_DEF bool gen_func(Generator *g, FILE *f);
TARE_DEF bool gen_op(Generator *g, FILE *f);

TARE_DEF void gen_ptr_add_op(Generator *g, FILE *f);
/* TARE_DEF bool gen_ptr_sub_op(Generator *g, FILE *f); */
TARE_DEF void gen_elem_add_op(Generator *g, FILE *f);
/* TARE_DEF bool gen_elem_sub_op(Generator *g, FILE *f); */
TARE_DEF bool gen_read_size_op(Generator *g, FILE *f, size_t r);
TARE_DEF void gen_conditional_op(Generator *g, FILE *f);
TARE_DEF void gen_goto_op(Generator *g, FILE *f);
TARE_DEF void gen_address_op(Generator *g, FILE *f);
TARE_DEF void gen_funcall_op(Generator *g, FILE *f);
TARE_DEF void gen_ret_op(Generator *g, FILE *f);
TARE_DEF void gen_write_op(Generator *g, FILE *f);
/* TARE_DEF bool gen_read_op(Generator *g, FILE *f); */
/* TARE_DEF bool gen_syscall_op(Generator *g, FILE *f); */

TARE_DEF void gen_tape_op(Generator *g, FILE *f);
/* TARE_DEF bool gen_head_op(Generator *g, FILE *f); */
TARE_DEF void gen_base_op(Generator *g, FILE *f);
/* TARE_DEF bool gen_index_op(Generator *g, FILE *f); */
/* TARE_DEF bool gen_const_op(Generator *g, FILE *f); */

/* TARE_DEF bool gen_push_op(Generator *g, FILE *f); */
/* TARE_DEF bool gen_pop_op(Generator *g, FILE *f); */

/* TARE_DEF bool gen_add_op(Generator *g, FILE *f); */
/* TARE_DEF bool gen_sub_op(Generator *g, FILE *f); */
/* TARE_DEF bool gen_mul_op(Generator *g, FILE *f); */
/* TARE_DEF bool gen_div_op(Generator *g, FILE *f); */
/* TARE_DEF bool gen_shl_op(Generator *g, FILE *f); */
/* TARE_DEF bool gen_shr_op(Generator *g, FILE *f); */
/* TARE_DEF bool gen_deref_op(Generator *g, FILE *f); */

/* TARE_DEF bool gen_arg_op(Generator *g, FILE *f); */
TARE_DEF void gen_num_op(Generator *g, FILE *f, bool mul, bool mask);

/* TARE_DEF bool gen_op_types_op(Generator *g, FILE *f); */

/* TARE_DEF void gen_conditional_op(Generator *gen, FILE *f, bool jz); */

// TODO: handle these functions
/* TARE_DEF const char *get_error_message(const Tokenizer *t, String *errors, Longs *error_indices); */
TARE_DEF void check_failure(const Tokenizer *t, String *errors, Longs *error_indices, size_t r, bool *fail);
/* TARE_DEF void generator_check_failure(Generator *gen); */

TARE_DEF size_t find_long(Longs *longs, size_t op);

#define R8_MAX ((size_t) 1<<8) - 1
#define R16_MAX ((size_t) 1<<16) - 1
#define R32_MAX ((size_t) 1<<32) - 1
#define R64_MAX (size_t) -1

#endif // GENERATOR_H_
#ifdef GENERATOR_IMPLEMENTATION

TARE_DEF bool gen_fasm(Parser *p, const char *output) {
  if (p == NULL || output == NULL) return false;
  
  Tokenizer *t = p->t;
  first_token(t);
  
  FILE *f = fopen(output, "wb");
  if (f == NULL) {
    fprintf(stderr, "error: couldn't open file %s\n", output);
  }

  Functions *fns = p->funcs;

  Longs longs = {0};
  Longs gotos = {0};
  Generator gen = {.longs = &longs, .t = t,
                   .fn = fns->items, .op = fns->items->items, .fns = fns,
                   .funcs = p->fns, .gotos = &gotos};

  // Boilerplate
  fprintf(f, "format ELF64 executable 3\n");
  fprintf(f, "entry start\n");
  fprintf(f, "start:\n");

  // Tare runtime
  fprintf(f, "push " TAPE_NAME "\n"); // base
  fprintf(f, "mov rbp, rsp\n");       // point rbp at the right place
  fprintf(f, "push " TAPE_NAME "\n"); // head
  fprintf(f, "push 8\n");             // read size in bytes (for addition)
  fprintf(f, "push 3\n");             // read size bit shifting
  fprintf(f, "push %zu\n", R64_MAX);  // read size bit mask
  
  fprintf(f, "push " ARGS_NAME "\n"); // args base
  fprintf(f, "push " ARGS_NAME "\n"); // args head
  
  fprintf(f, "push " RETS_NAME "\n"); // rets base
  fprintf(f, "push " RETS_NAME "\n"); // rets head
  
  fprintf(f, "push " VARS_NAME "\n"); // vars base
  fprintf(f, "push " VARS_NAME "\n"); // vars head
  
  fprintf(f, "push " OPS_NAME "\n");  // ops base
  fprintf(f, "push " OPS_NAME "\n");  // ops head

/* #define TAPE_NAME "tape" */
/* #define ARGS_NAME "function_arguments_stack" */
/* #define RETS_NAME "return_values_stack" */
/* #define VARS_NAME "global_variables_stack" */
/* #define OPS_NAME  "operations_stack" */

  fprintf(f, "call func_0\n");
  
  /* fprintf(f, "mov rdi, rax\n"); */
  
  fprintf(f, "mov rax, 60\n");
  
  /* fprintf(f, "mov rdi, 0\n"); */
  /* fprintf(f, "pop rdi\n"); */
  /* if (funcs.items[0].rets.count == 0) */
  /*   fprintf(f, "mov QWORD " RETS_HEAD ", 0\n"); */
  
  /* fprintf(f, "mov rdi, QWORD " RETS_HEAD "\n"); */
  /* fprintf(f, "mov rdi, [rdi]\n"); */
  fprintf(f, "mov rdi, 0\n");
  
  fprintf(f, "syscall\n");

  gen.r = 8;

  for (size_t i = 0; i < gen.fns->count; i++) {
    gen.fni = i;
    gen.fn = gen.fns->items + i;
    gen_func(&gen, f);
  }
  
  fprintf(f, "segment readable writeable\n");
  fprintf(f, TAPE_NAME " db %d dup (0)\n", TAPE_SIZE);

  
  fprintf(f, ARGS_NAME " db %d dup (0)\n", ARG_STACK_SIZE);
  fprintf(f, RETS_NAME " db %d dup (0)\n", RET_STACK_SIZE);
  fprintf(f, VARS_NAME " db %d dup (0)\n", VAR_STACK_SIZE);
  fprintf(f, OPS_NAME  " db %d dup (0)\n", OPS_STACK_SIZE);
  
  if (longs.count > 0) fprintf(f, "segment readable\n");
  for (size_t i = 0; i < longs.count; i++) {
    fprintf(f, "long_imm_%zu dq %zu\n", i, longs.items[i]);
  }

  /* size_t bytes = -1; */
  /* bytes += ((size_t)1 << (g->r*8)); */

  fprintf(f, "read_size_max_1 dq %zu\n", R8_MAX);
  fprintf(f, "read_size_max_2 dq %zu\n", R16_MAX);
  fprintf(f, "read_size_max_4 dq %zu\n", R32_MAX);
  fprintf(f, "read_size_max_8 dq %zu\n", R64_MAX);
  
  fclose(f);
  if (!gen.fail) printf("Successfully generated file %s\n", output);

  /* for (size_t i = 0 ; i < funcs.count; i++) { */
  /*   Func fn = funcs.items[i]; */
  /*   if (fn.args.items) free(fn.args.items); */
  /*   if (fn.rets.items) free(fn.rets.items); */
  /* } */

  /* if (funcs.items) free(funcs.items); */
  
  return !gen.fail;
}

TARE_DEF bool gen_func(Generator *g, FILE *f) {
  if (g == NULL || f == NULL) return false;

  Func *fn = g->funcs->items + g->fni;
  fprintf(f, ";; --------------------------------------------------\n");
  fprintf(f, ";; %.*s\n", SV_ARG(fn->name));
  fprintf(f, ";; --------------------------------------------------\n");
  fprintf(f, "func_%zu:\n", g->fni);
  
  for (size_t i = 0; i < g->fn->count; i++) {
    Operation *op = g->fn->items + i;
    g->op = op;
    if (!gen_op(g, f)) return false;
  }
  
  return true;
}

TARE_DEF bool gen_op(Generator *g, FILE *f) {
  if (g == NULL || f == NULL) return false;
  Tokenizer *t = g->t;
  Operation *op = g->op;
  t->t = op->start;

  bool noisy = true;
  if (noisy) fprintf(f, ";; %.*s\n", TOK_ARG(op->start));

  switch (op->type) {
  case OP_PTR_ADD: gen_ptr_add_op(g, f); break;
  case OP_PTR_SUB: unimpl("OP_PTR_SUB"); break;
  case OP_ELEM_ADD: gen_elem_add_op(g, f); break;
  case OP_ELEM_SUB: unimpl("OP_ELEM_SUB"); break;
  case OP_READ_SIZE: if (!gen_read_size_op(g, f, op->op)) return false; break;
  case OP_CONDITIONAL: gen_conditional_op(g, f); break;
    unimpl("OP_CONDITIONAL"); break;
  case OP_GOTO: gen_goto_op(g, f); break;
    unimpl("OP_GOTO"); break;
  case OP_ADDRESS: gen_address_op(g, f); break;
    unimpl("OP_ADDRESS"); break;
  case OP_FUNCALL: gen_funcall_op(g, f); break;
    unimpl("OP_FUNCALL"); break;
  case OP_RET: gen_ret_op(g, f); break;
  case OP_WRITE: gen_write_op(g, f); break;
  case OP_READ: unimpl("OP_READ"); break;
  case OP_SYSCALL: unimpl("OP_SYSCALL"); break;
  
  case OP_TAPE: gen_tape_op(g, f); break;
    unimpl("OP_TAPE"); break;
  case OP_HEAD: unimpl("OP_HEAD"); break;
  case OP_BASE: gen_base_op(g, f); break;
  case OP_INDEX: unimpl("OP_INDEX"); break;
  case OP_CONST: unimpl("OP_CONST"); break;

  case OP_PUSH: unimpl("OP_PUSH"); break;
  case OP_POP: unimpl("OP_POP"); break;

  case OP_ADD: unimpl("OP_ADD"); break;
  case OP_SUB: unimpl("OP_SUB"); break;
  case OP_MUL: unimpl("OP_MUL"); break;
  case OP_DIV: unimpl("OP_DIV"); break;
  case OP_SHL: unimpl("OP_SHL"); break;
  case OP_SHR: unimpl("OP_SHR"); break;
  case OP_DEREF: unimpl("OP_DEREF"); break;

  case OP_ARG: unimpl("OP_ARG"); break;
  case OP_NUM: gen_num_op(g, f, false, false); break; // mul, mask
  
  case OP_TYPES: unimpl("OP_TYPES"); break;
  }
  
  return true;
}

TARE_DEF void gen_ptr_add_op(Generator *g, FILE *f) {
  fprintf(f, "sub QWORD " OPS_HEAD ", 8\n");
  fprintf(f, "mov QWORD rcx, QWORD " OPS_HEAD "\n");
  fprintf(f, "mov QWORD rax, QWORD [rcx]\n");
  
  /* fprintf(f, "pop QWORD rax\n"); */
  
  fprintf(f, "mov cl, " READ_SHL "\n");
  fprintf(f, "shl QWORD rax, cl\n");
  fprintf(f, "add QWORD " TAPE_HEAD ", QWORD rax\n");
  (void)g; // ehhh???
}

/* TARE_DEF bool gen_ptr_sub_op(Generator *g, FILE *f); */

TARE_DEF void gen_elem_add_op(Generator *g, FILE *f) {
  fprintf(f, "sub QWORD " OPS_HEAD ", 8\n");
  fprintf(f, "mov QWORD rcx, QWORD " OPS_HEAD "\n");
  fprintf(f, "mov QWORD rax, QWORD [rcx]\n");
  
  /* fprintf(f, "pop QWORD rax\n"); */
  
  fprintf(f, "and QWORD rax, QWORD " READ_MASK "\n");
  
  fprintf(f, "mov QWORD rcx, QWORD " TAPE_HEAD "\n");
  fprintf(f, "add QWORD [rcx], QWORD rax\n");
  (void)g; // ehhh???
}

/* TARE_DEF bool gen_elem_sub_op(Generator *g, FILE *f); */

TARE_DEF bool gen_read_size_op(Generator *g, FILE *f, size_t r) {
  g->r = r;
  switch (g->r) {
  case 1: r = 0; break;
  case 2: r = 1; break;
  case 4: r = 2; break;
  case 8: r = 3; break;
  default: diag_err(g->t, g->t->t, "invalid read size!\n"); return false;
  }
  
  fprintf(f, "mov QWORD " READ_BYTES ", %zu\n", (size_t) (1 << r));
  fprintf(f, "mov QWORD " READ_SHL ", %zu\n", r);

  size_t bytes = -1;
  if (g->r < 4) {
    bytes += ((size_t)1 << (g->r*8));
    fprintf(f, "mov QWORD " READ_MASK ", %zu\n", bytes);
  } else {
    fprintf(f, "mov QWORD rax, QWORD [read_size_max_%zu]\n", g->r);
    fprintf(f, "mov QWORD " READ_MASK ", QWORD rax\n");
  }
  return true;
}

TARE_DEF void gen_conditional_op(Generator *g, FILE *f) {
  fprintf(f, "sub QWORD " OPS_HEAD ", 8\n");
  fprintf(f, "mov QWORD rcx, QWORD " OPS_HEAD "\n");
  fprintf(f, "mov QWORD rax, QWORD [rcx]\n");
  fprintf(f, "and QWORD rax, QWORD " READ_MASK "\n");

  fprintf(f, "cmp QWORD rax, 0\n");
  fprintf(f, "jz addr_%zu\n", g->op->op);
  
  /* Operation *op = g->op; */
  /* if (jz) fprintf(f, "jz addr_%zu\n", op->op); */
  /* else fprintf(f, "jnz addr_%zu\n", op->op); */
  
  (void)g;
}

TARE_DEF void gen_goto_op(Generator *g, FILE *f) {
  fprintf(f, "jmp addr_%zu\n", g->op->op);
}

TARE_DEF void gen_address_op(Generator *g, FILE *f) {
  fprintf(f, "addr_%zu:\n", g->op->op);
}

TARE_DEF void gen_funcall_op(Generator *g, FILE *f) {
  // TODO: handle this properly
  fprintf(f, "call func_%zu\n", g->op->op);
}

TARE_DEF void gen_ret_op(Generator *g, FILE *f) {
  fprintf(f, "ret\n");
  (void)g; // ehhh???
}

TARE_DEF void gen_write_op(Generator *g, FILE *f) {
  fprintf(f, "sub QWORD " OPS_HEAD ", 8\n");
  fprintf(f, "mov QWORD rcx, QWORD " OPS_HEAD "\n");
  fprintf(f, "mov QWORD rdx, QWORD [rcx]\n");
  /* fprintf(f, "pop QWORD rdx\n"); */
  
  fprintf(f, "mov cl, " READ_SHL "\n");
  fprintf(f, "shl QWORD rdx, cl\n");
  
  fprintf(f, "sub QWORD " OPS_HEAD ", 8\n");
  fprintf(f, "mov QWORD rcx, QWORD " OPS_HEAD "\n");
  fprintf(f, "mov QWORD rsi, QWORD [rcx]\n");
  /* fprintf(f, "pop QWORD rsi\n"); */
  
  fprintf(f, "mov rax, 1\n"); // write => syscall 1
  fprintf(f, "mov rdi, 1\n"); // stdout => fd 1
  
  fprintf(f, "syscall\n");
  (void)g; // ehhh???
}

/* TARE_DEF bool gen_read_op(Generator *g, FILE *f); */

/* TARE_DEF bool gen_syscall_op(Generator *g, FILE *f); */

TARE_DEF void gen_tape_op(Generator *g, FILE *f) {
  fprintf(f, "mov QWORD rax, QWORD " TAPE_BASE "\n");
  fprintf(f, "mov QWORD rax, QWORD [rax]\n");
  /* fprintf(f, "and QWORD rax, QWORD " READ_MASK "\n"); */
  
  fprintf(f, "mov QWORD rcx, QWORD " OPS_HEAD "\n");
  fprintf(f, "mov QWORD [rcx], QWORD rax\n");
  fprintf(f, "add QWORD " OPS_HEAD ", 8\n");

  (void)g;
}

/* TARE_DEF bool gen_head_op(Generator *g, FILE *f); */

TARE_DEF void gen_base_op(Generator *g, FILE *f) {
  fprintf(f, "mov QWORD rax, QWORD " TAPE_BASE "\n");
  
  fprintf(f, "mov QWORD rcx, QWORD " OPS_HEAD "\n");
  fprintf(f, "mov QWORD [rcx], QWORD rax\n");
  fprintf(f, "add QWORD " OPS_HEAD ", 8\n");
  
  /* fprintf(f, "push QWORD rax\n"); */
  (void)g; // ehhh???
}

/* TARE_DEF bool gen_index_op(Generator *g, FILE *f); */

/* TARE_DEF bool gen_const_op(Generator *g, FILE *f); */

/* TARE_DEF bool gen_push_op(Generator *g, FILE *f); */

/* TARE_DEF bool gen_pop_op(Generator *g, FILE *f); */


/* TARE_DEF bool gen_add_op(Generator *g, FILE *f); */

/* TARE_DEF bool gen_sub_op(Generator *g, FILE *f); */

/* TARE_DEF bool gen_mul_op(Generator *g, FILE *f); */

/* TARE_DEF bool gen_div_op(Generator *g, FILE *f); */

/* TARE_DEF bool gen_shl_op(Generator *g, FILE *f); */

/* TARE_DEF bool gen_shr_op(Generator *g, FILE *f); */

/* TARE_DEF bool gen_deref_op(Generator *g, FILE *f); */

/* TARE_DEF bool gen_arg_op(Generator *g, FILE *f); */

TARE_DEF void gen_num_op(Generator *g, FILE *f, bool mul, bool mask) {
  const Token *t = g->t->t;
  Operation *operation = g->op;
  size_t op = operation->op;
  
  if (g->r == 1 && op > R8_MAX) {
    diag_err(g->t, t, "warning: operand of 8 bits read tries to use more than 8 bits in its operand. Please fix this if this is an error.\n");
    op %= R8_MAX;
  } else if (g->r == 2 && op > R16_MAX) {
    diag_err(g->t, t, "warning: operand of 16 bits read tries to use more than 16 bits in its operand. Please fix this if this is an error.\n");
    op %= R16_MAX;
  } else if (g->r == 4 && op > R32_MAX) {
    diag_err(g->t, t, "warning: operand of 32 bits read tries to use more than 32 bits in its operand. Please fix this if this is an error.\n");
    op %= R32_MAX;
  } else if (g->r == 8 && op > R64_MAX) {
    diag_err(g->t, t, "warning: operand of 64 bits read tries to use more than 64 bits in its operand. Please fix this if this is an error.\n");
    op %= R64_MAX;
  }

  if (op < R32_MAX) fprintf(f, "mov QWORD rax, %zu\n", op);
  else {
    size_t index = find_long(g->longs, op);
    fprintf(f, "mov QWORD rax, [long_imm_%zu]\n", index);
  }

  if (mul) {
    fprintf(f, "mov cl, " READ_SHL "\n");
    fprintf(f, "shl QWORD rax, cl\n");
  }
  
  if (mask) fprintf(f, "and QWORD rax, QWORD " READ_MASK "\n");

  fprintf(f, "mov QWORD rcx, QWORD " OPS_HEAD "\n");
  fprintf(f, "mov QWORD [rcx], QWORD rax\n");
  fprintf(f, "add QWORD " OPS_HEAD ", 8\n");
  
  /* fprintf(f, "push QWORD rax\n"); */
}

/* TARE_DEF void gen_conditional_op(Generator *gen, FILE *f, bool jz) { */
/*   fprintf(f, "mov QWORD rax, QWORD [rsi]\n"); */
/*   switch (gen->r) { */
/*   case 1: fprintf(f, "test BYTE al, BYTE al\n"); break; */
/*   case 2: fprintf(f, "test WORD ax, WORD ax\n"); break; */
/*   case 4: fprintf(f, "test DWORD eax, DWORD eax\n"); break; */
/*   case 8: fprintf(f, "test QWORD rax, QWORD rax\n"); break; */
/*   default: */
/*     print_loc(stderr, gen->t, gen->t->t); */
/*     fprintf(stderr, "error: Invalid read size!\n"); */
/*     break; */
/*   } */
/*   if (jz) fprintf(f, "jz addr_%zu\n", gen->op->args->op); */
/*   else fprintf(f, "jnz addr_%zu\n", gen->op->args->op); */
/* } */

/* TARE_DEF const char *get_error_message(const Tokenizer *t, String *errors, Longs *error_indices) { */
/*   switch (t->t->k) { */
/*   case KEY_F: return "Can't move the pointer forward when the read size is undetermined!"; */
/*   case KEY_B: return "Can't move the pointer backward when the read size is undetermined!"; */
/*   case KEY_N: return "Can't move the pointer to the next element when the read size is undetermined!"; */
/*   case KEY_P: return "Can't move the pointer to the previous element when the read size is undetermined!"; */
/*   case KEY_A: return "Can't add to the element pointed at when the read size is undetermined!"; */
/*   case KEY_S: return "Can't subtract from the element pointed at when the read size is undetermined!"; */
/*   case KEY_I: return "Can't increment the element pointed at when the read size is undetermined!"; */
/*   case KEY_D: return "Can't decrement the element pointed at when the read size is undetermined!"; */
/*   case KEY_R8: return NULL; */
/*   case KEY_R16: return NULL; */
/*   case KEY_R32: return NULL; */
/*   case KEY_R64: return NULL; */
/*   case KEY_IF: return "Can't act on the if condition pointed at when the read size is undetermined!"; */
/*   case KEY_WHILE: return "Can't act on the while condition pointed at when the read size is undetermined!"; */
/*   case KEY_END: return NULL; // TODO: is this actually fine? */
/*     return NULL; */
/*   case KEY_FUNC: */
/*     return "error message for KEY_FUNC is unimplemented"; */
/*     break; */
/*   case KEY_RET: */
/*     return "error message for KEY_RET is unimplemented"; */
/*     break; */
/*   case KEY_SYSCALL: */
/*     // There should probably some error state. */
/*     return "error message for KEY_SYSCALL is unimplemented"; */
/*   case KEY_WRITE: */
/*     // There should probably some error state. */
/*     return "error message for KEY_WRITE is unimplemented"; */
/*   case KEY_READ: */
/*     // There should probably some error state. */
/*     return "error message for KEY_READ is unimplemented"; */
/*   /\* case KEY_START: *\/ */
/*   /\*   return "unsupported"; *\/ */
/*     /\* unimpl("KEY_START"); break; *\/ */
/*   case KEY_TAPE: */
/*     return "Can't read from the tape when the read size is undetermined!\n"; */
/*     /\* unimpl("KEY_TAPE"); break; *\/ */
/*   case KEY_HEAD: */
/*     return "Can't read the head when the read size is undetermined!\n"; */
/*     /\* unimpl("KEY_HEAD"); break; *\/ */
/*   case KEY_BASE: */
/*     return "Can't read the base when the read size is undetermined!\n"; */
/*     /\* unimpl("KEY_BASE"); break; *\/ */
/*   case KEY_INDEX: */
/*     return "Can't read the index when the read size is undetermined!\n"; */
/*     /\* unimpl("KEY_INDEX"); break; *\/ */
/*   case KEY_CONST: */
/*     return NULL; */
/*     /\* unimpl("KEY_CONST"); break; *\/ */
/*   case KEYWORD_TYPES: */
/*     /\* unimpl("KEYWORD_TYPES"); break; *\/ */
/*     { */
/*       size_t extra_error = errors->count; */
/*       if (!append_cstr_to_string(errors, "invalid keyword")) */
/*         return "error while generating error message with c string"; */
/*       if (!append_sv_to_string(errors, sv_from_token(t->t))) */
/*         return "error while generating error message with string view"; */
/*       da_append(errors, 0); */

/*       const char *invalid_keyword_error_message = errors->items + extra_error; */
    
/*       for (size_t i = 0; i < error_indices->count; i++) { */
/*         size_t index = error_indices->items[i]; */
/*         const char *match = errors->items + index; */
/*         if (strcmp(invalid_keyword_error_message, match) == 0) { */
/*           errors->count = extra_error; */
/*           return match; */
/*         } */
/*       } */
/*       return invalid_keyword_error_message; */
/*     } */
/*   default: */
/*     { */
/*       size_t invalid_token = errors->count; */
/*       if (!append_cstr_to_string(errors, "keyword type token ")) */
/*         return "error while generating error message with c string"; */
/*       da_append(errors, '`'); */
/*       if (!append_sv_to_string(errors, sv_from_token(t->t))) */
/*         return "error while generating error message with string view"; */
/*       da_append(errors, '`'); */
    
/*       const char *invalid_token_error_message = errors->items + invalid_token; */
    
/*       for (size_t i = 0; i < error_indices->count; i++) { */
/*         size_t index = error_indices->items[i]; */
/*         const char *match = errors->items + index; */
/*         if (strcmp(invalid_token_error_message, match) == 0) { */
/*           errors->count = invalid_token; */
/*           return match; */
/*         } */
/*       } */
/*       return invalid_token_error_message; */
/*     } */
/*   } */
/* } */

TARE_DEF void check_failure(const Tokenizer *t, String *errors, Longs *error_indices, size_t r, bool *fail) {
  /* const char *error_message = get_error_message(t, errors, error_indices); */
  const char *error_message = "";(void)errors;(void)error_indices;
  if (r > 0 || error_message == NULL) return;
  print_loc(stderr, t, t->t);
  fprintf(stderr, "error: %s\n", error_message);
  *fail = true;
}

TARE_DEF size_t find_long(Longs *longs, size_t op) {
  for (size_t i = 0; i < longs->count; i++) if (longs->items[i] == op) return i;
  da_append(longs, op);
  return longs->count - 1;
}

#endif // GENERATOR_IMPLEMENTATION

#ifndef CODEGEN_H_
#define CODEGEN_H_

#define TAPE_BASE TAPE_NAME

#define TAPE_HEAD_NAME "tape_head"
#define TAPE_HEAD "[" TAPE_HEAD_NAME "]"

#define ARGS_HEAD_NAME "args_head"
#define ARGS_HEAD "[" ARGS_HEAD_NAME "]"

#define RETS_HEAD_NAME "rets_head"
#define RETS_HEAD "[" RETS_HEAD_NAME "]"

/* #define GVARS_HEAD_NAME "global_vars_head" */
/* #define GVARS_HEAD "[" GVARS_HEAD_NAME "]" */

#define LVARS_HEAD_NAME "local_vars_head"
#define LVARS_HEAD "[" LVARS_HEAD_NAME "]"

// TODO: Make tape sizes compile-time and runtime variables.
#define TAPE_SIZE 1024
#define ARG_STACK_SIZE 1024
#define RET_STACK_SIZE 1024
#define GLOBAL_VAR_STACK_SIZE 1024
#define LOCAL_VAR_STACK_SIZE 1024
  
#define TAPE_NAME "tape"
#define ARGS_NAME "function_arguments_tape"
#define RETS_NAME "return_values_tape"
#define GLOBAL_VARS_NAME "global_variables_tape"
#define LOCAL_VARS_NAME "local_variables_tape"

typedef struct {
  Longs *longs; // Numbers too big to be passed directly to registers.
  Longs *gotos; // Current goto, e.g. in a loop.
  /* bool fail; */
  struct {
    size_t r; // current read size in bytes
    size_t mask;
    size_t shift;
  };
  Tokenizer *t;
  String errors; // Doesn't really make much sense anymore.
  Longs error_indices; // Doesn't really make much sense anymore.
  Function *fn;
  Operation *op;
  Functions *fns;
  size_t fni; // current function index
} Generator;

TARE_DEF bool gen_fasm(Parser *p, const char *output);

TARE_DEF bool gen_func(Generator *g, FILE *f);
TARE_DEF bool gen_op(Generator *g, FILE *f);

TARE_DEF void gen_ptr_add_op(Generator *g, FILE *f);
TARE_DEF void gen_ptr_sub_op(Generator *g, FILE *f);
TARE_DEF void gen_elem_add_op(Generator *g, FILE *f);
TARE_DEF void gen_elem_sub_op(Generator *g, FILE *f);
TARE_DEF bool gen_read_size_op(Generator *g, size_t r);
TARE_DEF void gen_conditional_op(Generator *g, FILE *f);
TARE_DEF void gen_goto_op(Generator *g, FILE *f);
TARE_DEF void gen_address_op(Generator *g, FILE *f);
TARE_DEF void gen_funcall(Generator *g, FILE *f, size_t fid);
TARE_DEF void gen_funcall_op(Generator *g, FILE *f);
TARE_DEF void gen_ret_op(FILE *f);
TARE_DEF void gen_write_op(Generator *g, FILE *f);
TARE_DEF void gen_read_op(Generator *g, FILE *f);
TARE_DEF void gen_syscall_op(Generator *g, FILE *f);

TARE_DEF void gen_tape_op(FILE *f);
TARE_DEF void gen_head_op(FILE *f);
TARE_DEF void gen_base_op(FILE *f);
TARE_DEF void gen_index_op(FILE *f);
/* TARE_DEF bool gen_const_op(Generator *g, FILE *f); */

TARE_DEF void gen_push_op(Generator *g, FILE *f);
TARE_DEF void gen_pop_op(Generator *g, FILE *f);

TARE_DEF void gen_add_op(FILE *f);
TARE_DEF void gen_sub_op(FILE *f);
TARE_DEF void gen_mul_op(FILE *f);
TARE_DEF void gen_div_op(FILE *f);
TARE_DEF void gen_shl_op(FILE *f);
TARE_DEF void gen_shr_op(FILE *f);
TARE_DEF void gen_not_op(FILE *f);

TARE_DEF void gen_bitwise(Generator *g, FILE *f);
TARE_DEF void gen_logical(Generator *g, FILE *f);

TARE_DEF void gen_comparison_op(Generator *g, FILE *f);

TARE_DEF void gen_deref_op(FILE *f);

TARE_DEF void gen_num_op(Generator *g, FILE *f);

TARE_DEF void gen_op_pop_from_ops(FILE *f);

TARE_DEF void gen_assign_op(FILE *f);

TARE_DEF void gen_gvid_op(Generator *g, FILE *f);
TARE_DEF void gen_lvid_op(Generator *g, FILE *f);
TARE_DEF void gen_rvid_op(Generator *g, FILE *f);
TARE_DEF void gen_avid_op(Generator *g, FILE *f);

TARE_DEF void gen_pop_from_ops_to_reg(FILE *f, const char *reg);
TARE_DEF void gen_pop_from_ops(FILE *f);
TARE_DEF void gen_push_to_ops_from_reg(FILE *f, const char *reg);
TARE_DEF void gen_push_to_ops(FILE *f);

// TODO: handle these functions
/* TARE_DEF const char *get_error_message(const Tokenizer *t, String *errors, Longs *error_indices); */
TARE_DEF void check_failure(const Tokenizer *t, String *errors, Longs *error_indices, size_t r, bool *fail);
/* TARE_DEF void generator_check_failure(Generator *gen); */

TARE_DEF size_t find_long(Longs *longs, size_t op);

#define R8_MAX ((size_t) 1<<8) - 1
#define R16_MAX ((size_t) 1<<16) - 1
#define R32_MAX ((size_t) 1<<32) - 1
#define R64_MAX (size_t) -1

#endif // CODEGEN_H_
#ifdef CODEGEN_IMPLEMENTATION

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
                   .gotos = &gotos};

  // Boilerplate
  fprintf(f, "format ELF64 executable 3\n");
  fprintf(f, "entry start\n");
  fprintf(f, "start:\n");

  // Tare runtime
  fprintf(f, "mov QWORD " TAPE_HEAD ", " TAPE_NAME "\n");
  fprintf(f, "mov QWORD " ARGS_HEAD ", " ARGS_NAME "\n");
  fprintf(f, "mov QWORD " RETS_HEAD ", " RETS_NAME "\n");
  /* fprintf(f, "mov QWORD " GVARS_HEAD ", " GVARS_HEAD_NAME "\n"); */
  fprintf(f, "mov QWORD " LVARS_HEAD ", " LOCAL_VARS_NAME "\n");


  gen_funcall(&gen, f, 0);
  if (fns->items[0].rets.count == 0) fprintf(f, "mov rdi, 0\n");
  else fprintf(f, "pop rdi\n");
  fprintf(f, "mov rax, 60\n");
  
  fprintf(f, "syscall\n");

  gen.r = 8;
  gen.mask = R64_MAX;
  gen.shift = 3;

  for (size_t i = 0; i < gen.fns->count; i++) {
    gen.fni = i;
    gen.fn = gen.fns->items + i;
    if (!gen_func(&gen, f)) return false;
  }
  
  fprintf(f, "segment readable writeable\n");
  fprintf(f, TAPE_NAME " db %d dup (0)\n", TAPE_SIZE);
  
  fprintf(f, ARGS_NAME " db %d dup (0)\n", ARG_STACK_SIZE);
  fprintf(f, RETS_NAME " db %d dup (0)\n", RET_STACK_SIZE);
  fprintf(f, GLOBAL_VARS_NAME " db %d dup (0)\n", GLOBAL_VAR_STACK_SIZE);
  fprintf(f, LOCAL_VARS_NAME " db %d dup (0)\n", LOCAL_VAR_STACK_SIZE);

  fprintf(f, TAPE_HEAD_NAME " dq 0\n");
  
  fprintf(f, ARGS_HEAD_NAME " dq 0\n");
  fprintf(f, RETS_HEAD_NAME " dq 0\n");
  /* fprintf(f, GVARS_HEAD_NAME " dq 0\n"); */
  fprintf(f, LVARS_HEAD_NAME " dq 0\n");

  if (longs.count > 0) fprintf(f, "segment readable\n");
  for (size_t i = 0; i < longs.count; i++) {
    fprintf(f, "long_imm_%zu dq %zu\n", i, longs.items[i]);
  }

  fprintf(f, "false dq 0\n");
  fprintf(f, "true dq 1\n");

  fclose(f);
  printf("Successfully generated file %s\n", output);

  if (gen.longs->items) free(gen.longs->items);
  if (gen.gotos->items) free(gen.gotos->items);
  
  return true;
}

TARE_DEF bool gen_func(Generator *g, FILE *f) {
  if (g == NULL || f == NULL) return false;

  Function *fn = g->fn;
  
  fprintf(f, ";; --------------------------------------------------\n");
  fprintf(f, ";; %.*s\n", SV_ARG(fn->name));
  fprintf(f, ";; --------------------------------------------------\n");
  fprintf(f, "func_%zu:\n", g->fni);

  size_t offset = 0;
  fprintf(f, "mov QWORD rax, QWORD " RETS_HEAD "\n");
  for (size_t i = 0; i < fn->rets.count; i++) {
    Var ret = fn->rets.items[i];
    fprintf(f, "add QWORD rax, %zu\n", offset);
    fprintf(f, "mov QWORD [rax], %zu\n", ret.initial);
    switch (ret.tid) {
    /* case TYPE_U8:  offset++; break; */
    /* case TYPE_U16: offset += 2; break; */
    /* case TYPE_U32: offset += 4; break; */
    /* case TYPE_U64: offset += 8; break; */
    case TYPE_U8:  offset = 1; break;
    case TYPE_U16: offset = 2; break;
    case TYPE_U32: offset = 4; break;
    case TYPE_U64: offset = 8; break;
    case TYPES_COUNT: 
    default: assert(false && "unimplemented");
    }
  }

  fprintf(f, "push rbp\n");
  fprintf(f, "mov rbp, rsp\n");

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
  if (noisy) {
    fprintf(f, ";; ");
    print_loc(f, t, t->t);
    fprintf(f, "%.*s\n", TOK_ARG(op->start));
  }

  switch (op->type) {
  case OP_PTR_ADD: gen_ptr_add_op(g, f); break;
  case OP_PTR_SUB: gen_ptr_sub_op(g, f); break;
  case OP_ELEM_ADD: gen_elem_add_op(g, f); break;
  case OP_ELEM_SUB: gen_elem_add_op(g, f); break;
  case OP_READ_SIZE: if (!gen_read_size_op(g, op->op)) return false; break;
  case OP_CONDITIONAL: gen_conditional_op(g, f); break;
  case OP_GOTO: gen_goto_op(g, f); break;
  case OP_ADDRESS: gen_address_op(g, f); break;
  case OP_FUNCALL: gen_funcall_op(g, f); break;
  case OP_RET: gen_ret_op(f); break;
  case OP_WRITE: gen_write_op(g, f); break;
  case OP_READ: gen_read_op(g, f); break;
  case OP_SYSCALL: gen_syscall_op(g, f); break;
  
  case OP_TAPE: gen_tape_op(f); break;
  case OP_HEAD: gen_head_op(f); break;
  case OP_BASE: gen_base_op(f); break;
  case OP_INDEX: gen_index_op(f); break;
  case OP_CONST: unimpl("OP_CONST"); break;

    // TODO: FIX `push` AND `pop`
  case OP_PUSH: gen_push_op(g, f); break;
  case OP_POP: gen_pop_op(g, f); break;

  case OP_ADD: gen_add_op(f); break;
  case OP_SUB: gen_sub_op(f); break;
  case OP_MUL: gen_mul_op(f); break;
  case OP_DIV: gen_div_op(f); break;
  case OP_SHL: gen_shl_op(f); break;
  case OP_SHR: gen_shr_op(f); break;
  case OP_NOT: gen_not_op(f); break;
  case OP_LESS: case OP_LESS_EQUAL: case OP_GREATER: case OP_GREATER_EQUAL:
  case OP_EQUAL: case OP_NOT_EQUAL: gen_comparison_op(g, f); break;
  case OP_BITWISE_AND: case OP_BITWISE_OR: gen_bitwise(g, f); break;
  case OP_LOGICAL_AND: case OP_LOGICAL_OR: gen_logical(g, f); break;

  case OP_DEREF: gen_deref_op(f); break;

  case OP_NUM: gen_num_op(g, f); break;
  
  case OP_POP_FROM_OPS: gen_op_pop_from_ops(f); break;

  case OP_ASSIGN: gen_assign_op(f); break;

  case OP_GVID: gen_gvid_op(g, f); break;
  case OP_LVID: gen_lvid_op(g, f); break;
  case OP_RVID: gen_rvid_op(g, f); break;
  case OP_AVID: gen_avid_op(g, f); break;
    
  case OP_TYPES: unimpl("OP_TYPES"); break;
  }
  
  return true;
}

// ptr_add: head += first
TARE_DEF void gen_ptr_add_op(Generator *g, FILE *f) {
  // First argument
  gen_pop_from_ops(f);
  
  // TODO: get rid of this hack
  if ((g->op - 1)->type != OP_INDEX) {
    fprintf(f, "shl QWORD rax, %zu\n", g->shift);
  }
  fprintf(f, "add QWORD " TAPE_HEAD ", QWORD rax\n");

  gen_push_to_ops(f);
}

// ptr_sub: head -= first
TARE_DEF void gen_ptr_sub_op(Generator *g, FILE *f) {
  // First argument
  gen_pop_from_ops(f);

  // TODO: get rid of this hack
  if ((g->op - 1)->type != OP_INDEX) {
    fprintf(f, "shl QWORD rax, %zu\n", g->shift);
  }
  fprintf(f, "sub QWORD " TAPE_HEAD ", QWORD rax\n");

  gen_push_to_ops(f);
}

// elem_add: [head] += first
TARE_DEF void gen_elem_add_op(Generator *g, FILE *f) {
  // First argument
  gen_pop_from_ops(f);

  fprintf(f, "mov QWORD rbx, QWORD " TAPE_HEAD "\n");
  
  switch (g->r) {
  case 1: fprintf(f, "add BYTE [rbx], al\n"); break;
  case 2: fprintf(f, "add WORD [rbx], ax\n"); break;
  case 4: fprintf(f, "add DWORD [rbx], eax\n"); break;
  case 8: fprintf(f, "add QWORD [rbx], QWORD rax\n"); break;
  default: assert(false && "unreachable");
  }

  gen_push_to_ops(f);
}

// elem_add: [head] -= first
TARE_DEF void gen_elem_sub_op(Generator *g, FILE *f) {
  // First argument
  gen_pop_from_ops(f);
  
  fprintf(f, "mov QWORD rbx, QWORD " TAPE_HEAD "\n");
  switch (g->r) {
  case 1: fprintf(f, "sub BYTE [rbx], al\n");
  case 2: fprintf(f, "sub WORD [rbx], ax\n");
  case 4: fprintf(f, "sub DWORD [rbx], eax\n");
  case 8: fprintf(f, "sub QWORD [rbx], QWORD rax\n");
  default: assert(false && "unreachable");
  }

  gen_push_to_ops(f);
}

TARE_DEF bool gen_read_size_op(Generator *g, size_t r) {
  g->r = r;
  
  switch (r) {
  case 1: g->shift = 0; g->mask = R8_MAX; break;
  case 2: g->shift = 1; g->mask = R16_MAX; break;
  case 4: g->shift = 2; g->mask = R32_MAX; break;
  case 8: g->shift = 3; g->mask = R64_MAX; break;
  default: diag_err(g->t, g->t->t, "invalid read size!\n"); return false;
  }

  return true;
}

// conditional: first ?
TARE_DEF void gen_conditional_op(Generator *g, FILE *f) {
  // First argument
  gen_pop_from_ops(f);

  switch (g->r) {
  case 1: fprintf(f, "cmp al, 0\n"); break;
  case 2: fprintf(f, "cmp ax, 0\n"); break;
  case 4: fprintf(f, "cmp eax, 0\n"); break;
  case 8: fprintf(f, "cmp QWORD rax, 0\n"); break;
  default: assert(false && "unreachable");
  }
  
  fprintf(f, "jz addr_%zu\n", g->op->op);
}

TARE_DEF void gen_goto_op(Generator *g, FILE *f) {
  fprintf(f, "jmp addr_%zu\n", g->op->op);
}

TARE_DEF void gen_address_op(Generator *g, FILE *f) {
  fprintf(f, "addr_%zu:\n", g->op->op);
}

TARE_DEF void gen_funcall(Generator *g, FILE *f, size_t fid) {
  Function *fn = g->fns->items + fid;

  size_t args_fn = get_args_size(fn);
  size_t rets_fn = get_rets_size(fn);
  
  if (args_fn % 8 != 0) args_fn += (8 - args_fn % 8);
  if (rets_fn % 8 != 0) rets_fn += (8 - rets_fn % 8);
  
  Function *fni = g->fns->items + g->fni;
  
  size_t args_size = get_args_size(fni);
  size_t rets_size = get_rets_size(fni);
  size_t lvars_size = get_lvars_size(fni);
  
  if (args_size % 8 != 0) args_size += (8 - args_size % 8);
  if (rets_size % 8 != 0) rets_size += (8 - rets_size % 8);
  if (lvars_size % 8 != 0) lvars_size += (8 - lvars_size % 8);

  if (args_size > 0) fprintf(f, "add QWORD " ARGS_HEAD ", %zu\n", args_size);
  if (rets_size > 0) fprintf(f, "add QWORD " RETS_HEAD ", %zu\n", rets_size);
  if (lvars_size > 0) fprintf(f, "add QWORD " LVARS_HEAD ", %zu\n", lvars_size);
  
  if (args_fn > 0) {
    fprintf(f, "mov QWORD rax, QWORD " ARGS_HEAD "\n");
    fprintf(f, "add QWORD rax, %zu\n", args_fn);
  }
  
  for (size_t i = 0; i < fn->args.count; i++) {
    fprintf(f, "pop QWORD rbx\n");
    Var arg = fn->args.items[i];
    switch (arg.tid) {
    case TYPE_U8:
      fprintf(f, "sub rax, 1\n");
      fprintf(f, "mov BYTE [rax], bl\n");
      break;
    case TYPE_U16:
      fprintf(f, "sub rax, 2\n");
      fprintf(f, "mov WORD [rax], bx\n");
      break;
    case TYPE_U32:
      fprintf(f, "sub rax, 4\n");
      fprintf(f, "mov DWORD [rax], ebx\n");
      break;
    case TYPE_U64:
      fprintf(f, "sub rax, 8\n");
      fprintf(f, "mov QWORD [rax], QWORD rbx\n");
      break;
    case TYPES_COUNT:
    default: assert(false && "unimplemented");
    }
  }
  
  fprintf(f, "call func_%zu\n", fid);

  if (rets_fn > 0) {
    fprintf(f, "mov QWORD rax, QWORD " RETS_HEAD "\n");
    fprintf(f, "add QWORD rax, %zu\n", rets_fn);
  }
  
  if (args_size > 0) fprintf(f, "sub QWORD " ARGS_HEAD ", %zu\n", args_size);
  if (rets_size > 0) fprintf(f, "sub QWORD " RETS_HEAD ", %zu\n", rets_size);
  if (lvars_size > 0) fprintf(f, "sub QWORD " LVARS_HEAD ", %zu\n", lvars_size);

  if (rets_size == 0) return;
  
  for (size_t i = 0; i < fn->rets.count; i++) {
    fprintf(f, "xor QWORD rbx, QWORD rbx\n");
    Var ret = fn->rets.items[i];
    switch (ret.tid) {
    case TYPE_U8:
      fprintf(f, "sub rax, 1\n");
      fprintf(f, "mov bl, BYTE [rax]\n");
      break;
    case TYPE_U16:
      fprintf(f, "sub rax, 2\n");
      fprintf(f, "mov bx, WORD [rax]\n");
      break;
    case TYPE_U32:
      fprintf(f, "sub rax, 4\n");
      fprintf(f, "mov ebx, DWORD [rax]\n");
      break;
    case TYPE_U64:
      fprintf(f, "sub rax, 8\n");
      fprintf(f, "mov QWORD rbx, QWORD [rax]\n");
      break;
    case TYPES_COUNT: 
    default: assert(false && "unimplemented");
    }
    fprintf(f, "push QWORD rbx\n");
  }
}

TARE_DEF void gen_funcall_op(Generator *g, FILE *f) {
  size_t fid = g->op->op;
  gen_funcall(g, f, fid);
}

TARE_DEF void gen_ret_op(FILE *f) {
  fprintf(f, "mov rsp, rbp\n");
  fprintf(f, "pop rbp\n");
  fprintf(f, "ret\n");
}

TARE_DEF void gen_write_op(Generator *g, FILE *f) {
  // Second argument
  gen_pop_from_ops_to_reg(f, "rdx");
  
  if (g->shift != 0) fprintf(f, "shl QWORD rdx, %zu\n", g->shift);
  
  // First argument
  gen_pop_from_ops_to_reg(f, "rsi");
  
  fprintf(f, "mov rax, 1\n"); // write => syscall 1
  fprintf(f, "mov rdi, 1\n"); // stdout => fd 1
  
  fprintf(f, "syscall\n");

  gen_push_to_ops(f);
}

TARE_DEF void gen_read_op(Generator *g, FILE *f) {
  // Second argument
  gen_pop_from_ops_to_reg(f, "rdx");
  
  if (g->shift != 0) fprintf(f, "shl QWORD rdx, %zu\n", g->shift);
  
  // First argument
  gen_pop_from_ops_to_reg(f, "rsi");
  
  fprintf(f, "mov rax, 0\n"); // read => syscall 0
  fprintf(f, "mov rdi, 0\n"); // stdin => fd 0
  
  fprintf(f, "syscall\n");

  gen_push_to_ops(f);
}

TARE_DEF void gen_syscall_op(Generator *g, FILE *f) {
  const char* regs[7] = {"rax", "rdi", "rsi", "rdx", "r10", "r8", "r9"};

  size_t pops = g->op->op;
  while (pops--) gen_pop_from_ops_to_reg(f, regs[pops]);
  fprintf(f, "syscall\n");

  gen_push_to_ops(f); // what about errno from rdx?
}

// tape: [head]
TARE_DEF void gen_tape_op(FILE *f) {
  fprintf(f, "mov QWORD rax, QWORD " TAPE_HEAD "\n");
  fprintf(f, "mov QWORD rax, QWORD [rax]\n");
  gen_push_to_ops(f);
}

// head: head
TARE_DEF void gen_head_op(FILE *f) {
  fprintf(f, "mov QWORD rax, QWORD " TAPE_HEAD "\n");
  gen_push_to_ops(f);
}

// base: base
TARE_DEF void gen_base_op(FILE *f) {
  fprintf(f, "mov QWORD rax, " TAPE_BASE "\n");
  gen_push_to_ops(f);
}

// index: head - base
TARE_DEF void gen_index_op(FILE *f) {
  fprintf(f, "mov QWORD rax, QWORD " TAPE_HEAD "\n");
  fprintf(f, "sub QWORD rax, " TAPE_BASE "\n");
  gen_push_to_ops(f);
}

/* TARE_DEF bool gen_const_op(Generator *g, FILE *f); */

// TODO: This is a mess.
TARE_DEF void gen_push_op(Generator *g, FILE *f) {
  /* fprintf(f, "sub QWORD " OPS_HEAD ", 8\n"); */
  /* fprintf(f, "mov QWORD rbx, QWORD " OPS_HEAD "\n"); */
  /* fprintf(f, "mov QWORD rax, QWORD [rbx]\n"); */

  /* fprintf(f, "push QWORD rax\n"); */

  /* gen_pop_from_ops_to_reg(f, "rbx"); */

  fprintf(f, "push QWORD [rbx]");
  
  (void)g;
}

// TODO: This is a mess.
TARE_DEF void gen_pop_op(Generator *g, FILE *f) {
  /* fprintf(f, "sub QWORD " OPS_HEAD ", 8\n"); */
  /* fprintf(f, "mov QWORD rbx, QWORD " OPS_HEAD "\n"); */
  /* fprintf(f, "mov QWORD rax, QWORD [rbx]\n"); */
  /* fprintf(f, "push QWORD rax\n"); */
  /* fprintf(f, "push QWORD [rbx]"); */
  
  fprintf(f, "pop QWORD rax\n");

  /* gen_push_to_ops_from_reg(g, f, "rax"); */
  
  /* fprintf(f, "mov QWORD rbx, QWORD " OPS_HEAD "\n"); */
  /* fprintf(f, "mov QWORD [rbx], QWORD rax\n"); */
  /* fprintf(f, "add QWORD " OPS_HEAD ", 8\n"); */
  (void)g;
}


// add: first + second
TARE_DEF void gen_add_op(FILE *f) {
  // Second argument
  gen_pop_from_ops_to_reg(f, "rbx");
  
  // First argument
  gen_pop_from_ops(f);
  fprintf(f, "add QWORD rax, QWORD rbx\n");
  gen_push_to_ops_from_reg(f, "rax");
}

// sub: first - second
TARE_DEF void gen_sub_op(FILE *f) {
  // Second argument
  gen_pop_from_ops_to_reg(f, "rbx");
  
  // First argument
  gen_pop_from_ops(f);
  fprintf(f, "sub QWORD rax, QWORD rbx\n");
  gen_push_to_ops_from_reg(f, "rbx");
}

// mul: first * second
TARE_DEF void gen_mul_op(FILE *f) {
  // Second argument
  gen_pop_from_ops_to_reg(f, "rbx");

  // First argument
  gen_pop_from_ops(f);
  fprintf(f, "mul QWORD rbx\n");
  gen_push_to_ops(f);
}

// div: first / second
TARE_DEF void gen_div_op(FILE *f) {
  // Second argument
  gen_pop_from_ops_to_reg(f, "rbx");

  // First argument
  gen_pop_from_ops(f);
  fprintf(f, "div QWORD rbx\n");
  gen_push_to_ops(f);
}

// shl: first << second
TARE_DEF void gen_shl_op(FILE *f) {
  // Second argument
  gen_pop_from_ops(f);
  fprintf(f, "mov cl, al");
  
  // First argument
  gen_pop_from_ops(f);
  fprintf(f, "shl QWORD rax, cl\n");
  gen_push_to_ops(f);
}

// shr: first >> second
TARE_DEF void gen_shr_op(FILE *f) {
  // Second argument
  gen_pop_from_ops(f);
  fprintf(f, "mov cl, al");
  
  // First argument
  gen_pop_from_ops(f);
  fprintf(f, "shr QWORD rax, cl\n");
  gen_push_to_ops(f);
}

// not: !first
TARE_DEF void gen_not_op(FILE *f) {
  // Get argument
  gen_pop_from_ops(f);
  fprintf(f, "not QWORD rax\n");
  gen_push_to_ops(f);
}

// bitwise_and: first & second
// bitwise_or: first | second
TARE_DEF void gen_bitwise(Generator *g, FILE *f) {
  // Second argument
  gen_pop_from_ops_to_reg(f, "rbx");
  
  // First argument
  gen_pop_from_ops(f);
  
  const char *inst = "";
  if (g->op->type == OP_BITWISE_AND) inst = "and";
  else if (g->op->type == OP_BITWISE_OR) inst = "or";
  else assert(false && "unreachable");

  fprintf(f, "%s QWORD rax, QWORD rbx", inst);

  gen_push_to_ops(f);
}

// logical_and: first && second
// logical_or: first || second
TARE_DEF void gen_logical(Generator *g, FILE *f) {
  // Second argument
  gen_pop_from_ops(f);
  
  fprintf(f, "mov QWORD rcx, QWORD [false]\n");
  fprintf(f, "cmp QWORD rax, QWORD rcx\n");
  fprintf(f, "cmovne QWORD rcx, QWORD [true]\n");
  fprintf(f, "mov QWORD rax, QWORD rcx\n");
  
  // First argument
  gen_pop_from_ops_to_reg(f, "rdx");
  
  fprintf(f, "mov QWORD rbx, QWORD [false]\n");
  fprintf(f, "cmp QWORD rbx, QWORD rdx\n");
  fprintf(f, "cmovne QWORD rbx, QWORD [true]\n");
  fprintf(f, "mov QWORD rdx, QWORD rbx\n");

  const char *inst = "";
  if (g->op->type == OP_LOGICAL_AND) inst = "and";
  else if (g->op->type == OP_LOGICAL_OR) inst = "or";
  else assert(false && "unreachable");

  fprintf(f, "%s QWORD rax, QWORD rdx", inst);

  gen_push_to_ops(f);
}

// less: first < second
// less_equal: first <= second
// greater: first > second
// greater_equal: first >= second
// equal: first == second
// not_equal: first != second
TARE_DEF void gen_comparison_op(Generator *g, FILE *f) {
  // Second argument
  gen_pop_from_ops_to_reg(f, "rbx");

  // First argument
  fprintf(f, "mov QWORD rcx, QWORD [false]\n");
  gen_pop_from_ops(f);
  fprintf(f, "cmp QWORD rax, QWORD rbx\n");

  const char *inst = "";
  
  if (g->op->type == OP_LESS) inst = "cmovl";
  else if (g->op->type == OP_LESS_EQUAL) inst = "cmovle";
  else if (g->op->type == OP_GREATER) inst = "cmovg";
  else if (g->op->type == OP_GREATER_EQUAL) inst = "cmovge";
  else if (g->op->type == OP_EQUAL) inst = "cmove";
  else if (g->op->type == OP_NOT_EQUAL) inst = "cmovne";
  else assert(false && "unreachable");

  fprintf(f, "%s QWORD rcx, QWORD [true]\n", inst);
  
  gen_push_to_ops_from_reg(f, "rcx");
}

// deref: [first]
TARE_DEF void gen_deref_op(FILE *f) {
  // First argument
  gen_pop_from_ops(f);
  gen_push_to_ops_from_reg(f, "[rax]");
}

TARE_DEF void gen_num_op(Generator *g, FILE *f) {
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

  gen_push_to_ops(f);
}

TARE_DEF void gen_op_pop_from_ops(FILE *f) {
  gen_pop_from_ops_to_reg(f, "rax");
}

// assign: [first] = second
TARE_DEF void gen_assign_op(FILE *f) {
  // Second argument
  gen_pop_from_ops_to_reg(f, "rbx");
  
  // First argument
  gen_pop_from_ops(f);

  fprintf(f, "mov QWORD [rax], QWORD rbx\n");
}

// TODO: make sure this works
TARE_DEF void gen_gvid_op(Generator *g, FILE *f) {
  size_t op = g->op->op;
  fprintf(f, "mov QWORD rax, " GLOBAL_VARS_NAME "\n");
  fprintf(f, "add QWORD rax, %zu\n", op * 8);
  gen_push_to_ops(f);
}

// TODO: make sure this works
TARE_DEF void gen_lvid_op(Generator *g, FILE *f) {
  size_t op = g->op->op;
  /* fprintf(f, "mov QWORD rax, " LOCAL_VARS_NAME "\n"); */
  fprintf(f, "mov QWORD rax, " LVARS_HEAD "\n");
  fprintf(f, "add QWORD rax, %zu\n", op * 8);
  gen_push_to_ops(f);
}

TARE_DEF void gen_rvid_op(Generator *g, FILE *f) {
  size_t op = g->op->op;
  fprintf(f, "mov QWORD rax, QWORD " RETS_HEAD "\n");
  fprintf(f, "add QWORD rax, %zu\n", op * 8);
  gen_push_to_ops(f);
}

TARE_DEF void gen_avid_op(Generator *g, FILE *f) {
  size_t op = g->op->op;
  fprintf(f, "mov QWORD rax, " ARGS_HEAD "\n");
  fprintf(f, "add QWORD rax, %zu\n", op * 8);
  gen_push_to_ops(f);
}

TARE_DEF void gen_pop_from_ops_to_reg(FILE *f, const char *reg) {
  fprintf(f, "pop QWORD %s\n", reg);
}

TARE_DEF void gen_pop_from_ops(FILE *f) {
  gen_pop_from_ops_to_reg(f, "rax");
}

TARE_DEF void gen_push_to_ops_from_reg(FILE *f, const char *reg) {
  fprintf(f, "push QWORD %s\n", reg);
}

TARE_DEF void gen_push_to_ops(FILE *f) {
  gen_push_to_ops_from_reg(f, "rax");
}

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

#endif // CODEGEN_IMPLEMENTATION

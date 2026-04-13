#ifndef CODEGEN_H_
#define CODEGEN_H_

TARE_DEF bool gen_fasm_no_parser(Tokenizer *t, const char *output);

TARE_DEF bool gen_from_token(Generator *g, FILE *f);
TARE_DEF bool gen_func_sig(Generator *g, FILE *f);
/* TARE_DEF bool gen_func_args(Generator *g, FILE *f, Func *fn); */
/* TARE_DEF bool gen_func_rets(Generator *g, FILE *f, Func *fn); */

/* TARE_DEF void gen_conditional(Generator *g, FILE *f, bool jz); */
TARE_DEF bool gen_conditional(Generator *g, FILE *f, bool jz);

// Seems unnecessary.
/* TARE_DEF void gen_tape_rax(const Generator *gen, FILE *f, bool mul); */
/* TARE_DEF void gen_head_rax(const Generator *gen, FILE *f); */
/* TARE_DEF void gen_base_rax(const Generator *gen, FILE *f); */
/* TARE_DEF void gen_index_rax(FILE *f); */

TARE_DEF void gen_tape_rbx(FILE *f, bool mul);
TARE_DEF void gen_head_rbx(FILE *f);
TARE_DEF void gen_base_rbx(FILE *f);
TARE_DEF void gen_index_rbx(FILE *f);

TARE_DEF bool gen_arg(const Token *t, FILE *f, bool mul);
TARE_DEF void gen_arg_(Generator *g, const Token *t, FILE *f, bool mul, bool mask);

TARE_DEF bool gen_pointer_inc_dec(Generator *g, FILE *f, bool iord);
TARE_DEF bool gen_pointer_add_sub(Generator *g, FILE *f, bool aors);
TARE_DEF bool gen_element_inc_dec(Generator *g, FILE *f, bool iord);
TARE_DEF bool gen_element_add_sub(Generator *g, FILE *f, bool aors);
TARE_DEF bool gen_read_size_inst(Generator *g, FILE *f, size_t r);
TARE_DEF bool gen_read_write_syscall_args(Generator *g, FILE *f, bool first);
TARE_DEF bool gen_read_write_syscall(Generator *g, FILE *f, bool worr);

TARE_DEF bool gen_syscall_args(Generator *g, FILE *f);
TARE_DEF bool gen_syscall(Generator *g, FILE *f);

TARE_DEF bool gen_fid(Generator *g, FILE *f);
TARE_DEF bool gen_vid(Generator *g, FILE *f);

TARE_DEF bool gen_push(Generator *g, FILE *f);
TARE_DEF bool gen_pop(Generator *g, FILE *f);

TARE_DEF size_t fix_size_and_warn(const Generator *g, const Token *t);

TARE_DEF size_t total_vars_length(const Vars *vars);

#endif // CODEGEN_H_

#ifdef CODEGEN_IMPLEMENTATION

TARE_DEF bool gen_fasm_no_parser(Tokenizer *t, const char *output) {
  if (t == NULL || output == NULL) return false;
  
  patch_tokenizer_builtin_types(t);
  if (!patch_tokenizer_bgn_end(t)) return false;
  
  Funcs funcs = {0};
  Func main = {.name = SV_MAKE(main)};
  da_append(&funcs, main);
  if (!patch_tokenizer_funcs(t, &funcs)) return false;

  first_token(t);

  if (funcs.items[0].end == 0) {
    diag_err(t, t->t, "error: no main function!\n");
    return false;
  }

  FILE *f = fopen(output, "wb");
  if (f == NULL) {
    fprintf(stderr, "error: couldn't open file %s\n", output);
  }

  // Boilerplate
  fprintf(f, "format ELF64 executable 3\n");
  fprintf(f, "entry start\n");
  fprintf(f, "start:\n");

  // Tare runtime
  fprintf(f, "push " TAPE_NAME "\n");         // base
  fprintf(f, "mov rbp, rsp\n");
  fprintf(f, "push " TAPE_NAME "\n");         // head
  fprintf(f, "push 8\n");            // read size in bytes (for addition)
  fprintf(f, "push 3\n");            // read size bit shifting
  fprintf(f, "push %zu\n", R64_MAX); // read size bit mask
  /* fprintf(f, "push 0xFFFFFFFFFFFFFFFF\n"); // read size bit mask */

  fprintf(f, "push " ARGS_NAME "\n"); // args base
  fprintf(f, "push " ARGS_NAME "\n"); // args head

  fprintf(f, "push " RETS_NAME "\n"); // rets base
  fprintf(f, "push " RETS_NAME "\n"); // rets head
  
  fprintf(f, "push " VARS_NAME "\n"); // vars base
  fprintf(f, "push " VARS_NAME "\n"); // vars head
  
  Longs longs = {0};
  Generator gen = {.t = t, .funcs = &funcs, .longs = &longs};

  /* for (size_t i = 0; i < funcs.items[0].rets.count; i++) fprintf(f, "push 0\n"); */
  /* if (funcs.items[0].rets.count == 0) fprintf(f, "push 0\n"); */
  
  fprintf(f, "call func_0\n");
  
  /* fprintf(f, "mov rdi, rax\n"); */
  
  fprintf(f, "mov rax, 60\n");
  
  /* fprintf(f, "mov rdi, 0\n"); */
  /* fprintf(f, "pop rdi\n"); */
  /* if (funcs.items[0].rets.count == 0) */
  /*   fprintf(f, "mov QWORD " RETS_HEAD ", 0\n"); */
  fprintf(f, "mov rdi, QWORD " RETS_HEAD "\n");
  fprintf(f, "mov rdi, [rdi]\n");
  
  fprintf(f, "syscall\n");

  gen.r = 8;

  while (true) {
    if (!gen_from_token(&gen, f)) return false;
    if (!next_token(t)) break;
  }

  fprintf(f, "segment readable writeable\n");
  fprintf(f, TAPE_NAME " db %d dup (0)\n", TAPE_SIZE);

  
  fprintf(f, ARGS_NAME " db %d dup (0)\n", ARG_STACK_SIZE);
  fprintf(f, RETS_NAME " db %d dup (0)\n", RET_STACK_SIZE);
  fprintf(f, VARS_NAME " db %d dup (0)\n", VAR_STACK_SIZE);

  if (longs.count > 0) fprintf(f, "segment readable\n");
  for (size_t i = 0; i < longs.count; i++) {
    fprintf(f, "long_imm_%zu dq %zu\n", i, longs.items[i]);
  }

  fprintf(f, "read_size_max_1 dq %zu\n", R8_MAX);
  fprintf(f, "read_size_max_2 dq %zu\n", R16_MAX);
  fprintf(f, "read_size_max_4 dq %zu\n", R32_MAX);
  fprintf(f, "read_size_max_8 dq %zu\n", R64_MAX);
  
  fclose(f);
  if (!gen.fail) printf("Successfully generated file %s\n", output);

  for (size_t i = 0 ; i < funcs.count; i++) {
    Func fn = funcs.items[i];
    if (fn.args.items) free(fn.args.items);
    if (fn.rets.items) free(fn.rets.items);
  }

  if (funcs.items) free(funcs.items);
  
  return !gen.fail;
}

TARE_DEF bool gen_from_token(Generator *g, FILE *f) {
  Tokenizer *t = g->t;

  switch (g->t->t->t) {
  case TOKEN_TYPE_NAME: unimpl("TOKEN_TYPE_NAME"); break;
  case TOKEN_TYPE_WHOLE_NUM: unimpl("TOKEN_TYPE_WHOLE_NUM"); break;
  case TOKEN_TYPE_FRAC_NUM: unimpl("TOKEN_TYPE_FRAC_NUM"); break;
  case TOKEN_TYPE_KEYWORD:
    check_failure(g->t, &g->errors, &g->error_indices, g->r, &g->fail);
    switch (g->t->t->k) {
    case KEY_F: case KEY_B:
      if (!gen_pointer_add_sub(g, f, g->t->t->k == KEY_F)) return false;
      break;
    case KEY_N: case KEY_P:
      if (!gen_pointer_inc_dec(g, f, g->t->t->k == KEY_N)) return false;
      break;
    case KEY_A: case KEY_S:
      if (!gen_element_add_sub(g, f, g->t->t->k == KEY_A)) return false;
      break;
    case KEY_I: case KEY_D:
      if (!gen_element_inc_dec(g, f, g->t->t->k == KEY_I)) return false;
      break;
    case KEY_R8:
      if (!gen_read_size_inst(g, f, 1)) return false;
      break;
    case KEY_R16:
      if (!gen_read_size_inst(g, f, 2)) return false;
      break;
    case KEY_R32:
      if (!gen_read_size_inst(g, f, 4)) return false;
      break;
    case KEY_R64:
      if (!gen_read_size_inst(g, f, 8)) return false;
      break;
    case KEY_IF:
      if (!gen_conditional(g, f, true)) return false;
      break;
    case KEY_WHILE:
      fprintf(f, "addr_%zu:\n", t->index);
      if (!gen_conditional(g, f, true)) return false;
      break;
    case KEY_FUNC: if (!gen_func_sig(g, f)) return false; break;
    case KEY_RET:
      {
        Func *fn = g->funcs->items + g->t->t->fid;
        size_t args_bytes = fn->args.count*8;
        if (args_bytes > 0) fprintf(f, "sub rsp, %zu\n", args_bytes);
        /* for (size_t j = 0; j < fn->args.count; j++) { */
        /*   fprintf(f, "pop r14\n"); */
        /* } */
        /* if (fn->rets.count > 0) fprintf(f, "pop rax\n"); */
        
        /* size_t rc = fn->rets.count; */
        /* switch (rc) { */
        /* case 0: break; */
        /* case 1: */
        /*   fprintf(f, "pop rax\n"); break; */
        /* default: unimpl("unsure what to do here"); break; */
        /* } */
      }
      if (!expect_special(g->t, END)) return false;
      fprintf(f, "ret\n");
      break;
    case KEY_SYSCALL: if (!gen_syscall(g, f)) return false; break;
    case KEY_WRITE:
      if (!gen_read_write_syscall(g, f, true)) return false;
      break;
    case KEY_READ:
      if (!gen_read_write_syscall(g, f, false)) return false;
      break;
    case KEY_TAPE: unimpl("KEY_TAPE"); break;
    case KEY_HEAD: unimpl("KEY_HEAD"); break;
    case KEY_BASE: unimpl("KEY_BASE"); break;
    case KEY_INDEX: unimpl("KEY_INDEX"); break;
    case KEY_CONST: unimpl("KEY_CONST"); break;
      
    case KEY_PUSH: if (!gen_push(g, f)) return false; break;
    case KEY_POP: if (!gen_pop(g, f)) return false; break;
    case KEY_ADD: unimpl("KEY_ADD"); break;
    case KEY_SUB: unimpl("KEY_SUB"); break;
    case KEY_MUL: unimpl("KEY_MUL"); break;
    case KEY_DIV: unimpl("KEY_DIV"); break;
    case KEY_SHL: unimpl("KEY_SHL"); break;
    case KEY_SHR: unimpl("KEY_SHR"); break;
    case KEY_NOT: unimpl("KEY_NOT"); break;
    case KEY_DEREF: unimpl("KEY_DEREF"); break;
    
    case KEYWORD_TYPES:
      diag_errf(g->t, g->t->t, "invalid keyword `%.*s`\n", TOK_ARG(g->t->t));
      return false;
    default:
      {
        const char *stuff = "keyword type token ";
        size_t len = g->t->t->l;
        size_t stuff_len = strlen(stuff);
        char *key = malloc(len + stuff_len + 1 + 2);
        strncpy(key, stuff, stuff_len);
        key[stuff_len++] = '`';
        strncpy(key + stuff_len, g->t->t->f, len);
        key[len + stuff_len++] = '`';
        key[len + stuff_len] = 0;
        unimpl(key);
      }
    }
    break;
  case TOKEN_TYPE_SPECIAL:
    switch (g->t->t->s) {
    case PAR_BGN: unimpl("PAR_BGN"); break;
    case PAR_END: unimpl("PAR_END"); break;
    case GRP_BGN: unimpl("GRP_BGN"); break;
    case GRP_END: unimpl("GRP_END"); break;
    case BLK_BGN: unimpl("BLK_BGN"); break;
    case BLK_END:
      if (g->t->t->jmp2 == 0) break;
      {
        Token *cond = g->t->items + g->t->t->jmp2;
        if (cond->k == KEY_IF) {
          fprintf(f, "addr_%zu:\n", g->t->index);
          break;
        }
        
        fprintf(f, "jmp addr_%zu\n", g->t->t->jmp2);
        fprintf(f, "addr_%zu:\n", g->t->index);
      }
      break;
    case DQUOTE: unimpl("DQUOTE"); break;
    case SQUOTE: unimpl("SQUOTE"); break;
    case ESC: unimpl("ESC"); break;
    case SEP: unimpl("SEP"); break;
    case END:
      if (g->t->t->jmp2 == 0) break;
      {
        Token *cond = g->t->items + g->t->t->jmp2;
        if (cond->k == KEY_IF) {
          fprintf(f, "addr_%zu:\n", g->t->index);
          break;
        }
        
        fprintf(f, "jmp addr_%zu\n", g->t->t->jmp2);
        fprintf(f, "addr_%zu:\n", g->t->index);
      }
      break;
    case DOT: unimpl("DOT"); break;
    case DEF: unimpl("DEF"); break;
    case DIV: unimpl("DIV"); break;
    case MULT: unimpl("MULT"); break;
    case ADD: unimpl("ADD"); break;
    case SUB: unimpl("SUB"); break;
    case LESS: unimpl("LESS"); break;
    case GREATER: unimpl("GREATER"); break;
    case EQUAL: unimpl("EQUAL"); break;
    case NOT: unimpl("NOT"); break;
    case SPECIAL_TYPES: unimpl("SPECIAL_TYPES"); break;
    }
    break;
  case TOKEN_TYPE_STRING: unimpl("TOKEN_TYPE_STRING"); break;
  case TOKEN_TYPE_CHAR: unimpl("TOKEN_TYPE_CHAR"); break;
  case TOKEN_TYPE_VID: if (!gen_vid(g, f)) return false; break;
  case TOKEN_TYPE_TID: unimpl("TOKEN_TYPE_TID"); break;
  case TOKEN_TYPE_FID: if (!gen_fid(g, f)) return false; break;
  case TOKEN_TYPES: default:
    diag_errf(g->t, g->t->t, "invalid token `%.*s`\n", TOK_ARG(g->t->t));
    return false;
  }

  return true;
}

TARE_DEF bool gen_func_sig(Generator *g, FILE *f) {
  if (!expect_fid(g->t)) return false;
  size_t fid = g->t->t->fid;
  Func *fn = g->funcs->items + fid;
  fprintf(f, ";; --------------------------------------------------\n");
  fprintf(f, ";; %.*s\n", SV_ARG(fn->name));
  fprintf(f, ";; --------------------------------------------------\n");
  fprintf(f, "func_%zu:\n", g->t->t->fid);
  if (!to_token(g->t, fn->start)) return false;
  /* for (size_t i = 0; i < fn->rets.count; i++) { */
  /*   /\* Var var = fn->rets.items[i]; *\/ */
  /*   /\* Tokenizer *t = g->t; *\/ */
  /*   /\* switch (var.tid) { *\/ */
  /*   /\* case TYPE_U8: unimpl("TYPE_U8"); break; *\/ */
  /*   /\* case TYPE_U16: unimpl("TYPE_U16"); break; *\/ */
  /*   /\* case TYPE_U32: unimpl("TYPE_U32"); break; *\/ */
  /*   /\* case TYPE_U64: unimpl("TYPE_U64"); break; *\/ */
  /*   /\* case TYPES_COUNT: unimpl("TYPES_COUNT"); break; *\/ */
  /*   /\* default: printf("uhhh...\n"); break; *\/ */
  /*   /\* } *\/ */
  /*   fprintf(f, "push 0\n"); */
  /* } */
  
  /* for (size_t i = 0; i < fn->args.count; i++) { */
  /*   fprintf(f, "push 0\n"); */
  /* } */

  g->fni = fid;
  
  return true;
}

/* TARE_DEF bool gen_func_args(Generator *g, FILE *f, Func *fn) { */
/*   if (fn->args.count == 0) return true; */
/*   if (!expect_special(g->t, PAR_BGN)) return false; */
/*   for (size_t i = 0; i < fn->args.count; i++) { */
/*   } */
/*   if (!expect_special(g->t, PAR_END)) return false; */
/*   return true; */
/* } */

/* TARE_DEF bool gen_func_rets(Generator *g, FILE *f, Func *fn) { */
/*   return true; */
/* } */

/* TARE_DEF void gen_conditional(Generator *g, FILE *f, bool jz) { */
/*   fprintf(f, "mov rcx, [rbp-8]\n"); */
/*   fprintf(f, "mov QWORD rax, QWORD [rcx]\n"); */

/*   fprintf(f, "and QWORD rax, QWORD [rbp-32]\n"); */
/*   fprintf(f, "cmp QWORD rax, 0\n"); */
  
/*   if (jz) fprintf(f, "jz addr_%zu\n", g->t->t->jmp); */
/*   else fprintf(f, "jnz addr_%zu\n", g->t->t->jmp); */

/*   // -------------------------------------------------- */
  
/*   /\* switch (g->r) { *\/ */
/*   /\* case 1: fprintf(f, "cmp BYTE al, 0\n"); break; *\/ */
/*   /\* case 2: fprintf(f, "cmp WORD ax, 0\n"); break; *\/ */
/*   /\* case 4: fprintf(f, "cmp DWORD eax, 0\n"); break; *\/ */
/*   /\* case 8: fprintf(f, "cmp QWORD rax, 0\n"); break; *\/ */
/*   /\* default: *\/ */
/*   /\*   /\\* print_loc(stderr, g->t, g->t->t); *\\/ *\/ */
/*   /\*   /\\* fprintf(stderr, "error: Invalid read size!\n"); *\\/ *\/ */
/*   /\*   diag_err(g->t, g->t->t, "Invalid read size!\n"); *\/ */
/*   /\*   break; *\/ */
/*   /\* } *\/ */
  
/*   /\* if (jz) fprintf(f, "jz addr_%zu\n", g->t->t->jmp); *\/ */
/*   /\* else fprintf(f, "jnz addr_%zu\n", g->t->t->jmp); *\/ */
  
/*   // -------------------------------------------------- */
/* } */

TARE_DEF bool gen_conditional(Generator *g, FILE *f, bool jz) {
  Token *cond = g->t->t;
  size_t jmp = cond->jmp;
  size_t cond_index = g->t->index;
  if (!expect_special(g->t, PAR_BGN)) return false;
  if (!expect_num_or_vid_or_tape(g->t)) return false;
  Token *t = g->t->t;
  if (!expect_special(g->t, PAR_END)) return false;
  if (!expect_special(g->t, DEF)) return false;

  size_t point = g->t->index;
  if (!next_token(g->t)) return false;
  Token *start = g->t->t;
  Token *end = g->t->t;
  
  if (start->t == TOKEN_TYPE_SPECIAL) {
    if (start->s != BLK_BGN) return false;
    jmp = start->jmp;
    end = g->t->items + jmp;
  } else {
    
    end = g->t->t;
    Token *save = end;
    while (next_token(g->t)) {
      end = g->t->t;
      if (end->t != TOKEN_TYPE_SPECIAL) continue;
      if (end->s == END) break;
    }

    // TODO: better error handling
    if (end == save) return false;
    if (end->t != TOKEN_TYPE_SPECIAL) return false;
    if (end->s != END) return false;
    jmp = g->t->index;
    if (!to_token(g->t, point)) return false;
  }
  
  cond->jmp = jmp;
  end->k = cond->k;
  end->jmp2 = cond_index;

  if (t->k == KEY_TAPE) gen_tape_rbx(f, false);
  else if (t->k == KEY_HEAD) gen_head_rbx(f);
  else if (t->k == KEY_BASE) gen_base_rbx(f);
  else if (t->k == KEY_INDEX) gen_index_rbx(f);
  else {
    size_t op = fix_size_and_warn(g, t);
    if (op < R32_MAX) fprintf(f, "mov QWORD rbx, %zu\n", op);
    else {
      size_t index = find_long(g->longs, op);
      fprintf(f, "mov QWORD rbx, [long_imm_%zu]\n", index);
    }
    fprintf(f, "and QWORD rbx, QWORD " READ_MASK "\n");
  }

  /* fprintf(f, "and QWORD rbx, QWORD [rbp-32]\n"); */
  fprintf(f, "cmp QWORD rbx, 0\n");
  
  if (jz) fprintf(f, "jz addr_%zu\n", jmp);
  else fprintf(f, "jnz addr_%zu\n", jmp);

  return true;
  
  switch (g->r) {
  case 1: fprintf(f, "cmp BYTE bl, 0\n"); break;
  case 2: fprintf(f, "cmp WORD bx, 0\n"); break;
  case 4: fprintf(f, "cmp DWORD ebx, 0\n"); break;
  case 8: fprintf(f, "cmp QWORD rbx, 0\n"); break;
  default:
    print_loc(stderr, g->t, g->t->t);
    fprintf(stderr, "error: Invalid read size!\n");
    break;
  }
  
  if (jz) fprintf(f, "jz addr_%zu\n", t->jmp);
  else fprintf(f, "jnz addr_%zu\n", t->jmp);

  return true;
}

// Seems unnecessary.
/* TARE_DEF void gen_tape_rax(const Generator *gen, FILE *f, bool mul) { */
/*   fprintf(f, "push QWORD [rsi]\n"); */
/*   fprintf(f, "pop QWORD rax\n"); */
/*   if (mul) { */
/*     if (gen->r > 1) fprintf(f, "mul QWORD [read_size_%zu]\n", gen->r); */
/*   } */
/*   fprintf(f, "xor QWORD rbx, QWORD rbx\n"); */
/*   if (gen->r == 1) fprintf(f, "mov bl, al\n"); */
/*   else if (gen->r == 2) fprintf(f, "mov bx, ax\n"); */
/*   else if (gen->r == 4) fprintf(f, "mov ebx, eax\n"); */
/*   else if (gen->r == 8) fprintf(f, "mov QWORD rbx, QWORD rax\n"); */
/* } */

/* TARE_DEF void gen_head_rax(const Generator *gen, FILE *f) { */
/*   fprintf(f, "xor QWORD rax, QWORD rax\n"); */
/*   if (gen->r == 1) fprintf(f, "mov al, sil\n"); */
/*   else if (gen->r == 2) fprintf(f, "mov ax, si\n"); */
/*   else if (gen->r == 4) fprintf(f, "mov eax, esi\n"); */
/*   else if (gen->r == 8) fprintf(f, "mov QWORD rax, QWORD rsi\n"); */
/* } */

/* TARE_DEF void gen_base_rax(const Generator *gen, FILE *f) { */
/*   fprintf(f, "mov rbx, tape\n"); */
/*   fprintf(f, "xor QWORD rax, QWORD rax\n"); */
/*   if (gen->r == 1) fprintf(f, "mov al, bl\n"); */
/*   else if (gen->r == 2) fprintf(f, "mov ax, bx\n"); */
/*   else if (gen->r == 4) fprintf(f, "mov eax, ebx\n"); */
/*   else if (gen->r == 8) fprintf(f, "mov QWORD rax, QWORD rbx\n"); */
/* } */

/* TARE_DEF void gen_index_rax(FILE *f) { */
/*   fprintf(f, "mov rax, rsi\n"); */
/*   fprintf(f, "sub rax, tape\n"); */
/* } */

TARE_DEF void gen_tape_rbx(FILE *f, bool mul) {
  fprintf(f, "mov rcx, " TAPE_HEAD "\n");
  fprintf(f, "mov QWORD rax, QWORD [rcx]\n");
  
  if (mul) {
    /* if (gen->r > 1) fprintf(f, "mul QWORD [rbp-16]\n"); */
    /* fprintf(f, "mul QWORD [rbp-16]\n"); */
    
    fprintf(f, "mov cl, " READ_SHL "\n");
    fprintf(f, "shl QWORD rax, cl\n");
  }
  fprintf(f, "and QWORD rax, QWORD " READ_MASK "\n");
  fprintf(f, "mov QWORD rbx, QWORD rax\n");
  
  /* fprintf(f, "xor QWORD rbx, QWORD rbx\n"); */
  /* if (gen->r == 1) fprintf(f, "mov bl, al\n"); */
  /* else if (gen->r == 2) fprintf(f, "mov bx, ax\n"); */
  /* else if (gen->r == 4) fprintf(f, "mov ebx, eax\n"); */
  /* else if (gen->r == 8) fprintf(f, "mov QWORD rbx, QWORD rax\n"); */
}

TARE_DEF void gen_head_rbx(FILE *f) {
  fprintf(f, "mov QWORD rbx, QWORD " TAPE_HEAD "\n");
}

TARE_DEF void gen_base_rbx(FILE *f) {
  fprintf(f, "mov QWORD rbx, QWORD " TAPE_BASE "\n");
}

TARE_DEF void gen_index_rbx(FILE *f) {
  fprintf(f, "mov rbx, " TAPE_HEAD "\n");
  fprintf(f, "sub rbx, " TAPE_BASE "\n");
}

TARE_DEF bool gen_arg(const Token *t, FILE *f, bool mul) {
  if (t->k == KEY_TAPE) gen_tape_rbx(f, mul);
  else if (t->k == KEY_HEAD) gen_head_rbx(f);
  else if (t->k == KEY_BASE) gen_base_rbx(f);
  else if (t->k == KEY_INDEX) gen_index_rbx(f);
  else return false;
  return true;
}

TARE_DEF void gen_arg_(Generator *g, const Token *t, FILE *f, bool mul, bool mask) {
  if (t->t == TOKEN_TYPE_KEYWORD) {
    if (t->k == KEY_TAPE) gen_tape_rbx(f, mul);
    else if (t->k == KEY_HEAD) gen_head_rbx(f);
    else if (t->k == KEY_BASE) gen_base_rbx(f);
    else if (t->k == KEY_INDEX) gen_index_rbx(f);
    return;
  }

  if (t->t == TOKEN_TYPE_VID) {
    Func *fn = g->funcs->items + g->fni;
    size_t ac = fn->args.count;
    size_t rc = fn->rets.count;
  
    size_t index = 0;
    bool is_arg = false;
    bool is_ret = false;
    bool is_var = false;
  
    if (t->vid < ac) {
      is_arg = true;
      for (size_t i = 0; i < ac; i++) {
        Var arg = fn->args.items[i];
        if (arg.vid == t->vid) break;
        switch (arg.tid) {
        case TYPE_U8: index++; break;
        case TYPE_U16: index += 2; break;
        case TYPE_U32: index += 4; break;
        case TYPE_U64: index += 8; break;
        default: diag_err(g->t, t, "default switch (tid) in gen_vid");
        }
      }
    } else if (t->vid >= ac && t->vid < ac + rc) {
      is_ret = true;
      for (size_t i = 0; i < rc; i++) {
        Var ret = fn->rets.items[i];
        if (ret.vid == t->vid) break;
        switch (ret.tid) {
        case TYPE_U8: index++; break;
        case TYPE_U16: index += 2; break;
        case TYPE_U32: index += 4; break;
        case TYPE_U64: index += 8; break;
        default: diag_err(g->t, t, "default switch (tid) in gen_vid");
        }
      }
    } else {
      is_var = true;
      diag_err(g->t, t, "index in gen_vid");
    }

    if (is_arg) {
      fprintf(f, "mov QWORD rcx, " ARGS_HEAD "\n");
    } else if (is_ret) {
      fprintf(f, "mov QWORD rcx, " RETS_HEAD "\n");
    } else if (is_var) {
      fprintf(f, "mov QWORD rcx, " VARS_HEAD "\n");
      diag_err(g->t, t, "is_var in gen_vid");
    }

    fprintf(f, "add rcx, %zu\n", index);

    fprintf(f, "xor QWORD rbx, QWORD rbx\n");
    switch (t->tid) {
    case TYPE_U8: fprintf(f, "mov bl, BYTE [rcx]\n"); break;
    case TYPE_U16: fprintf(f, "mov bx, WORD [rcx]\n"); break;
    case TYPE_U32: fprintf(f, "mov ebx, DWORD [rcx]\n"); break;
    case TYPE_U64: fprintf(f, "mov QWORD rbx, QWORD [rcx]\n"); break;
    default: diag_err(g->t, t, "default switch (tid) in gen_vid");
    }
  } else if (t->t == TOKEN_TYPE_WHOLE_NUM) {
    size_t op = fix_size_and_warn(g, t);
    if (op < R32_MAX) fprintf(f, "mov QWORD rbx, %zu\n", op);
    else {
      size_t index = find_long(g->longs, op);
      fprintf(f, "mov QWORD rbx, [long_imm_%zu]\n", index);
    }
  }

  if (mul) {
    fprintf(f, "mov cl, " READ_SHL "\n");
    fprintf(f, "shl QWORD rbx, cl\n");
  }
  
  if (mask) fprintf(f, "and QWORD rbx, QWORD " READ_MASK "\n");
}

TARE_DEF bool gen_pointer_inc_dec(Generator *g, FILE *f, bool iord) {
  if (!expect_special(g->t, PAR_BGN)) return false;
  if (!expect_special(g->t, PAR_END)) return false;
  if (!expect_special(g->t, END)) return false;
  
  const char *inst = iord ? "add" : "sub";
  fprintf(f, "mov QWORD rax, QWORD " READ_BYTES "\n");
  fprintf(f, "%s QWORD " TAPE_HEAD ", QWORD rax\n", inst);
  
  return true;
}

TARE_DEF bool gen_pointer_add_sub(Generator *g, FILE *f, bool aors) {
  if (!expect_special(g->t, PAR_BGN)) return false;
  if (!expect_num_or_vid_or_tape(g->t)) return false;
  Token *t = g->t->t;
  if (!expect_special(g->t, PAR_END)) return false;
  if (!expect_special(g->t, END)) return false;

  // NOTE: this works also with true, true => the second referring to
  // the mask. The mask is perhaps less useful in this situation, but
  // it is, in a sense, more correct. Tough decision.
  gen_arg_(g, t, f, true, false);
  
  /* if (!gen_arg(t, f, true)) { */
  /*   fprintf(f, "mov QWORD rax, %zu\n", t->u64); */
    
  /*   fprintf(f, "mov cl, " READ_SHL "\n"); */
  /*   fprintf(f, "shl QWORD rax, cl\n"); */
    
  /*   fprintf(f, "mov QWORD rbx, QWORD rax\n"); */
  /* } */
  
  const char *inst = aors ? "add" : "sub";
  fprintf(f, "%s " TAPE_HEAD ", rbx\n", inst);
  
  return true;
}

TARE_DEF bool gen_element_inc_dec(Generator *g, FILE *f, bool iord) {
  if (!expect_special(g->t, PAR_BGN)) return false;
  if (!expect_special(g->t, PAR_END)) return false;
  if (!expect_special(g->t, END)) return false;
  
  const char *inst = iord ? "inc" : "dec";
  fprintf(f, "mov rcx, " TAPE_HEAD "\n");

  fprintf(f, "%s QWORD [rcx]\n", inst);
  return true;
}

TARE_DEF bool gen_element_add_sub(Generator *g, FILE *f, bool aors) {
  if (!expect_special(g->t, PAR_BGN)) return false;
  if (!expect_num_or_vid_or_tape(g->t)) return false;
  Token *t = g->t->t;
  if (!expect_special(g->t, PAR_END)) return false;
  if (!expect_special(g->t, END)) return false;

  const char *inst = aors ? "add" : "sub";
  
  fprintf(f, "mov rcx, " TAPE_HEAD "\n");

  // NOTE: this also works with false, true => that is probably more correct.
  gen_arg_(g, t, f, false, false);
  
  /* if (!gen_arg(t, f, false)) { */
  /*   size_t op = fix_size_and_warn(g, t); */
  /*   if (op < R32_MAX) fprintf(f, "mov QWORD rbx, %zu\n", op); */
  /*   else { */
  /*     size_t index = find_long(g->longs, op); */
  /*     fprintf(f, "mov QWORD rbx, [long_imm_%zu]\n", index); */
  /*   } */
  /* } */

  fprintf(f, "%s QWORD [rcx], QWORD rbx\n", inst);
  
  return true;
}

TARE_DEF bool gen_read_size_inst(Generator *g, FILE *f, size_t r) {
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

TARE_DEF bool gen_read_write_syscall_args(Generator *g, FILE *f, bool first) {
  if (first) if (!expect_special(g->t, PAR_BGN)) return false;
  if (!expect_num_or_vid_or_tape(g->t)) return false;

  // NOTE: this also works with !first, false => it's unclear how
  // necessary the mask is at this point.
  gen_arg_(g, g->t->t, f, !first, true);

  /* if (!gen_arg(g->t->t, f, !first)) { */
  /*   size_t op = fix_size_and_warn(g, g->t->t); */
  /*   if (op < R32_MAX) fprintf(f, "mov QWORD rbx, %zu\n", op); */
  /*   else { */
  /*     size_t index = find_long(g->longs, op); */
  /*     fprintf(f, "mov QWORD rbx, [long_imm_%zu]\n", index); */
  /*   } */
  /*   if (!first) { */
  /*     fprintf(f, "mov cl, " READ_SHL "\n"); */
  /*     fprintf(f, "shl QWORD rbx, cl\n"); */
  /*   } */
  /*   fprintf(f, "and QWORD rbx, QWORD " READ_MASK "\n"); */
  /* } */
  
  if (first) fprintf(f, "mov rsi, rbx\n");
  else fprintf(f, "mov rdx, rbx\n");
  
  if (first) if (!expect_special(g->t, SEP)) return false;
  if (!first) {
    if (!expect_special(g->t, PAR_END)) return false;
    if (!expect_special(g->t, END)) return false;
  }
  
  return true;
}

TARE_DEF bool gen_read_write_syscall(Generator *g, FILE *f, bool worr) {
  if (!gen_read_write_syscall_args(g, f, true)) return false;
  if (!gen_read_write_syscall_args(g, f, false)) return false;

  size_t inst = worr ? 1 : 0;

  fprintf(f, "mov rax, %zu\n", inst);
  fprintf(f, "mov rdi, %zu\n", inst);
  /* fprintf(f, "pop QWORD rdx\n"); */
  /* fprintf(f, "pop QWORD rsi\n"); */
  fprintf(f, "syscall\n");
  return true;
  // Useful reference:
  /* fprintf(f, "mov QWORD rsi, QWORD [rsp+8]\n"); */
  /* fprintf(f, "mov QWORD rdx, QWORD [rsp]\n"); */
  /* fprintf(f, "add rsp, 16\n"); */
}

TARE_DEF bool gen_syscall_args(Generator *g, FILE *f) {
  /* if (!expect_num_or_tape(g->t)) return false; */
  if (!expect_num_or_vid_or_tape(g->t)) return false;

  // This seems to also work with false, false => unclear how
  // necessary it is to keep the mask around.
  gen_arg_(g, g->t->t, f, false, true);

  /* if (!gen_arg(g->t->t, f, false)) { */
  /*   size_t op = fix_size_and_warn(g, g->t->t); */
  /*   if (op < R32_MAX) fprintf(f, "mov QWORD rbx, %zu\n", op); */
  /*   else { */
  /*     size_t index = find_long(g->longs, op); */
  /*     fprintf(f, "mov QWORD rbx, [long_imm_%zu]\n", index); */
  /*   } */
  /*   fprintf(f, "and QWORD rbx, QWORD " READ_MASK "\n"); */
  /* } */
  
  fprintf(f, "push rbx\n");

  return true;
}

TARE_DEF bool gen_syscall(Generator *g, FILE *f) {
  if (!expect_special(g->t, PAR_BGN)) return false;
  size_t pops = 0;
  const char* regs[7] = {"rax", "rdi", "rsi", "rdx", "r10", "r8", "r9"};
  for (size_t i = 0; i < 6; i++) {
    if (!gen_syscall_args(g, f)) return false;
    pops++;
    if (!expect_token_type(g->t, TOKEN_TYPE_SPECIAL)) return false;
    SpecialType s = g->t->t->s;
    if (s == SEP) {
      if (peek_next_token(g->t).t != TOKEN_TYPE_SPECIAL) continue;
      if (!expect_special(g->t, PAR_END)) return false;
      if (!expect_special(g->t, END)) return false;
      break;
    } else if (s == PAR_END) {
      if (!expect_special(g->t, END)) return false;
      break;
    } else return false;
  }

  while (pops--) fprintf(f, "pop %s\n", regs[pops]);
  fprintf(f, "syscall\n");
  return true;
}

TARE_DEF bool gen_fid(Generator *g, FILE *f) {
  Token *tok = g->t->t;

  size_t fid = tok->fid;
  Func *fn = g->funcs->items + fid;
  
  /* for (size_t i = 0; i < fn->rets.count; i++) fprintf(f, "push 0\n"); */
  Func *cfn = g->funcs->items + g->fni;
    
  size_t c_args_len = total_vars_length(&cfn->args);
  size_t c_rets_len = total_vars_length(&cfn->rets);
  
  /* if (fid != g->fni) { */
    fprintf(f, "add QWORD " ARGS_HEAD ", %zu\n", c_args_len);
    fprintf(f, "add QWORD " RETS_HEAD ", %zu\n", c_rets_len);
    
    // Not sure how to handle this
    /* fprintf(f, "add QWORD " VARS_HEAD ", %zu\n", c_vars_len); */
  /* } */
  
  size_t args_len = total_vars_length(&fn->args);
  /* size_t rets_len = total_vars_length(&fn->rets); */

  size_t current_arg = 0;
  
  if (!expect_special(g->t, PAR_BGN)) return false;
  for (size_t i = 0; i < fn->args.count; i++) {
    if (!expect_num_or_vid_or_tape(g->t)) return false;
    gen_arg_(g, g->t->t, f, false, false); // Should it be false, true?
    Var arg = fn->args.items[i];
    fprintf(f, "mov QWORD rcx, QWORD " ARGS_HEAD "\n");
    fprintf(f, "add QWORD rcx, %zu\n", current_arg);
    switch (arg.tid) {
    case TYPE_U8:
      fprintf(f, "mov BYTE [rcx], bl\n");
      current_arg++;
      break;
    case TYPE_U16:
      fprintf(f, "mov WORD [rcx], bx\n");
      current_arg += 2;
      break;
    case TYPE_U32:
      fprintf(f, "mov DWORD [rcx], ebx\n");
      current_arg += 4;
      break;
    case TYPE_U64:
      fprintf(f, "mov QWORD [rcx], QWORD rbx\n");
      current_arg += 8;
      break;
    case TYPES_COUNT:
      {Tokenizer *t = g->t; unimpl("TYPES_COUNT");}
      break;
    }
    
    if (!expect_special_many(g->t, SEP, PAR_END)) return false;
    if (g->t->t->s == SEP) {
      if (peek_next_token(g->t).t == TOKEN_TYPE_SPECIAL) {
        if (!expect_special(g->t, PAR_END)) return false;
      }
    }
    if (g->t->t->s == PAR_END) {
      if (i + 1 < fn->args.count) {
        // TODO: better error message.
        diag_err(g->t, g->t->t, "incorrect function signature!\n");
        diag_notef(g->t, g->t->t, "function `%.*s` was declared with the following signature\n", SV_ARG(fn->name));
        Token *first = fn->first;
        Token *end = first;
        for (Token *to = fn->first; to < g->t->items + g->t->count; to++) {
          if (to->t != TOKEN_TYPE_SPECIAL) continue;
          if (to->s != DEF) continue;
          end = to;
          break;
        }
        diag_notef(g->t, first, "%.*s\n",
                   (int) (end->f - first->f + end->l), first->f);
        size_t cursor = Keywords[KEY_FUNC].l + 2;
        cursor += strlen(g->t->path);
        size_t row = first->row;
        size_t col = first->col;
        
        while (row != 0) {
          row /= 10;
          cursor++;
        }
        cursor++;
        
        while (col != 0) {
          col /= 10;
          cursor++;
        }
        cursor++;

        cursor += strlen("note: ");
        
        cursor++;
        printf("%*s^~~~~~~~~~~\n", (int) cursor, "");
        /* printf("%*shere\n", (int) cursor, ""); */
        return false;
      }
      break;
    }
  }
  
  if (args_len == 0) if (!expect_special(g->t, PAR_END)) return false;
  if (!expect_special(g->t, END)) return false;
  
  fprintf(f, "call func_%zu\n", fid);
  /* if (fid == g->fni) return true; */
  
  /* size_t rc = fn->rets.count; */
  
  /* size_t rc = rets_len; */
  
  /* for (size_t i = 0; i < ; i++) fprintf(f, "push 0\n"); */
  
  switch (fn->rets.count) {
  case 0: break;
  case 1: fprintf(f, "mov QWORD rax, QWORD " RETS_HEAD "\n"); break;
  default: {Tokenizer *t = g->t; unimpl("not sure what to put here"); break;}
  }
  
  fprintf(f, "sub QWORD " ARGS_HEAD ", %zu\n", c_args_len);
  fprintf(f, "sub QWORD " RETS_HEAD ", %zu\n", c_rets_len);
  
  // Not sure how to handle this
  /* fprintf(f, "sub QWORD " VARS_HEAD ", %zu\n", c_vars_len); */
  
  /* switch (rc) { */
  /* case 0: break; */
  /* case 1: fprintf(f, "pop rax\n"); break; */
  /* default: {Tokenizer *t = g->t; unimpl("not sure what to put here"); break;} */
  /* } */
  
  return true;
}

TARE_DEF bool gen_vid(Generator *g, FILE *f) {
  Tokenizer *t = g->t; // for unimpl

  size_t vid = g->t->t->vid;
  size_t tid = g->t->t->tid;
  if (!expect_special(g->t, EQUAL)) return false;
  if (!expect_num_or_vid_or_tape(g->t)) return false;

  // NOTE: this seems to also work with false, true => unclear what
  // this should mean or imply.
  gen_arg_(g, g->t->t, f, false, false);

  /* if (!gen_arg(g->t->t, f, false)) { */
  /*   size_t op = fix_size_and_warn(g, g->t->t); */
  /*   if (op < R32_MAX) fprintf(f, "mov QWORD rbx, %zu\n", op); */
  /*   else { */
  /*     size_t index = find_long(g->longs, op); */
  /*     fprintf(f, "mov QWORD rbx, [long_imm_%zu]\n", index); */
  /*   } */
  /* } */
  
  /* unimpl("gen_vid"); return true; */
  
  Func *fn = g->funcs->items + g->fni;
  size_t ac = fn->args.count;
  size_t rc = fn->rets.count;
  
  /* assert(vid <= ac + rc && "properly handle vids ffs"); */
  
  size_t index = 0;
  bool is_arg = false;
  bool is_ret = false;
  bool is_var = false;
  
  if (vid < ac) {
    is_arg = true;
    for (size_t i = 0; i < ac; i++) {
      Var arg = fn->args.items[i];
      if (arg.vid == vid) break;
      switch (arg.tid) {
      case TYPE_U8: index++; break;
      case TYPE_U16: index += 2; break;
      case TYPE_U32: index += 4; break;
      case TYPE_U64: index += 8; break;
      default: unimpl("default switch (tid) in gen_vid");
      }
    }
  } else if (vid >= ac && vid < ac + rc) {
    is_ret = true;
    for (size_t i = 0; i < rc; i++) {
      Var ret = fn->rets.items[i];
      if (ret.vid == vid) break;
      switch (ret.tid) {
      case TYPE_U8: index++; break;
      case TYPE_U16: index += 2; break;
      case TYPE_U32: index += 4; break;
      case TYPE_U64: index += 8; break;
      default: unimpl("default switch (tid) in gen_vid");
      }
    }
  } else {
    is_var = true;
    unimpl("index in gen_vid");
  }

  if (is_arg) {
    fprintf(f, "mov QWORD rcx, " ARGS_HEAD "\n");
  } else if (is_ret) {
    fprintf(f, "mov QWORD rcx, " RETS_HEAD "\n");
  } else if (is_var) {
    fprintf(f, "mov QWORD rcx, " VARS_HEAD "\n");
    unimpl("is_var in gen_vid");
  }

  fprintf(f, "add rcx, %zu\n", index);

  switch (tid) {
  case TYPE_U8: fprintf(f, "mov BYTE [rcx], bl\n"); break;
  case TYPE_U16: fprintf(f, "mov WORD [rcx], bx\n"); break;
  case TYPE_U32: fprintf(f, "mov DWORD [rcx], ebx\n"); break;
  case TYPE_U64: fprintf(f, "mov QWORD [rcx], QWORD rbx\n"); break;
  default: unimpl("default switch (tid) in gen_vid");
  }
  
  /* switch (vid) { */
  /* case TYPE_U8: */
  /*   /\* fprintf(f, "mov BYTE [rsp+%zu], bl\n", vid*8); break; *\/ */
  /*   fprintf(f, "mov al, bl\n"); break; */
  /*   unimpl("TYPE_U8"); break; */
  /* case TYPE_U16: */
  /*   /\* fprintf(f, "mov WORD [rsp+%zu], bx\n", vid*8); break; *\/ */
  /*   fprintf(f, "mov ax, bx\n"); break; */
  /*   unimpl("TYPE_U16"); break; */
  /* case TYPE_U32: */
  /*   /\* fprintf(f, "mov DWORD [rsp+%zu], ebx\n", vid*8); break; *\/ */
  /*   fprintf(f, "mov eax, ebx\n"); break; */
  /*   unimpl("TYPE_U32"); break; */
  /* case TYPE_U64: */
  /*   /\* fprintf(f, "mov QWORD [rsp+%zu], QWORD rbx\n", vid*8); break; *\/ */
  /*   fprintf(f, "mov QWORD rax, QWORD rbx\n"); break; */
  /*   unimpl("TYPE_U64"); break; */
  /* case TYPES_COUNT: unimpl("TYPES_COUNT"); break; */
  /* default: unimpl("vid-default"); break; */
  /* } */

  /* fprintf(f, "push QWORD rax\n"); */
  
  /* if (!expect_special(g->t, PAR_END)) return false; */
  if (!expect_special(g->t, END)) return false;
  return true;
}

TARE_DEF bool gen_push(Generator *g, FILE *f) {
  if (!expect_special(g->t, PAR_BGN)) return false;
  if (!expect_num_or_vid_or_tape(g->t)) return false;
  Token *t = g->t->t;
  if (!expect_special(g->t, PAR_END)) return false;
  if (!expect_special(g->t, END)) return false;

  // NOTE: this seems to also work with false, false => what does this
  // mean? Still unclear.
  gen_arg_(g, t, f, false, true);

  /* if (!gen_arg(t, f, false)) { */
  /*   size_t op = fix_size_and_warn(g, t); */
  /*   if (op < R32_MAX) fprintf(f, "mov QWORD rbx, %zu\n", op); */
  /*   else { */
  /*     size_t index = find_long(g->longs, op); */
  /*     fprintf(f, "mov QWORD rbx, [long_imm_%zu]\n", index); */
  /*   } */
  /* } */

  /* fprintf(f, "and QWORD rbx, " READ_MASK "\n"); */
  fprintf(f, "push QWORD rbx\n");
  
  return true;
}

TARE_DEF bool gen_pop(Generator *g, FILE *f) {
  if (!expect_special(g->t, PAR_BGN)) return false;
  if (!expect_vid_or_tape(g->t)) return false;
  Token *t = g->t->t;
  if (!expect_special(g->t, PAR_END)) return false;
  if (!expect_special(g->t, END)) return false;

  /* if (t->t == TOKEN_TYPE_WHOLE_NUM || t->k == KEY_INDEX) return false; */
  if (t->k == KEY_INDEX) return false; // Handle this better.

  fprintf(f, "pop QWORD rax\n");
  fprintf(f, "and QWORD rax, QWORD " READ_MASK "\n");
    
  if (t->k == KEY_TAPE) {
    fprintf(f, "mov QWORD rbx, QWORD " TAPE_HEAD "\n");
    fprintf(f, "mov QWORD [rbx], QWORD rax\n");
  } else if (t->k == KEY_HEAD) {
    fprintf(f, "mov QWORD " TAPE_HEAD ", QWORD rax\n");
  } else if (t->k == KEY_BASE) {
    fprintf(f, "mov QWORD " TAPE_BASE ", QWORD rax\n");
  } else {
    Func *fn = g->funcs->items + g->fni;
    size_t ac = fn->args.count;
    size_t rc = fn->rets.count;
  
    size_t index = 0;
    bool is_arg = false;
    bool is_ret = false;
    bool is_var = false;
  
    if (t->vid < ac) {
      is_arg = true;
      for (size_t i = 0; i < ac; i++) {
        Var arg = fn->args.items[i];
        if (arg.vid == t->vid) break;
        switch (arg.tid) {
        case TYPE_U8: index++; break;
        case TYPE_U16: index += 2; break;
        case TYPE_U32: index += 4; break;
        case TYPE_U64: index += 8; break;
        default: diag_err(g->t, t, "default switch (tid) in gen_vid");
        }
      }
    } else if (t->vid >= ac && t->vid < ac + rc) {
      is_ret = true;
      for (size_t i = 0; i < rc; i++) {
        Var ret = fn->rets.items[i];
        if (ret.vid == t->vid) break;
        switch (ret.tid) {
        case TYPE_U8: index++; break;
        case TYPE_U16: index += 2; break;
        case TYPE_U32: index += 4; break;
        case TYPE_U64: index += 8; break;
        default: diag_err(g->t, t, "default switch (tid) in gen_vid");
        }
      }
    } else {
      is_var = true;
      diag_err(g->t, t, "index in gen_vid");
    }

    if (is_arg) {
      fprintf(f, "mov QWORD rcx, " ARGS_HEAD "\n");
    } else if (is_ret) {
      fprintf(f, "mov QWORD rcx, " RETS_HEAD "\n");
    } else if (is_var) {
      fprintf(f, "mov QWORD rcx, " VARS_HEAD "\n");
      diag_err(g->t, t, "is_var in gen_vid");
    }

    fprintf(f, "add QWORD rcx, %zu\n", index);

    switch (t->tid) {
    case TYPE_U8: fprintf(f, "mov BYTE [rcx], al\n"); break;
    case TYPE_U16: fprintf(f, "mov WORD [rcx], ax\n"); break;
    case TYPE_U32: fprintf(f, "mov DWORD [rcx], eax\n"); break;
    case TYPE_U64: fprintf(f, "mov QWORD [rcx], QWORD rax\n"); break;
    default: diag_err(g->t, t, "default switch (tid) in gen_vid");
    }
  }
  
  return true;
}

TARE_DEF size_t fix_size_and_warn(const Generator *g, const Token *t) {
  size_t op = t->u64;
  if (g->r == 1 && op > R8_MAX) {
    diag_warn(g->t, t, "operand of 8 bits read tries to use more than 8 bits in its operand. Please fix this if this is an error.\n");
    op %= R8_MAX;
  } else if (g->r == 2 && op > R16_MAX) {
    diag_warn(g->t, t, "operand of 16 bits read tries to use more than 16 bits in its operand. Please fix this if this is an error.\n");
    op %= R16_MAX;
  } else if (g->r == 4 && op > R32_MAX) {
    diag_warn(g->t, t, "operand of 32 bits read tries to use more than 32 bits in its operand. Please fix this if this is an error.\n");
    op %= R32_MAX;
  } else if (g->r == 8 && op > R64_MAX) {
    diag_warn(g->t, t, "operand of 64 bits read tries to use more than 64 bits in its operand. Please fix this if this is an error.\n");
    op %= R64_MAX;
  }

  return op;
}

TARE_DEF size_t total_vars_length(const Vars *vars) {
  size_t total = 0;
  
  for (size_t i = 0; i < vars->count; i++) {
    Var var = vars->items[i];
    switch (var.tid) {
    case TYPE_U8: total++; break;
    case TYPE_U16: total += 2; break;
    case TYPE_U32: total += 4; break;
    case TYPE_U64: total += 8; break;
    case TYPES_COUNT: assert(false && "unimplemented"); break;
    }
  }
  
  return total;
}

#endif // CODEGEN_IMPLEMENTATION

#ifndef SIMULATOR_H_
#define SIMULATOR_H_

// NOTE: the simulator is currently deprecated!

typedef union {
  char *u8;
  unsigned short *u16;
  unsigned int *u32;
  size_t *u64;
} TapeElement;

typedef struct {
  char *items;
  size_t count;
  size_t capacity;
  size_t index;
  TapeElement value;
} Tape;

typedef struct {
  Token *args;
  size_t args_count;
  size_t start;
  size_t end;
} SimFunc;

typedef struct {
  SimFunc *items;
  size_t count;
  size_t capacity;
} SimFuncs;

typedef struct {
  Tape *tape;
  bool fail;
  size_t r; // current read size in bytes
  Tokenizer *t;
  String errors;
  Longs error_indices;
  SimFuncs fns;
} Simulator;

TARE_DEF bool sim_tare(Tokenizer *t);
TARE_DEF TapeElement read_from_tape(Tape *tape, size_t r);
TARE_DEF size_t check_for_warning(const Simulator *sim, size_t op);

TARE_DEF bool patch_tokenizer_sim_funcs(Tokenizer *t, Simulator *sim);

TARE_DEF size_t calculate_tape(const Simulator *sim, const Tape *tape);
TARE_DEF size_t calculate_head(const Simulator *sim, const Tape *tape);
TARE_DEF size_t calculate_base(const Simulator *sim, const Tape *tape);


#endif // SIMULATOR_H_

#ifdef SIMULATOR_IMPLEMENTATION

TARE_DEF bool sim_tare(Tokenizer *t) {
  if (t == NULL) return false;
  
  first_token(t);
  if (!patch_tokenizer_jmp(t)) return false;

  Tape tape = {0};
  Simulator sim = {.t = t, .tape = &tape};
  TapeElement value = {0};
  SimFunc main_func = {0};
  da_append(&sim.fns, main_func);
  char tape_fixed[1024*1024] = {0};
  tape.items = tape_fixed;
  size_t op = 0;
  Longs ret_stack = {0};
  if (!patch_tokenizer_sim_funcs(t, &sim)) return false;

  while (!sim.fail) {
    check_failure(t, &sim.errors, &sim.error_indices, sim.r, &sim.fail);
    if (sim.fail) return false;
    value = read_from_tape(sim.tape, sim.r);
    switch (t->t->t) {
    case TOKEN_TYPE_NAME: unimpl("TOKEN_TYPE_NAME"); break;
    case TOKEN_TYPE_WHOLE_NUM: unimpl("TOKEN_TYPE_WHOLE_NUM"); break;
    case TOKEN_TYPE_FRAC_NUM: unimpl("TOKEN_TYPE_FRAC_NUM"); break;
    case TOKEN_TYPE_KEYWORD:
      switch (t->t->k) {
      case KEY_F:
        if (!expect_num_or_tape(t)) return false;
        op = check_for_warning(&sim, t->t->u64);
        if (t->t->k == KEY_TAPE) op = calculate_tape(&sim, &tape);
        else if (t->t->k == KEY_HEAD) op = calculate_head(&sim, &tape);
        else if (t->t->k == KEY_BASE) op = calculate_base(&sim, &tape);
        else if (t->t->k == KEY_INDEX) op = tape.index;
        if (t->t->k == KEY_TAPE || t->t->t == TOKEN_TYPE_WHOLE_NUM) op *= sim.r;
        tape.index += op;
        if (tape.index >= 1024*1024) {
          print_loc(stderr, t, t->t);
          fprintf(stderr, "error: Stack Overflow!\n");
          return false;
        }
        break;
      case KEY_B:
        if (!expect_num_or_tape(t)) return false;
        op = check_for_warning(&sim, t->t->u64);
        if (t->t->k == KEY_TAPE) op = calculate_tape(&sim, &tape);
        else if (t->t->k == KEY_HEAD) op = calculate_head(&sim, &tape);
        else if (t->t->k == KEY_BASE) op = calculate_base(&sim, &tape);
        else if (t->t->k == KEY_INDEX) op = tape.index;
        if (t->t->k == KEY_TAPE || t->t->t == TOKEN_TYPE_WHOLE_NUM) op *= sim.r;
        if (tape.index >= op) tape.index -= op;
        else {
          print_loc(stderr, t, t->t);
          fprintf(stderr, "error: Stack Underflow!\n");
          return false;
        }
        break;
      case KEY_N: tape.index += sim.r; break;
      case KEY_P: if (tape.index >= sim.r) tape.index -= sim.r; break;
      case KEY_A:
        if (!expect_num_or_tape(t)) return false;
        op = check_for_warning(&sim, t->t->u64);
        if (t->t->k == KEY_TAPE) op = calculate_tape(&sim, &tape);
        else if (t->t->k == KEY_HEAD) op = calculate_head(&sim, &tape);
        else if (t->t->k == KEY_BASE) op = calculate_base(&sim, &tape);
        else if (t->t->k == KEY_INDEX) op = tape.index;
        if (sim.r == 1) *value.u8 += op;
        else if (sim.r == 2) *(unsigned short*)value.u16 += op;
        else if (sim.r == 4) *(unsigned int*)value.u32 += op;
        else if (sim.r == 8) *(size_t*)value.u64 += op;
        break;
      case KEY_S:
        if (!expect_num_or_tape(t)) return false;
        op = check_for_warning(&sim, t->t->u64);
        if (t->t->k == KEY_TAPE) op = calculate_tape(&sim, &tape);
        else if (t->t->k == KEY_HEAD) op = calculate_head(&sim, &tape);
        else if (t->t->k == KEY_BASE) op = calculate_base(&sim, &tape);
        else if (t->t->k == KEY_INDEX) op = tape.index;
        if (sim.r == 1) *value.u8 -= op;
        else if (sim.r == 2) *(unsigned short*)value.u16 -= op;
        else if (sim.r == 4) *(unsigned int*)value.u32 -= op;
        else if (sim.r == 8) *(size_t*)value.u64 -= op;
        break;
      case KEY_I:
        if (sim.r == 1) (*value.u8)++;
        else if (sim.r == 2) (*(unsigned short*)value.u16)++;
        else if (sim.r == 4) (*(unsigned int*)value.u32)++;
        else if (sim.r == 8) (*(size_t*)value.u64)++;
        break;
      case KEY_D:
        if (sim.r == 1) (*value.u8)--;
        else if (sim.r == 2) (*(unsigned short*)value.u16)--;
        else if (sim.r == 4) (*(unsigned int*)value.u32)--;
        else if (sim.r == 8) (*(size_t*)value.u64)--;
        break;
      case KEY_R8: sim.r = 1; break;
      case KEY_R16: sim.r = 2; break;
      case KEY_R32: sim.r = 4; break;
      case KEY_R64: sim.r = 8; break;
      case KEY_IF:
        if (sim.r == 1) op = *value.u8;
        else if (sim.r == 2) op = *(unsigned short*)value.u16;
        else if (sim.r == 4) op = *(unsigned int*)value.u32;
        else if (sim.r == 8) op = *(size_t*)value.u64;
        if (!op) t->index = t->t->jmp;
        break;
      case KEY_WHILE:
        if (sim.r == 1) op = *value.u8;
        else if (sim.r == 2) op = *(unsigned short*)value.u16;
        else if (sim.r == 4) op = *(unsigned int*)value.u32;
        else if (sim.r == 8) op = *(size_t*)value.u64;
        if (op) break;
        t->index = t->t->jmp;
        continue;
        break;
      case KEY_END:
        {
          Token *cond = t->items + t->t->jmp;
          if (cond->k == KEY_IF) break;
        
          if (sim.r == 1) op = *value.u8;
          else if (sim.r == 2) op = *(unsigned short*)value.u16;
          else if (sim.r == 4) op = *(unsigned int*)value.u32;
          else if (sim.r == 8) op = *(size_t*)value.u64;
          if (op) t->index = t->t->jmp;
        }
        break;
      case KEY_FUNC:
        {
        if (!expect_fid(t)) return false;
        SimFunc fn = sim.fns.items[t->t->fid];
        size_t end = fn.end;
        if (!to_token(t, end)) return false;
        }
        break;
      /* case KEY_START: break; */
      case KEY_RET: t->index = ret_stack.items[--ret_stack.count]; break;
      case KEY_SYSCALL: unimpl("KEY_SYSCALL"); break;
      case KEY_WRITE:
        {
          if (!expect_num_or_tape(t)) return false;
          size_t arg1 = t->t->u64;
          if (t->t->k == KEY_TAPE) arg1 = calculate_tape(&sim, &tape);
          else if (t->t->k == KEY_HEAD) arg1 = calculate_head(&sim, &tape);
          else if (t->t->k == KEY_BASE) arg1 = calculate_base(&sim, &tape);
          else if (t->t->k == KEY_INDEX) arg1 = tape.index;
          /* if (t->t->k == KEY_TAPE || t->t->t == TOKEN_TYPE_WHOLE_NUM) */
          /*   arg1 *= sim.r; */
          
          if (!expect_num_or_tape(t)) return false;
          size_t arg2 = t->t->u64;
          if (t->t->k == KEY_TAPE) arg2 = calculate_tape(&sim, &tape);
          else if (t->t->k == KEY_HEAD) arg2 = calculate_head(&sim, &tape);
          else if (t->t->k == KEY_BASE) arg2 = calculate_base(&sim, &tape);
          else if (t->t->k == KEY_INDEX) arg2 = tape.index;
          if (t->t->k == KEY_TAPE || t->t->t == TOKEN_TYPE_WHOLE_NUM)
            arg2 *= sim.r;

          size_t w = write(1, (char *)arg1, arg2);
          if (w != arg2) fprintf(stderr, "Error?\n");
        }
        break;
      case KEY_READ:
        {
          if (!expect_num_or_tape(t)) return false;
          size_t arg1 = t->t->u64;
          if (t->t->k == KEY_TAPE) arg1 = calculate_tape(&sim, &tape);
          else if (t->t->k == KEY_HEAD) arg1 = calculate_head(&sim, &tape);
          else if (t->t->k == KEY_BASE) arg1 = calculate_base(&sim, &tape);
          else if (t->t->k == KEY_INDEX) arg1 = tape.index;
          /* if (t->t->k == KEY_TAPE || t->t->t == TOKEN_TYPE_WHOLE_NUM) */
          /*   arg1 *= sim.r; */
          
          if (!expect_num_or_tape(t)) return false;
          size_t arg2 = t->t->u64;
          if (t->t->k == KEY_TAPE) arg2 = calculate_tape(&sim, &tape);
          else if (t->t->k == KEY_HEAD) arg2 = calculate_head(&sim, &tape);
          else if (t->t->k == KEY_BASE) arg2 = calculate_base(&sim, &tape);
          else if (t->t->k == KEY_INDEX) arg2 = tape.index;
          if (t->t->k == KEY_TAPE || t->t->t == TOKEN_TYPE_WHOLE_NUM)
            arg2 *= sim.r;
          
          size_t r = read(0, (char *)arg1, arg2);
          if (r != arg2) fprintf(stderr, "Error?\n");
        }
        break;
      case KEY_TAPE: unimpl("KEY_TAPE"); break;
      case KEY_HEAD: unimpl("KEY_HEAD"); break;
      case KEY_BASE: unimpl("KEY_BASE"); break;
      case KEY_INDEX: unimpl("KEY_INDEX"); break;
      case KEY_CONST: unimpl("KEY_CONST"); break;
      case KEYWORD_TYPES: unimpl("KEYWORD_TYPES"); break;
      default: unimpl("fuck you too\n"); break;
      }
      break;
    case TOKEN_TYPE_SPECIAL: unimpl("TOKEN_TYPE_SPECIAL"); break;
    case TOKEN_TYPE_STRING: unimpl("TOKEN_TYPE_STRING"); break;
    case TOKEN_TYPE_CHAR: unimpl("TOKEN_TYPE_CHAR"); break;
    case TOKEN_TYPE_VID: unimpl("TOKEN_TYPE_VID"); break;
    case TOKEN_TYPE_TID: unimpl("TOKEN_TYPE_TID"); break;
    case TOKEN_TYPE_FID:
      {
        SimFunc fn = sim.fns.items[t->t->fid];
        bool stack_empty = ret_stack.count == 0;
        size_t last_ret = 0;
        if (!stack_empty) last_ret = ret_stack.items[ret_stack.count - 1];
        size_t ret = t->index;
        if (stack_empty || ret != last_ret) da_append(&ret_stack, ret);
        t->index = fn.start;
      }
      break;
    case TOKEN_TYPES: unimpl("TOKEN_TYPES"); break;
    }

    if (!next_token(t)) break;
  }

  return !sim.fail;
}

TARE_DEF TapeElement read_from_tape(Tape *tape, size_t r) {
  TapeElement element = {0};
  
  if (r == 0) return element;

  char *point = tape->items + tape->index;
  
  if (r == 1) element.u8 = point;
  else if (r == 2) element.u16 = (unsigned short *) point;
  else if (r == 4) element.u32 = (unsigned int *) point;
  else if (r == 8) element.u64 = (size_t *) point;

  tape->value = element;
  
  return element;
}

TARE_DEF size_t check_for_warning(const Simulator *sim, size_t op) {
  if (sim->r == 1 && op >= R8_MAX) {
    print_loc(stderr, sim->t, sim->t->t);
    fprintf(stderr, "WARNING: operand of 8 bits read tries to use"
            " more than 8 bits in its operand. Please fix this if"
            " this is an error.\n");
    op %= R8_MAX;
  } else if (sim->r == 2 && op >= R16_MAX) {
    print_loc(stderr, sim->t, sim->t->t);
    fprintf(stderr, "WARNING: operand of 16 bits read tries to use"
            " more than 16 bits in its operand. Please fix this if"
            " this is an error.\n");
    op %= R16_MAX;
  } else if (sim->r == 4 && op >= R32_MAX) {
    print_loc(stderr, sim->t, sim->t->t);
    fprintf(stderr, "WARNING: operand of 32 bits read tries to use"
            " more than 32 bits in its operand. Please fix this if"
            " this is an error.\n");
    op %= R32_MAX;
  } else if (sim->r == 8 && op >= R64_MAX) {
    print_loc(stderr, sim->t, sim->t->t);
    fprintf(stderr, "WARNING: operand of 64 bits read tries to use"
            " more than 64 bits in its operand. Please fix this if"
            " this is an error.\n");
    op %= R64_MAX;
  }

  return op;
}

TARE_DEF bool patch_tokenizer_sim_funcs(Tokenizer *t, Simulator *sim) {
  size_t point = t->index;
  
  first_token(t);
  while (true) {
    if (t->t->t != TOKEN_TYPE_KEYWORD) {
      if (!next_token(t)) break;
      continue;
    }
    if (t->t->k != KEY_FUNC) {
      if (!next_token(t)) break;
      continue;
    }
    
    // Get the function's name.
    if (!expect_name(t)) return false;
    t->t->t = TOKEN_TYPE_FID;
    t->t->fid = sim->fns.count;
    Token *final_token = t->items + t->count;
    for (Token *tok = t->t; tok < final_token; tok++) {
      if (tok->t != TOKEN_TYPE_NAME) continue;
      if (tok_eq(t->t, tok)) {
        tok->t = TOKEN_TYPE_FID;
        tok->fid = sim->fns.count;
      }
    }

    SimFunc fn = {0};

    // Gather function arguments.
    /* while (peek_next_token(t).k != KEY_START) { */
    /*   if (!expect_name_or_start(t)) return false; */
    /*   if (fn.args != NULL) fn.args = t->t; */
    /*   fn.args++; */
    /*   for (Token *tok = t->t; tok < final_token; tok++) { */
    /*     if (tok->k == KEY_RET) break; */
    /*     if (tok->t != TOKEN_TYPE_NAME) continue; */
    /*     if (tok_eq(t->t, tok)) { */
    /*       tok->t = TOKEN_TYPE_VID; */
    /*       tok->vid = fn.args_count; */
    /*     } */
    /*   } */
    /* } */

    if (!next_token(t)) return false;
    fn.start = t->index;
    while (t->t->k != KEY_RET) if (!next_token(t)) return false;
    fn.end = t->index;
    da_append(&sim->fns, fn);
    if (!next_token(t)) break;
  }

  return to_token(t, point);
}

TARE_DEF size_t calculate_tape(const Simulator *sim, const Tape *tape) {
  size_t op = 0;
  if (sim->r == 1) op = tape->items[tape->index];
  else if (sim->r == 2) op = *(unsigned short*)(tape->items + tape->index);
  else if (sim->r == 4) op = *(unsigned int*)(tape->items + tape->index);
  else if (sim->r == 8) op = *(size_t*)(tape->items + tape->index);
  return op;
}

TARE_DEF size_t calculate_head(const Simulator *sim, const Tape *tape) {
  size_t head = 0;
  const char *addr = tape->items + tape->index;
  if (sim->r == 1) head = (char) (size_t) addr;
  else if (sim->r == 2) head = (unsigned short) (size_t) addr;
  else if (sim->r == 4) head = (unsigned int) (size_t) addr;
  else if (sim->r == 8) head = (size_t) addr;
  
  return head;
}

TARE_DEF size_t calculate_base(const Simulator *sim, const Tape *tape) {
  size_t base = 0;
  if (sim->r == 1) base = (char) (size_t) tape->items;
  else if (sim->r == 2) base = (unsigned short) (size_t) tape->items;
  else if (sim->r == 4) base = (unsigned int) (size_t) tape->items;
  else if (sim->r == 8) base = (size_t) tape->items;
  return base;
}

#endif // SIMULATOR_IMPLEMENTATION

#ifndef SIMULATOR_H_
#define SIMULATOR_H_

typedef union {
  unsigned char *u8;
  unsigned short *u16;
  unsigned int *u32;
  size_t *u64;
} TapeElement;

typedef struct {
  char *items;
  size_t capacity;
  size_t base;
  size_t head;
  TapeElement value;
} Tape;

typedef struct {
  ScopeType type;
  size_t vid;
} Vid;

typedef struct {
  Vid *items;
  size_t count;
  size_t capacity;
} Vids;

typedef struct {
  Tape *tape;
  bool fail;
  size_t r; // current read size in bytes
  Parser *p;
  Tape *args;
  Tape *rets;
  Tape *globals;
  Tape *locals;
  Function *fn;
  Operation *op;
  Functions *fns;
  size_t fni; // current function index
  size_t opi; // current operation index
  Vars *global_vars;
  Longs *stack;
  Longs ret_addrs;
  Longs addrs;
  Vids *vids;
} Simulator;

TARE_DEF int sim_tare(Parser *p);

TARE_DEF bool sim_func(Simulator *sim);
TARE_DEF bool sim_op(Simulator *sim);

TARE_DEF bool sim_funcall(Simulator *sim, size_t fid);

TARE_DEF void read_from_tape(Simulator *sim);
TARE_DEF void write_to_tape(Simulator *sim, size_t value);
TARE_DEF size_t value_from_element(Simulator *sim);

TARE_DEF void init_tape(Tape *tape, size_t tape_size);

#define STACK_SIZE 1024*1024*8

#endif // SIMULATOR_H_

#ifdef SIMULATOR_IMPLEMENTATION

TARE_DEF int sim_tare(Parser *p) {
  if (p == NULL) return -1;

  Tape tape = {0};
  init_tape(&tape, TAPE_SIZE);
  Tape args = {0};
  init_tape(&args, ARG_STACK_SIZE);
  Tape rets = {0};
  init_tape(&rets, RET_STACK_SIZE);
  Tape globals = {0};
  init_tape(&globals, GLOBAL_VAR_STACK_SIZE);
  Tape locals = {0};
  init_tape(&locals, LOCAL_VAR_STACK_SIZE);
  Longs stack = {0};
  Vids vids = {0};
  
  Simulator sim = { .tape = &tape, .r = 8, .p = p,
                    .args = &args, .rets = &rets,
                    .globals = &globals, .locals = &locals,
                    .stack = &stack, .global_vars = p->globals,
                    .vids = &vids, .fns = p->funcs, };
  
  if (!sim_funcall(&sim, 0)) return -1;
  
  int ret = 0;

  if (sim.fni != 0) return -1;
  if (stack.count > 0) {
    ret = (int) stack.items[--stack.count];
  }
  putchar(10);


  for (size_t i = 0; i < stack.count; i++) {
    size_t num = stack.items[i];
    printf("stack[%zu] = %zu\n", i, num);
  }
  
  if (tape.items) free(tape.items);
  if (args.items) free(args.items);
  if (rets.items) free(rets.items);
  if (globals.items) free(globals.items);
  if (locals.items) free(locals.items);
  if (stack.items) free(stack.items);
  if (vids.items) free(vids.items);
  if (sim.ret_addrs.items) free(sim.ret_addrs.items);
  if (stack.count == 0) return ret;

  return -1;
}

TARE_DEF bool sim_func(Simulator *sim) {
  Function *fn = sim->fns->items + sim->fni;

  sim->opi = 0;

  while (sim->opi < fn->count) {
    Operation *op = fn->items + sim->opi;
    sim->op = op;
    if (!sim_op(sim)) return false;
    if (op->type == OP_RET) break;
    sim->opi++;
  }

  return true;
}

TARE_DEF bool sim_op(Simulator *sim) {
  Parser *p = sim->p;
  Operation *op = sim->op;

  Tokenizer *t = p->t;
  t->t = op->start;

  read_from_tape(sim);
  size_t value = value_from_element(sim);

  Tape *tape = sim->tape;

  Longs *stack = sim->stack;
  Longs *addrs = &sim->addrs;
  
  switch (op->type) {
  case OP_PTR_ADD:
  case OP_PTR_SUB:
    {
      if (stack->count < 1) return false;
      size_t first = stack->items[--stack->count];
      if (op->type == OP_PTR_ADD) tape->head += (first * sim->r);
      else if (op->type == OP_PTR_SUB) tape->head -= (first * sim->r);
      else assert(false && "unreachable");
    }
    break;
  case OP_ELEM_ADD:
  case OP_ELEM_SUB:
    {
      if (stack->count < 1) return false;
      size_t first = stack->items[--stack->count];
      if (op->type == OP_ELEM_ADD) write_to_tape(sim, value + first);
      else if (op->type == OP_ELEM_SUB) write_to_tape(sim, value - first);
      else assert(false && "unreachable");
    }
    break;
  case OP_READ_SIZE: sim->r = op->op; break;
  case OP_CONDITIONAL:
    {
      if (stack->count < 1) return false;
      size_t first = stack->items[--stack->count];
      if (first == 0) sim->opi = op->op;
    }
    break;
  case OP_GOTO: sim->opi = op->op; break;
  case OP_ADDRESS: break;
    da_append(addrs, op->op); break;
  case OP_FUNCALL:
    if (!sim_funcall(sim, op->op)) return false;
    break;
  case OP_RET:
    if (sim->ret_addrs.count < 1) return false;
    sim->fni = sim->ret_addrs.items[--sim->ret_addrs.count];
    sim->fn = sim->fns->items + sim->fni;
    break;
  case OP_WRITE:
  case OP_READ:
    {
      if (stack->count < 2) return false;
      size_t second = stack->items[--stack->count] * sim->r;
      char *first = (char *) (stack->items[--stack->count]);
      if (second > tape->capacity) second = tape->capacity;
      size_t result = 0;
      if (op->type == OP_WRITE) result = write(STDOUT_FILENO, first, second);
      else if (op->type == OP_READ) result = read(STDIN_FILENO, first, second);
      else assert(false && "uncreachable");
      result = second;
      if (result != second) return false;
    }
    break;
  case OP_SYSCALL:
    {
      size_t args[7] = {0};
      size_t pops = op->op;
      while (pops--) {
        if (stack->count < 1) return false;
        args[pops] = stack->items[--stack->count];
      }
      long result = 0;
      if (op->op == 1) {
        result = syscall(args[0]);
      } else if (op->op == 2) {
        result = syscall(args[0], args[1]);
      } else if (op->op == 3) {
        result = syscall(args[0], args[1], args[2]);
      } else if (op->op == 4) {
        result = syscall(args[0], args[1], args[2], args[3]);
      } else if (op->op == 5) {
        result = syscall(args[0], args[1], args[2], args[3], args[4]);
      } else if (op->op == 6) {
        result = syscall(args[0], args[1], args[2], args[3], args[4], args[5]);
      } else if (op->op == 7) {
        result = syscall(args[0], args[1], args[2], args[3], args[4],
                         args[5], args[6]);
      }
      if (result == ENOSYS) {
        diag_errf(t, t->t, "syscall number %zu has not been implemented!\n",
                  args[0]);
      }
    }
    break;
  
  case OP_TAPE: da_append(stack, value); break;
  case OP_HEAD: da_append(stack, tape->head); break;
  case OP_BASE: da_append(stack, tape->base); break;
  case OP_INDEX: da_append(stack, tape->head - tape->base); break;
  case OP_LENGTH: da_append(stack, tape->capacity); break;
  
  case OP_CONST: unimpl("OP_CONST"); break;

  case OP_PUSH: case OP_POP:
    if (stack->count < 1) return false;
    stack->count--;
    break;

  case OP_ADD: case OP_SUB:
  case OP_MUL: case OP_DIV: case OP_MOD:
  case OP_SHL: case OP_SHR:
  case OP_LESS: case OP_LESS_EQUAL:
  case OP_GREATER: case OP_GREATER_EQUAL:
  case OP_EQUAL: case OP_NOT_EQUAL:
  case OP_BITWISE_AND: case OP_BITWISE_OR:
  case OP_LOGICAL_AND: case OP_LOGICAL_OR:
    {
      if (stack->count < 2) return false;
      size_t second = stack->items[--stack->count];
      size_t first = stack->items[--stack->count];

      size_t val = 0;
      if (op->type == OP_ADD) val = first + second;
      else if (op->type == OP_SUB) val = first - second;
      else if (op->type == OP_MUL) val = first * second;
      else if (op->type == OP_DIV) val = first / second;
      else if (op->type == OP_MOD) val = first % second;
      else if (op->type == OP_SHL) val = first << second;
      else if (op->type == OP_SHR) val = first >> second;
      else if (op->type == OP_LESS) val = first < second;
      else if (op->type ==  OP_LESS_EQUAL) val = first <= second;
      else if (op->type == OP_GREATER) val = first > second;
      else if (op->type ==  OP_GREATER_EQUAL) val = first >= second;
      else if (op->type == OP_EQUAL) val = first == second;
      else if (op->type ==  OP_NOT_EQUAL) val = first != second;
      else if (op->type ==  OP_BITWISE_AND) val = first & second;
      else if (op->type ==  OP_BITWISE_OR) val = first | second;
      else if (op->type ==  OP_LOGICAL_AND) val = first && second;
      else if (op->type ==  OP_LOGICAL_OR) val = first || second;
      else assert(false && "unreachable");
      
      da_append(stack, val);
    }
    break;
    
  case OP_NOT:
    {
      if (stack->count < 1) return false;
      size_t first = stack->items[--stack->count];
      da_append(stack, !first);
    }
    break;
  
  case OP_DEREF:
    {
      if (stack->count < 1) return false;
      if (sim->vids->count < 1) return false;
      size_t first = stack->items[--stack->count];
      char *item = (char *) first;
      Vid vid = sim->vids->items[--sim->vids->count];

      const Var *var = NULL;
      
      switch (vid.type) {
      case SCOPE_GLOBAL:
        if (vid.vid >= sim->global_vars->capacity) return false;
        var = sim->global_vars->items + vid.vid;
        break;
      case SCOPE_LOCAL:
        if (vid.vid >= sim->fn->lvars.capacity) return false;
        var = sim->fn->lvars.items + vid.vid;
        break;
      case SCOPE_ARGUMENT:
        if (vid.vid >= sim->fn->args.capacity) return false;
        var = sim->fn->args.items + vid.vid;
        break;
      case SCOPE_RETURN:
        if (vid.vid >= sim->fn->rets.capacity) return false;
        var = sim->fn->rets.items + vid.vid;
        break;
      case SCOPE_TYPES: default: assert(false && "unreachable");
      }

      if (var == NULL) return false;

      size_t val = 0;

      switch (var->tid) {
      case TYPE_U8: val = *((unsigned char *) item); break;
      case TYPE_U16: val = *((unsigned short *) item); break;
      case TYPE_U32: val = *((unsigned int *) item); break;
      case TYPE_U64: val = *((size_t *) item); break;
      case TYPES_COUNT: default: assert(false && "unimplemented");
      }

      da_append(stack, val);
    }
    break;

  case OP_NUM: da_append(stack, op->op); break;

  case OP_POP_FROM_OPS:
    if (stack->count < 1) return false;
    stack->count--;
    break;

  case OP_GVID:
  case OP_LVID:
  case OP_RVID:
  case OP_AVID:
    {
      char *item = NULL;
      char *head = NULL;
      size_t offset = 0;

      Vid vid = { .vid = op->op };

      if (op->type == OP_GVID) {
        offset = get_var_offset(op->op, sim->global_vars);
        head = (char *) sim->globals->head;
        vid.type = SCOPE_GLOBAL;
      } else if (op->type == OP_LVID) {
        offset = get_var_offset(op->op, &sim->fn->lvars);
        head = (char *) sim->locals->head;
        vid.type = SCOPE_LOCAL;
      } else if (op->type == OP_RVID) {
        offset = get_var_offset(op->op, &sim->fn->rets);
        head = (char *) sim->rets->head;
        vid.type = SCOPE_RETURN;
      } else if (op->type == OP_AVID) {
        offset = get_var_offset(op->op, &sim->fn->args);
        head = (char *) sim->args->head;
        vid.type = SCOPE_ARGUMENT;
      } else assert(false && "unreachable");
      item = head + offset;
      da_append(stack, (size_t) item);
      da_append(sim->vids, vid);
    }
    break;

  case OP_ASSIGN:
    {
      if (stack->count < 2) return false;
      if (sim->vids->count < 1) return false;
      size_t second = stack->items[--stack->count];
      size_t first = stack->items[--stack->count];
      char *item = (char *) first;
      Vid vid = sim->vids->items[--sim->vids->count];
      
      const Var *var = NULL;
      
      switch (vid.type) {
      case SCOPE_GLOBAL:
        if (vid.vid >= sim->global_vars->capacity) return false;
        var = sim->global_vars->items + vid.vid;
        break;
      case SCOPE_LOCAL:
        if (vid.vid >= sim->fn->lvars.capacity) return false;
        var = sim->fn->lvars.items + vid.vid;
        break;
      case SCOPE_ARGUMENT:
        if (vid.vid >= sim->fn->args.capacity) return false;
        var = sim->fn->args.items + vid.vid;
        break;
      case SCOPE_RETURN:
        if (vid.vid >= sim->fn->rets.capacity) return false;
        var = sim->fn->rets.items + vid.vid;
        break;
      case SCOPE_TYPES: default: assert(false && "unreachable");
      }

      if (var == NULL) return false;
      
      switch (var->tid) {
      case TYPE_U8:
        *item = (unsigned char) second;
        break;
      case TYPE_U16:
        *((unsigned short *)item) = (unsigned short) second;
        break;
      case TYPE_U32:
        *((unsigned int *)item) = (unsigned int) second;
        break;
      case TYPE_U64:
        *((size_t *)item) = second;
        break;
      case TYPES_COUNT: 
      default: assert(false && "unimplemented");
      }
    }
    break;

  case OP_TYPES: unimpl("OP_TYPES"); break;
  default: assert(false && "unreachable");
  }

  return true;
}

TARE_DEF bool sim_funcall(Simulator *sim, size_t fid) {
  Parser *p = sim->p;
  Longs *stack = sim->stack;
  size_t save = sim->opi;
  da_append(&sim->ret_addrs, sim->fni);
  
  Function *fn = sim->fns->items + fid;
  size_t args_fn = get_args_size(fn);
  size_t rets_fn = get_rets_size(fn);
  size_t lvars_fn = get_lvars_size(fn);

  Function *fni = sim->fns->items + sim->fni;
  size_t args_size = get_args_size(fni);
  size_t rets_size = get_rets_size(fni);
  size_t lvars_size = get_lvars_size(fni);

  sim->args->head += args_size;
  sim->rets->head += rets_size;
  sim->locals->head += lvars_size;

  size_t lvars_head = sim->locals->head;

  lvars_head += lvars_fn;
  
  for (size_t i = 0; i < fn->lvars.count; i++) {
    Var var = fn->lvars.items[i];
    switch (var.tid) {
    case TYPE_U8:
      lvars_head--;
      *((unsigned char *) lvars_head) = 0;
      /* lvars_head -= 7; */
      break;
    case TYPE_U16:
      lvars_head -= 2;
      *((unsigned short *) lvars_head) = 0;
      /* lvars_head -= 6; */
      break;
    case TYPE_U32:
      lvars_head -= 4;
      *((unsigned int *) lvars_head) = 0;
      /* lvars_head -= 4; */
      break;
    case TYPE_U64:
      lvars_head -= 8;
      *((size_t *) lvars_head) = 0;
      break;
    case TYPES_COUNT:
    default: assert(false && "unimplemented");
    }
  }
  
  size_t args_head = sim->args->head;
  
  args_head += args_fn;
  
  for (size_t i = 0; i < fn->args.count; i++) {
    size_t argument = stack->items[--stack->count];
    Var arg = fn->args.items[i];
    switch (arg.tid) {
    case TYPE_U8:
      args_head--;
      *((unsigned char *) args_head) = ((unsigned char) argument);
      /* args_head -= 7; */
      break;
    case TYPE_U16:
      args_head -= 2;
      *((unsigned short *) args_head) = ((unsigned short) argument);
      /* args_head -= 6; */
      break;
    case TYPE_U32:
      args_head -= 4;
      *((unsigned int *) args_head) = ((unsigned int) argument);
      /* args_head -= 4; */
      break;
    case TYPE_U64:
      args_head -= 8;
      *((size_t *) args_head) = ((size_t) argument);
      break;
    case TYPES_COUNT:
    default: assert(false && "unimplemented");
    }
  }
  
  sim->fni = fid;
  sim->fn = p->funcs->items + sim->fni;
  if (!(sim_func(sim))) return false;
  sim->opi = save;

  size_t rets_head = sim->rets->head;
  rets_head += rets_fn;

  sim->args->head -= args_size;
  sim->rets->head -= rets_size;
  sim->locals->head -= lvars_size;
  
  for (size_t i = 0; i < fn->rets.count; i++) {
    size_t ret_val = 0;
    Var ret = fn->rets.items[i];
    switch (ret.tid) {
    case TYPE_U8:
      rets_head--;
      ret_val = *((unsigned char *) rets_head);
      /* rets_head -= 7; */
      break;
    case TYPE_U16:
      rets_head -= 2;
      ret_val = *((unsigned short *) rets_head);
      /* rets_head -= 6; */
      break;
    case TYPE_U32:
      rets_head -= 4;
      ret_val = *((unsigned int *) rets_head);
      /* rets_head -= 4; */
      break;
    case TYPE_U64:
      rets_head -= 8;
      ret_val = *((size_t *) rets_head);
      break;
    case TYPES_COUNT: 
    default: assert(false && "unimplemented");
    }
    da_append(stack, ret_val);
  }
  
  return true;
}

TARE_DEF void read_from_tape(Simulator *sim) {
  TapeElement element = {0};
  
  if (sim->r == 1) element.u8 = (unsigned char *) sim->tape->head;
  else if (sim->r == 2) element.u16 = (unsigned short *) sim->tape->head;
  else if (sim->r == 4) element.u32 = (unsigned int *) sim->tape->head;
  else if (sim->r == 8) element.u64 = (size_t *) sim->tape->head;
  else assert(false && "unreachable");

  if (element.u64 >= (size_t *) (sim->tape->items + sim->tape->capacity)) {
    element.u64 = (size_t *) (sim->tape->items + sim->tape->capacity) - 1;
  }

  sim->tape->value = element;
}

TARE_DEF void write_to_tape(Simulator *sim, size_t value) {
  TapeElement element = sim->tape->value;
  
  if (sim->r == 1) *element.u8 = (unsigned char) value;
  else if (sim->r == 2) *element.u16 = (unsigned short) value;
  else if (sim->r == 4) *element.u32 = (unsigned int) value;
  else if (sim->r == 8) *element.u64 = value;
  else assert(false && "unreachable");
}

TARE_DEF size_t value_from_element(Simulator *sim) {
  TapeElement element = sim->tape->value;

  if (sim->r == 1) return (size_t) (*element.u8);
  else if (sim->r == 2) return (size_t) (*element.u16);
  else if (sim->r == 4) return (size_t) (*element.u32);
  else if (sim->r == 8) return (size_t) (*element.u64);
  else assert(false && "unreachable");
}

TARE_DEF void init_tape(Tape *tape, size_t tape_size) {
  tape->items = calloc(tape_size, 1);
  assert(tape->items != NULL && "allocation failed!");
  tape->capacity = tape_size;
  tape->base = (size_t) tape->items;
  tape->head = tape->base;
  tape->value.u64 = (size_t *) tape->items;
}

#endif // SIMULATOR_IMPLEMENTATION

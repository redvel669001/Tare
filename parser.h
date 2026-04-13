#ifndef PARSER_H_
#define PARSER_H_

typedef struct Operation Operation;

typedef struct {
  StringView name;
  size_t vid;
  size_t tid;
} Var;

typedef struct {
  Var *items;
  size_t count;
  size_t capacity;
} Vars;

typedef struct {
  StringView name;
  Token *first;
  
  Vars args;
  Vars rets;
  
  size_t start;
  size_t end;
} Func;

typedef struct {
  Func *items;
  size_t count;
  size_t capacity;
} Funcs;

typedef enum {
  OP_PTR_ADD = 0,
  OP_PTR_SUB,
  OP_ELEM_ADD,
  OP_ELEM_SUB,
  OP_READ_SIZE,
  OP_CONDITIONAL,
  OP_GOTO,
  OP_ADDRESS,
  OP_FUNCALL,
  OP_RET,
  OP_WRITE,
  OP_READ,
  OP_SYSCALL,
  
  OP_TAPE,
  OP_HEAD,
  OP_BASE,
  OP_INDEX,
  OP_CONST,

  OP_PUSH,
  OP_POP,

  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_SHL,
  OP_SHR,
  OP_NOT,
  OP_DEREF,

  OP_ARG,
  OP_NUM,
  
  OP_TYPES,
} OpType;

// TODO: fill this properly
typedef enum {
  PREC_ADD_SUB = 0,
  PREC_MUL_DIV,
  PREC_PAR_BGN,
  OP_PRECS,
} OpPrec;

struct Operation {
  Token *start;
  OpType type;
  Operation *args;
  size_t args_count;
  size_t op;
  StringView name;
};

typedef struct {
  Operation *items;
  size_t count;
  size_t capacity;
  StringView name;
} Function;

typedef struct {
  Function *items;
  size_t count;
  size_t capacity;
} Functions;

typedef struct {
  size_t *items;
  size_t count;
  size_t capacity;
} Longs;

typedef struct {
  Tokenizer *t;
  Functions *funcs;
  Function *func;
  Funcs *fns;
  Longs *gotos;
} Parser;

TARE_DEF bool parse_file(Parser *p);
TARE_DEF StringView sv_from_token(const Token *t);

TARE_DEF bool parse_statement(Parser *p);
TARE_DEF bool parse_expression(Parser *p);

TARE_DEF bool optimize_expression(Parser *p, Operation *op);

TARE_DEF bool check_for_continued_expression(Parser *p);
TARE_DEF OpPrec get_prec_by_op_type(OpType type);
TARE_DEF OpPrec get_prec_by_special_type(SpecialType s);

TARE_DEF bool parse_func_sig(Parser *p);

TARE_DEF void patch_tokenizer_builtin_types(Tokenizer *t);
TARE_DEF bool patch_tokenizer_funcs(Tokenizer *t, Funcs *fns);
TARE_DEF bool patch_tokenizer_func(Tokenizer *t, Funcs *fns);
TARE_DEF bool patch_tokenizer_args(Tokenizer *t, Func *fn, bool args);
TARE_DEF bool patch_tokenizer_rets(Tokenizer *t, Funcs *fns);
TARE_DEF bool patch_tokenizer_bgn_end(Tokenizer *t);

TARE_DEF const char *op_type_as_string(OpType type); // For debugging.
TARE_DEF void print_function(const Tokenizer *t, const Function *func);
TARE_DEF void print_functions(const Parser *p);

#endif // PARSER_H_

#ifdef PARSER_IMPLEMENTATION

TARE_DEF bool parse_file(Parser *p) {
  if (p == NULL) return false;
  
  Tokenizer *t = p->t;
  if (!first_token(t)) return false;

  patch_tokenizer_builtin_types(t);
  if (!patch_tokenizer_bgn_end(t)) return false;
  
  Funcs fns = {0};
  Func main = {.name = SV_MAKE(main)};
  da_append(&fns, main);
  if (!patch_tokenizer_funcs(t, &fns)) return false;

  for (size_t i = 0; i < fns.count; i++) da_append(p->funcs, (Function) {0});

  p->func = p->funcs->items;
  p->fns = &fns;
  
  while (true) {
    if (!parse_statement(p)) return false;
    if (!next_token(t)) break;
  }

  bool debug = false;
  if (debug) print_functions(p);

  // For reference for later, when these things get freed in the
  // proper place. The parser is still in its early stages, so this
  // can wait.
  
  /* // Free fns */
  /* for (size_t i = 0 ; i < fns.count; i++) { */
  /*   Func fn = fns.items[i]; */
  /*   if (fn.args.items) free(fn.args.items); */
  /*   if (fn.rets.items) free(fn.rets.items); */
  /* } */

  /* if (fns.items) free(fns.items); */
  
  /* // Free funcs */
  /* for (size_t i = 0 ; i < funcs->count; i++) { */
  /*   Function fn = funcs->items[i]; */
  /*   if (fn.items) free(fn.items); */
  /* } */

  /* if (funcs->items) free(fns.items); */
  
  return true;
}

TARE_DEF StringView sv_from_token(const Token *t) {
  return (StringView) {.s = t->f, .l = t->l};
}

TARE_DEF bool parse_statement(Parser *p) {
  if (p == NULL) return false;
  
  Tokenizer *t = p->t;

  Operation op = {.start = t->t};
  
  switch (t->t->t) {
  case TOKEN_TYPE_NAME: unimpl("TOKEN_TYPE_NAME"); break;
  case TOKEN_TYPE_WHOLE_NUM: unimpl("TOKEN_TYPE_WHOLE_NUM"); break;
  case TOKEN_TYPE_FRAC_NUM: unimpl("TOKEN_TYPE_FRAC_NUM"); break;
  case TOKEN_TYPE_KEYWORD:
    switch (t->t->k) {
    case KEY_F: case KEY_B:
    case KEY_N: case KEY_P:
    case KEY_A: case KEY_S:
    case KEY_I: case KEY_D:
      if (!parse_expression(p)) return false;
      if (!expect_special(t, END)) return false;
      break;
    case KEY_R8: case KEY_R16: case KEY_R32: case KEY_R64:
      op.type = OP_READ_SIZE;
      if (t->t->k == KEY_R8) op.op = 1;
      else if(t->t->k == KEY_R16) op.op = 2;
      else if(t->t->k == KEY_R32) op.op = 4;
      else if(t->t->k == KEY_R64) op.op = 8;
      da_append(p->func, op);
      break;
    case KEY_IF: case KEY_WHILE:
      {
        Operation address = {.start = op.start, .type = OP_ADDRESS,
                             .op = t->index};
        op.type = OP_CONDITIONAL;
        da_append(p->func, address);
        if (!next_token(t)) return false;
        if (!parse_expression(p)) return false;
        if (!expect_special(t, DEF)) return false;
        if (!next_token(t)) return false;
        if (t->t->t == TOKEN_TYPE_SPECIAL) {
          if (t->t->s == BLK_BGN) {
            op.op = t->t->jmp;
            if (!next_token(t)) return false;
          }
        } else {
          bool found = false;
          for (Token *tok = t->t; tok < t->items + t->count; tok++) {
            if (tok->t != TOKEN_TYPE_SPECIAL) continue;
            if (tok->s == END) {
              op.op = (size_t) (tok - t->items);
              found = true;
            } else if (tok->s == BLK_BGN) {
              op.op = t->t->jmp;
              found = true;
            }
            if (found) break;
          }
          if (!found) return false;
        }
        
        da_append(p->func, op);
        da_append(p->gotos, op.op);
        while (t->index != op.op) {
          if (!parse_statement(p)) return false;
          if (t->index == op.op) break;
          if (!next_token(t)) return false;
        }
        op.start = t->t;
        op.type = OP_GOTO;
        op.op = address.op;
        if (address.start->k == KEY_WHILE) da_append(p->func, op);

        address.start = t->t;
        address.op = t->index;
        da_append(p->func, address);
      }
      break;
    case KEY_FUNC:
      if (!parse_func_sig(p)) return false;
      {
        size_t end = t->t->jmp;
        while (true) {
          if (!next_token(t)) return false;
          if (t->index == end) break;
          if (!parse_statement(p)) return false;
        }
      }
      /* if (!expect_special(t, END)) return false; */
      break;
    case KEY_RET:
      if (!expect_special(t, END)) return false;
      op.type = OP_RET;
      da_append(p->func, op);
      break;
    case KEY_WRITE: case KEY_READ:
      if (!parse_expression(p)) return false;
      if (!expect_special(t, END)) return false;
      break;
    case KEY_SYSCALL:
      if (!parse_expression(p)) return false;
      if (!expect_special(t, END)) return false;
      break;
    case KEY_TAPE: case KEY_HEAD: case KEY_BASE: case KEY_INDEX:
      diag_errf(t, t->t,
                "tare statements do not begin with keyword `%.*s`.\n",
                SV_ARG(Keywords[t->t->k]));
      return false;
    case KEY_CONST: unimpl("KEY_CONST"); break;

      // TODO: FIX `push` AND `pop`
    case KEY_PUSH:
      if (!parse_expression(p)) return false;
      if (!expect_special(t, END)) return false;
      break;
    case KEY_POP:
      if (!parse_expression(p)) return false;
      if (!expect_special(t, END)) return false;
      break;

    case KEY_ADD: case KEY_SUB:
    case KEY_MUL: case KEY_DIV:
    case KEY_SHL: case KEY_SHR:
    case KEY_NOT:
      if (!parse_expression(p)) return false;
      if (!expect_special(t, END)) return false;
      break;
    case KEY_DEREF: unimpl("KEY_DEREF"); break;
    
    case KEYWORD_TYPES: unimpl("KEYWORD_TYPES"); break;

    default: unimpl("default case in parse_keyword_as_op"); break;
    }
    break;
  case TOKEN_TYPE_SPECIAL: unimpl("TOKEN_TYPE_SPECIAL"); break;
  case TOKEN_TYPE_STRING: unimpl("TOKEN_TYPE_STRING"); break;
  case TOKEN_TYPE_CHAR: unimpl("TOKEN_TYPE_CHAR"); break;
  case TOKEN_TYPE_VID: unimpl("TOKEN_TYPE_VID"); break;
  case TOKEN_TYPE_TID: unimpl("TOKEN_TYPE_TID"); break;
  case TOKEN_TYPE_FID:
    if (!parse_expression(p)) return false;
    if (!expect_special(t, END)) return false;
    break;
  case TOKEN_TYPES: unimpl("TOKEN_TYPES"); break;
  default: unimpl("default case in parse_token_as_op"); break;
  }

  return true;
}

TARE_DEF bool parse_expression(Parser *p) {
  if (p == NULL) return false;
  
  Tokenizer *t = p->t;

  Operation op = {.start = t->t};

  switch (t->t->t) {
  case TOKEN_TYPE_NAME: unimpl("TOKEN_TYPE_NAME"); break;
  case TOKEN_TYPE_WHOLE_NUM:
    op.type = OP_NUM;
    op.op = t->t->u64;
    da_append(p->func, op);
    if (!check_for_continued_expression(p)) break;
    if (peek_prev_token(t).t == TOKEN_TYPE_SPECIAL) {
      SpecialType s = peek_prev_token(t).s;
      if (s == DIV || s == MULT || s == ADD || s == SUB) break;
    }
    if (!next_token(t)) return false;
    if (!parse_expression(p)) return false;
    break;
  case TOKEN_TYPE_FRAC_NUM: unimpl("TOKEN_TYPE_FRAC_NUM"); break;
  case TOKEN_TYPE_KEYWORD:
    switch (t->t->k) {
    case KEY_F: case KEY_B:
    case KEY_A: case KEY_S:
      if (t->t->k == KEY_F) op.type = OP_PTR_ADD;
      else if (t->t->k == KEY_B) op.type = OP_PTR_SUB;
      else if (t->t->k == KEY_A) op.type = OP_ELEM_ADD;
      else if (t->t->k == KEY_S) op.type = OP_ELEM_SUB;
      else return false;
      if (!expect_special(t, PAR_BGN)) return false;
      if (!next_token(t)) return false;
      if (!parse_expression(p)) return false;
      if (!expect_special(t, PAR_END)) return false;
      da_append(p->func, op);
      break;
    case KEY_N: case KEY_P:
    case KEY_I: case KEY_D:
      if (t->t->k == KEY_N) op.type = OP_PTR_ADD;
      else if (t->t->k == KEY_P) op.type = OP_PTR_SUB;
      else if (t->t->k == KEY_I) op.type = OP_ELEM_ADD;
      else if (t->t->k == KEY_D) op.type = OP_ELEM_SUB;
      else return false;
      {
        Operation operation = {.start = op.start, .type = OP_NUM, .op = 1};
        da_append(p->func, operation);
      }
      if (!expect_special(t, PAR_BGN)) return false;
      if (!expect_special(t, PAR_END)) return false;
      da_append(p->func, op);
      break;
    case KEY_R8: case KEY_R16: case KEY_R32: case KEY_R64:
      diag_errf(t, t->t,
                "tare expressions do not begin with keyword `%.*s`.\n",
                SV_ARG(Keywords[t->t->k]));
      break;
    case KEY_IF: case KEY_WHILE:
      diag_errf(t, t->t,
                "tare expressions do not begin with keyword `%.*s`.\n",
                SV_ARG(Keywords[t->t->k]));
      break;
    case KEY_FUNC:
      {
        if (!expect_fid(t)) return false;
        const Token *name = t->t;
        size_t fid = t->t->fid;
        Func *fn = p->fns->items + fid;
        // Should probably become more involved than this.
        if (!to_token(t, fn->start)) return false;
        p->func = p->funcs->items + fid;
        p->func->name = sv_from_token(name);
      }
      break;
    case KEY_RET:
      diag_errf(t, t->t,
                "tare expressions do not begin with keyword `%.*s`.\n",
                SV_ARG(Keywords[t->t->k]));
      break;
    case KEY_WRITE: case KEY_READ:
      if (t->t->k == KEY_WRITE) op.type = OP_WRITE;
      else if (t->t->k == KEY_READ) op.type = OP_READ;
      else return false;
      if (!expect_special(t, PAR_BGN)) return false;
      if (!next_token(t)) return false;
      if (!parse_expression(p)) return false;
      if (!expect_special(t, SEP)) return false;
      if (!next_token(t)) return false;
      if (!parse_expression(p)) return false;
      if (!expect_special(t, PAR_END)) return false;
      da_append(p->func, op);
      break;
    case KEY_SYSCALL:
      op.type = OP_SYSCALL;
      if (!expect_special(t, PAR_BGN)) return false;
      while (true) {
        if (!next_token(t)) return false;
        if (!parse_expression(p)) return false;
        if (op.op > 6) return false;
        op.op++;
        if (!expect_token_type(t, TOKEN_TYPE_SPECIAL)) return false;
        SpecialType s = t->t->s;
        if (s == SEP) {
          if (peek_next_token(t).t != TOKEN_TYPE_SPECIAL) continue;
          if (!expect_special(t, PAR_END)) return false;
          break;
        } else if (s == PAR_END) break;
        else return false;
      }
      da_append(p->func, op);
      break;
    case KEY_TAPE: case KEY_HEAD: case KEY_BASE: case KEY_INDEX:
      if (t->t->k == KEY_TAPE) op.type = OP_TAPE;
      else if (t->t->k == KEY_HEAD) op.type = OP_HEAD;
      else if (t->t->k == KEY_BASE) op.type = OP_BASE;
      else if (t->t->k == KEY_INDEX) op.type = OP_INDEX;
      else return false;
      da_append(p->func, op);
      break;
    case KEY_CONST: unimpl("KEY_CONST"); break;

      // TODO: FIX `push` AND `pop`
    case KEY_PUSH:
      op.type = OP_PUSH;
      if (!expect_special(t, PAR_BGN)) return false;
      if (!next_token(t)) return false;
      if (!parse_expression(p)) return false;
      if (!expect_special(t, PAR_END)) return false;
      da_append(p->func, op);
      break;
    case KEY_POP:
      op.type = OP_POP;
      if (!expect_special(t, PAR_BGN)) return false;
      if (!next_token(t)) return false;
      if (!parse_expression(p)) return false;
      if (!expect_special(t, PAR_END)) return false;
      da_append(p->func, op);
      break;

    case KEY_ADD: case KEY_SUB:
    case KEY_MUL: case KEY_DIV:
    case KEY_SHL: case KEY_SHR:
      if (t->t->k == KEY_ADD) op.type = OP_ADD;
      else if (t->t->k == KEY_SUB) op.type = OP_SUB;
      else if (t->t->k == KEY_MUL) op.type = OP_MUL;
      else if (t->t->k == KEY_DIV) op.type = OP_DIV;
      else if (t->t->k == KEY_SHL) op.type = OP_SHL;
      else if (t->t->k == KEY_SHR) op.type = OP_SHR;
      else return false;
      if (!expect_special(t, PAR_BGN)) return false;
      if (!next_token(t)) return false;
      if (!parse_expression(p)) return false;
      if (!expect_special(t, SEP)) return false;
      if (!next_token(t)) return false;
      if (!parse_expression(p)) return false;
      if (!expect_special(t, PAR_END)) return false;
      if (!optimize_expression(p, &op)) return false;
      da_append(p->func, op);
      break;
    case KEY_NOT:
      op.type = OP_NOT;
      if (!next_token(t)) return false;
      if (!parse_expression(p)) return false;
      if (!optimize_expression(p, &op)) return false;
      da_append(p->func, op);
      break;
    case KEY_DEREF: unimpl("KEY_DEREF"); break;
    
    case KEYWORD_TYPES: unimpl("KEYWORD_TYPES"); break;

    default: unimpl("default case in parse_expression keyword switch"); break;
    }
    break;
  case TOKEN_TYPE_SPECIAL:
    switch (t->t->s) {
    case PAR_BGN:
      {
        size_t end = t->t->jmp;
        while (true) {
          if (!next_token(t)) return false;
          if (t->index == end) break;
          if (!parse_expression(p)) return false;
        }
        if (check_for_continued_expression(p)) {
          if (!next_token(t)) return false;
          if (!parse_expression(p)) return false;
        }
      }
      break;
    case PAR_END:
    case GRP_BGN: case GRP_END:
    case BLK_BGN: case BLK_END:
    case END:
    case DOT:
    case DEF:
      diag_errf(t, t->t,
                "tare expressions do not begin with special token `%c`.\n",
                Specials[t->t->s]);
      return false;
    case DQUOTE: case SQUOTE: case ESC:
      diag_errf(t, t->t,
                "special token `%c` at beginning of expression doesn't make sense. This is probably a tokenizer error!\n", Specials[t->t->s]);
      return false;
    case SEP: unimpl("SEP"); break;
      
    case DIV: case MULT: case ADD: case SUB: 
      if (t->t->s == ADD) op.type = OP_ADD;
      else if (t->t->s == SUB) op.type = OP_SUB;
      else if (t->t->s == MULT) op.type = OP_MUL;
      else if (t->t->s == DIV) op.type = OP_DIV;
      else return false;
      // TODO: handle shl and shr with special haracters
      if (!next_token(t)) return false;
      if (!parse_expression(p)) return false;
      if (check_for_continued_expression(p)) {
        OpPrec current = get_prec_by_op_type(op.type);
        OpPrec next = get_prec_by_special_type(peek_next_token(t).s);
        if (next > current) {
          if (!next_token(t)) return false;
          if (!parse_expression(p)) return false;
        }
      }
      if (!optimize_expression(p, &op)) return false;
      da_append(p->func, op);
      
      if (check_for_continued_expression(p)) {
        if (!next_token(t)) return false;
        if (!parse_expression(p)) return false;
      }
      break;
      
    case LESS: unimpl("LESS"); break;
    case GREATER: unimpl("GREATER"); break;
    case EQUAL: unimpl("EQUAL"); break;
    case NOT:
      op.type = OP_NOT;
      if (!next_token(t)) return false;
      if (!parse_expression(p)) return false;
      if (!optimize_expression(p, &op)) return false;
      da_append(p->func, op);
      
      if (check_for_continued_expression(p)) {
        if (!next_token(t)) return false;
        if (!parse_expression(p)) return false;
      }
      break;
    case SPECIAL_TYPES: unimpl("SPECIAL_TYPES"); break;
    }
    
    break;
  case TOKEN_TYPE_STRING: unimpl("TOKEN_TYPE_STRING"); break;
  case TOKEN_TYPE_CHAR: unimpl("TOKEN_TYPE_CHAR"); break;
  case TOKEN_TYPE_VID: unimpl("TOKEN_TYPE_VID"); break;
  case TOKEN_TYPE_TID: unimpl("TOKEN_TYPE_TID"); break;
  case TOKEN_TYPE_FID:
    /* if (p == NULL) return false; */
    /* Tokenizer *t = p->t; */
    op.type = OP_FUNCALL;
    op.op = t->t->fid;
    if (!expect_special(t, PAR_BGN)) return false;
    {
      size_t end = t->t->jmp;
      if (!next_token(t)) return false;
      while (t->index != end) {
        if (!parse_expression(p)) return false;
        if (!expect_special_many(t, SEP, PAR_END)) return false;
        if (!next_token(t)) return false;
      }
    }
    da_append(p->func, op);
    break;
  case TOKEN_TYPES: unimpl("TOKEN_TYPES"); break;
  default: unimpl("default case in parse_expression token type switch"); break;
  }

  return true;  
}

TARE_DEF bool optimize_expression(Parser *p, Operation *op) {
  if (p == NULL) return false;

  switch (op->type) {
  case OP_PTR_ADD: case OP_PTR_SUB: case OP_ELEM_ADD: case OP_ELEM_SUB:
  case OP_READ_SIZE:
  case OP_CONDITIONAL: case OP_GOTO: case OP_ADDRESS:
  case OP_FUNCALL: case OP_RET: case OP_WRITE: case OP_READ: case OP_SYSCALL:
  case OP_TAPE: case OP_HEAD: case OP_BASE: case OP_INDEX:
  case OP_CONST: case OP_PUSH: case OP_POP: case OP_ARG: case OP_NUM:
  case OP_TYPES: default: return false;

  case OP_ADD:
  case OP_SUB:
  case OP_MUL:
  case OP_DIV:
  case OP_SHL:
  case OP_SHR:
    if (p->func->count < 2) return false;
    else {
      Operation *second = p->func->items + p->func->count - 1;
      Operation *first = second - 1;
      if (first->type != OP_NUM || second->type != OP_NUM) return true;
      
      if (op->type == OP_ADD) op->op = first->op + second->op;
      else if (op->type == OP_SUB) op->op = first->op - second->op;
      else if (op->type == OP_MUL) op->op = first->op * second->op;
      else if (op->type == OP_DIV) op->op = first->op / second->op;
      else if (op->type == OP_SHL) op->op = first->op << second->op;
      else if (op->type == OP_SHR) op->op = first->op >> second->op;
      p->func->count -= 2;
      op->type = OP_NUM;
    }
    break;
  case OP_NOT:
    if (p->func->count < 1) return false;
    else {
      Operation *operation = p->func->items + p->func->count - 1;
      if (operation->type == OP_NUM) {
        op->op = !operation->op;
        p->func->count--;
        op->type = OP_NUM;
      }
    }
    break;
    
  case OP_DEREF:
    {
      Tokenizer *t = p->t;
      t->t = op->start;
      unimpl("OP_DEREF optimization");
      /* return false; */
    }
    break;
  }

  return true;
}

TARE_DEF bool check_for_continued_expression(Parser *p) {
  Tokenizer *t = p->t;
  if (peek_next_token(t).t != TOKEN_TYPE_SPECIAL) return false;
  SpecialType s = peek_next_token(t).s;
  if (s != DIV && s != MULT && s != ADD && s != SUB) return false;
  return true;
}

TARE_DEF OpPrec get_prec_by_op_type(OpType type) {
  switch (type) {
  case OP_PTR_ADD: case OP_PTR_SUB: case OP_ELEM_ADD: case OP_ELEM_SUB:
  case OP_READ_SIZE:
  case OP_CONDITIONAL: case OP_GOTO: case OP_ADDRESS:
  case OP_FUNCALL: case OP_RET: case OP_WRITE: case OP_READ: case OP_SYSCALL:
  case OP_TAPE: case OP_HEAD: case OP_BASE: case OP_INDEX:
  case OP_CONST: case OP_PUSH: case OP_POP: case OP_ARG: case OP_NUM:
  case OP_TYPES:
    return OP_PRECS;
    
  case OP_ADD: case OP_SUB: return PREC_ADD_SUB;
  case OP_MUL: case OP_DIV: return PREC_MUL_DIV;
    
  case OP_SHL: return OP_PRECS;
  case OP_SHR: return OP_PRECS;
  case OP_DEREF: return OP_PRECS;

  case OP_NOT: return OP_PRECS;
  default: return OP_PRECS;
  }
}

TARE_DEF OpPrec get_prec_by_special_type(SpecialType s) {
  switch (s) {
  case PAR_BGN: return PREC_PAR_BGN;
  case PAR_END: case GRP_BGN: case GRP_END: case BLK_BGN: case BLK_END:
  case DQUOTE: case SQUOTE: case ESC: case SEP: case END: case DOT: case DEF:
    return OP_PRECS;
  case DIV: case MULT: return PREC_MUL_DIV;
  case ADD: case SUB: return PREC_ADD_SUB;
      
  case LESS: return OP_PRECS;
  case GREATER: return OP_PRECS;
    
  case EQUAL: case SPECIAL_TYPES: case NOT: default: return OP_PRECS;
  }
}

TARE_DEF bool parse_func_sig(Parser *p) {
  Tokenizer *t = p->t;
  if (!expect_fid(t)) return false;
  const Token *name = t->t;
  size_t fid = t->t->fid;
  Func *fn = p->fns->items + fid;
  // Should probably become more involved than this.
  if (!to_token(t, fn->start)) return false;
  p->func = p->funcs->items + fid;
  p->func->name = sv_from_token(name);
  return true;
}

TARE_DEF void patch_tokenizer_builtin_types(Tokenizer *t) {
  for (size_t i = 0; i < TYPES_COUNT; i++) {
    for (size_t j = 0; j < t->count; j++) {
      Token *tok = t->items + j;
      if (tok->t != TOKEN_TYPE_NAME) continue;
      if (!tok_sv_cmp(tok, VariableTypes[i])) continue;
      tok->t = TOKEN_TYPE_TID;
      tok->tid = i;
    }
  }
}

TARE_DEF bool patch_tokenizer_funcs(Tokenizer *t, Funcs *fns) {
  if (t == NULL) return false;
  if (fns == NULL) return false;
  
  size_t point = t->index;
  if (!first_token(t)) return false;
  
  while (true) {
    if (t->t->t != TOKEN_TYPE_KEYWORD) {
      if (!next_token(t)) break;
      continue;
    }
    if (t->t->k != KEY_FUNC) {
      if (!next_token(t)) break;
      continue;
    }

    if (!patch_tokenizer_func(t, fns)) return false;
    if (!next_token(t)) break;
  }

  return to_token(t, point);
}

TARE_DEF bool patch_tokenizer_func(Tokenizer *t, Funcs *fns) {
  Token *first = t->t;
  // Get the function's name.
  if (!expect_name(t)) return false;
  Func fn = {.name = sv_from_token(t->t), .first = first};
  size_t fid = fns->count;
  if (sv_eq(fn.name, fns->items[0].name)) fid = 0;
  assert(fns->count > 0 && "please don't corrupt the memory");
  /* da_append(fns, fn); */
  Func *f = fns->items + fid;
  if (fid != 0) da_append(fns, fn);
  t->t->t = TOKEN_TYPE_FID;
  t->t->fid = fid;
  Token *final_token = t->items + t->count;
  for (Token *tok = t->items; tok < final_token; tok++) {
    if (tok->t != TOKEN_TYPE_NAME) continue;
    if (tok_eq(t->t, tok)) {
      tok->t = TOKEN_TYPE_FID;
      tok->fid = fid;
    }
  }

  size_t point = t->index;
  bool valid_func = false;

  while (next_token(t)) {
    if (t->t->t != TOKEN_TYPE_SPECIAL) continue;
    if (t->t->s != DEF) continue;
    if (!expect_special(t, BLK_BGN)) return false;
    f->start = t->index;
    f->end = t->t->jmp;

    for (size_t i = f->end; i > f->start; i--) {
      Token *tok = t->items + i;
      if (tok->t != TOKEN_TYPE_KEYWORD) continue;
      if (tok->k != KEY_RET) continue;
      valid_func = true;
      break;
    }

    if (!valid_func) break;
    break;
  }

  if (!to_token(t, point)) return false;
  if (!valid_func) {
    print_loc(stderr, t, t->t);
    fprintf(stderr, "error: invalid function signature!\n");
    print_loc(stdout, t, t->t);
    fprintf(stdout, "note: a proper function signature would like one of the following four options: \n");
    fprintf(stdout, "%*.sfunc NAME (ARGS) => (RETS): {BODY}\n", 4, "");
    fprintf(stdout, "%*.sfunc NAME (ARGS): {BODY}\n", 4, "");
    fprintf(stdout, "%*.sfunc NAME => (RETS): {BODY}\n", 4, "");
    fprintf(stdout, "%*.sfunc NAME: {BODY}\n", 4, "");
    print_loc(stdout, t, t->t);
    fprintf(stdout, "note: while the space between `func` and `NAME` is necessary, the other whitespaces shown in this signature are unnecessary and ignored by the lexer and tokenizer.\n");
    print_loc(stdout, t, t->t);
    fprintf(stdout, "note: furthermore, `BODY` must include in it a `%.*s` statement.\n", SV_ARG(Keywords[KEY_RET]));
    return false;
  }
  
  bool patching_args = true;
  /* bool patching_rets = true; */

  // Check for function arguments or for return values.
  if (!expect_special_many(t, PAR_BGN, DEF, EQUAL)) return false;
  if (t->t->s == EQUAL) {
    if (!expect_special(t, GREATER)) return false;
    if (!expect_special_many(t, PAR_BGN, DEF)) return false;
    patching_args = false;
  }
  
  // Exit early if there are no arguments and no return values.
  if (t->t->s == DEF) return true;

  // Gather function arguments, if there are any.
  if (patching_args) {
    if (!patch_tokenizer_args(t, f, true)) return false;
    if (!expect_special_many(t, DEF, EQUAL)) return false;
  }

  if (t->t->s == DEF) return true;
  
  if (patching_args) {
    if (!expect_special(t, GREATER)) return false;
    if (!expect_special(t, PAR_BGN)) return false;
  }

  // Gather return values, if there are any.
  if (!patch_tokenizer_args(t, f, false)) return false;
  if (!expect_special(t, DEF)) return false;

  return true;
}

TARE_DEF bool patch_tokenizer_args(Tokenizer *t, Func *fn, bool args) {
  size_t vid = 0;
  Token *end = t->items + fn->end;

  while (t->t->s != PAR_END) {
    if (!expect_tid_or_const(t)) return false;
    if (t->t->t == TOKEN_TYPE_KEYWORD) {
      if (t->t->k != KEY_CONST) {
        print_loc(stderr, t, t->t);
        fprintf(stderr, "error: tokenizer error\n");
      }
      if (!expect_tid(t)) return false;
    }
    size_t tid = t->t->tid;
    if (!expect_name(t)) return false;
    /* if (args) vid = fn->args.count; */
    /* else vid = fn->rets.count; */
    vid = fn->args.count + fn->rets.count; // TODO: make this actually work
    
    Var arg = {
      .name = sv_from_token(t->t),
      .vid = vid,
      .tid = tid,
    };
      
    for (Token *tok = t->t; tok < end; tok++) {
      if (tok->k == KEY_RET) break;
      if (tok->t != TOKEN_TYPE_NAME) continue;
      if (tok_eq(t->t, tok)) {
        tok->t = TOKEN_TYPE_VID;
        tok->vid = arg.vid;
      }
    }

    if (args) da_append(&fn->args, arg);
    else da_append(&fn->rets, arg);

    if (!expect_special_many(t, SEP, PAR_END)) return false;
    if (t->t->s == SEP) {
      Token next = peek_next_token(t);
      if (next.t != TOKEN_TYPE_SPECIAL) continue;
      if (next.s == PAR_END) if (!next_token(t)) return false;
    }
  }

  return true;
}

TARE_DEF bool patch_tokenizer_rets(Tokenizer *t, Funcs *fns) {
  if (t == NULL || fns == NULL) return false;
  
  for (size_t i = 0; i < fns->count; i++) {
    Func *fn = fns->items + i;
    for (size_t j = fn->start; j < fn->end; j++) {
      Token *tok = t->items + j;
      if (tok->t != TOKEN_TYPE_KEYWORD) continue;
      if (tok->k != KEY_RET) continue;
      tok->fid = i;
    }
  }

  return true;
}

TARE_DEF bool patch_tokenizer_bgn_end(Tokenizer *t) {
  Longs par = {0};
  Longs grp = {0};
  Longs blk = {0};

  for (size_t i = 0; i < t->count; i++) {
    Token *tok = t->items + i;
    if (tok->t != TOKEN_TYPE_SPECIAL) continue;
    if (tok->s == PAR_BGN) da_append(&par, i);
    else if (tok->s == GRP_BGN) da_append(&grp, i);
    else if (tok->s == BLK_BGN) da_append(&blk, i);
    else if (tok->s == PAR_END) {
      if (par.count == 0) {
        fprintf(stderr, "%s:%zu:%zu: error: "
                "can't close parentheses that haven't been opened.\n",
                t->path, tok->row, tok->col);
      }
      size_t jmp = par.items[--par.count];
      tok->jmp = jmp;
      tok = t->items + jmp;
      tok->jmp = i;
    } else if (tok->s == GRP_END) {
      if (grp.count == 0) {
        fprintf(stderr, "%s:%zu:%zu: error: "
                "can't end a group that hasn't been started.\n",
                t->path, tok->row, tok->col);
      }
      size_t jmp = grp.items[--grp.count];
      tok->jmp = jmp;
      tok = t->items + jmp;
      tok->jmp = i;
    } else if (tok->s == BLK_END) {
      if (blk.count == 0) {
        fprintf(stderr, "%s:%zu:%zu: error: "
                "can't close a block that hasn't been started.\n",
                t->path, tok->row, tok->col);
      }
      size_t jmp = blk.items[--blk.count];
      tok->jmp = jmp;
      tok = t->items + jmp;
      tok->jmp = i;
    }
  }

  if (par.count > 0) {
    for (size_t i = 0; i < par.count; i++) {
      size_t jmp = par.items[i];
      Token *tok = t->items + jmp;
      fprintf(stderr, "%s:%zu:%zu: error: "
              "parentheses opened but didn't close.\n",
              t->path, tok->row, tok->col);
    }
  }

  if (grp.count > 0) {
    for (size_t i = 0; i < grp.count; i++) {
      size_t jmp = grp.items[i];
      Token *tok = t->items + jmp;
      fprintf(stderr, "%s:%zu:%zu: error: "
              "group started but didn't end.\n",
              t->path, tok->row, tok->col);
    }
  }

  if (blk.count > 0) {
    for (size_t i = 0; i < blk.count; i++) {
      size_t jmp = blk.items[i];
      Token *tok = t->items + jmp;
      fprintf(stderr, "%s:%zu:%zu: error: "
              "block started but didn't close.\n",
              t->path, tok->row, tok->col);
    }
  }

  if (par.items) free(par.items);
  if (grp.items) free(grp.items);
  if (blk.items) free(blk.items);
  
  return (par.count == 0) && (grp.count == 0) && (blk.count == 0);
}

TARE_DEF const char *op_type_as_string(OpType type) {
  switch (type) {
  case OP_PTR_ADD: return "OP_PTR_ADD";
  case OP_PTR_SUB: return "OP_PTR_SUB";
  case OP_ELEM_ADD: return "OP_ELEM_ADD";
  case OP_ELEM_SUB: return "OP_ELEM_SUB";
  case OP_READ_SIZE: return "OP_READ_SIZE";
  case OP_CONDITIONAL: return "OP_CONDITIONAL";
  case OP_GOTO: return "OP_GOTO";
  case OP_ADDRESS: return "OP_ADDRESS";
  case OP_FUNCALL: return "OP_FUNCALL";
  case OP_RET: return "OP_RET";
  case OP_WRITE: return "OP_WRITE";
  case OP_READ: return "OP_READ";
  case OP_SYSCALL: return "OP_SYSCALL";
  
  case OP_TAPE: return "OP_TAPE";
  case OP_HEAD: return "OP_HEAD";
  case OP_BASE: return "OP_BASE";
  case OP_INDEX: return "OP_INDEX";
  case OP_CONST: return "OP_CONST";

  case OP_PUSH: return "OP_PUSH";
  case OP_POP: return "OP_POP";

  case OP_ADD: return "OP_ADD";
  case OP_SUB: return "OP_SUB";
  case OP_MUL: return "OP_MUL";
  case OP_DIV: return "OP_DIV";
  case OP_SHL: return "OP_SHL";
  case OP_SHR: return "OP_SHR";
  case OP_NOT: return "OP_NOT";
  case OP_DEREF: return "OP_DEREF";

  case OP_ARG: return "OP_ARG";
  case OP_NUM: return "OP_NUM";
  
  case OP_TYPES: return "OP_TYPES";
  default: return "";
  }
}

TARE_DEF void print_function(const Tokenizer *t, const Function *func) {
  for (size_t i = 0; i < func->count; i++) {
    Operation op = func->items[i];
    printf("--------------------------------------------------\n");
    debug_print_token(t, op.start);
    printf("op (%zu) = {\n", i);
    printf("  .start = ");
    debug_print_token(t, op.start);
    printf("  .type = %s\n", op_type_as_string(op.type));
    printf("  .args = %p\n", (void*)op.args);
    printf("  .args_count = %zu\n", op.args_count);
    printf("  .op = %zu\n", op.op);
    if (op.name.s != NULL) printf("  .name = %.*s\n", SV_ARG(op.name));
    else printf("  .name = {.s = %p, .l = %zu}\n", op.name.s, op.name.l);
    printf("}\n");
    printf("--------------------------------------------------\n\n");
  }
}

TARE_DEF void print_functions(const Parser *p) {
  for (size_t i = 0; i < p->funcs->count; i++) {
    Function *func = p->funcs->items + i;
    printf("\n--------------------------------------------------\n");
    printf("Function `%.*s`\n", SV_ARG(func->name));
    print_function(p->t, func);
    printf("\n--------------------------------------------------\n");
  }
}

#endif // PARSER_IMPLEMENTATION

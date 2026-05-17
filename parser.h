#ifndef PARSER_H_
#define PARSER_H_

typedef struct Operation Operation;

typedef struct {
  Operation *items;
  size_t count;
  size_t capacity;
} Operations;

typedef enum {
  SCOPE_GLOBAL,
  SCOPE_LOCAL,
  SCOPE_ARGUMENT,
  SCOPE_RETURN,
  SCOPE_TYPES,
} ScopeType;

typedef struct {
  ScopeType type;
  size_t vid;
  size_t tid;
  StringView name;
  size_t initial; // initial value
} Var;

typedef struct {
  Var *items;
  size_t count;
  size_t capacity;
} Vars;

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
  OP_LENGTH,
  
  OP_CONST,

  OP_PUSH,
  OP_POP,

  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_MOD,
  OP_SHL,
  OP_SHR,
  OP_NOT,
  OP_BITWISE_AND,
  OP_BITWISE_OR,
  OP_LOGICAL_AND,
  OP_LOGICAL_OR,
  
  OP_LESS,
  OP_LESS_EQUAL,
  OP_GREATER,
  OP_GREATER_EQUAL,
  OP_EQUAL,
  OP_NOT_EQUAL,
  
  OP_DEREF,

  OP_NUM,

  OP_POP_FROM_OPS,

  OP_GVID,
  OP_LVID,
  OP_RVID,
  OP_AVID,
  
  OP_ASSIGN,

  OP_TYPES,
} OpType;

// TODO: fill this properly
typedef enum {
  PREC_LOGICAL_OR = 0,
  PREC_LOGICAL_AND,
  PREC_BITWISE_OR,
  /* PREC_BITWISE_XOR, */
  PREC_BITWISE_AND,
  PREC_EQUAL_NEQUAL,
  PREC_LESS_GREATER_LEQUAL_GEQUAL,
  PREC_SHL_SHR,
  PREC_ADD_SUB,
  PREC_MUL_DIV_REM,
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
  StringView name;

  Vars args;
  Vars rets;

  Vars lvars;

  Token *first;
  size_t start;
  size_t end;
  
  Operation *items;
  size_t count;
  size_t capacity;
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
  Longs *gotos;
  Vars *globals;
  Operations stack;
} Parser;

#define append_op(parser, operation)            \
  do {                                          \
    da_append((parser)->func, (operation));     \
    da_append(&(parser)->stack, (operation));   \
  } while (0)

TARE_DEF bool parse_file(Parser *p);
TARE_DEF StringView sv_from_token(const Token *t);

TARE_DEF bool parse_statement(Parser *p);
TARE_DEF bool parse_expression(Parser *p);

TARE_DEF bool parse_expression_arithmetics(Parser *p);

TARE_DEF bool optimize_expression(Parser *p, Operation *op);

TARE_DEF bool check_for_continued_expression(Parser *p);
TARE_DEF OpPrec get_prec_by_op_type(OpType type);
TARE_DEF OpPrec get_prec_by_special_type(SpecialType s);
TARE_DEF OpPrec parse_prec_for_next_token(Parser *p);
TARE_DEF bool is_token_expression(const Token *t);

TARE_DEF bool parse_func_sig(Parser *p);

TARE_DEF bool validate_function_signature(Parser *p, Operation *op, size_t args);

TARE_DEF bool pop_stack(Parser *p);
TARE_DEF bool op_has_side_effect(Operation *op);

TARE_DEF void patch_tokenizer_builtin_types(Tokenizer *t);
TARE_DEF bool patch_tokenizer_funcs(Tokenizer *t, Functions *fns);
TARE_DEF bool patch_tokenizer_func(Tokenizer *t, Functions *fns);
TARE_DEF bool patch_tokenizer_args(Tokenizer *t, Function *fn, bool args);
TARE_DEF bool patch_tokenizer_bgn_end(Tokenizer *t);

TARE_DEF bool patch_tokenizer_gvids(Parser *p);
TARE_DEF bool patch_tokenizer_lvids(Parser *p);

TARE_DEF size_t get_vars_size(Vars *vars);
TARE_DEF size_t get_vars_size_with_padding(Vars *vars);
TARE_DEF size_t get_args_size(Function *fn);
TARE_DEF size_t get_rets_size(Function *fn);
TARE_DEF size_t get_lvars_size(Function *fn);

TARE_DEF const char *op_type_as_string(OpType type); // For debugging.
TARE_DEF void print_var(const Var *var, size_t i);
TARE_DEF void print_vars(const Vars *vars);
TARE_DEF void print_op(const Tokenizer *t, const Operation *op, size_t i);
TARE_DEF void print_function(const Tokenizer *t, const Function *func);
TARE_DEF void print_functions(const Parser *p);

#endif // PARSER_H_

#ifdef PARSER_IMPLEMENTATION

#define TEST_EXPR(expr) printf(#expr " = %zu\n", (size_t) (expr));

TARE_DEF bool parse_file(Parser *p) {
  {
    Tokenizer *t = p->t;
    bool debug = false;
    for (Token *tok = t->items; (tok < t->items + t->count) && debug; tok++)
      debug_print_token(t, tok);
    if (debug) return true;
  }
  
  if (p == NULL) return false;
  
  Tokenizer *t = p->t;
  if (!first_token(t)) return false;

  patch_tokenizer_builtin_types(t);
  if (!patch_tokenizer_bgn_end(t)) return false;
  
  Function main = {.name = SV_MAKE(main)};
  da_append(p->funcs, main);
  if (!patch_tokenizer_funcs(t, p->funcs)) return false;

  if (!patch_tokenizer_gvids(p)) return false;
  if (!patch_tokenizer_lvids(p)) return false;

  p->func = p->funcs->items;

  while (true) {
    if (!parse_statement(p)) return false;
    if (!next_token(t)) break;
  }

  bool debug = false;
  if (debug) print_functions(p);

  /* p->stack.count = 0; */
  /* if (p->stack.items) free(p->stack.items); */
  
  return true;
}

TARE_DEF StringView sv_from_token(const Token *t) {
  return (StringView) {.s = t->f, .l = t->l};
}

static_assert(OP_TYPES == 47, "update parse_statement");
TARE_DEF bool parse_statement(Parser *p) {
  if (p == NULL) return false;
  
  Tokenizer *t = p->t;

  if (is_token_expression(t->t)) {
    size_t save = p->func->count;
    if (!parse_expression(p)) return false;
    if (!expect_special(t, END)) return false;
    if (!pop_stack(p)) return false;
    Operation *operation = p->func->items + p->func->count - 1;
    if (op_has_side_effect(operation)) {
      if (operation->type == OP_FUNCALL) {
        Function *fn = p->funcs->items + operation->op;
        if (fn->rets.count > 0) {
          Operation pop = {.start = t->t, .type = OP_POP_FROM_OPS,};
          da_append(p->func, pop);
        }
      }
    } else p->func->count = save;
    return true;
  }
  
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
      // TODO: decide if these should be expressions or not
      /* if (!parse_expression(p)) return false; */
      /* if (!expect_special(t, END)) return false; */
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
        size_t save = p->func->count;
        Operation address = {.start = op.start, .type = OP_ADDRESS,
                             /* .op = t->index}; */
                             .op = p->func->count};
        op.type = OP_CONDITIONAL;
        da_append(p->func, address);
        if (!next_token(t)) return false;
        if (!parse_expression(p)) return false;
        if (!pop_stack(p)) return false; // TODO: handle stack
        bool condition = true;
        {
          Operation *cond = p->func->items + p->func->count - 1;
          if (cond->type == OP_NUM) condition = (bool) (cond->op);
        }
        if (!expect_special(t, DEF)) return false;
        if (!next_token(t)) return false;
        
        /* if (t->t->t == TOKEN_TYPE_SPECIAL) { */
        /*   if (t->t->s == BLK_BGN) { */
        /*     op.op = t->t->jmp; */
        /*     if (!next_token(t)) return false; */
        /*   } */
        /* } else { */
        /*   bool found = false; */
        /*   for (Token *tok = t->t; tok < t->items + t->count; tok++) { */
        /*     if (tok->t != TOKEN_TYPE_SPECIAL) continue; */
        /*     if (tok->s == END) { */
        /*       op.op = (size_t) (tok - t->items); */
        /*       found = true; */
        /*     } else if (tok->s == BLK_BGN) { */
        /*       op.op = t->t->jmp; */
        /*       found = true; */
        /*     } */
        /*     if (found) break; */
        /*   } */
        /*   if (!found) return false; */
        /* } */
        
        size_t tokenizer_save = t->index;
        size_t parser_save = p->func->count;
        if (op.start->k == KEY_WHILE) {
          da_append(p->gotos, 0);
          da_append(p->gotos, 0);
        }
        if (!parse_statement(p)) return false;
        if (!to_token(t, tokenizer_save)) return false;
        op.op = p->func->count + 1;
        if (address.start->k == KEY_WHILE) op.op++;
        p->func->count = parser_save;

        if (op.start->k == KEY_WHILE) {
          if (p->gotos->count < 2) return false;
          p->gotos->count -= 2;
        }
        
        da_append(p->func, op);
        
        if (op.start->k == KEY_WHILE) {
          da_append(p->gotos, address.op);
          da_append(p->gotos, op.op);
        }
        
        /* while (t->index != op.op) { */
        /*   if (!parse_statement(p)) return false; */
        /*   if (t->index == op.op) break; */
        /*   if (!next_token(t)) return false; */
        /* } */

        if (!parse_statement(p)) return false;
        
        if (op.start->k == KEY_WHILE) {
          if (p->gotos->count < 2) return false;
          p->gotos->count -= 2;
        }
        
        op.start = t->t;
        op.type = OP_GOTO;
        op.op = address.op;
        if (address.start->k == KEY_WHILE) da_append(p->func, op);

        address.start = t->t;
        /* address.op = t->index; */
        address.op = p->func->count;
        da_append(p->func, address);
        if (!condition) p->func->count = save;
      }
      break;
    case KEY_BREAK: case KEY_CONT:
      op.type = OP_GOTO;
      if (p->gotos->count < 2) return false;
      /* assert(p->gotos->count == 2 && "what?"); */
      {
        size_t count = p->gotos->count;
        size_t *gotos = p->gotos->items;
        
        if (t->t->k == KEY_BREAK) op.op = gotos[count - 1];
        else if (t->t->k == KEY_CONT) op.op = gotos[count - 2];
        else return false;
      }
      if (!expect_special(t, END)) return false;
      da_append(p->func, op);
      break;
    case KEY_FUNC:
      /* { */
      /*   if (!expect_fid(t)) return false; */
      /*   const Token *name = t->t; */
      /*   size_t fid = t->t->fid; */
      /*   Func *fn = p->fns->items + fid; */
      /*   // Should probably become more involved than this. */
      /*   if (!to_token(t, fn->start)) return false; */
      /*   p->func = p->funcs->items + fid; */
      /*   p->func->name = sv_from_token(name); */
      /* } */
      /* return true; */

      if (!parse_func_sig(p)) return false;
      {
        size_t end = t->t->jmp;
        while (true) {
          if (!next_token(t)) return false;
          if (t->index == end) break;
          if (!parse_statement(p)) return false;
        }
      }
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
    case KEY_LENGTH:
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
    case KEY_MUL: case KEY_DIV: case KEY_MOD:
    case KEY_SHL: case KEY_SHR:
    case KEY_NOT:
    case KEY_BITWISE_AND: case KEY_BITWISE_OR:
    case KEY_LOGICAL_AND: case KEY_LOGICAL_OR:
    case KEY_LESS: case KEY_LESS_EQUAL:
    case KEY_GREATER: case KEY_GREATER_EQUAL:
    case KEY_EQUAL: case KEY_NOT_EQUAL:
      if (!parse_expression(p)) return false;
      if (!expect_special(t, END)) return false;
      break;
    case KEY_DEREF: unimpl("KEY_DEREF"); break;
    
    case KEYWORD_TYPES: unimpl("KEYWORD_TYPES"); break;

    default: unimpl("default case in parse_keyword_as_op"); break;
    }
    break;
  case TOKEN_TYPE_SPECIAL:
    switch (t->t->s) {
    case PAR_BGN: case PAR_END: 
    case GRP_BGN: case GRP_END:
      diag_errf(t, t->t,
                "tare statements do not begin with special token `%c`.\n",
                Specials[t->t->s]);
      return false;
      
    case BLK_BGN:
      {
        size_t end = t->t->jmp;
        while (true) {
          if (!next_token(t)) return false;
          if (t->index == end) break;
          if (!parse_statement(p)) return false;
        }
      }
      break;
    case BLK_END:
    case DQUOTE: case SQUOTE: 
    case ESC: case SEP: 
      diag_errf(t, t->t,
                "tare statements do not begin with special token `%c`.\n",
                Specials[t->t->s]);
      return false;
    case DOT: case DEF: case DIV: case MOD: case MULT:
    case ADD: case SUB: case LESS: case GREATER:
      diag_errf(t, t->t,
                "tare statements do not begin with special token `%c`.\n",
                Specials[t->t->s]);
      return false;
    case EQUAL: unimpl("EQUAL"); break;
    case NOT: case AND: case OR:
      diag_errf(t, t->t,
                "tare statements do not begin with special token `%c`.\n",
                Specials[t->t->s]);
      return false;
    case END:
      break;
    case SPECIAL_TYPES: unimpl("SPECIAL_TYPES"); break;
    }
    break;
  case TOKEN_TYPE_STRING: unimpl("TOKEN_TYPE_STRING"); break;
  case TOKEN_TYPE_CHAR: unimpl("TOKEN_TYPE_CHAR"); break;
    
  case TOKEN_TYPE_GVID:
  case TOKEN_TYPE_LVID:
  case TOKEN_TYPE_RVID:
  case TOKEN_TYPE_AVID:
    {
      if (peek_next_token(t).t == TOKEN_TYPE_SPECIAL) {
        if (peek_next_token(t).s == END) break;
      }
    
      if (t->t->t == TOKEN_TYPE_GVID) {
        op.type = OP_GVID;
        op.op = t->t->gvid;
      }
      else if (t->t->t == TOKEN_TYPE_LVID) {
        op.type = OP_LVID;
        op.op = t->t->lvid;
      }
      else if (t->t->t == TOKEN_TYPE_RVID) {
        op.type = OP_RVID;
        op.op = t->t->rvid;
      }
      else if (t->t->t == TOKEN_TYPE_AVID) {
        op.type = OP_AVID;
        op.op = t->t->avid;
      }
      else {
        unimpl("in parse statement");
        return false;
      }
      
      size_t lhs_index = p->func->count;
      da_append(p->func, op);
    
      if (!expect_special_many(t, SEP, EQUAL,
                               DIV, MOD, MULT, ADD, SUB, AND, OR))
        return false;

      Operation potential_arithmetics = {.start = t->t};
      if (t->t->s == DIV) {
        potential_arithmetics.type = OP_DIV;
      } else if (t->t->s == MOD) {
        potential_arithmetics.type = OP_MOD;
      } else if (t->t->s == MULT) {
        potential_arithmetics.type = OP_MUL;
      } else if (t->t->s == ADD) {
        potential_arithmetics.type = OP_ADD;
        if (peek_next_token(t).t == TOKEN_TYPE_SPECIAL) {
          if (peek_next_token(t).s != EQUAL) {
            if (!expect_special(t, ADD)) return false;
            if (!expect_special_many(t, END, SEP)) return false;
            da_append(p->func, op);
            Operation deref = {.start = op.start, .type = OP_DEREF};
            da_append(p->func, deref);
            Operation one = {.start = op.start, .type = OP_NUM, .op = 1};
            da_append(p->func, one);
            da_append(p->func, potential_arithmetics);
            op.type = OP_ASSIGN;
            da_append(p->func, op);
            if (t->t->s == SEP) {
              if (!next_token(t)) return false;
              if (!parse_statement(p)) return false;
            }
            break;
          }
        }
      } else if (t->t->s == SUB) {
        potential_arithmetics.type = OP_SUB;
        if (peek_next_token(t).t == TOKEN_TYPE_SPECIAL) {
          if (peek_next_token(t).s != EQUAL) {
            if (!expect_special(t, SUB)) return false;
            if (!expect_special_many(t, END, SEP)) return false;
            da_append(p->func, op);
            Operation deref = {.start = op.start, .type = OP_DEREF};
            da_append(p->func, deref);
            Operation one = {.start = op.start, .type = OP_NUM, .op = 1};
            da_append(p->func, one);
            da_append(p->func, potential_arithmetics);
            op.type = OP_ASSIGN;
            da_append(p->func, op);
            if (t->t->s == SEP) {
              if (!next_token(t)) return false;
              if (!parse_statement(p)) return false;
            }
            break;
          }
        }
      } else if (t->t->s == AND) {
        if (peek_next_token(t).t == TOKEN_TYPE_SPECIAL) {
          if (peek_next_token(t).s != EQUAL) {
            if (!expect_special(t, AND)) return false;
            potential_arithmetics.type = OP_LOGICAL_AND;
          }
        } else potential_arithmetics.type = OP_BITWISE_AND;
      } else if (t->t->s == OR) {
        if (peek_next_token(t).t == TOKEN_TYPE_SPECIAL) {
          if (peek_next_token(t).s != EQUAL) {
            if (!expect_special(t, OR)) return false;
            potential_arithmetics.type = OP_LOGICAL_OR;
          }
        } else potential_arithmetics.type = OP_BITWISE_OR;
      } else {
        if (t->t->s != SEP && t->t->s != EQUAL) {
          unimpl("in parse statement");
          return false;
        }
      }
      
      bool assign_has_op = t->t->s != SEP && t->t->s != EQUAL;
      if (assign_has_op) {
        if (!expect_special_many(t, SEP, EQUAL)) return false;
      } else op.type = OP_ASSIGN;
      
      if (t->t->s == SEP) {
        unimpl("SEP in parse statement");
        return false;
      }
      
      if (t->t->s == EQUAL) {
        if (!next_token(t)) return false;
        if (!parse_expression(p)) return false;
        if (assign_has_op) {
          da_append(p->func, op);
          Operation deref = {.start = op.start, .type = OP_DEREF};
          da_append(p->func, deref);
          da_append(p->func, potential_arithmetics);
          op.type = OP_ASSIGN;
        }
        da_append(p->func, op);
      }

      Operation *lhs = p->func->items + lhs_index;
      size_t potential_tid_index = (size_t) (lhs->start - t->items - 1);
      if (!check_bounds(potential_tid_index, t->count)) break;
      Token *potential_tid = t->items + potential_tid_index;
      if (potential_tid->t != TOKEN_TYPE_TID) break;
      size_t error_index = 0;
      bool error = false;
      for (size_t i = lhs_index + 1; i < p->func->count; i++) {
        Operation *rhs = p->func->items + i;
        if (lhs->type != rhs->type) continue;
        if (lhs->op != rhs->op) continue;
        error = true;
        error_index = i;
        break;
      }
      if (!error) break;
      Operation *error_op = p->func->items + error_index;
      diag_errf(t, lhs->start, "can't use variable `%.*s` in the expression assigned to itself at decleration!\n", TOK_ARG(lhs->start));
      diag_notef(t, error_op->start, "attempted use of `%.*s` in assignment while declaring it here.\n", TOK_ARG(error_op->start));
    }
    break;
  case TOKEN_TYPE_TID:
    if (!next_token(t)) return false;
    if (t->t->t != TOKEN_TYPE_GVID &&
        t->t->t != TOKEN_TYPE_LVID &&
        t->t->t != TOKEN_TYPE_RVID &&
        t->t->t != TOKEN_TYPE_AVID) return false;
    if (!parse_statement(p)) return false;
    break;
  case TOKEN_TYPE_FID:
    if (!parse_expression(p)) return false;
    if (!expect_special(t, END)) return false;
    break;
  case TOKEN_TYPES: unimpl("TOKEN_TYPES"); break;
  default: unimpl("default case in parse_token_as_op"); break;
  }

  return true;
}

static_assert(OP_TYPES == 47, "update parse_expression");
TARE_DEF bool parse_expression(Parser *p) {
  if (p == NULL) return false;
  
  Tokenizer *t = p->t;

  Operation op = {.start = t->t};

  Token prev = peek_prev_token(t);

  switch (t->t->t) {
  case TOKEN_TYPE_NAME: unimpl("TOKEN_TYPE_NAME"); break;
  case TOKEN_TYPE_WHOLE_NUM:
    op.type = OP_NUM;
    op.op = t->t->u64;
    append_op(p, op);
    if (!check_for_continued_expression(p)) return true;
    if (peek_prev_token(t).t == TOKEN_TYPE_SPECIAL) {
      SpecialType s = peek_prev_token(t).s;
      if (s == DIV || s == MULT || s == ADD || s == SUB
          || s == NOT) return true;
    }
    break;
  case TOKEN_TYPE_FRAC_NUM: unimpl("TOKEN_TYPE_FRAC_NUM"); break;
  case TOKEN_TYPE_KEYWORD:
    switch (t->t->k) {
    case KEY_F: case KEY_B:
    case KEY_A: case KEY_S:
      // TODO: decide if these should be expressions or not
      if (t->t->k == KEY_F) op.type = OP_PTR_ADD;
      else if (t->t->k == KEY_B) op.type = OP_PTR_SUB;
      else if (t->t->k == KEY_A) op.type = OP_ELEM_ADD;
      else if (t->t->k == KEY_S) op.type = OP_ELEM_SUB;
      else {
        unimpl("in parse expression");
        return false;
      }
      if (!expect_special(t, PAR_BGN)) return false;
      if (!next_token(t)) return false;
      if (!parse_expression(p)) return false;
      if (!expect_special(t, PAR_END)) return false;
      append_op(p, op);
      return true;
      break;
    case KEY_N: case KEY_P:
    case KEY_I: case KEY_D:
      // TODO: decide if these should be expressions or not
      if (t->t->k == KEY_N) op.type = OP_PTR_ADD;
      else if (t->t->k == KEY_P) op.type = OP_PTR_SUB;
      else if (t->t->k == KEY_I) op.type = OP_ELEM_ADD;
      else if (t->t->k == KEY_D) op.type = OP_ELEM_SUB;
      else {
        unimpl("in parse expression");
        return false;
      }
      {
        Operation operation = {.start = op.start, .type = OP_NUM, .op = 1};
        append_op(p, operation);
      }
      if (!expect_special(t, PAR_BGN)) return false;
      if (!expect_special(t, PAR_END)) return false;
      append_op(p, op);
      return true;
      break;
    case KEY_R8: case KEY_R16: case KEY_R32: case KEY_R64:
    case KEY_IF: case KEY_WHILE: case KEY_BREAK: case KEY_CONT:
    case KEY_FUNC:
    case KEY_RET:
      diag_errf(t, t->t,
                "tare expressions do not begin with keyword `%.*s`.\n",
                SV_ARG(Keywords[t->t->k]));
      return false;
    case KEY_WRITE: case KEY_READ:
      if (t->t->k == KEY_WRITE) op.type = OP_WRITE;
      else if (t->t->k == KEY_READ) op.type = OP_READ;
      else {
        unimpl("in parse expression");
        return false;
      }
      if (!expect_special(t, PAR_BGN)) return false;
      if (!next_token(t)) return false;
      if (!parse_expression(p)) return false;
      if (!expect_special(t, SEP)) return false;
      if (!next_token(t)) return false;
      if (!parse_expression(p)) return false;
      if (!expect_special(t, PAR_END)) return false;
      append_op(p, op);
      break;
    case KEY_SYSCALL:
      op.type = OP_SYSCALL;
      if (!expect_special(t, PAR_BGN)) return false;
      while (true) {
        if (!next_token(t)) return false;
        if (!parse_expression(p)) return false;
        if (op.op > 6) {
          diag_err(t, t->t, "tare currently doesn't support syscalls with more than 6 arguments.\n");
          diag_notef(t, t->t, "the first argument for keyword `%.*s` is the syscall index, and the following arguments are the actual arguemtns to the syscall. So the keyword takes 7 arguemnts, but the syscall only takes 6 arguments.\n", SV_ARG(Keywords[KEY_SYSCALL]));
          return false;
        }
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
      append_op(p, op);
      break;
    case KEY_TAPE: case KEY_HEAD: case KEY_BASE: case KEY_INDEX:
    case KEY_LENGTH:
      if (t->t->k == KEY_TAPE) op.type = OP_TAPE;
      else if (t->t->k == KEY_HEAD) op.type = OP_HEAD;
      else if (t->t->k == KEY_BASE) op.type = OP_BASE;
      else if (t->t->k == KEY_INDEX) op.type = OP_INDEX;
      else if (t->t->k == KEY_LENGTH) op.type = OP_LENGTH;
      else {
        unimpl("in parse expression");
        return false;
      }
      append_op(p, op);
      if (peek_prev_token(t).t == TOKEN_TYPE_SPECIAL) {
        SpecialType s = peek_prev_token(t).s;
        if (s == DIV || s == MULT || s == ADD || s == SUB
            || s == NOT) return true;
      }
      break;
    case KEY_CONST: unimpl("KEY_CONST"); break;

      // TODO: FIX `push` AND `pop`
    case KEY_PUSH:
      op.type = OP_PUSH;
      if (!expect_special(t, PAR_BGN)) return false;
      if (!next_token(t)) return false;
      if (!parse_expression(p)) return false;
      if (!expect_special(t, PAR_END)) return false;
      append_op(p, op);
      return true;
      break;
    case KEY_POP:
      op.type = OP_POP;
      if (!expect_special(t, PAR_BGN)) return false;
      if (!next_token(t)) return false;
      if (!parse_expression(p)) return false;
      if (!expect_special(t, PAR_END)) return false;
      append_op(p, op);
      break;

    case KEY_ADD: case KEY_SUB:
    case KEY_MUL: case KEY_DIV: case KEY_MOD:
    case KEY_SHL: case KEY_SHR:
    case KEY_BITWISE_AND: case KEY_BITWISE_OR:
    case KEY_LOGICAL_AND: case KEY_LOGICAL_OR:
    case KEY_LESS: case KEY_LESS_EQUAL:
    case KEY_GREATER: case KEY_GREATER_EQUAL:
    case KEY_EQUAL: case KEY_NOT_EQUAL:
      if (t->t->k == KEY_ADD) op.type = OP_ADD;
      else if (t->t->k == KEY_SUB) op.type = OP_SUB;
      else if (t->t->k == KEY_MUL) op.type = OP_MUL;
      else if (t->t->k == KEY_DIV) op.type = OP_DIV;
      else if (t->t->k == KEY_MOD) op.type = OP_MOD;
      else if (t->t->k == KEY_SHL) op.type = OP_SHL;
      else if (t->t->k == KEY_SHR) op.type = OP_SHR;
      else if (t->t->k == KEY_BITWISE_AND) op.type = OP_BITWISE_AND;
      else if (t->t->k == KEY_BITWISE_OR) op.type = OP_BITWISE_OR;
      else if (t->t->k == KEY_LOGICAL_AND) op.type = OP_LOGICAL_AND;
      else if (t->t->k == KEY_LOGICAL_OR) op.type = OP_LOGICAL_OR;
      else if (t->t->k == KEY_LESS) op.type = OP_LESS;
      else if (t->t->k == KEY_LESS_EQUAL) op.type = OP_LESS_EQUAL;
      else if (t->t->k == KEY_GREATER) op.type = OP_GREATER;
      else if (t->t->k == KEY_GREATER_EQUAL) op.type = OP_GREATER_EQUAL;
      else if (t->t->k == KEY_EQUAL) op.type = OP_EQUAL;
      else if (t->t->k == KEY_NOT_EQUAL) op.type = OP_NOT_EQUAL;
      else {
        unimpl("in parse expression");
        return false;
      }
      if (!expect_special(t, PAR_BGN)) return false;
      if (!next_token(t)) return false;
      if (!parse_expression(p)) return false;
      if (!expect_special(t, SEP)) return false;
      if (!next_token(t)) return false;
      if (!parse_expression(p)) return false;
      if (!expect_special(t, PAR_END)) return false;
      if (!optimize_expression(p, &op)) return false;
      append_op(p, op);
      break;
    case KEY_NOT:
      op.type = OP_NOT;
      if (!next_token(t)) return false;
      if (!parse_expression(p)) return false;
      if (!optimize_expression(p, &op)) return false;
      append_op(p, op);
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
                "special token `%c` at beginning of tare expression doesn't make sense. This is probably a tokenizer error!\n", Specials[t->t->s]);
      return false;
    case SEP: unimpl("SEP"); break;
      
    case DIV: case MOD: case MULT: case ADD: case SUB: case AND: case OR:
      diag_errf(t, t->t,
                "tare expressions do not begin with special token `%c`.\n",
                Specials[t->t->s]);
      return false;
      
    case LESS: unimpl("LESS"); break;
    case GREATER: unimpl("GREATER"); break;
    case EQUAL: unimpl("EQUAL"); break;
    case NOT:
      op.type = OP_NOT;
      if (!next_token(t)) return false;
      if (t->t->t == TOKEN_TYPE_SPECIAL) {
        if (t->t->s == EQUAL) {
          if (!prev_token(t)) return false;
          if (!parse_expression_arithmetics(p)) return false;
          return true;
        }
      }
      if (!parse_expression(p)) return false;
      if (!optimize_expression(p, &op)) return false;
      append_op(p, op);
      break;
    case SPECIAL_TYPES: unimpl("SPECIAL_TYPES"); break;
    }
    
    break;
  case TOKEN_TYPE_STRING: unimpl("TOKEN_TYPE_STRING"); break;
  case TOKEN_TYPE_CHAR: unimpl("TOKEN_TYPE_CHAR"); break;
    
  case TOKEN_TYPE_GVID:
  case TOKEN_TYPE_LVID:
  case TOKEN_TYPE_RVID:
  case TOKEN_TYPE_AVID:
    if (t->t->t == TOKEN_TYPE_GVID) {
      op.type = OP_GVID;
      op.op = t->t->gvid;
    }
    else if (t->t->t == TOKEN_TYPE_LVID) {
      op.type = OP_LVID;
      op.op = t->t->lvid;
    }
    else if (t->t->t == TOKEN_TYPE_RVID) {
      op.type = OP_RVID;
      op.op = t->t->rvid;
    }
    else if (t->t->t == TOKEN_TYPE_AVID) {
      op.type = OP_AVID;
      op.op = t->t->avid;
    }
    else {
      unimpl("in parse expression");
      return false;
    }
    
    append_op(p, op);
    op.type = OP_DEREF;
    append_op(p, op);
    break;
    
  case TOKEN_TYPE_TID: unimpl("TOKEN_TYPE_TID"); break;
  case TOKEN_TYPE_FID:
    op.type = OP_FUNCALL;
    op.op = t->t->fid;
    if (!expect_special(t, PAR_BGN)) return false;
    {
      /* Function *fn = p->funcs->items + op.op; */
      size_t args = 0;
      // TODO: type-checking
      size_t end = t->t->jmp;
      if (!next_token(t)) return false;
      while (t->index != end) {
        if (!parse_expression(p)) return false;
        args++;
        if (!expect_special_many(t, SEP, PAR_END)) return false;
        if (t->index == end) break;
        if (!next_token(t)) return false;
      }
      if (!validate_function_signature(p, &op, args)) return false;
    }
    append_op(p, op);
    break;
  case TOKEN_TYPES: unimpl("TOKEN_TYPES"); break;
  default: unimpl("default case in parse_expression token type switch"); break;
  }

  if (prev.t == TOKEN_TYPE_SPECIAL) {
    if (prev.s == NOT) return true;
  }
  
  while (check_for_continued_expression(p)) {
    if (!next_token(t)) return false;
    if (!parse_expression_arithmetics(p)) return false;
  }

  return true;
}

static_assert(OP_TYPES == 47, "update parse_expression_arithmetics");
TARE_DEF bool parse_expression_arithmetics(Parser *p) {
  if (p == NULL) return false;
  
  Tokenizer *t = p->t;

  if (t->t->t != TOKEN_TYPE_SPECIAL) return false;

  Operation op = {.start = t->t};

  switch (t->t->s) {
  case PAR_BGN:
    assert(false && "this really shouldn't happen");
    break;
  case PAR_END:
  case GRP_BGN: case GRP_END:
  case BLK_BGN: case BLK_END:
  case END:
  case DOT:
  case DEF:
    diag_errf(t, t->t, "tare expressions arithmetics do not begin with special token `%c`.\n", Specials[t->t->s]);
    return false;
  case DQUOTE: case SQUOTE: case ESC:
    diag_errf(t, t->t,
              "special token `%c` at beginning of tare expression arithmetifcs doesn't make sense. This is probably a tokenizer error!\n", Specials[t->t->s]);
    return false;
  case SEP: unimpl("SEP"); break;
      
  case DIV: case MOD: case MULT: case ADD: case SUB:
    if (t->t->s == ADD) op.type = OP_ADD;
    else if (t->t->s == SUB) op.type = OP_SUB;
    else if (t->t->s == MULT) op.type = OP_MUL;
    else if (t->t->s == DIV) op.type = OP_DIV;
    else if (t->t->s == MOD) op.type = OP_MOD;
    else {
      unimpl("in parse expression arithmetic");
      return false;
    }
    if (!next_token(t)) return false;
    if (!parse_expression(p)) return false;
    if (check_for_continued_expression(p)) {
      OpPrec current = get_prec_by_op_type(op.type);
      OpPrec next = parse_prec_for_next_token(p);
      if (next > current) {
        if (!next_token(t)) return false;
        if (!parse_expression_arithmetics(p)) return false;
      }
    }
    if (!optimize_expression(p, &op)) return false;
    append_op(p, op);
    break;
      
  case LESS:
    op.type = OP_LESS;
    if (peek_next_token(t).t == TOKEN_TYPE_SPECIAL) {
      if (!expect_special_many(t, LESS, EQUAL)) return false;
      if (t->t->s == LESS) op.type = OP_SHL;
      else if (t->t->s == EQUAL) op.type = OP_LESS_EQUAL;
      else {
        unimpl("in parse expression arithmetic");
        return false;
      }
    }

    if (!next_token(t)) return false;
    if (!parse_expression(p)) return false;
    if (check_for_continued_expression(p)) {
      OpPrec current = get_prec_by_op_type(op.type);
      OpPrec next = parse_prec_for_next_token(p);
      if (next > current) {
        if (!next_token(t)) return false;
        if (!parse_expression_arithmetics(p)) return false;
      }
    }
    if (!optimize_expression(p, &op)) return false;
    append_op(p, op);
    break;
  case GREATER:
    op.type = OP_GREATER;
    if (peek_next_token(t).t == TOKEN_TYPE_SPECIAL) {
      if (!expect_special_many(t, GREATER, EQUAL)) return false;
      if (t->t->s == GREATER) op.type = OP_SHR;
      else if (t->t->s == EQUAL) op.type = OP_GREATER_EQUAL;
      else {
        unimpl("in parse expression arithmetic");
        return false;
      }
    }
    if (!next_token(t)) return false;
    if (!parse_expression(p)) return false;
    if (check_for_continued_expression(p)) {
      OpPrec current = get_prec_by_op_type(op.type);
      OpPrec next = parse_prec_for_next_token(p);
      if (next > current) {
        if (!next_token(t)) return false;
        if (!parse_expression_arithmetics(p)) return false;
      }
    }
    if (!optimize_expression(p, &op)) return false;
    append_op(p, op);
    break;
  case EQUAL: case NOT:
    if (!expect_special(t, EQUAL)) return false;
    if (op.start->s == EQUAL) op.type = OP_EQUAL;
    else if (op.start->s == NOT) op.type = OP_NOT_EQUAL;
    else {
      unimpl("in parse expression arithmetic");
      return false;
    }
    if (!next_token(t)) return false;
    if (!parse_expression(p)) return false;
    if (check_for_continued_expression(p)) {
      OpPrec current = get_prec_by_op_type(op.type);
      OpPrec next = parse_prec_for_next_token(p);
      if (next > current) {
        if (!next_token(t)) return false;
        if (!parse_expression_arithmetics(p)) return false;
      }
    }
    if (!optimize_expression(p, &op)) return false;
    append_op(p, op);
    break;

  case AND:
    op.type = OP_BITWISE_AND;
    if (peek_next_token(t).t == TOKEN_TYPE_SPECIAL) {
      if (!expect_special(t, AND)) return false;
      op.type = OP_LOGICAL_AND;
    }
    if (!next_token(t)) return false;
    if (!parse_expression(p)) return false;
    if (check_for_continued_expression(p)) {
      OpPrec current = get_prec_by_op_type(op.type);
      OpPrec next = parse_prec_for_next_token(p);
      if (next > current) {
        if (!next_token(t)) return false;
        if (!parse_expression_arithmetics(p)) return false;
      }
    }
    if (!optimize_expression(p, &op)) return false;
    append_op(p, op);
    break;
  case OR:
    op.type = OP_BITWISE_OR;
    if (peek_next_token(t).t == TOKEN_TYPE_SPECIAL) {
      if (!expect_special(t, OR)) return false;
      op.type = OP_LOGICAL_OR;
    }
    if (!next_token(t)) return false;
    if (!parse_expression(p)) return false;
    if (check_for_continued_expression(p)) {
      OpPrec current = get_prec_by_op_type(op.type);
      OpPrec next = parse_prec_for_next_token(p);
      if (next > current) {
        if (!next_token(t)) return false;
        if (!parse_expression_arithmetics(p)) return false;
      }
    }
    if (!optimize_expression(p, &op)) return false;
    append_op(p, op);
    break;
  case SPECIAL_TYPES: unimpl("SPECIAL_TYPES"); break;
  }

  return true;  
}

TARE_DEF bool optimize_expression(Parser *p, Operation *op) {
  if (p == NULL) return false;

  // TODO: attach types to expressions.
  switch (op->type) {
  case OP_PTR_ADD: case OP_PTR_SUB: case OP_ELEM_ADD: case OP_ELEM_SUB:
  case OP_READ_SIZE:
  case OP_CONDITIONAL: case OP_GOTO: case OP_ADDRESS:
  case OP_FUNCALL: case OP_RET: case OP_WRITE: case OP_READ: case OP_SYSCALL:
  case OP_TAPE: case OP_HEAD: case OP_BASE: case OP_INDEX: case OP_LENGTH:
  case OP_CONST: case OP_PUSH: case OP_POP: case OP_NUM:
  case OP_POP_FROM_OPS: case OP_TYPES: default: return false;

  case OP_ADD: case OP_SUB:
  case OP_MUL: case OP_DIV: case OP_MOD:
  case OP_SHL: case OP_SHR:
  case OP_LESS: case OP_LESS_EQUAL:
  case OP_GREATER: case OP_GREATER_EQUAL:
  case OP_EQUAL: case OP_NOT_EQUAL:
  case OP_BITWISE_AND: case OP_BITWISE_OR:
  case OP_LOGICAL_AND: case OP_LOGICAL_OR:
    if (p->stack.count < 2) return false;
    else {
      Operation *second = p->func->items + p->func->count - 1;
      Operation *first = second - 1;
      if (first->type != OP_NUM || second->type != OP_NUM) return true;

      if (op->type == OP_ADD) op->op = first->op + second->op;
      else if (op->type == OP_SUB) op->op = first->op - second->op;
      else if (op->type == OP_MUL) op->op = first->op * second->op;
      else if (op->type == OP_DIV) op->op = first->op / second->op;
      else if (op->type == OP_MOD) op->op = first->op % second->op;
      else if (op->type == OP_SHL) op->op = first->op << second->op;
      else if (op->type == OP_SHR) op->op = first->op >> second->op;
      else if (op->type == OP_LESS) op->op = first->op < second->op;
      else if (op->type == OP_LESS_EQUAL) op->op = first->op <= second->op;
      else if (op->type == OP_GREATER) op->op = first->op > second->op;
      else if (op->type == OP_GREATER_EQUAL) op->op = first->op >= second->op;
      else if (op->type == OP_EQUAL) op->op = first->op == second->op;
      else if (op->type == OP_NOT_EQUAL) op->op = first->op != second->op;
      else if (op->type == OP_BITWISE_AND) op->op = first->op & second->op;
      else if (op->type == OP_BITWISE_OR) op->op = first->op | second->op;
      else if (op->type == OP_LOGICAL_AND) op->op = first->op && second->op;
      else if (op->type == OP_LOGICAL_OR) op->op = first->op || second->op;
      else {
        Tokenizer *t = p->t;
        t->t = op->start;
        unimpl("in optimize expression");
        return false;
      }
      p->func->count -= 2;
      p->stack.count -= 2;
      op->type = OP_NUM;
    }
    break;
  case OP_NOT:
    if (p->stack.count < 1) return false;
    else {
      Operation *operation = p->func->items + p->func->count - 1;
      if (operation->type == OP_NUM) {
        op->op = !operation->op;
        p->func->count--;
        p->stack.count--;
        op->type = OP_NUM;
      }
    }
    break;
    
  case OP_DEREF:
    {
      Tokenizer *t = p->t;
      t->t = op->start;
      unimpl("OP_DEREF optimization");
      return false;
    }
    break;
    
  case OP_ASSIGN:
    {
      Tokenizer *t = p->t;
      t->t = op->start;
      unimpl("OP_ASSIGN optimization");
      return false;
    }
  case OP_GVID: case OP_LVID: case OP_RVID: case OP_AVID:
    {
      Tokenizer *t = p->t;
      t->t = op->start;
      unimpl("OP_*VID optimization");
      return false;
    }
  }

  return true;
}

TARE_DEF bool check_for_continued_expression(Parser *p) {
  Tokenizer *t = p->t;
  if (peek_next_token(t).t != TOKEN_TYPE_SPECIAL) return false;
  SpecialType s = peek_next_token(t).s;
  switch (s) {
  case PAR_BGN: case PAR_END: case GRP_BGN: case GRP_END:
  case BLK_BGN: case BLK_END: case DQUOTE: case SQUOTE:
  case ESC: case SEP: case END: case DOT: case DEF:
    return false;
  case DIV: case MOD: case MULT: case ADD: case SUB:
  case LESS: case GREATER: case EQUAL: case NOT:
  case AND: case OR:
    return true;
  case SPECIAL_TYPES: return false;
  }
  return false;
}

static_assert(OP_PRECS == 10, "update get_prec_by_op_type");
TARE_DEF OpPrec get_prec_by_op_type(OpType type) {
  switch (type) {
  case OP_PTR_ADD: case OP_PTR_SUB: case OP_ELEM_ADD: case OP_ELEM_SUB:
  case OP_READ_SIZE:
  case OP_CONDITIONAL: case OP_GOTO: case OP_ADDRESS:
  case OP_FUNCALL: case OP_RET: case OP_WRITE: case OP_READ: case OP_SYSCALL:
  case OP_TAPE: case OP_HEAD: case OP_BASE: case OP_INDEX: case OP_LENGTH:
  case OP_CONST: case OP_PUSH: case OP_POP: case OP_NUM:
  case OP_POP_FROM_OPS: case OP_TYPES:
    return OP_PRECS;
    
  case OP_ADD: case OP_SUB: return PREC_ADD_SUB;
  case OP_MUL: case OP_DIV: case OP_MOD: return PREC_MUL_DIV_REM;

  case OP_LESS: case OP_LESS_EQUAL:
    /* return OP_PRECS; // TODO: fix this */
  case OP_GREATER: case OP_GREATER_EQUAL:
    return PREC_LESS_GREATER_LEQUAL_GEQUAL;
    return OP_PRECS; // TODO: fix this
  case OP_EQUAL: case OP_NOT_EQUAL:
    return PREC_EQUAL_NEQUAL;
    /* return OP_PRECS; // TODO: fix this */
  case OP_BITWISE_AND: return PREC_BITWISE_AND;
  case OP_BITWISE_OR: return PREC_BITWISE_OR;
    /* return OP_PRECS; // TODO: fix this */
  case OP_LOGICAL_AND: return PREC_LOGICAL_AND;
  case OP_LOGICAL_OR: return PREC_LOGICAL_OR;
    /* return OP_PRECS; // TODO: fix this */
  
  case OP_SHL: case OP_SHR: return PREC_SHL_SHR;
  case OP_DEREF: return OP_PRECS;

  case OP_NOT: return OP_PRECS;
    
  case OP_ASSIGN:
  case OP_GVID: case OP_LVID: case OP_RVID: case OP_AVID:
  default: return OP_PRECS;
  }
}

static_assert(OP_PRECS == 10, "update get_prec_by_special_type");
TARE_DEF OpPrec get_prec_by_special_type(SpecialType s) {
  switch (s) {
  case PAR_BGN: return PREC_PAR_BGN;
  case PAR_END: case GRP_BGN: case GRP_END: case BLK_BGN: case BLK_END:
  case DQUOTE: case SQUOTE: case ESC: case SEP: case END: case DOT: case DEF:
    return OP_PRECS;
  case DIV: case MOD: case MULT: return PREC_MUL_DIV_REM;
  case ADD: case SUB: return PREC_ADD_SUB;
      
  case LESS: case GREATER:
    return PREC_LESS_GREATER_LEQUAL_GEQUAL;
    /* return PREC_LESS_GREATER; */
    
  case NOT:
    return PREC_EQUAL_NEQUAL;
    /* return OP_PRECS; */

  case AND:
    return PREC_BITWISE_AND;
  case OR:
    return PREC_BITWISE_OR;
    /* return OP_PRECS; // TODO: fix */
    
  case EQUAL:
    return PREC_EQUAL_NEQUAL;
  case SPECIAL_TYPES: default: return OP_PRECS;
  }
}

static_assert(OP_PRECS == 10, "update parse_prec_for_next_token");
TARE_DEF OpPrec parse_prec_for_next_token(Parser *p) {
  Tokenizer *t = p->t;

  Token next = peek_next_token(t);
  SpecialType s = next.s;
  OpPrec prec = get_prec_by_special_type(s);
  if (prec != PREC_LESS_GREATER_LEQUAL_GEQUAL) {
    if (prec != PREC_BITWISE_OR && prec != PREC_BITWISE_AND)
    return prec;
  }
  
  // Handle `<<` and `>>`
  next = peek_forward_token(t, 2);
  if (next.t != TOKEN_TYPE_SPECIAL) return prec;
  if (next.s != s) return prec;
  if (next.s == OR) return PREC_LOGICAL_OR;
  if (next.s == AND) return PREC_LOGICAL_AND;
  return PREC_SHL_SHR;
}

TARE_DEF bool is_token_expression(const Token *t) {
  switch (t->t) {
  case TOKEN_TYPE_NAME: return false;
  case TOKEN_TYPE_WHOLE_NUM: return true;
  case TOKEN_TYPE_FRAC_NUM: return false; // should be true later, I suppose
  case TOKEN_TYPE_KEYWORD:
    switch (t->k) {
    case KEY_F: case KEY_B:
    case KEY_A: case KEY_S:
    case KEY_N: case KEY_P:
    case KEY_I: case KEY_D:
      return true; // TODO: decide if these should be expressions or not
    case KEY_R8: case KEY_R16: case KEY_R32: case KEY_R64:
    case KEY_IF: case KEY_WHILE: case KEY_BREAK: case KEY_CONT:
      return false;
    case KEY_FUNC:
      return false;
    case KEY_RET:
      return false;
    case KEY_WRITE: case KEY_READ:
    case KEY_SYSCALL:
    case KEY_TAPE: case KEY_HEAD: case KEY_BASE: case KEY_INDEX:
    case KEY_LENGTH:
      return true;
    case KEY_CONST: return false;

      // TODO: FIX `push` AND `pop`
    case KEY_PUSH:
    case KEY_POP:
      return true;

    case KEY_ADD: case KEY_SUB:
    case KEY_MUL: case KEY_DIV: case KEY_MOD:
    case KEY_SHL: case KEY_SHR:
    case KEY_NOT:
    case KEY_BITWISE_AND: case KEY_BITWISE_OR:
    case KEY_LOGICAL_AND: case KEY_LOGICAL_OR:
    case KEY_LESS: case KEY_LESS_EQUAL:
    case KEY_GREATER: case KEY_GREATER_EQUAL:
    case KEY_EQUAL: case KEY_NOT_EQUAL:
      return true;
    case KEY_DEREF:
      return false;
    
    case KEYWORD_TYPES: default: return false;
    }
    break;
  case TOKEN_TYPE_SPECIAL:
    switch (t->s) {
    case PAR_BGN: return true;
    case PAR_END:
    case GRP_BGN: case GRP_END:
    case BLK_BGN: case BLK_END:
    case END:
    case DOT:
    case DEF:
    case DQUOTE: case SQUOTE: case ESC:
      return false;
    case SEP: return false; // ???
      
    case DIV: case MOD: case MULT: case ADD: case SUB: case AND: case OR:
      /* return true; */
      return false;
      
    case LESS: case GREATER: case EQUAL: return false;
    case NOT:
      return true;
      /* return false; */
    case SPECIAL_TYPES: return false;
    }
    break;
  case TOKEN_TYPE_STRING: return false;
  case TOKEN_TYPE_CHAR: return false;

  case TOKEN_TYPE_GVID: return false;
  case TOKEN_TYPE_LVID: return false;
  case TOKEN_TYPE_RVID: return false;
  case TOKEN_TYPE_AVID: return false;
    
  case TOKEN_TYPE_TID: return false;
  case TOKEN_TYPE_FID: return true;
  case TOKEN_TYPES: default: return false;
  }
  
  return false;
}

TARE_DEF bool parse_func_sig(Parser *p) {
  Tokenizer *t = p->t;
  if (!expect_fid(t)) return false;
  const Token *name = t->t;
  size_t fid = t->t->fid;
  /* Func *fn = p->fns->items + fid; */
  Function *fn = p->funcs->items + fid;
  
  // Should probably become more involved than this.
  if (!to_token(t, fn->start)) return false;
  p->func = p->funcs->items + fid;
  p->func->name = sv_from_token(name);
  return true;
}

TARE_DEF bool validate_function_signature(Parser *p, Operation *op, size_t args) {
  Function *fn = p->funcs->items + op->op;
  if (args == fn->args.count) return true;

  Tokenizer *t = p->t;
  
  const char *plural = fn->args.count == 1 ? "" : "s";
  const char *preamble = args > fn->args.count ? "many" : "few";
  const char *plural2 = args == 1 ? "has" : "have";
  
  diag_errf(t, t->t, "too %s arguments! Function `%.*s` requires %zu argument%s, yet %zu %s been provided.\n", preamble, SV_ARG(fn->name), fn->args.count, plural, args, plural2);
  diag_errf(t, op->start, "incorrect function signature! Function `%.*s` has been defined with the following signature:\n", SV_ARG(fn->name));
  Token *first = fn->first;
  Token *last = t->items + fn->start;
  size_t length = (size_t) (last->f - first->f) + 1;
  StringView note = {.s = first->f, .l = length};
  diag_notef(t, first, "%.*s\n", SV_ARG(note));

  length = strlen(t->path);
  length++; // ':'
  Loc loc = get_token_loc(t, fn->first);
  while (loc.row != 0) {
    loc.row = (loc.row - (loc.row % 10)) / 10;
    length++;
  }
  length++; // ':'
  while (loc.col != 0) {
    loc.col = (loc.col - (loc.col % 10)) / 10;
    length++;
  }
  length++; // ':'
  length++; // ' '
  length += strlen("note: func ");
          
  fprintf(stderr, "%*s^", (int) length, "");
  for (size_t i = 0; i < 10; i++) fprintf(stderr, "~");
  fprintf(stderr, "\n");
  return false;
}

TARE_DEF bool pop_stack(Parser *p) {
  if (p->stack.count < 1) return false;
  Operation op = p->stack.items[--p->stack.count];
  bool debug = false;
  if (debug) printf("--------------------------------------------------\n");
  if (debug) debug_print_token(p->t, op.start);
  if (debug) printf("stack before pop: %zu\n", p->stack.count + 1);
  // add 1 to account for the prefix decrement in the assign  ^
  switch (op.type) {
  case OP_PTR_ADD: case OP_PTR_SUB: case OP_ELEM_ADD: case OP_ELEM_SUB:
    if (!pop_stack(p)) return false;
    break;
  case OP_READ_SIZE: return true;
  case OP_CONDITIONAL:
    break; // handle properly
  case OP_GOTO: case OP_ADDRESS: return true;
  case OP_FUNCALL: return true; // TODO: handle this properly
  case OP_RET: return true;
  case OP_WRITE: case OP_READ:
    if (!pop_stack(p)) return false;
    if (!pop_stack(p)) return false;
    break;
  case OP_SYSCALL:
    while (op.op--) if (!pop_stack(p)) return false;
    break;
  case OP_TAPE: case OP_HEAD: case OP_BASE: case OP_INDEX: case OP_LENGTH:
    break;
    return true;
  case OP_CONST: return true; // not sure how to handle this yet

  case OP_PUSH: case OP_POP: return true;

  case OP_ADD: case OP_SUB: case OP_MUL: case OP_DIV: case OP_MOD:
  case OP_SHL: case OP_SHR:
    if (!pop_stack(p)) return false;
    if (!pop_stack(p)) return false;
    break;
  case OP_NOT:
    if (!pop_stack(p)) return false;
    break;
  case OP_BITWISE_AND: case OP_BITWISE_OR:
  case OP_LOGICAL_AND: case OP_LOGICAL_OR:
  case OP_LESS: case OP_LESS_EQUAL:
  case OP_GREATER: case OP_GREATER_EQUAL:
  case OP_EQUAL: case OP_NOT_EQUAL:
    if (!pop_stack(p)) return false;
    if (!pop_stack(p)) return false;
    break;
  
  case OP_DEREF: return true; // not sure how to handle this yet

  case OP_NUM:
    break;
    return true;
  case OP_POP_FROM_OPS: return true; // ????
  case OP_ASSIGN: return false; // ???
  case OP_TYPES: return false; // this should be an error
  case OP_GVID: case OP_LVID: case OP_RVID: case OP_AVID: return true;
  }
  if (debug) printf("stack after pop: %zu\n", p->stack.count);
  if (debug) printf("--------------------------------------------------\n\n");
  return true;
}

TARE_DEF bool op_has_side_effect(Operation *op) {
  switch (op->type) {
  case OP_PTR_ADD: case OP_PTR_SUB: case OP_ELEM_ADD: case OP_ELEM_SUB:
  case OP_READ_SIZE:
  case OP_CONDITIONAL: case OP_GOTO: case OP_ADDRESS:
  case OP_FUNCALL: case OP_RET:
  case OP_WRITE: case OP_READ: case OP_SYSCALL:
    return true;
  
  case OP_TAPE: case OP_HEAD: case OP_BASE: case OP_INDEX:
  case OP_LENGTH: return false;
  case OP_CONST: return true; // ????

  case OP_PUSH: case OP_POP: return true; // ???

  case OP_ADD: case OP_SUB:
  case OP_MUL: case OP_DIV: case OP_MOD:
  case OP_SHL: case OP_SHR:
  case OP_NOT:
  case OP_BITWISE_AND: case OP_BITWISE_OR:
  case OP_LOGICAL_AND: case OP_LOGICAL_OR:
  case OP_LESS: case OP_LESS_EQUAL:
  case OP_GREATER: case OP_GREATER_EQUAL:
  case OP_EQUAL: case OP_NOT_EQUAL:
    return false;
  
  case OP_DEREF: return true; // ????

  case OP_NUM: return false;

  case OP_POP_FROM_OPS: return true;

  case OP_ASSIGN: return true;

  case OP_GVID: case OP_LVID: case OP_RVID: case OP_AVID: return false;
    
  case OP_TYPES: return false; // should be an error.
  default: assert(false && "unreachable");
  }
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

TARE_DEF bool patch_tokenizer_funcs(Tokenizer *t, Functions *fns) {
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

TARE_DEF bool patch_tokenizer_func(Tokenizer *t, Functions *fns) {
  Token *first = t->t;
  // Get the function's name.
  if (!expect_name(t)) return false;
  Function fn = {.name = sv_from_token(t->t), .first = first};
  /* Function fn = {.name = sv_from_token(t->t)}; */
  size_t fid = fns->count;
  if (sv_eq(fn.name, fns->items[0].name)) fid = 0;
  assert(fns->count > 0 && "please don't corrupt the memory");
  /* da_append(fns, fn); */
  Function *f = fns->items + fid;
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
    diag_err(t, first, "invalid function signature!\n");
    diag_notef(t, first, "a proper function signature would be one of the following four options: \n"
               "%*.sfunc NAME (ARGS) => (RETS): {BODY}\n"
               "%*.sfunc NAME (ARGS): {BODY}\n"
               "%*.sfunc NAME => (RETS): {BODY}\n"
               "%*.sfunc NAME: {BODY}\n",
               4, "", 4, "", 4, "", 4, "");
    diag_note(t, first, "while the space between `func` and `NAME` is necessary, the other whitespaces shown in this signature are unnecessary and ignored by the lexer and tokenizer.\n");
    diag_notef(t, first, "furthermore, `BODY` must include in it a `%.*s` statement.\n", SV_ARG(Keywords[KEY_RET]));
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

TARE_DEF bool patch_tokenizer_args(Tokenizer *t, Function *fn, bool args) {
  size_t vid = 0;
  ScopeType scope_type = 0;
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
    if (args) {
      vid = fn->args.count;
      scope_type = SCOPE_ARGUMENT;
    }
    else {
      vid = fn->rets.count;
      scope_type = SCOPE_RETURN;
    }
    /* vid = fn->args.count + fn->rets.count; // TODO: make this actually work */
    
    Var arg = {
      .name = sv_from_token(t->t),
      .vid = vid,
      .tid = tid,
      .type = scope_type,
    };
      
    for (Token *tok = t->t; tok < end; tok++) {
      if (tok->t != TOKEN_TYPE_NAME) continue;
      if (tok_eq(t->t, tok)) {
        switch (scope_type) {
        case SCOPE_GLOBAL:
          assert(false && "SCOPE_GLOBAL should be impossible");
          break;
        case SCOPE_LOCAL:
          assert(false && "SCOPE_LOCAL should be impossible");
          break;
        case SCOPE_ARGUMENT:
          tok->t = TOKEN_TYPE_AVID;
          tok->avid = arg.vid;
          break;
        case SCOPE_RETURN:
          tok->t = TOKEN_TYPE_RVID;
          tok->rvid = arg.vid;
          break;
        case SCOPE_TYPES:
          assert(false && "SCOPE_TYPES should be impossible");
          break;
        }
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
        diag_err(t, tok, "can't close parentheses that haven't been opened.\n");
      }
      size_t jmp = par.items[--par.count];
      tok->jmp = jmp;
      tok = t->items + jmp;
      tok->jmp = i;
    } else if (tok->s == GRP_END) {
      if (grp.count == 0) {
        diag_err(t, tok, "can't end a group that hasn't been started.\n");
      }
      size_t jmp = grp.items[--grp.count];
      tok->jmp = jmp;
      tok = t->items + jmp;
      tok->jmp = i;
    } else if (tok->s == BLK_END) {
      if (blk.count == 0) {
        diag_err(t, tok, "can't close a block that hasn't been started.\n");
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
      diag_err(t, tok, "parentheses opened but didn't close.\n");
    }
  }

  if (grp.count > 0) {
    for (size_t i = 0; i < grp.count; i++) {
      size_t jmp = grp.items[i];
      Token *tok = t->items + jmp;
      diag_err(t, tok, "group started but didn't end.\n");
    }
  }

  if (blk.count > 0) {
    for (size_t i = 0; i < blk.count; i++) {
      size_t jmp = blk.items[i];
      Token *tok = t->items + jmp;
      diag_err(t, tok, "block started but didn't close.\n");
    }
  }

  if (par.items) free(par.items);
  if (grp.items) free(grp.items);
  if (blk.items) free(blk.items);
  
  return (par.count == 0) && (grp.count == 0) && (blk.count == 0);
}

TARE_DEF bool patch_tokenizer_gvids(Parser *p) {
  Tokenizer *t = p->t;
  Vars *globals = p->globals;
  size_t save = t->index;

  if (!first_token(t)) return false;

  // The way structs and enums will likely be implemented, this will
  // need to be modified.
  do {
    if (t->t->t == TOKEN_TYPE_FID) {
      Function *func = p->funcs->items + t->t->fid;
      if (!to_token(t, func->end)) return false;
    }
    if (t->t->t != TOKEN_TYPE_TID) continue;
    size_t tid = t->t->tid;
    if (!expect_name(t)) return false;
    Var var = {
      .type = SCOPE_GLOBAL,
      .vid = globals->count, .tid = tid,
      .name = sv_from_token(t->t),
    };
    for (size_t i = 0; i < t->count; i++) {
      Token *tok = t->items + i;
      if (tok->t != TOKEN_TYPE_NAME) continue;
      if (tok_eq(t->t, tok)) {
        tok->t = TOKEN_TYPE_GVID;
        tok->gvid = var.vid;
      }
    }
    if (!expect_special_many(t, END, EQUAL)) return false;
    da_append(globals, var);
  } while (next_token(t));
  
  return to_token(t, save);
}

TARE_DEF bool patch_tokenizer_lvids(Parser *p) {
  Tokenizer *t = p->t;
  size_t save = t->index;
  
  Longs scopes = {0};
  
  for (size_t i = 0; i < p->funcs->count; i++) {
    Function *func = p->funcs->items + i;
    if (!to_token(t, func->start)) return false;
    Vars *lvars = &func->lvars;

    da_append(&scopes, t->index);
    
    while (next_token(t)) {
      if (t->index >= func->end) break;

      if (t->t->t == TOKEN_TYPE_SPECIAL) {
        if (t->t->s == BLK_BGN) da_append(&scopes, t->index);
        else if (t->t->s == BLK_END) scopes.count--;
      }
      if (t->t->t != TOKEN_TYPE_TID) continue;
      size_t tid = t->t->tid;
      if (!expect_name(t)) return false;
      
      Var var = {
        .type = SCOPE_LOCAL,
        .vid = lvars->count, .tid = tid,
        .name = sv_from_token(t->t),
      };

      Token *start = t->t;
      Token *scope = t->items + scopes.items[scopes.count - 1];
      Token *end = t->items + scope->jmp;
      for (Token *tok = start; tok < end; tok++) {
        if (tok->t != TOKEN_TYPE_NAME) continue;
        if (tok_eq(t->t, tok)) {
          tok->t = TOKEN_TYPE_LVID;
          tok->gvid = var.vid;
        }
      }
      if (!expect_special_many(t, END, EQUAL)) return false;
      da_append(lvars, var);
    } 
  }

  if (scopes.items) free(scopes.items);

  return to_token(t, save);
}

TARE_DEF size_t get_vars_size(Vars *vars) {
  size_t result = 0;
  for (size_t i = 0; i < vars->count; i++) {
    Var arg = vars->items[i];
    switch (arg.tid) {
    case TYPE_U8: result++; break;
    case TYPE_U16: result += 2; break;
    case TYPE_U32: result += 4; break;
    case TYPE_U64: result += 8; break;
    case TYPES_COUNT: 
    default: assert(false && "unimplemented");
    }
  }
  return result;
}

TARE_DEF size_t get_vars_size_with_padding(Vars *vars) {
  size_t result = 0;
  for (size_t i = 0; i < vars->count; i++) {
    Var var = vars->items[i];
    switch (var.tid) {
    case TYPE_U8: result++; break;
    case TYPE_U16: result += 2; break;
    case TYPE_U32: result += 4; break;
    case TYPE_U64: result += 8; break;
    case TYPES_COUNT: 
    default: assert(false && "unimplemented");
    }
    if (result % 8 != 0) result += (8 - result % 8);
  }
  return result;
}

TARE_DEF size_t get_args_size(Function *fn) {
  return get_vars_size(&fn->args);
}

TARE_DEF size_t get_rets_size(Function *fn) {
  return get_vars_size(&fn->rets);
}

TARE_DEF size_t get_lvars_size(Function *fn) {
  return get_vars_size(&fn->lvars);
}

TARE_DEF size_t get_args_size_with_padding(Function *fn) {
  return get_vars_size_with_padding(&fn->args);
}

TARE_DEF size_t get_rets_size_with_padding(Function *fn) {
  return get_vars_size_with_padding(&fn->rets);
}

TARE_DEF size_t get_lvars_size_with_padding(Function *fn) {
  return get_vars_size_with_padding(&fn->lvars);
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
  case OP_LENGTH: return "OP_LENGTH";
    
  case OP_CONST: return "OP_CONST";

  case OP_PUSH: return "OP_PUSH";
  case OP_POP: return "OP_POP";

  case OP_ADD: return "OP_ADD";
  case OP_SUB: return "OP_SUB";
  case OP_MUL: return "OP_MUL";
  case OP_DIV: return "OP_DIV";
  case OP_MOD: return "OP_MOD";
  case OP_SHL: return "OP_SHL";
  case OP_SHR: return "OP_SHR";
  case OP_LESS: return "OP_LESS";
  case OP_LESS_EQUAL: return "OP_LESS_EQUAL";
  case OP_GREATER: return "OP_GREATER";
  case OP_GREATER_EQUAL: return "OP_GREATER_EQUAL";
  case OP_EQUAL: return "OP_EQUAL";
  case OP_NOT_EQUAL: return "OP_NOT_EQUAL";
  case OP_NOT: return "OP_NOT";
  case OP_BITWISE_AND: return "OP_BITWISE_AND";
  case OP_BITWISE_OR: return "OP_BITWISE_OR";
  case OP_LOGICAL_AND: return "OP_LOGICAL_AND";
  case OP_LOGICAL_OR: return "OP_LOGICAL_OR";
    
  case OP_DEREF: return "OP_DEREF";

  case OP_NUM: return "OP_NUM";
  
  case OP_POP_FROM_OPS: return "OP_POP_FROM_OPS";
    
  case OP_ASSIGN: return "OP_ASSIGN";
    
  case OP_GVID: return "OP_GVID";
  case OP_LVID: return "OP_LVID";
  case OP_RVID: return "OP_RVID";
  case OP_AVID: return "OP_AVID";
    
  case OP_TYPES: return "OP_TYPES";
  default: return "";
  }
}

static_assert(sizeof(Var) == 48, "Struct `Var` has been updated. Make sure debugging works out properly.");
TARE_DEF void print_var(const Var *var, size_t i) {
  printf("--------------------------------------------------\n");
  printf("Var (%zu) = {\n", i);
  printf("  .type = %u\n", var->type);
  printf("  .vid = %zu\n", var->vid);
  printf("  .tid = %zu\n", var->tid);
  printf("  .name = %.*s\n", SV_ARG(var->name));
  printf("}\n");
  printf("--------------------------------------------------\n");
}

TARE_DEF void print_vars(const Vars *vars) {
  for (size_t i = 0; i < vars->count; i++) {
    Var *var = vars->items + i;
    print_var(var, i);
  }
}

static_assert(sizeof(Operation) == 56, "Struct `Operation` has been updated. Make sure debugging works out properly.");
TARE_DEF void print_op(const Tokenizer *t, const Operation *op, size_t i) {
  printf("--------------------------------------------------\n");
  debug_print_token(t, op->start);
  printf("op (%zu) = {\n", i);
  printf("  .start = ");
  debug_print_token(t, op->start);
  printf("  .type = %s\n", op_type_as_string(op->type));
  printf("  .args = %p\n", (void*)op->args);
  printf("  .args_count = %zu\n", op->args_count);
  printf("  .op = %zu\n", op->op);
  if (op->name.s != NULL) printf("  .name = %.*s\n", SV_ARG(op->name));
  else printf("  .name = {.s = %p, .l = %zu}\n", op->name.s, op->name.l);
  printf("}\n");
  printf("--------------------------------------------------\n\n");
}

static_assert(sizeof(Function) == 136, "Struct `Function` has been updated. Make sure debugging works out properly.");
TARE_DEF void print_function(const Tokenizer *t, const Function *func) {
  printf("\n--------------------------------------------------\n");
  printf("Function `%.*s`\n", SV_ARG(func->name));
  for (size_t i = 0; i < func->count; i++) {
    const Operation *op = func->items + i;
    print_op(t, op, i);
  }
  printf("------------------- FUNCTION ARGUMENTS -------------------\n");
  print_vars(&func->args);
  printf("------------------- RETURN VALUES -------------------\n");
  print_vars(&func->rets);
  printf("------------------- LOCAL VARIABLES -------------------\n");
  print_vars(&func->lvars);
  printf("\n--------------------------------------------------\n");
}

static_assert(sizeof(Parser) == 64, "Struct `Parser` has been updated. Make sure debugging works out properly.");
TARE_DEF void print_functions(const Parser *p) {
  for (size_t i = 0; i < p->funcs->count; i++) {
    Function *func = p->funcs->items + i;
    print_function(p->t, func);
    print_vars(p->globals);
  }
}

#endif // PARSER_IMPLEMENTATION

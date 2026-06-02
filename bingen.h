#ifndef BINGEN_H_
#define BINGEN_H_

#include <elf.h>

typedef struct {
  Longs *longs; // Numbers too big to be passed directly to registers.
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
} BinaryGenerator;

TARE_DEF bool gen_elf(const char *output, BinaryGenerator *b);

#endif // BINGEN_H_
#ifdef BINGEN_IMPLEMENTATION

TARE_DEF bool gen_elf(const char *output, BinaryGenerator *b) {
  if (output == NULL || b == NULL) return false;
  printf("%s:%d: UNIMPLEMENTED!\n", __FILE__, __LINE__);
  return true;
}

#endif // BINGEN_IMPLEMENTATION


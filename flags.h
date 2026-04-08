#ifndef FLAGS_H_
#define FLAGS_H_

typedef enum {
  FLAG_SIMULATE,
  FLAG_COMPILE_FASM,
  FLAG_RUN,
  FLAG_TYPES,
} FlagType;

typedef struct {
  FlagType type;
  StringView name;
  bool on;
  bool flip;
} Flag;

typedef struct {
  Flag *items;
  size_t count;
  size_t capacity;
} Flags;

StringView FlagNames[FLAG_TYPES] = {
  SV_MAKE(-s),
  SV_MAKE(-c),
  SV_MAKE(-r),
};

TARE_DEF void init_flags(Flags *flags);
TARE_DEF void check_flags(StringView arg, Flags *flags);
TARE_DEF void parse_flags(int argc, const char **argv, Flags *flags);

TARE_DEF void print_flag(Flag flag);
TARE_DEF const char *flag_type_to_cstr(FlagType type);

#endif // FLAGS_H_

#ifdef FLAGS_IMPLEMENTATION

TARE_DEF void init_flags(Flags *flags) {
  for (size_t i = 0; i < FLAG_TYPES; i++) {
    Flag flag = {.type = i, .name = FlagNames[i]};
    da_append(flags, flag);
  }
}

TARE_DEF void check_flags(StringView arg, Flags *flags) {
  for (size_t i = 0; i < flags->count; i++) {
    Flag *flag = flags->items + i;
    if (flag->on && !flag->flip) continue;
    if (sv_eq(arg, flag->name)) flag->on = !flag->on;
  }
}

TARE_DEF void parse_flags(int argc, const char **argv, Flags *flags) {
  for (int i = 0; i < argc; i++) {
    StringView arg = sv_from_cstr(argv[i]);
    check_flags(arg, flags);
  }
}

TARE_DEF void print_flag(Flag flag) {
  printf("--------------------------------------------------\n");
  printf("{\n");
  printf("  .type = %s,\n", flag_type_to_cstr(flag.type));
  printf("  .name = %.*s,\n", SV_ARG(flag.name));
  printf("  .on = %s\n", flag.on ? "true" : "false");
  printf("  .flip = %s\n", flag.flip ? "true" : "false");
  printf("}\n");
  printf("--------------------------------------------------\n");
}

TARE_DEF const char *flag_type_to_cstr(FlagType type) {
  switch (type) {
  case FLAG_SIMULATE: return "FLAG_SIMULATE";
  case FLAG_COMPILE_FASM: return "FLAG_COMPILE_FASM";
  case FLAG_RUN: return "FLAG_RUN";
  case FLAG_TYPES: return "FLAG_TYPES";
  }
  return NULL;
}

#endif // FLAGS_IMPLEMENTATION

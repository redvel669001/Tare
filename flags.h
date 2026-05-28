#ifndef FLAGS_H_
#define FLAGS_H_

typedef struct {
  const char *name_short;
  const char *name_long;
  bool on;
  bool flip;
} Flag;

typedef struct {
  Flag **items;
  size_t count;
} Flags;

TARE_DEF void check_flags(const char *arg, Flags *flags);
TARE_DEF void parse_flags(int argc, const char **argv, Flags *flags);

TARE_DEF void print_flag(Flag flag);

#endif // FLAGS_H_

#ifdef FLAGS_IMPLEMENTATION

TARE_DEF void check_flags(const char *arg, Flags *flags) {
  for (size_t i = 0; i < flags->count; i++) {
    Flag *flag = flags->items[i];
    if (flag->on && !flag->flip) continue;
    if (strcmp(arg, flag->name_short) == 0 ||
        strcmp(arg, flag->name_long) == 0) flag->on = !flag->on;
  }
}

TARE_DEF void parse_flags(int argc, const char **argv, Flags *flags) {
  for (int i = 0; i < argc; i++) {
    const char *arg = argv[i];
    check_flags(arg, flags);
  }
}

TARE_DEF void print_flag(Flag flag) {
  printf("--------------------------------------------------\n");
  printf("{\n");
  printf("  .name_short = %s,\n", flag.name_short);
  printf("  .name_long = %s,\n", flag.name_long);
  printf("  .on = %s\n", flag.on ? "true" : "false");
  printf("  .flip = %s\n", flag.flip ? "true" : "false");
  printf("}\n");
  printf("--------------------------------------------------\n");
}

#endif // FLAGS_IMPLEMENTATION

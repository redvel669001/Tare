#ifndef FLAGS_H_
#define FLAGS_H_

typedef enum {
  FLAG_TYPE_BOOL = 0,
  FLAG_TYPE_U64,
  FLAG_TYPE_STR,
  FLAG_TYPES,
} FlagValueType;

typedef union {
  bool on;
  size_t u64;
  const char *str;
} FlagValue;

typedef struct {
  const char *name_short;
  const char *name_long;
  const char *description;
  FlagValueType type;
  bool parsed;
  FlagValue value;
  FlagValue default_value;
} Flag;

typedef struct {
  Flag **items;
  size_t count;
} Flags;

typedef struct {
  const char **args;
  size_t args_count;
  size_t index;
  const char *arg;
} Args;

TARE_DEF bool to_arg(Args *args, size_t index);
TARE_DEF bool first_arg(Args *args);
TARE_DEF bool next_arg(Args *args);

TARE_DEF bool arg_to_u64(const char *arg, size_t *u64);
TARE_DEF bool parse_flags(Args *args, Flags *flags);

TARE_DEF void print_flag(Flag flag);
TARE_DEF size_t max_pad_from_max_length(size_t max_length);
TARE_DEF void print_usage(FILE *out, const char *program, Flags flags);

#endif // FLAGS_H_

#ifdef FLAGS_IMPLEMENTATION

TARE_DEF bool to_arg(Args *args, size_t index) {
  if (index >= args->args_count) return false;
  args->index = index;
  args->arg = args->args[args->index];
  return true;
}

TARE_DEF bool first_arg(Args *args) {
  return to_arg(args, 0);
}

TARE_DEF bool next_arg(Args *args) {
  return to_arg(args, args->index + 1);
}

TARE_DEF bool arg_to_u64(const char *arg, size_t *u64) {
  size_t length = strlen(arg);
  *u64 = 0;
  for (size_t i = 0; i < length; i++) {
    char c = arg[i];
    if (c < '0' || c > '9') return false;
    *u64 = *u64 * 10 + (size_t) (c - '0');
  }
  return true;
}

TARE_DEF bool parse_flags(Args *args, Flags *flags) {
  if (!first_arg(args)) return false;
  do {
    for (size_t i = 0; i < flags->count; i++) {
      Flag *flag = flags->items[i];
      if (strcmp(args->arg, flag->name_short) != 0 &&
          strcmp(args->arg, flag->name_long) != 0) continue;
      flag->parsed = true;
      switch (flag->type) {
      case FLAG_TYPE_BOOL: flag->value.on = true; break;
      case FLAG_TYPE_U64:
        if (!next_arg(args)) return false;
        if (!arg_to_u64(args->arg, &flag->value.u64)) return false;
        break;
      case FLAG_TYPE_STR:
        if (!next_arg(args)) return false;
        flag->value.str = args->arg;
        break;
      case FLAG_TYPES: default: assert(false && "unreachable");
      }
      break;
    }
  } while (next_arg(args));

  for (size_t i = 0; i < flags->count; i++) {
      Flag *flag = flags->items[i];
      if (flag->parsed) continue;
      flag->value = flag->default_value;
  }
  return true;
}

TARE_DEF void print_flag(Flag flag) {
  printf("--------------------------------------------------\n");
  printf("{\n");
  printf("  .name_short = %s,\n", flag.name_short);
  printf("  .name_long = %s,\n", flag.name_long);
  switch (flag.type) {
  case FLAG_TYPE_BOOL:
    printf("  .on = %s\n", flag.value.on ? "true" : "false"); break;
  case FLAG_TYPE_U64: printf("  .u64 = %zu\n", flag.value.u64); break;
  case FLAG_TYPE_STR: printf("  .str = %s\n", flag.value.str); break;
  case FLAG_TYPES: default: assert(false && "unreachable");
  }
  printf("}\n");
  printf("--------------------------------------------------\n");
}

TARE_DEF size_t max_pad_from_max_length(size_t max_length) {
  size_t max_pad = max_length;
  size_t max_pad_rem = max_pad % 4;
  if (max_pad_rem != 0) max_pad += (4 - max_pad_rem);
  if (max_pad <= max_length + 1) max_pad += 2;
  return max_pad;
}

TARE_DEF void print_usage(FILE *out, const char *program, Flags flags) {
  fprintf(out, "Usage: %s [FLAGS]\n\n", program);
  fprintf(out, "FLAGS:\n");

  size_t length = 0;
  size_t name_short_max_length = 0;
  size_t name_long_max_length = 0;
  size_t description_max_length = 0;
  for (size_t i = 0; i < flags.count; i++) {
    Flag *flag = flags.items[i];
    switch (flag->type) {
    case FLAG_TYPE_BOOL:
      length = strlen(flag->name_short);
      if (length > name_short_max_length) name_short_max_length = length;
      length = strlen(flag->name_long);
      if (length > name_long_max_length) name_long_max_length = length;
      break;
    case FLAG_TYPE_U64:
      length = strlen(flag->name_short) + strlen(" <size>");
      if (length > name_short_max_length) name_short_max_length = length;
      length = strlen(flag->name_long) + strlen(" <size>");
      if (length > name_long_max_length) name_long_max_length = length;
      break;
    case FLAG_TYPE_STR:
      length = strlen(flag->name_short) + strlen(" <str>");
      if (length > name_short_max_length) name_short_max_length = length;
      length = strlen(flag->name_long) + strlen(" <str>");
      if (length > name_long_max_length) name_long_max_length = length;
      break;
    case FLAG_TYPES: default: assert(false && "unreachable");
    }
    length = strlen(flag->description);
    if (length > description_max_length) description_max_length = length;
  }

  size_t name_short_max_pad = max_pad_from_max_length(name_short_max_length);
  size_t name_long_max_pad = max_pad_from_max_length(name_long_max_length);
  size_t description_max_pad = max_pad_from_max_length(description_max_length);

  for (size_t i = 0; i < flags.count; i++) {
    Flag *flag = flags.items[i];
    size_t name_short_pad = name_short_max_pad - strlen(flag->name_short);
    size_t name_long_pad = name_long_max_pad - strlen(flag->name_long);
    size_t description_pad = description_max_pad - strlen(flag->description);
    switch (flag->type) {
    case FLAG_TYPE_BOOL:
      fprintf(out, "  %s%*s|  %s%*s:  %s%*s|  Default value: %s\n",
              flag->name_short, (int) name_short_pad, "",
              flag->name_long, (int) name_long_pad, "",
              flag->description, (int) description_pad, "",
              flag->default_value.on ? "true" : "false");
      break;
    case FLAG_TYPE_U64:
      name_short_pad -= strlen(" <size>");
      name_long_pad -= strlen(" <size>");
      fprintf(out, "  %s <size>%*s|  %s <size>%*s:  %s%*s|  Default value: %zu\n",
              flag->name_short, (int) name_short_pad, "",
              flag->name_long, (int) name_long_pad, "",
              flag->description, (int) description_pad, "",
              flag->default_value.u64);
      break;
    case FLAG_TYPE_STR:
      name_short_pad -= strlen(" <str>");
      name_long_pad -= strlen(" <str>");
      fprintf(out, "  %s <str>%*s|  %s <str>%*s:  %s%*s|  Default value: %s\n",
              flag->name_short, (int) name_short_pad, "",
              flag->name_long, (int) name_long_pad, "",
              flag->description, (int) description_pad, "",
              flag->default_value.str);
      break;
    case FLAG_TYPES: default: assert(false && "unreachable");
    }
  }
  putchar(10);
}

#endif // FLAGS_IMPLEMENTATION

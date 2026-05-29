#ifndef FLAGS_H_
#define FLAGS_H_

typedef struct {
  const char *name_short;
  const char *name_long;
  const char *description;
  bool on;
} Flag;

typedef struct {
  Flag **items;
  size_t count;
} Flags;

TARE_DEF void check_flags(const char *arg, Flags *flags);
TARE_DEF void parse_flags(int argc, const char **argv, Flags *flags);

TARE_DEF void print_flag(Flag flag);
TARE_DEF void print_usage(FILE *out, const char *program, const char *input, Flags flags);

#endif // FLAGS_H_

#ifdef FLAGS_IMPLEMENTATION

TARE_DEF void check_flags(const char *arg, Flags *flags) {
  for (size_t i = 0; i < flags->count; i++) {
    Flag *flag = flags->items[i];
    if (strcmp(arg, flag->name_short) == 0 ||
        strcmp(arg, flag->name_long) == 0) flag->on = true;
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
  printf("}\n");
  printf("--------------------------------------------------\n");
}

TARE_DEF void print_usage(FILE *out, const char *program, const char *input, Flags flags) {
  fprintf(out, "Usage:\n");
  fprintf(out, "%s %s[FLAGS]\n", program, input);
  fprintf(out, "\nFLAGS:\n");

  size_t length = 0;
  size_t name_short_max_length = 0;
  size_t name_long_max_length = 0;
  for (size_t i = 0; i < flags.count; i++) {
    Flag *flag = flags.items[i];
    length = strlen(flag->name_short);
    if (length > name_short_max_length) name_short_max_length = length;
    length = strlen(flag->name_long);
    if (length > name_long_max_length) name_long_max_length = length;
  }

  size_t name_short_label_length = strlen("SHORT");
  length = name_short_label_length;
  if (length > name_short_max_length) name_short_max_length = length;
  size_t name_long_label_length = strlen("LONG NAME");
  length = name_long_label_length;
  if (length > name_long_max_length) name_long_max_length = length;

  size_t name_short_max_pad = name_short_max_length;
  size_t max_pad_rem = name_short_max_pad % 4;
  if (max_pad_rem != 0) name_short_max_pad += (4 - max_pad_rem);
  if (name_short_max_pad <= name_short_max_length + 1) name_short_max_pad++;
  
  size_t name_long_max_pad = name_long_max_length;
  max_pad_rem = name_long_max_pad % 4;
  if (max_pad_rem != 0) name_long_max_pad += (4 - max_pad_rem);
  if (name_long_max_pad <= name_long_max_length + 1) name_long_max_pad++;

  printf("  SHORT%*s|  LONG NAME%*s|  DESCRIPTION\n",
         (int) (name_short_max_pad - name_short_label_length), "",
         (int) (name_long_max_pad - name_long_label_length), "");
  length = name_short_max_pad + name_long_max_pad
    + strlen("DESCRIPTION") + strlen("  |  | \n") + 80;
  if (length > 80) length = 80;
  for (size_t i = 0; i < length; i++) putchar('-');
  putchar(10);

  for (size_t i = 0; i < flags.count; i++) {
    Flag *flag = flags.items[i];
    size_t name_short_pad = name_short_max_pad - strlen(flag->name_short);
    size_t name_long_pad = name_long_max_pad - strlen(flag->name_long);
    fprintf(out, "  %s%*s|  %s%*s:  %s\n",
            flag->name_short, (int) name_short_pad, "",
            flag->name_long, (int) name_long_pad, "",
            flag->description);
  }
}

#endif // FLAGS_IMPLEMENTATION

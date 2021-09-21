void pstrcpy(char *buf, int buf_size, const char *str) {
  int c;
  char *q = buf;

  if (buf_size <= 0)
    return;

  for (;;) {
    c = *str++;
    if (c == 0 || q >= buf + buf_size - 1)
      break;
    *q++ = c;
  }
  *q = '\0';
}

#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static void reverse(char *s, int len) {
  int i, j;
  char temp;
  for (i = 0, j = len - 1; i < j; i++, j--) {
    temp = s[i];
    s[i] = s[j];
    s[j] = temp;
  }
}

static int itoa(int n, char *s, int base){
  assert(base <= 16);
  int i = 0, sign = n, bit;
  if(sign < 0) n = -n;
  do{
    bit = n % base;
    if(bit >= 10) s[i++] = 'a' + bit - 10;
    else s[i++] = '0' + bit;
  }while((n /= base) > 0);
  if(sign < 0) s[i++] = '-';
  s[i] = '\0';
  reverse(s, i);
  return i;
}

int printf(const char *fmt, ...) {
  char buffer[2048]; // 必须开大一点!
  va_list args;
  va_start(args, fmt);
  int ret = vsprintf(buffer, fmt, args);
  putstr(buffer);
  va_end(args);
  return ret;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  char *start = out;
  for (; *fmt != '\0'; ++fmt) {
    if (*fmt != '%') {
      *out = *fmt;
      ++out;
    } else {
      switch (*(++fmt)) {
      case '%': *out = *fmt; ++out; break;
      case 'd': out += itoa(va_arg(ap, int), out, 10); break;
      case 's':
        char *s = va_arg(ap, char*);
        strcpy(out, s);
        out += strlen(out);
        break;
      case 'c':
        *out = (char)va_arg(ap, int);
        ++out;
        break;
      }
    }
  }
  
  *out = '\0';
  return out - start;
}

int sprintf(char *out, const char *fmt, ...) {
  // panic("Not implemented");
  va_list pArgs;
  va_start(pArgs, fmt);
  char *start = out;

  for(;*fmt != '\0';++fmt){
    if(*fmt != '%'){
      *out = *fmt;
      ++out;
    }
    else{
      switch(*(++fmt)){
        case '%': *out = *fmt; ++out; break;
        case 'd': out += itoa(va_arg(pArgs, int), out, 10); break;
        case 's':
        char *s = va_arg(pArgs, char*);
        strcpy(out, s);
        out += strlen(out);
        break;
      }
    }
  }
  *out = '\0';
  va_end(pArgs);

  return out - start;
}
int snprintf(char *buffer, size_t size, const char *format, ...) {
  va_list args;
  va_start(args, format);
  int written = vsnprintf(buffer, size, format, args);
  putstr(buffer);
  va_end(args);
  return written;
}

int vsnprintf(char *buffer, size_t size, const char *format, va_list args) {
  int length = 0;
  char *end = buffer + size - 1;

  while (*format != '\0') {
    if (*format == '%') {
      format++;
      switch (*format) {
        case '%':
          if (buffer < end) *buffer = *format;
          ++buffer;
          ++length;
          break;
        case 's': {
          char *str = va_arg(args, char *);
          while (*str != '\0') {
            if (buffer < end) *buffer = *str;
            buffer++;
            str++;
            length++;
          }
          break;
        }
        case 'd': {
          int num = va_arg(args, int);
          if (num == 0) {
            if (buffer < end) *buffer = '0';
            buffer++;
            length++;
            break;
          }
          if (num < 0) {
            if (buffer < end) *buffer = '-';
            buffer++;
            length++;
            num = -num;
          }
          assert(num > 0);
          char number[12];
          int i = 0;
          while (num != 0) {
            number[i] = '0' + num % 10;
            i++;
            num /= 10;
          }
          while (i > 0) {
            i--;
            if (buffer < end) *buffer = number[i];
            buffer++;
            length++;
          }
          break;
        }
        case 'c': {
          if (buffer < end) *buffer = (char)va_arg(args, int);
          buffer++;
          length++;
          break;
        }
      }
      format++;
    } else {
      if (buffer < end) *buffer = *format;
      buffer++;
      format++;
      length++;
    }
  }
  *buffer = '\0';
  return length;
}

#endif

#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t len = 0;
  while(*s != '\0')
  {
    len ++; s++;
  }
  return len;
}

char *strcpy(char *dst, const char *src) {
  char *tmp = dst;
  while(*src != '\0')
  {
    *dst = *src; dst ++; src ++;
  }
  *dst = '\0';
  return tmp;
}

char *strncpy(char *dst, const char *src, size_t n) {
  char *tmp = dst;
  size_t i;
  for(i = 0; i < n && src[i] != '\0'; i ++)
  {
    dst[i] = src[i];
  }
  for(;i < n; i ++)
  {
    dst[i] = '\0';
  }
  return tmp;
}

char *strcat(char *dst, const char *src) {
  char *tmp = dst;
  while(*dst != '\0')
  {
    dst ++;
  }
  while(*src != '\0')
  {
    *dst = *src; dst ++; src ++;
  }
  *dst = '\0';
  return tmp;
}

int strcmp(const char *s1, const char *s2) {
  // panic("Not implemented");
  while (*s1 != '\0' && *s2 != '\0') {
    if (*s1 != *s2) {
      return (*s1 - *s2);
    }
    s1++;
    s2++;
  }
    // 如果两个字符串都相等或者都到达了末尾，则返回 0
  return (*s1 - *s2);
}

int strncmp(const char *s1, const char *s2, size_t n) {
  while (n > 0 && *s1 && *s2 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) {
        return 0;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

void *memset(void *s, int c, size_t n) {
  unsigned char *tmp = (unsigned char*) s;
    while (n > 0) {
        *tmp = (unsigned char)c;
        tmp++;
        n--;
    }
    return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  unsigned char *tmp = dst;
    const unsigned char *s = src;
    if (tmp < s || tmp >= s + n) {
        while (n > 0) {
            *tmp++ = *s++;
            n--;
        }
    } else {
        tmp += n;
        s += n;
        while (n > 0) {
            *--tmp = *--s;
            n--;
        }
    }
    return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  unsigned char *dst = out;
    const unsigned char *src = in;
    while (n > 0) {
        *dst++ = *src++;
        n--;
    }
    return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const unsigned char *p1 = s1;
    const unsigned char *p2 = s2;
    while (n > 0) {
        if (*p1!= *p2) {
            return (int)(*p1) - (int)(*p2);
        }
        p1++;
        p2++;
        n--;
    }
    return 0;
}

#endif

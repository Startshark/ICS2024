#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
    // panic("Not implemented");
  size_t len = 0;
  while(*s != '\0'){
    s++;
    len++;
  }
  return len;
}

char *strcpy(char *dst, const char *src) {
  // panic("Not implemented");
  char *head = dst;
  while(*src != '\0'){
    *dst = *src;
    src++;
    dst++;
  }
  *dst = '\0';
  return head;
}

char *strncpy(char *dst, const char *src, size_t n) {
  // panic("Not implemented");
  char* head = dst;
  if(strlen(src) < n){
    head = strcpy(dst, src);
  }
  else{
    while(n){
      *dst = *src;
      src++;
      dst++;
      n--;
    }
    *dst = '\0';
  }
  return head;
}

char *strcat(char *dst, const char *src) {
  // panic("Not implemented");
  char* head = dst;
  while(*dst != '\0') dst++;
  while(*src != '\0'){
    *dst = *src;
    dst++;
    src++;
  }
  *dst = '\0';
  return head;
}

int strcmp(const char *s1, const char *s2) {
  // panic("Not implemented");
  int res = 0;
  while(*s1 != '\0' || *s2 != '\0'){
    if(*s1 == '\0'){
      res = - *s2;
      break;
    }
    else if(*s2 == '\0'){
      res = *s1;
      break;
    }
    else if(*s1 != *s2){
      res = *s1 - *s2;
      break;
    }
    s1++;
    s2++;
  }
  return res;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  // panic("Not implemented");
  int res = 0;
  while((*s1 != '\0' || *s2 != '\0') && n > 0){
    if(*s1 == '\0'){
      res = - *s2;
      break;
    }
    else if(*s2 == '\0'){
      res = *s1;
      break;
    }
    else if(*s1 != *s2){
      res = *s1 - *s2;
      break;
    }
    s1++;
    s2++;
    n--;
  }
  return res;
}

void *memset(void *s, int c, size_t n) {
  // panic("Not implemented");
  unsigned char *p = (unsigned char*)s;
  while(n > 0){
    *p = (unsigned char)c;
    p++;
    n--;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  // panic("Not implemented");
  unsigned char *d = (unsigned char*)dst;
  const unsigned char *s = src;
  if (d < s || d >= s + n) {
        while (n > 0) {
            *d++ = *s++;
            n--;
        }
    } else {
        d += n;
        s += n;
        while (n > 0) {
            *--d = *--s;
            n--;
        }
    }
    return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  // panic("Not implemented");
  unsigned char *dest = out;
  const unsigned char *src = in;

  while(n > 0){
    *dest = *src;
    dest++;
    src++;
    n--;
  }
  
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  // panic("Not implemented");
  const unsigned char *p1 = s1;
  const unsigned char *p2 = s2;

  while(n > 0){
    if(*p1 != *p2){
      return (int)*p1 - (int)*p2;
    }
    p1++;
    p2++;
    n--;
  }
  return 0;
}

#endif

// Included in sh.c for !! functionallity

#pragma once
#include "types.h"
void rmvnline(char *s);
int is_white_space(char c);
int get_first_position(char const *str);
int get_str_len(char const *str);
int get_last_position(char const *str);
int get_trim_len(char const *str);
int strncmp(const char *p, const char *q, size_t n);
char *
strncpy(char *s, const char *t, int n);
void *
memcpy(void *dst, const void *src, size_t n);
char *
strstr(const char *s1, const char *s2);
char *strtrim(char const *str);
char *str_replace(char *orig, char *rep, char *with);

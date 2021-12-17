#ifndef _MY_FUNCTIONS_H
#define _MY_FUNCTIONS_H

#include <stdio.h>

int str_length(char* mystring);
void str_copy(char* source, char* destination);
int str_find(char* needle, char* hasystack);
int get_unnamed_argument(int index, int argc, char **argv, char* result);
int get_named_argument(int index, int argc, char **argv, char* result);
void get_arg_name_and_value(char *arg, int len, char *name, char *val);

#endif

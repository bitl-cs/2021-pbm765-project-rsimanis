#include "pong_args.h"

/* assume that mystring won't be more than 256 characters long */
int str_length(char* mystring)
{
	int i;

	for (i = 0; mystring[i] != '\0'; i++)
		;
	return i;
}

/* assumes that there will be enough space in destination */
void str_copy(char* source, char* destination)
{
	int i;

	for (i = 0; (destination[i] = source[i]) != '\0'; i++)
		;
}

int str_find(char* needle, char* haystack)
{
	int nlen, hlen, i, j, match = 0;

	nlen = str_length(needle);
	hlen = str_length(haystack);

	if (nlen == 0 || hlen == 0)
		return -1;

	for (i = 0; i <= hlen - nlen; i++) {
		match = 1;
		for (j = i; j < i + nlen; j++) {
			if (haystack[j] != needle[j - i]) {
				match = 0;
				break;
			}
		}
		if (match)
			return i;
	}
	return -1;
}

int get_unnamed_argument(int index, int argc, char **argv, char* result)
{
	int i, u;
	
	for (i = 0, u = -1; i < argc; i++) {
		if (str_length(argv[i]) == 2 && argv[i][0] == '-' && argv[i][1] == '-') {
			str_copy("", result);
			return -1;
		}
		if (str_find("=", argv[i]) == -1)	
			u++;
		if (u == index) {
			str_copy(argv[i], result);
			return str_length(argv[i]);
		}
	}
	str_copy("", result);
	return -1;
}

int get_named_argument(int index, int argc, char **argv, char* result)
{
	int i, u;

	for (i = 0, u = -1; i < argc; i++) {
		if (str_length(argv[i]) == 2 && argv[i][0] == '-' && argv[i][1] == '-') {
			str_copy("", result);
			return -1;
		}
		if (str_find("=", argv[i]) != -1)	
			u++;
		if (u == index) {
			str_copy(argv[i], result);
			return str_length(argv[i]);
		}
	}
	str_copy("", result);
	return -1;
}


void get_arg_name_and_value(char *arg, int len, char *name, char *val) {
	int i, j;

	for (i = 0; arg[i] != '='; i++) {
		name[i] = arg[i];
	}
	name[i++] = '\0';
	for (j = 0; (j + i) < len; j++) {
		val[j] = arg[j + i];
	}
	val[j] = '\0';
}

int get_argument_by_name(char* name, int argc, char** argv, char* result){
  int i, len, nlen;
  char tmp[256] = "";
  nlen = str_length(name);
  if(nlen<1) return -1;

  for(i=0;i<argc;i++){
    len = get_named_argument(i, argc, argv, tmp);
    if(len == -1) return -1;
    if(str_find(name, tmp) == 0){
      if(str_find("=",tmp) == nlen){
        str_copy(tmp+nlen+1, result);
        return str_length(result);
      }
    }

  }
  return -1;
}




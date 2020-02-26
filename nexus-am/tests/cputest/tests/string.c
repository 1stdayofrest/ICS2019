#include "trap.h"

char *s[] = {
	"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 
	"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab",
	"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
	", World!\n",
	"Hello, World!\n",
	"#####"
};

char str1[] = "Hello";
char str[20];

int main() {	
	nemu_assert(strcmp( strcat(strcpy(str, str1), s[3]), s[4]) == 0);

	nemu_assert(memcmp(memset(str, '#', 5), s[5], 5) == 0);

	return 0;
}

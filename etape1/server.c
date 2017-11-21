#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {

	if (argc < 3) {
		printf("%s -protocole\n", argv[0]);
	}

	int fd = socket(int domain, int type, int protocole);

	return 0;
}
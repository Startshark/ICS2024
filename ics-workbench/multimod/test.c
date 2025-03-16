#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

uint64_t right(uint64_t a, uint64_t b, uint64_t m) {
	char command[256];
	unsigned long long p = a, q = b, r = m;
	snprintf(command, sizeof(command), "python3 -c 'print((%llu * %llu) %% %llu)'", p, q, r);
	FILE *fp = popen(command, "r");
	assert(fp);
	char buf[64];
	fscanf(fp, "%s", buf);
	//printf("popen() returns: %s\n", buf);
	pclose(fp);
	uint64_t result = strtoull(buf, NULL, 10);
	return result;
}

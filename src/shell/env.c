#include "vendor/data-structures/cds_types.h"
#include "vendor/data-structures/hash_map.h"
#include <stdio.h>
#include <string.h>
#include "shell/env.h"
#include "shell/consts.h"


hashmap_t *envMap;
extern char **environ;

cds_err_t initEnv(void) {
	int i;
	cds_err_t cds_err;
	envMap = createHashMap(0, 0, INITIAL_MAP_SIZE);

	for(i = 0; environ[i] != NULL; i++) {
		char *delimit_ptr, *key, *value;
		char buf[ENVIRON_BUF];

		if(strlen(environ[i]) > ENVIRON_BUF) {
			fputs("Environment variable too long, skipping\n",stderr);
			fprintf(stderr, "  Buffer Size: %d\n", ENVIRON_BUF);
			fprintf(stderr, "  Env Var Size: %zu\n", strlen(environ[i]));
			continue;
		}

		strcpy(buf, environ[i]);
		/* TODO: extract strchr logic to helper fn to use here and in
		 * builtins.c/builtin_export */
		delimit_ptr = strchr(buf, '=');
		if(delimit_ptr == NULL) {
			fprintf(stderr, "strchr in initEnv in env.c: Malformed input\n");
			return -1;
		}
		*delimit_ptr = '\0';
		key = buf;
		value = delimit_ptr + 1;

		cds_err = insertToHashMap(envMap, key, value);
		if(cds_err != CDS_OK) {
			return cds_err;
		}
	}
	
	return CDS_OK;
}

char *getEnv(char *key) {
	char *value;
	value = getFromHashMap(envMap, key);
	return value;
}

cds_err_t setEnv(char *key, char *value) {
	return insertToHashMap(envMap, key, value);
}

cds_err_t removeEnv(char *key) {
	return removeFromHashMap(envMap, key);
}

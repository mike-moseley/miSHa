#include "shell/builtins.h"
#include "shell/consts.h"
#include "shell/env.h"
#include "shell/history.h"
#include "shell/jobs.h"
#include "vendor/data-structures/cds_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

BuiltinCommmands get_builtin_command(char *command) {
	if(strcmp("cd", command) == 0) {
		return BUILTIN_CD;
	} else if(strcmp("pwd", command) == 0) {
		return BUILTIN_PWD;
	} else if(strcmp("export", command) == 0) {
		return BUILTIN_EXPORT;
	} else if(strcmp("unset", command) == 0) {
		return BUILTIN_UNSET;
	} else if(strcmp("history", command) == 0) {
		return BUILTIN_HISTORY;
	} else if(strcmp("exit", command) == 0) {
		return BUILTIN_EXIT;
	} else if(strcmp("jobs", command) == 0) {
		return BUILTIN_JOBS;
	} else {
		return BUILTIN_NOT_FOUND;
	}
}

BuiltinCode builtin_cd(char **argv) {
	char *home;
	home = getEnv("HOME");
	if(argv[1] == NULL || *argv[1] == '\0') {
		if(home != NULL) {
			if(chdir(home) == -1) {
				perror("chdir to home in builtin_cd in builtins.c failed");
				return BUILTIN_FAILED;
			}
		}
	} else if(chdir(argv[1]) == -1) {
		perror("chdir in builtin_cd in builtins.c failed");
		return BUILTIN_FAILED;
	}
	return BUILTIN_OK;
}

BuiltinCode builtin_pwd(void) {
	char buf[BUF_SIZE];

	if(getcwd(buf, BUF_SIZE) == NULL) {
		return BUILTIN_FAILED;
	}

	puts(buf);
	return BUILTIN_OK;
}

BuiltinCode builtin_export(char **argv) {
	char *delimit_ptr, *key, *value;
	cds_err_t err;
	if(argv[1] == NULL || *argv[1] == '\0') {
		puts("Please provide environment variable in this syntax `export "
		     "$KEY=value`");
		return BUILTIN_FAILED;
	}
	delimit_ptr = strchr(argv[1], '=');
	if(delimit_ptr == NULL) {
		fprintf(stderr, "strchr in builtin_export in builtins.c: Possibly "
		                "malformed input\n");
		return BUILTIN_FAILED;
	}
	*delimit_ptr = '\0';
	key = argv[1];
	value = delimit_ptr + 1;
	err = setEnv(key, value);
	if(err != CDS_OK) {
		fprintf(stderr,
		        "setenv in builtin_export in builtins.c: cds_err_t:%d\n", err);
		return BUILTIN_FAILED;
	}
	return BUILTIN_OK;
}

BuiltinCode builtin_unset(char **argv) {
	cds_err_t err;

	if(argv[1] == NULL || *argv[1] == '\0') {
		puts("Please provide environment variable in this syntax `unset KEY`");
		return BUILTIN_FAILED;
	}
	err = removeEnv(argv[1]);
	if(err != CDS_OK) {
		fprintf(stderr,
		        "unsetenv in builtin_unset in builtins.c: cds_err_t:%d\n", err);
		return BUILTIN_FAILED;
	}

	return BUILTIN_OK;
}

void builtin_history(void) { printHistory(); }

void builtin_jobs(void) { printJobs(); }

BuiltinCode handle_builtins(char **argv) {
	char *command;
	BuiltinCommmands builtin;
	command = argv[0];
	builtin = get_builtin_command(command);

	switch(builtin) {
	case BUILTIN_EXIT:
		exit(EXIT_SUCCESS);
	case BUILTIN_CD:
		return builtin_cd(argv);
	case BUILTIN_PWD:
		return builtin_pwd();
	case BUILTIN_EXPORT:
		return builtin_export(argv);
	case BUILTIN_UNSET:
		return builtin_unset(argv);
	case BUILTIN_HISTORY:
		builtin_history();
		return BUILTIN_OK;
	case BUILTIN_JOBS:
		builtin_jobs();
		return BUILTIN_OK;
	case BUILTIN_NOT_FOUND:
		return NOT_BUILTIN;
	default:
		return BUILTIN_FAILED;
	}
}

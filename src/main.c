#include "shell/builtins.h"
#include "shell/consts.h"
#include "shell/env.h"
#include "shell/executor.h"
#include "shell/history.h"
#include "shell/input.h"
#include "shell/jobs.h"
#include "shell/parser.h"
#include "vendor/alloc/arena.h"
#include "vendor/alloc/pool.h"
#include "vendor/data-structures/slice.h"
#include <bits/types/sig_atomic_t.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

arena_t *parser_arena;
pool_t *parser_pool;
slice_t *input;
static volatile pid_t exited_pids[MAX_JOBS];
static volatile sig_atomic_t exited_count = 0;

void cleanupArenas(void) { arenaDestroy(parser_arena); }

void cleanupPools(void) { poolDestroy(parser_pool); }

void sigchld_handler(int sig) {
	pid_t pid;
	(void)sig;

	while((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
		exited_pids[exited_count] = pid;
		exited_count++;
	}
}

int main(void) {
	BuiltinCode builtin_code;
	command_t *cmd;

	struct sigaction sa;
	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	if(sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("Error registering sigaction");
		exit(EXIT_FAILURE);
	}

	arenaCreate((MAX_ARGS * sizeof(char *) + (MAX_ARGS * BUF_SIZE)) *
	                PIPE_DEPTH,
	            &parser_arena);

	atexit(cleanupArenas);
	poolCreate(MAX_ARGS, sizeof(command_t), &parser_pool);
	atexit(cleanupPools);

	initTerminal();
	initEnv();
	initHistory();
	initJobs();

	enableRawMode();
	atexit(restoreTerminal);

	while(1) {
		int i;

		input = createSlice(sizeof(char), 0);
		if(readline_raw(input) == -1) break;

		if(((char *)input->arr)[0] == '\0') continue;

		pushHistory((char *)input->arr);
		cmd = parseCommands((char *)input->arr, parser_pool, parser_arena);
		if(cmd == NULL) continue;
		builtin_code = handle_builtins(cmd->argv);
		if(builtin_code == NOT_BUILTIN) {
			execute(cmd);
		}

		for(i = 0; i < exited_count; i++) {
			pid_t pid = exited_pids[i];
			job_t *job = getJobByPID(pid);
			if (job == NULL) continue;
			removeJob(job);
			printf("Job %d `%s` finished\n", job->jobid, job->cmd_name);
			exited_pids[i] = 0;
		}
		exited_count = 0;

		arenaReset(parser_arena);
		poolReset(parser_pool);
		freeSlice(input, NULL);
	}

	return 0;
}

#include "shell/executor.h"
#include "shell/input.h"
#include "shell/parser.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int jobid;

int execute(command_t *cmd) {
	pid_t pid;
	int status;
	status = 0;
	jobid = 1;

	if(cmd == NULL) return -1;

	if(cmd->next == NULL) {
		int redirect_fd;
		pid = fork();
		switch(pid) {
		case -1:
			perror("\nfork in non-pipe execute in executor.c\n");
			exit(EXIT_FAILURE);
		case 0:
			signal(SIGINT, SIG_DFL);
			if(cmd->background == 0) {
				restoreTerminal();
			}
			if(cmd->redirect_out != NULL) {
				redirect_fd = open(cmd->redirect_out,
				                   O_WRONLY | O_CREAT |
				                       (cmd->append ? O_APPEND : O_TRUNC),
				                   0644);
				if(redirect_fd == -1) {
					perror("Error opening file in execute in executor.c");
					exit(EXIT_FAILURE);
				}
				dup2(redirect_fd, STDOUT_FILENO);
				close(redirect_fd);
			}
			if(cmd->redirect_in != NULL) {
				redirect_fd = open(cmd->redirect_in, O_RDONLY);
				if(redirect_fd == -1) {
					perror("Error opening file in execute in executor.c");
					exit(EXIT_FAILURE);
				}
				dup2(redirect_fd, STDIN_FILENO);
				close(redirect_fd);
			}
			if(cmd->redirect_err != NULL) {
				redirect_fd = open(cmd->redirect_err,
				                   O_WRONLY | O_CREAT |
				                       (cmd->append_err ? O_APPEND : O_TRUNC),
				                   0644);
				if(redirect_fd == -1) {
					perror("Error opening file in execute in executor.c");
					exit(EXIT_FAILURE);
				}
				dup2(redirect_fd, STDERR_FILENO);
				close(redirect_fd);
			}
			execvp(cmd->argv[0], cmd->argv);
			/* Only reachable if execvp fails */
			perror("\nexecvp in non-pipe execute in executor.c\n");
			exit(EXIT_FAILURE);
		default:
			if(cmd->background == 0) {
				int result;
				do {
					result = waitpid(pid, &status, 0);
				} while(result == -1 && errno == EINTR);
			} else {
				printf("[%d] %d\n", jobid, pid);
			}
			enableRawMode();
			return status;
		}
	} else {
		int pipefd[2];
		int lhs_pid;
		int rhs_pid;
		if(pipe(pipefd) == -1) return -1;
		lhs_pid = fork();
		switch(lhs_pid) {
		case -1:
			perror("\nfork in pipe execute");
			exit(EXIT_FAILURE);
		case 0:
			/* Restore SIGINT handling on child */
			signal(SIGINT, SIG_DFL);
			restoreTerminal();
			dup2(pipefd[1], STDOUT_FILENO);
			close(pipefd[0]);
			close(pipefd[1]);
			execvp(cmd->argv[0], cmd->argv);
			/* Only reachable if execvp fails */
			perror("\nexecvp in pipe execute in executor.c\n");
			exit(EXIT_FAILURE);
		default:
			signal(SIGINT, SIG_DFL);
			rhs_pid = fork();
			switch(rhs_pid) {
			case -1:
				perror("\nfork in pipe execute");
				exit(EXIT_FAILURE);
			case 0:
				restoreTerminal();
				dup2(pipefd[0], STDIN_FILENO);
				close(pipefd[0]);
				close(pipefd[1]);
				execute(cmd->next);
				exit(EXIT_SUCCESS);
			default:
				close(pipefd[0]);
				close(pipefd[1]);
				waitpid(lhs_pid, &status, 0);
				waitpid(rhs_pid, &status, 0);
				enableRawMode();
				return status;
			}
		}
	}
}

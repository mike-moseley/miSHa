#ifndef JOBS_H
#define JOBS_H

#include "shell/consts.h"
#include "vendor/data-structures/cds_types.h"
#include <fcntl.h>

typedef enum { JOB_DONE = 0, JOB_RUNNING = 1, JOB_STOPPED = 2 } job_status;

typedef struct job {
	int jobid;
	pid_t pid;
	job_status status;
	char cmd_name[BUF_SIZE];
} job_t;

cds_err_t initJobs(void);
cds_err_t addJob(job_t *job);
cds_err_t removeJob(job_t *job);
job_t *getJobByPID(pid_t pid);
job_t *getJobByJobID(int jobid);
cds_err_t updateJobStatus(pid_t pid, job_status status);
void printJobs(void);

#endif

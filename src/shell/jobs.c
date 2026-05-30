#include "shell/jobs.h"
#include "vendor/data-structures/cds_types.h"
#include "vendor/data-structures/slice.h"
#include <stdio.h>

static slice_t *jobs;

static int job_counter;

cds_err_t initJobs(void) {
	jobs = createSlice(sizeof(job_t), 0);
	if(jobs == NULL) return CDS_ERR_OOM;
	job_counter = 1;
	return CDS_OK;
}

cds_err_t addJob(job_t *job) {
	job->jobid = job_counter;
	job_counter++;
	return appendSlice(jobs, job);
}

cds_err_t removeJob(job_t *job) {
	size_t i;
	job_t *tmp_job;

	for(i = 0; i < jobs->len; i++) {
		tmp_job = (job_t *)getFromSlice(jobs, i);
		if(job->pid == tmp_job->pid) {
			return sliceRemoveElement(jobs, i, NULL);
		}
	}

	return CDS_ERR_NOT_FOUND;
}

job_t *getJobByPID(pid_t pid) {
	size_t i;
	job_t *job;

	for(i = 0; i < jobs->len; i++) {
		job = (job_t *)getFromSlice(jobs, i);
		if(job->pid == pid) return job;
	}

	return NULL;
}

job_t *getJobByJobID(int job_id) {
	size_t i;
	job_t *job;

	for(i = 0; i < jobs->len; i++) {
		job = (job_t *)getFromSlice(jobs, i);
		if(job->jobid == job_id) return job;
	}

	return NULL;
}

cds_err_t updateJobStatus(pid_t pid, job_status status) {
	job_t *job;
	job = getJobByPID(pid);

	if(job == NULL) return CDS_ERR_NOT_FOUND;

	job->status = status;
	return CDS_OK;
}

void printJobs(void) {
	size_t i;
	job_t *job;

	for(i = 0; i < jobs->len; i++) {
		char *status_str = "";
		job = getFromSlice(jobs, i);

		if(job == NULL || job->status == JOB_DONE) continue;

		switch(job->status) {
		case JOB_RUNNING:
			status_str = "RUNNING";
			break;
		case JOB_STOPPED:
			status_str = "STOPPED";
			break;
		default:
			break;
		}

		printf("Job: %s\n", job->cmd_name);
		printf("  Job ID: %d PID: %d Status: %s\n", job->jobid, job->pid,
		       status_str);
	}
}

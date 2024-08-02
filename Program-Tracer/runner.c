#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/resource.h>

#include <bpf/libbpf.h>
#include <bpf/bpf.h>

#include "program_tracer.skel.h"
#include "event.h"

static int print_execs(int fd) {
	int err; 
	struct event ev;
	pid_t lookup_key = 0;
	int next_key;

	while (!bpf_map_get_next_key(fd, &lookup_key, &next_key)) {
		err = bpf_map_lookup_elem(fd,&next_key, &ev);
		if (err < 0) {
			fprintf(stderr, "Failed to lookup execution in BPF MAP - %d\n", err);
			return -1;
		}

		printf("path=%s, name=%s, uid=%u, pid=%u\n", ev.fpath, ev.comm, ev.uid, ev.pid);

		err = bpf_map_delete_elem(fd, &next_key);
		if (err < 0) {
			fprintf(stderr, "Failed to delete execution from BPF MAP - %d\n", err);
			return -1;
		}

		lookup_key = next_key;
	}

	return 0;
	
}

int main(void)
{
	struct program_tracer_bpf *obj;
	int err = 0;
	int fd;

	struct rlimit rlim = {
		.rlim_cur = 512UL << 20,
		.rlim_max = 512UL << 20,
	};

	err = setrlimit(RLIMIT_MEMLOCK, &rlim);
	if (err) {
		fprintf(stderr, "Failed to change rlimit\n");
		return 1;
	}


	obj = program_tracer_bpf__open();
	if (!obj) {
		fprintf(stderr, "Failed to open BPF object\n");
		return 1;
	}

	err = program_tracer_bpf__load(obj);
	if (err) {
		fprintf(stderr, "Failed to load BPF object %d\n", err);
		goto cleanup;
	}

	err = program_tracer_bpf__attach(obj);
	if (err) {
		fprintf(stderr, "Failed to attach BPF program\n");
		goto cleanup;
	}

	fd = bpf_map__fd(obj->maps.execs);

	while (1) {
		print_execs(fd);
		fd = bpf_map__fd(obj->maps.execs);
	}

cleanup:
	program_tracer_bpf__destroy(obj);
	return err != 0;
}

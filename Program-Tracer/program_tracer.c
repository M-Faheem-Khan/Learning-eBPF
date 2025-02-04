#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include "event.h"

struct {
	__uint(type, BPF_MAP_TYPE_HASH);
	__uint(max_entries, 128);
	__type(key, pid_t);
	__type(value, struct event);
} execs SEC(".maps");

SEC("tracepoint/syscalls/sys_enter_execve")
int program_tracer(struct trace_event_raw_sys_enter *ctx) {
    
    struct event *event;
    pid_t pid;
    u64 id;

    uid_t uid = (u32) bpf_get_current_pid_tgid();
    id = bpf_get_current_pid_tgid();
	pid = (pid_t)id;

	if (bpf_map_update_elem(&execs, &pid, &((struct event){}), 1)) {
		return 0;
	}

	event = bpf_map_lookup_elem(&execs, &pid);
	if (!event) {
		return 0;
	}

    char *tmp;
    bpf_probe_read(&tmp, sizeof(tmp), &ctx->args[0]);
    bpf_probe_read_str(event->fpath, sizeof(event->fpath), tmp);

    event->pid = pid;
	event->uid = uid;
	bpf_get_current_comm(&event->comm, sizeof(event->comm));

    return 0;
}


char LICENSE[] SEC("license") = "GPL";

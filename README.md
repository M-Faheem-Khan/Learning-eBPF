# Learning eBPF

## Table of Contents
- How to run eBPF code?
	- Manually Register eBPF bytecode in Kernel
	- Automate eBPF bytecode Kernel registration & clean up
- Hello World
- Process Monitor
	- Process Details
	- System Calls
- Resources

# How to run eBPF code?
## Manually Register eBPF bytecode in Kernel

All eBPF code must be registered with the kernel. eBPF programs have their own filesystem at `/sys/fs/bpf/` and all registered code is stored here.

`bpftool prog load HelloWorld.o /sys/fs/bpf/HelloWorld type raw_tracepoint`

## Automate eBPF bytecode Kernel registration & clean up

Once the eBPF code is converted into bytecode aka `.o` file. We can generate a skeleton file using `bpftool` which will be loaded/executed by our `runner`. We are going to write some boilerplate code which increases the `rlimit`, loads the ebpf code, attach it to the kernel & cleans up after the `runner` has stopped running. The following is the runner code.

```C
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/resource.h>

#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include "hello.skel.h" // <-- Your eBPF bytecode skeleton file

// Reading the kernel trace to see the logs
void read_trace_pipe(void)
{
	int trace_fd;

	trace_fd = open("/sys/kernel/debug/tracing/trace_pipe", O_RDONLY, 0);
	if (trace_fd < 0)
		return;

	while (1) {
		static char buf[4096];
		ssize_t sz;

		sz = read(trace_fd, buf, sizeof(buf) - 1);
		if (sz > 0) {
			buf[sz] = 0;
			puts(buf);
		}
	}
}

int main(void)
{
	struct hello_bpf *obj;
	int err = 0;

	struct rlimit rlim = {
		.rlim_cur = 512UL << 20,
		.rlim_max = 512UL << 20,
	};

	err = setrlimit(RLIMIT_MEMLOCK, &rlim);
	if (err) {
		fprintf(stderr, "Failed to change rlimit\n");
		return 1;
	}


	obj = hello_bpf__open();
	if (!obj) {
		fprintf(stderr, "Failed to open BPF object\n");
		return 1;
	}

	err = hello_bpf__load(obj);
	if (err) {
		fprintf(stderr, "Failed to load BPF object %d\n", err);
		goto cleanup;
	}

	err = hello_bpf__attach(obj);
	if (err) {
		fprintf(stderr, "Failed to attach eBPF program\n");
		goto cleanup;
	}

	read_trace_pipe();

cleanup:
	hello_bpf__destroy(obj);
	return err != 0;
}
```

**Pre-Build**
We need a static copy of `libbpf`.
```sh
# Clone Libbpf repoistory
git clone https://github.com/libbpf/libbpf && cd libbpf/src/

# Static build of libbpf
make BUILD_STATIC_ONLY=1 OBJDIR=../build/libbpf DESTDIR=../build INCLUDEDIR=
 LIBDIR= UAPIDIR= install
```

**Build**
```sh
# vmlinux static kernel file
bpftool btf dump file /sys/kernel/btf/vmlinux format c > vmlinux.h

# Generate bytecode
clang -g -O2 -target bpf -I . -c hello.bpf.c -o hello.bpf.o

# Generate Skeleton from the bytecode
bpftool gen skeleton hello.bpf.o > hello.skel.h

# Generate runner bytecode
clang -g -O2 -Wall -I . -c runner.c -o runner.o

# Build the runner
# Update the libbpf build path
clang -Wall -O2 -g hello.o libbpf/build/libbpf.a -lelf -lz -o runner

# Run the runner
sudo ./runner
```
# Hello World

The following code prints Hello World to the logs when ever an new program is executed.

```C
#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

// execve - syscall is made whenever a program is executed.
SEC("tracepoint/syscalls/sys_enter_execve")
int tracepoint__syscalls__sys_enter_execve(strcut trace_event_raw_sys_enter *ctx) {
	bpf_printk("Hello World\n");
	return 0;
}

// Define License - Only GPL is accepted.
char LICENSE[] SEC("license") = "GPL";

```

**Build**
Using `clang` to generate the ebpf bytecode.
`clang -target bpf -Wall -O2 -g -c HelloWorld.bpf.c -o HelloWorld.o`

**View compiled bytecode**
`llvm-objdump -d -r -S --print-imm-hex HelloWorld.o`

**Load Bytecode into Kernel**
`bpftool prog load HelloWorld.o /sys/fs/bpf/HelloWorld type raw_tracepoint`

**Testing**
`bpftool prog run pinned /sys/fs/bpf/HelloWorld repeat 0`

**View Logs**
`sudo cat /sys/kernel/debug/tracing/trace`
# Process Monitor
**Goal:** Write a eBPF program that lists the `path`, `pid`, `args` of any executed program.


**Syscall:** `execve` [manpage](https://man7.org/linux/man-pages/man2/execve.2.html)
*executes the program referred to by _pathname_.  This causes the program that is currently being run by the calling process to be replaced with a new program, with newly initialized stack, heap, and (initialized and uninitialized) data segments.*


Using the `execve` syscall to monitor for all newly executed program and use it to gather and display information about the program.
## Process Details

## Systems Calls



# Resources
- [https://github.com/sartura/ebpf-hello-world](https://github.com/sartura/ebpf-hello-world)
- [https://github.com/lizrice/learning-ebpf](https://github.com/lizrice/learning-ebpf)

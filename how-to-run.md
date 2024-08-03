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

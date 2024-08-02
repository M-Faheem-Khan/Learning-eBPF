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

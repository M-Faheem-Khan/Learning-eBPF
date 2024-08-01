#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

SEC("hello")
int hello(void *ctx) {
  bpf_printk("Hello World\n");
  return 0;
}

char LICENSE[] SEC("license") = "GPL";

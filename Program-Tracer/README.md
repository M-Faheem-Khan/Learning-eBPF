# Program Tracer  

**[Work In Progress]**

**Goal:** Write a eBPF program that lists the `path`, `pid`, `args` of any executed program.  

**Syscall:** `execve` [manpage](https://man7.org/linux/man-pages/man2/execve.2.html)
*executes the program referred to by _pathname_.  This causes the program that is currently being run by the calling process to be replaced with a new program, with newly initialized stack, heap, and (initialized and uninitialized) data segments.*  

Using the `execve` syscall to monitor for all newly executed program & gather details and display information about the program.

```
PID  UID  FILE    ARGS
2132 1000 /bin/ls -la --color=auto
``

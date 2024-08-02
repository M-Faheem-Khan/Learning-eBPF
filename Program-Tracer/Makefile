PROGRAM_NAME=program_tracer

build-ebpf:
	clang -g -O2 -target bpf -I . -c $(PROGRAM_NAME).bpf.c -o $(PROGRAM_NAME).bpf.o
	bpftool gen skeleton $(PROGRAM_NAME).bpf.o > $(PROGRAM_NAME).skel.h

build-runner: runner.c
	sed -i 's/PROGRAM_NAME/$(PROGRAM_NAME)/g' runner.c
	clang -g -O2 -Wall -I . -c runner.c -o runner.o
	clang -Wall -O2 -g runner.o ~/Desktop/libbpf/build/libbpf/libbpf.a -lelf -lz -o runner

clean:
	rm runner *.o

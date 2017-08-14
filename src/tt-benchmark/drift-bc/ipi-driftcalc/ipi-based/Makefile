obj-m += epsilon-table-ipi.o

Q	= @

all: CPUSEQ
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

CPUSEQ:
	@echo "generating sequential cpu list"
	$(Q)./gen-cpuseq

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

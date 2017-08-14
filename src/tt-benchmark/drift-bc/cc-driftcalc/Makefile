obj-m += drift-calc.o

Q	= @

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

CPUSEQ:
	@echo "generating sequential cpu list"
	$(Q)./gen-cpuseq

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

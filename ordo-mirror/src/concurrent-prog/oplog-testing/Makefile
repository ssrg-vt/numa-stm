# Quiet compile, unset for verbose output
Q	= @

O	= binaries
CC	= gcc

CWARNS	= -Wformat=2 -Wextra -Wmissing-noreturn -Wwrite-strings -Wshadow \
	  -Wno-unused-parameter -Wmissing-format-attribute -fno-builtin \
	  -Wswitch-default -Wmissing-prototypes \
	  -Wmissing-declarations
CFLAGS  = $(DEFS) -Wall -Werror -fno-strict-aliasing $(CWARNS) \
	  -g -O3 -D_GNU_SOURCE

LDFLAGS = -lrt -lm

LIBS	= bench.c rbtree.c interval_tree.c
DEPS	= compiler.h bench.h config.h interval_tree_generic.h interval_tree.h \
	  rbtree_augmented.h rbtree.h

BINS	= spinlockrefcount xchgrefcount oplogrefcount rwspinlockrmap oplogrmap
BINS	:= $(addprefix $(O)/, $(BINS))


all: CPUSEQ $(BINS)

CPUSEQ:
	@echo "generating sequential cpu"
	$(Q)./gen_cpuseq.py

$(O)/%: %.c $(DEPS) $(CPUSEQ)
	@mkdir -p $(@D)
	@echo "CC	$@"
	$(Q)$(CC) $(CFLAGS) -o $@ $< $(LIBS) -lpthread $(LDFLAGS)

clean:
	@echo "CLEAN"
	$(Q)rm -rf $(O) && rm -rf cpuseq.h

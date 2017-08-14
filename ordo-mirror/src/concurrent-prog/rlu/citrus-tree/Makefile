
EER_PRCU=	eer_prcu.c prcu.h
DEER_PRCU=	deer_prcu.c prcu.h
D_PRCU=		d_prcu.c prcu.h

CFLAGS += -D_REENTRANT
CFLAGS += -Wall -Winline
CFLAGS += -mrtm

ifdef DEBUG
	CFLAGS += -O0 -g
else
	CFLAGS += -DNDEBUG
	CFLAGS += -O3
endif

IS_RLU = -DIS_RLU
IS_RCU = -DIS_RCU

LDFLAGS += -lpthread
LDFLAGS += -ljemalloc

BINS = bench_rcu bench_rlu

.PHONY: all clean

all: $(BINS)

bench_rcu:
	$(CC) -DIS_RCU $(CFLAGS) bench.c citrus.c rlu.c d_prcu.c $(LDFLAGS) -o bench_rcu

bench_rlu:
	$(CC) -DIS_RLU $(CFLAGS) bench.c citrus.c rlu.c citrus-rlu.c d_prcu.c $(LDFLAGS) -o bench_rlu

libeerprcu.so: $(EER_PRCU)
	gcc -shared -O3 -fPIC -o $@ -I. $(EER_PRCU) -lpthread

libdeerprcu.so: $(DEER_PRCU)
	gcc -shared -O3 -fPIC -DRCU_HAS_RANGE -o $@ -I. $(DEER_PRCU) -lpthread

libdprcu.so: $(D_PRCU)
	gcc -shared -O3 -fPIC -DRCU_HAS_RANGE -o $@ -I. $(D_PRCU) -lpthread

clean:
	rm -rf libeerprcu.so libdeerprcu.so libdprcu.so *.o $(BINS)

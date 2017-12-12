# ==============================================================================
#
# Defines.common.mk
#
# ==============================================================================
#CFLAGS += -DHUGE_READSET
CFLAGS += -DLIST_NO_DUPLICATES
CFLAGS += -DCHUNK_STEP1=12

PROG := genome$(EXE_P)

SRCS += \
	gene.c \
	genome.c \
	segments.c \
	sequencer.c \
	table.c \
	$(LIB)/bitmap.c \
	$(LIB)/hash.c \
	$(LIB)/hashtable.c \
	$(LIB)/pair.c \
	$(LIB)/random.c \
	$(LIB)/list.c \
	$(LIB)/mt19937ar.c \
	$(LIB)/thread.c \
	$(LIB)/vector.c \
	$(LIB)/../tm/WriteSet.c \
	$(LIB)/../tm/tm_thread.c \
#
OBJS := ${SRCS:.c=.o}


# ==============================================================================
#
# End of Defines.common.mk
#
# ==============================================================================

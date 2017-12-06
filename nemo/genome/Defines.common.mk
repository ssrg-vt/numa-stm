# ==============================================================================
#
# Defines.common.mk
#
# ==============================================================================

CFLAGS += -DSUPER_GL_LIMIT=1 
CFLAGS += -DSUPER_GL_LIMIT2=1
CFLAGS += -DRETRY_SHORT=5
CFLAGS += -DBLOOM_SIZE=1024

CFLAGS += -DLIST_NO_DUPLICATES
CFLAGS += -DCHUNK_STEP1=12

PROG := genome

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
	$(LIB)/htm.c \
#
OBJS := ${SRCS:.c=.o}


# ==============================================================================
#
# End of Defines.common.mk
#
# ==============================================================================

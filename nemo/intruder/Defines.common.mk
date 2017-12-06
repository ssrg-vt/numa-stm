# ==============================================================================
#
# Defines.common.mk
#
# ==============================================================================
CFLAGS += -DSUPER_GL_LIMIT=1 
CFLAGS += -DSUPER_GL_LIMIT2=1
CFLAGS += -DRETRY_SHORT=5
CFLAGS += -DBLOOM_SIZE=1024

PROG := intruder

SRCS += \
	decoder.c \
	detector.c \
	dictionary.c \
	intruder.c \
	packet.c \
	preprocessor.c \
	stream.c \
	$(LIB)/list.c \
	$(LIB)/mt19937ar.c \
	$(LIB)/pair.c \
	$(LIB)/queue.c \
	$(LIB)/random.c \
	$(LIB)/rbtree.c \
	$(LIB)/thread.c \
	$(LIB)/vector.c \
	$(LIB)/htm.c \
#
OBJS := ${SRCS:.c=.o}

CFLAGS += -DMAP_USE_RBTREE


# ==============================================================================
#
# End of Defines.common.mk
#
# ==============================================================================

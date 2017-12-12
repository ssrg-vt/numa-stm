# ==============================================================================
#
# Defines.common.mk
#
# ==============================================================================
PROG := intruder$(EXE_P)

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
	$(LIB)/../tm/WriteSet.c \
	$(LIB)/../tm/tm_thread.c \
#
OBJS := ${SRCS:.c=.o}

CFLAGS += -DMAP_USE_RBTREE


# ==============================================================================
#
# End of Defines.common.mk
#
# ==============================================================================

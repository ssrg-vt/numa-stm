# ==============================================================================
#
# Defines.common.mk
#
# ==============================================================================
CFLAGS += -DSUPER_GL_LIMIT=100 
CFLAGS += -DSUPER_GL_LIMIT2=100
CFLAGS += -DRETRY_SHORT=5
CFLAGS += -DBLOOM_SIZE=6114

LIBS += -lm

PROG := labyrinth

SRCS += \
	coordinate.c \
	grid.c \
	labyrinth.c \
	maze.c \
	router.c \
	$(LIB)/list.c \
	$(LIB)/mt19937ar.c \
	$(LIB)/pair.c \
	$(LIB)/queue.c \
	$(LIB)/random.c \
	$(LIB)/thread.c \
	$(LIB)/vector.c \
	$(LIB)/htm.c \
	$(LIB)/stm/SpookyV2.c \
#
OBJS := ${SRCS:.c=.o}

CFLAGS += -DUSE_EARLY_RELEASE


# ==============================================================================
#
# End of Defines.common.mk
#
# ==============================================================================

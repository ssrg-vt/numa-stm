# ==============================================================================
#
# Defines.common.mk
#
# ==============================================================================
CFLAGS += -DSUPER_GL_LIMIT=1 
CFLAGS += -DSUPER_GL_LIMIT2=1
CFLAGS += -DRETRY_SHORT=5
CFLAGS += -DBLOOM_SIZE=1024

PROG := redblacktree

SRCS += \
	redblacktree.c \
	rbtree.c \
	$(LIB)/mt19937ar.c \
	$(LIB)/random.c \
	$(LIB)/thread.c \
	$(LIB)/htm.c \
#
OBJS := ${SRCS:.c=.o}


# ==============================================================================
#
# End of Defines.common.mk
#
# ==============================================================================

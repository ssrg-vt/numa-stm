# ==============================================================================
#
# Defines.common.mk
#
# ==============================================================================
CFLAGS += -DSUPER_GL_LIMIT=1 
CFLAGS += -DSUPER_GL_LIMIT2=1
CFLAGS += -DRETRY_SHORT=5
CFLAGS += -DBLOOM_SIZE=1024

PROG := ssca2

SRCS += \
	alg_radix_smp.c \
	computeGraph.c \
	createPartition.c \
	cutClusters.c \
	findSubGraphs.c \
	genScalData.c \
	getStartLists.c \
	getUserParameters.c \
	globals.c \
	ssca2.c \
	$(LIB)/mt19937ar.c \
	$(LIB)/random.c \
	$(LIB)/thread.c \
	$(LIB)/htm.c \
#
OBJS := ${SRCS:.c=.o}

#CFLAGS += -DUSE_PARALLEL_DATA_GENERATION
#CFLAGS += -DWRITE_RESULT_FILES
CFLAGS += -DENABLE_KERNEL1
#CFLAGS += -DENABLE_KERNEL2 -DENABLE_KERNEL3
#CFLAGS += -DENABLE_KERNEL4


# ==============================================================================
#
# End of Defines.common.mk
#
# ==============================================================================

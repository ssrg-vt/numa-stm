# ==============================================================================
#
# Defines.common.mk
#
# ==============================================================================


CC       := g++
CFLAGS   += -g -w -pthread -mrtm -fpermissive
CFLAGS   += -O2
CFLAGS   += -I$(LIB)
CFLAGS	 += -DSTATISTICS

CPP      := g++
CPPFLAGS += $(CFLAGS)
LD       := g++
LIBS     += -lpthread -lnuma

# Remove these files when doing clean
OUTPUT +=

LIB := ../lib


# ==============================================================================
#
# End of Defines.common.mk
#
# ==============================================================================

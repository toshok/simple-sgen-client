all : test-sgen

MONO_DIR?=mono

EGLIB_SOURCES = \
	$(MONO_DIR)/eglib/src/gdate-unix.c \
	$(MONO_DIR)/eglib/src/gerror.c \
	$(MONO_DIR)/eglib/src/glist.c \
	$(MONO_DIR)/eglib/src/goutput.c \
	$(MONO_DIR)/eglib/src/gmem.c \
	$(MONO_DIR)/eglib/src/gmisc-unix.c \
	$(MONO_DIR)/eglib/src/ghashtable.c \
	$(MONO_DIR)/eglib/src/gstr.c \
	$(MONO_DIR)/eglib/src/vasprintf.c

MONO_SOURCES = \
	$(MONO_DIR)/mono/metadata/sgen-gc.c		\
	$(MONO_DIR)/mono/metadata/sgen-alloc.c	\
	$(MONO_DIR)/mono/metadata/sgen-nursery-allocator.c	\
	$(MONO_DIR)/mono/metadata/sgen-simple-nursery.c	\
	$(MONO_DIR)/mono/metadata/sgen-split-nursery.c	\
	$(MONO_DIR)/mono/metadata/sgen-marksweep.c	\
	$(MONO_DIR)/mono/metadata/sgen-los.c	\
	$(MONO_DIR)/mono/metadata/sgen-cardtable.c	\
	$(MONO_DIR)/mono/metadata/sgen-descriptor.c	\
	$(MONO_DIR)/mono/metadata/sgen-fin-weak-hash.c	\
	$(MONO_DIR)/mono/metadata/sgen-gray.c	\
	$(MONO_DIR)/mono/metadata/sgen-pointer-queue.c	\
	$(MONO_DIR)/mono/metadata/sgen-internal.c	\
	$(MONO_DIR)/mono/metadata/sgen-hash-table.c	\
	$(MONO_DIR)/mono/metadata/sgen-pinning.c	\
	$(MONO_DIR)/mono/metadata/sgen-pinning-stats.c	\
	$(MONO_DIR)/mono/metadata/sgen-protocol.c	\
	$(MONO_DIR)/mono/metadata/sgen-workers.c	\
	$(MONO_DIR)/mono/metadata/sgen-memory-governor.c	\
	$(MONO_DIR)/mono/metadata/sgen-debug.c	\
	$(MONO_DIR)/mono/metadata/gc-parse.c	\
	$(MONO_DIR)/mono/metadata/gc-memfuncs.c	\
	$(MONO_DIR)/mono/metadata/gc-stats.c	\
	$(MONO_DIR)/mono/utils/mono-mutex.c	\
	$(MONO_DIR)/mono/utils/monobitset.c	\
	$(MONO_DIR)/mono/utils/lock-free-queue.c	\
	$(MONO_DIR)/mono/utils/lock-free-alloc.c	\
	$(MONO_DIR)/mono/utils/lock-free-array-queue.c	\
	$(MONO_DIR)/mono/utils/hazard-pointer.c

SGEN_SOURCES= $(EGLIB_SOURCES) $(MONO_SOURCES)

INCLUDES=				\
	-I.				\
	-I./$(MONO_DIR)/		\
	-I./$(MONO_DIR)/eglib/src

CFLAGS=-std=gnu99 -Wall -DHAVE_SGEN_GC -DSGEN_CLIENT_HEADER=\"simple-client.h\" -DSGEN_WITHOUT_MONO -O0 -g $(INCLUDES)
LDFLAGS=-lpthread -lm


all: test-sgen

libsgen_SOURCES=$(SGEN_SOURCES)
libsgen_OBJECTS=$(libsgen_SOURCES:%.c=%.o)

libsgen.a: $(libsgen_OBJECTS)
	ar cru $@ $(libsgen_OBJECTS)

test_sgen_SOURCES=test-sgen.c simple-client.c
test_sgen_OBJECTS=$(test_sgen_SOURCES:%.c=%.o)

test-sgen: $(test_sgen_OBJECTS) Makefile libsgen.a
	$(CC) -o $@ $(test_sgen_OBJECTS) libsgen.a $(LDFLAGS)

simple-client.o: simple-client.h

clean:
	rm -f test-sgen libsgen.a
	$(MAKE) -C mono/ clean

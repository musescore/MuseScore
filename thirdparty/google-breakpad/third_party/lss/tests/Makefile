# Copyright 2018, Google Inc.  All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

top_srcdir ?= ..

DEF_FLAGS = -g -pipe
DEF_WFLAGS = -Wall
CFLAGS ?= $(DEF_FLAGS)
CXXFLAGS ?= $(DEF_FLAGS)
CFLAGS += $(DEF_WFLAGS) -Wstrict-prototypes
CXXFLAGS += $(DEF_WFLAGS)
CPPFLAGS += -I$(top_srcdir)
# We use static linking here so that if people run through qemu/etc... by hand,
# it's a lot easier to run/debug.  Same for strace output.
LDFLAGS += -static

TESTS = \
	fallocate \
	sigaction \
	getrandom \
	sigtimedwait \
	unlink \

all: check

%_test: %.c test_skel.h $(top_srcdir)/linux_syscall_support.h
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $<

%_test: %.cc test_skel.h $(top_srcdir)/linux_syscall_support.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $<

%_run: %_test
	@t=$(@:_run=_test); \
	echo "./$$t"; \
	if ! env -i ./$$t; then \
		env -i strace -f -v ./$$t; \
		echo "TRY: gdb -q -ex r -ex bt ./$$t"; \
		exit 1; \
	fi

ALL_TEST_TARGETS = $(TESTS:=_test)
compile_tests: $(ALL_TEST_TARGETS)

ALL_RUN_TARGETS = $(TESTS:=_run)
check: $(ALL_RUN_TARGETS)

# The "tempfile" targets are the names we use with temp files.
# Clean them out in case some tests crashed in the middle.
clean:
	rm -f *~ *.o tempfile.* a.out core $(ALL_TEST_TARGETS)

.SUFFIXES:
.PHONY: all check clean compile_tests
.SECONDARY: $(ALL_TEST_TARGETS)

# Try to cross-compile the tests for all our supported arches.  We test with
# both gcc and clang.  We don't support execution (yet?), but just compiling
# & linking helps catch common bugs.
.PHONY: cross compile_cross
cross_compile:
	@echo "Running: $(MAKE) $@ CC='$(CC)' CXX='$(CXX)'"; \
	if (echo '#include <stdio.h>' | $(CC) -x c -c -o /dev/null -) 2>/dev/null; then \
		$(MAKE) -s clean; \
		$(MAKE) -k --no-print-directory compile_tests; \
	else \
		echo "Skipping $(CC) test: not installed"; \
	fi; \
	echo

# The names here are a best effort.  Not easy to probe for.
cross:
	@for cc in \
		"x86_64-pc-linux-gnu-gcc" \
		"i686-pc-linux-gnu-gcc" \
		"x86_64-pc-linux-gnu-gcc -mx32" \
		"armv7a-unknown-linux-gnueabi-gcc -marm -mhard-float" \
		"armv7a-unknown-linux-gnueabi-gcc -mthumb -mhard-float" \
		"powerpc-unknown-linux-gnu-gcc" \
		"aarch64-unknown-linux-gnu-gcc" \
		"mips64-unknown-linux-gnu-gcc -mabi=64" \
		"mips64-unknown-linux-gnu-gcc -mabi=32" \
		"mips64-unknown-linux-gnu-gcc -mabi=n32" \
		"s390-ibm-linux-gnu-gcc" \
		"s390x-ibm-linux-gnu-gcc" \
	; do \
		cxx=`echo "$$cc" | sed 's:-gcc:-g++:'`; \
		$(MAKE) --no-print-directory CC="$$cc" CXX="$$cxx" cross_compile; \
		\
		sysroot=`$$cc --print-sysroot 2>/dev/null`; \
		gccdir=`$$cc -print-file-name=libgcc.a 2>/dev/null`; \
		gccdir=`dirname "$$gccdir"`; \
		: Skip building for clang for mips/o32 and s390/31-bit until it works.; \
		case $$cc in \
			mips64*-mabi=32) continue;; \
			s390-*) continue;; \
		esac; \
		set -- $$cc; \
		tuple=$${1%-gcc}; \
		shift; \
		cc="clang -target $$tuple $$*"; \
		: Assume the build system is x86_64 based, so ignore the sysroot.; \
		case $$tuple in \
			x86_64*) ;; \
			*) cc="$$cc --sysroot $$sysroot -B$$gccdir -L$$gccdir";; \
		esac; \
		cxx=`echo "$$cc" | sed 's:^clang:clang++:'`; \
		$(MAKE) --no-print-directory CC="$$cc" CXX="$$cxx" cross_compile; \
	done

# Writed by yijian (eyjian@gmail.com)
# Redis Cluster Client Makefile
#
# Dependencies are stored in the Makefile.dep file. To rebuild this file
# Just use 'make dep > Makefile.dep', but this is only needed by developers.

LIBNAME=libr3c
CMD=tests/r3c_cmd
TEST=tests/r3c_test
STRESS=tests/r3c_stress
ROBUST=tests/r3c_robust
STREAM=tests/r3c_stream
EXTENSION=tests/redis_command_extension.so

HIREDIS?=/usr/local/hiredis
PREFIX?=/usr/local
INCLUDE_PATH?=include/r3c
LIBRARY_PATH?=lib

# redis-cluster configuration used for testing
REDIS_CLUSTER_NODES?=192.168.1.31:6379,192.168.1.31:6380

INSTALL_BIN=$(PREFIX)/bin
INSTALL_INCLUDE_PATH= $(PREFIX)/$(INCLUDE_PATH)
INSTALL_LIBRARY_PATH= $(PREFIX)/$(LIBRARY_PATH)
INSTALL?=install

CPLUSPLUSONEONE=$(shell gcc --version|awk -F[\ .]+ '/GCC/{if($$3>=4&&$$4>=7) printf("-std=c++11");}')
#OPTIMIZATION?=-O2
DEBUG?=-g -ggdb $(CPLUSPLUSONEONE) -DSLEEP_USE_POLL # -DR3C_TEST
WARNINGS=-Wall -W -Wwrite-strings -Wno-missing-field-initializers
REAL_CPPFLAGS=$(CPPFLAGS) -I. -I$(HIREDIS)/include -DSLEEP_USE_POLL=1 -D__STDC_FORMAT_MACROS=1 -D__STDC_CONSTANT_MACROS -fstrict-aliasing -fPIC  -pthread $(DEBUG) $(OPTIMIZATION) $(WARNINGS)
REAL_LDFLAGS=$(LDFLAGS) -fPIC -pthread $(HIREDIS)/lib/libhiredis.a

CXX:=$(shell sh -c 'type $(CXX) >/dev/null 2>/dev/null && echo $(CXX) || echo g++')
STLIBSUFFIX=a
STLIBNAME=$(LIBNAME).$(STLIBSUFFIX)
STLIB_MAKE_CMD=ar rcs

all: $(HIREDIS) $(STLIBNAME) $(CMD) $(TEST) $(STRESS) $(ROBUST) $(STREAM) $(EXTENSION)

# Deps (use make dep to generate this)
sha1.o: sha1.cpp
utils.o: utils.h utils.cpp
r3c.o: r3c.cpp r3c.h r3c.cpp utils.h utils.cpp
tests/r3c_cmd.o: tests/r3c_cmd.cpp r3c.h r3c.cpp utils.h utils.cpp
tests/r3c_test.o: tests/r3c_test.cpp r3c.h r3c.cpp utils.h utils.cpp
tests/r3c_stress.o: tests/r3c_stress.cpp r3c.h r3c.cpp utils.cpp
tests/r3c_robust.o: tests/r3c_robust.cpp r3c.h r3c.cpp utils.h utils.cpp
tests/r3c_stream.o: tests/r3c_stream.cpp r3c.h r3c.cpp utils.h utils.cpp
tests/redis_command_extension.o: tests/redis_command_extension.cpp r3c.h r3c.cpp utils.h utils.cpp

sha1.o: sha1.cpp
	$(CXX) -o $@ -c $< $(REAL_CPPFLAGS)
utils.o: utils.cpp
	$(CXX) -o $@ -c $< $(REAL_CPPFLAGS)
r3c.o: r3c.cpp
	$(CXX) -o $@ -c $< $(REAL_CPPFLAGS)
tests/r3c_cmd.o: tests/r3c_cmd.cpp
	$(CXX) -o $@ -c $< $(REAL_CPPFLAGS)
tests/r3c_test.o: tests/r3c_test.cpp
	$(CXX) -o $@ -c $< $(REAL_CPPFLAGS)
tests/r3c_stress.o: tests/r3c_stress.cpp
	$(CXX) -o $@ -c $< $(REAL_CPPFLAGS)
tests/r3c_robust.o: tests/r3c_robust.cpp
	$(CXX) -o $@ -c $< $(REAL_CPPFLAGS)
tests/r3c_stream.o: tests/r3c_stream.cpp
	$(CXX) -o $@ -c $< $(REAL_CPPFLAGS)
tests/redis_command_extension.o: tests/redis_command_extension.cpp
	$(CXX) -o $@ -c $< $(REAL_CPPFLAGS)

$(HIREDIS):
	@if test -d "$(HIREDIS)"; then \
		echo -e "\033[1;33mFound hiredis at $(HIREDIS)"; \
		true; \
	else \
		echo -e "\033[0;32;31mNot found hiredis at $(HIREDIS)\033[m"; \
		echo -e "\033[1;33mUsage: make HIREDIS=hiredis-install-directory\033[m"; \
		echo -e "\033[0;36mExample1: make HIREDIS=/usr/local/hiredis\033[m"; \
		echo -e "\033[0;36mExample2: make install PREFIX=/usr/local/r3c\033[m"; \
		echo -e "\033[0;36mExample3: make install HIREDIS=/usr/local/hiredis PREFIX=/usr/local/r3c\033[m"; \
		false; \
	fi

$(STLIBNAME): sha1.o utils.o r3c.o
	rm -f $@;$(STLIB_MAKE_CMD) $@ $^

$(CMD): tests/r3c_cmd.o $(STLIBNAME)
	$(CXX) -o $@ $^ $(REAL_LDFLAGS)

$(TEST): tests/r3c_test.o $(STLIBNAME)
	$(CXX) -o $@ $^ $(REAL_LDFLAGS)

$(STRESS): tests/r3c_stress.o $(STLIBNAME)
	$(CXX) -o $@ $^ $(REAL_LDFLAGS)

$(ROBUST): tests/r3c_robust.o $(STLIBNAME)
	$(CXX) -o $@ $^ $(REAL_LDFLAGS) -pthread

$(STREAM): tests/r3c_stream.o $(STLIBNAME)
	$(CXX) -o $@ $^ $(REAL_LDFLAGS) -pthread

$(EXTENSION): tests/redis_command_extension.o $(STLIBNAME)
	$(CXX) -o $@ -shared $^ $(REAL_LDFLAGS)

clean:
	rm -f $(STLIBNAME) $(CMD) $(TEST) $(STRESS) $(ROBUST) $(STREAM) $(EXTENSION) *.o core core.* tests/*.o tests/core tests/core.*
.PHONY: clean

install: $(STLIBNAME)
	$(INSTALL) -d $(INSTALL_INCLUDE_PATH)
	$(INSTALL) -d $(INSTALL_LIBRARY_PATH)
	$(INSTALL) -m 664 r3c.h $(INSTALL_INCLUDE_PATH)
	$(INSTALL) -m 664 $(STLIBNAME) $(INSTALL_LIBRARY_PATH)

dep:
	$(CXX) -MM *.cpp
.PHONY: dep

test: $(TEST)
	$(TEST) $(REDIS_CLUSTER_NODES)
.PHONY: test

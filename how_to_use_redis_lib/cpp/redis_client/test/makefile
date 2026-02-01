RM = rm -f
MAKE = make
COPY = cp
CC = g++
export CC
AR = ar
PS = cpp
LIBEXT = a

#CFLAGS = -rdynamic -Wall -m64  -pipe -fopenmp  -std=c++11
CFLAGS = -Wall -m64 -pipe -std=c++11
export CFLAGS

#RELEASE_FLAGS = -O2 -Wno-deprecated	
RELEASE_FLAGS = -O2 -Wno-deprecated	-D_PRINT_HANDLE_TIME_

DEBUG_FLAGS = -pg -g -ggdb -O0 -D_DEBUG -D_PRINT_HANDLE_TIME_ -Wno-deprecated 
#DEBUG_FLAGS = -pg -g -ggdb  -O0 -D_DEBUG  -Wno-deprecated 

RELEASE	= client

DEBUG	= $(RELEASE)_debug

TARGET = $(RELEASE) $(DEBUG)

DEPFLAGS = -MMD

LIBFLAGS =  rcs

#SOURCES = $(wildcard ./*.$(PS))
#SOURCES += $(wildcard ./common/*.$(PS))

ALL_INCLUDE = . ./common /usr/local/mysql/include

ifeq ($(shell uname), Darwin)
	ALL_LIB_PATH = ./lib/mac
else
	ALL_LIB_PATH = ./lib/linux
endif

#ALL_LIBS = pthread m rt dl memcached protobuf  wurfl xml2  tcmalloc  mysqlclient config config++
#ALL_LIBS = pthread m dl mysqlclient hiredis rt z tcmalloc
ALL_LIBS = hiredis pthread

#PATHS = .  ./include  ./common ./common/configure ./common/log ./common/message ./database ./common/sha-1  \
#		./common/redis ./common/cjson  ./common/md5 ./common/urlencode ./interface ./tracking 
PATHS = . ../src

SOURCES = $(foreach dir, $(PATHS), $(wildcard $(dir)/*.$(PS)))

OBJECTS_R = $(patsubst %.$(PS),%.o,$(SOURCES))

OBJECTS_D = $(patsubst %.$(PS),%d.o,$(SOURCES))

OBJS = $(OBJECTS_R) $(OBJECTS_D)

LIB_PATH = $(addprefix -L, $(ALL_LIB_PATH))

LIB_LINK = $(addprefix -l, $(ALL_LIBS))

DEPS := $(patsubst %.o, %.d, $(OBJS))

MISSING_DEPS := $(filter-out $(wildcard $(DEPS)),$(DEPS))

MISSING_DEPS_SOURCES := $(wildcard $(patsubst %.d,%.$(PS),$(MISSING_DEPS)))

#thirdparty software dirs-specified
SUBDIRS = ./common/protobuf


INSTALL = ../../bin

.PHONY : all install deps objs clean veryclean debug release rebuild subdirs

#all :   debug $(RELEASE)
all :   debug

debug : $(DEBUG)
#subdirs: $(SUBDIRS)
#	@for dir in $(SUBDIRS);\
#	do\
#		$(MAKE) -C $${dir};\
#	done
install : debug
	@echo '----- copy new version -----'

release : debug $(RELEASE)
	@echo '----- copy new version -----'

$(DEBUG) : $(OBJECTS_D) $(addprefix -l,$(LIBS_D))
	$(CC) $(DEBUG_FLAGS) $(CFLAGS)   -Wno-deprecated -o $@  $^  $(LIB_PATH) $(LIB_LINK)
#	@now=`date -d 'now' "+%Y%m%d%H%M%S"`;tar jcvf  $(DEBUG)$${now}.tar.bz2  $(DEBUG)
$(RELEASE) : $(OBJECTS_R)  $(addprefix -l,$(LIBS_R))
	$(CC) $(RELEASE_FLAGS) $(CFLAGS)  -Wno-deprecated -o $@  $^  $(LIB_PATH) $(LIB_LINK)
#	@now=`date -d 'now' "+%Y%m%d%H%M%S"`;tar jcvf  $(DEBUG)$${now}.tar.bz2  $(DEBUG)
deps : $(DEPS)

objs : $(OBJS)

clean:
	@echo '----- cleaning obj file -----'
	@$(RM) $(OBJS)
	@echo '----- cleaning dependency file -----'
	@$(RM) $(DEPS)
	@echo '----- cleaning binary file -----'
	@$(RM) -f $(RELEASE)
	@$(RM) -f $(DEBUG)
	@$(RM) -f *.log *.out *.bz2 core.* tags	cscope.*

veryclean : clean cleansub
		@echo '----- cleaning execute file -----'
		@$(RM) $(TARGET)

rebuild: veryclean all

ifneq ($(MISSING_DEPS),)
$(MISSING_DEPS) : subdirs 
	@$(RM) $(patsubst %.d,%.o,$@)
endif

-include $(DEPS)

$(OBJECTS_D) : %d.o : %.$(PS)
	$(CC) $(DEPFLAGS) $(DEBUG_FLAGS) $(CFLAGS) $(addprefix -I,$(ALL_INCLUDE)) -c $< -o $@

$(OBJECTS_R) : %.o : %.$(PS)
	$(CC) $(DEPFLAGS) $(RELEASE_FLAGS) $(CFLAGS) $(addprefix -I,$(ALL_INCLUDE)) -c $< -o $@

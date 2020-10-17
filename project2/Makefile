.SUFFIXES: .cpp .o

CC=g++

SRCDIR=src/
INC=include/
LIBS=lib/

# SRCS:=$(wildcard src/*.cpp)
# OBJS:=$(SRCS:.cpp=.o)

# main source file
TARGET_SRC:=$(SRCDIR)main.cpp
TARGET_OBJ:=$(SRCDIR)main.o

# Include more files if you write another source file.
SRCS_FOR_LIB:=$(SRCDIR)bpt.cpp
OBJS_FOR_LIB:=$(SRCS_FOR_LIB:.cpp=.o)

SRCS_FOR_SYS:=$(SRCDIR)file.cpp
OBJS_FOR_SYS:=$(SRCS_FOR_SYS:.cpp=.o)

CFLAGS+= -g -fPIC -I $(INC)

TARGET=main

all: $(TARGET)

$(TARGET): $(TARGET_OBJ)
	$(CC) $(CFLAGS) -o $(OBJS_FOR_LIB) -c $(SRCS_FOR_LIB)
	$(CC) $(CFLAGS) -o $(OBJS_FOR_SYS) -c $(SRCS_FOR_SYS)
	make static_library
	$(CC) $(CFLAGS) -o $@ $^ -L $(LIBS) -lbpt

clean:
	rm $(TARGET) $(TARGET_OBJ) $(OBJS_FOR_LIB) $(OBJS_FOR_SYS) $(LIBS)*

library:
	g++ -shared -Wl,-soname,libbpt.so -o $(LIBS)libbpt.so $(OBJS_FOR_LIB) $(OBJS_FOR_SYS)

static_library:
	ar cr $(LIBS)libbpt.a $(OBJS_FOR_LIB) $(OBJS_FOR_SYS)

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

SRCS_FOR_API:=$(SRCDIR)db.cpp
OBJS_FOR_API:=$(SRCS_FOR_API:.cpp=.o)

SRCS_FOR_BUF:=$(SRCDIR)buffer.cpp
OBJS_FOR_BUF:=$(SRCS_FOR_BUF:.cpp=.o)

CFLAGS+= -g -fPIC -I $(INC) -pg

TARGET=main

all: $(TARGET)

$(TARGET): $(TARGET_OBJ)
	$(CC) $(CFLAGS) -o $(OBJS_FOR_LIB) -c $(SRCS_FOR_LIB)
	$(CC) $(CFLAGS) -o $(OBJS_FOR_SYS) -c $(SRCS_FOR_SYS)
	$(CC) $(CFLAGS) -o $(OBJS_FOR_API) -c $(SRCS_FOR_API)
	$(CC) $(CFLAGS) -o $(OBJS_FOR_BUF) -c $(SRCS_FOR_BUF)
	make static_library
	$(CC) $(CFLAGS) -o $@ $^ -L $(LIBS) -lbpt

clean:
	rm $(TARGET) $(TARGET_OBJ) $(OBJS_FOR_LIB) $(OBJS_FOR_SYS) $(OBJS_FOR_API) $(OBJS_FOR_BUF) $(LIBS)*

library:
	g++ -shared -Wl,-soname,libbpt.so -o $(LIBS)libbpt.so $(OBJS_FOR_LIB) $(OBJS_FOR_SYS) $(OBJS_FOR_API) $(OBJS_FOR_BUF)

static_library:
	ar cr $(LIBS)libbpt.a $(OBJS_FOR_LIB) $(OBJS_FOR_SYS) $(OBJS_FOR_API) $(OBJS_FOR_BUF)

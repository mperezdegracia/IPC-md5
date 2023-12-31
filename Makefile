CC=gcc
CFLAGS=-Wall -pedantic -Iinclude

# Descomentar para debuguear
# CFLAGS+=-g
# LDFLAGS=-fsanitize=address

SRC=$(wildcard src/*.c)
SRC_APP=$(wildcard src/app/*.c)
SRC_SLAVE=$(wildcard src/slave/*.c)
SRC_VIEW=$(wildcard src/view/*.c)

OBJ=$(SRC:.c=.o)
OBJ_APP=$(SRC_APP:.c=.o)
OBJ_SLAVE=$(SRC_SLAVE:.c=.o)
OBJ_VIEW=$(SRC_VIEW:.c=.o)

APP=build/app
SLAVE=build/slave
VIEW=build/view

all: dir $(APP) $(SLAVE) $(VIEW)

dir:
	mkdir -p build

$(APP): $(OBJ_APP) $(OBJ)
	$(CC) -o $(APP) $^ $(LDFLAGS)

$(SLAVE): $(OBJ_SLAVE) $(OBJ)
	$(CC) -o $(SLAVE) $^ $(LDFLAGS)

$(VIEW): $(OBJ_VIEW) $(OBJ)
	$(CC) -o $(VIEW) $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build
	rm -f $(OBJ_APP) $(OBJ_SLAVE) $(OBJ_VIEW) $(OBJ)

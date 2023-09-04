CC=gcc
CFLAGS=-Wall -pedantic
CFLAGS+=-Iinclude

SRC_APP=$(wildcard src/app/*.c)
SRC_SLAVE=$(wildcard src/slave/*.c)
SRC_VIEW=$(wildcard src/view/*.c)

OBJ_APP=$(SRC_APP:.c=.o)
OBJ_SLAVE=$(SRC_SLAVE:.c=.o)
OBJ_VIEW=$(SRC_VIEW:.c=.o)

APP=build/app
SLAVE=build/slave
VIEW=build/view

all: dir $(APP) $(SLAVE) $(VIEW)

dir:
	mkdir -p build

$(APP): $(OBJ_APP)
	$(CC) -o $(APP) $^ $(LDFLAGS)

$(SLAVE): $(OBJ_SLAVE)
	$(CC) -o $(SLAVE) $^ $(LDFLAGS)

$(VIEW): $(OBJ_VIEW)
	$(CC) -o $(VIEW) $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build
	rm -f $(OBJ_APP) $(OBJ_SLAVE) $(OBJ_VIEW)

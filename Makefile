INSTALL_PREFIX ?= /usr
CFLAGS := -g -O2 -fPIC -Isrc -I$(INSTALL_PREFIX)/include/moar -I$(INSTALL_PREFIX)/include/dyncall
OBJ := src/port.o
EXAMPLE_OBJ := src/example.o

default: all

all: static dynamic example-static example-dynamic

dynamic: $(OBJ)
	$(CC) $(OBJ) -shared -o libport.so -Wl,-rpath=$(INSTALL_PREFIX)/lib/ -L$(INSTALL_PREFIX)/lib -lmoar

static: $(OBJ)
	ar rcs libport.a $(OBJ)

example-dynamic: dynamic $(EXAMPLE_OBJ)
	$(CC) $(EXAMPLE_OBJ) -o example-dynamic -Wl,-rpath=. -L. -lport

example-static: static $(EXAMPLE_OBJ)
	$(CC) $(EXAMPLE_OBJ) libport.a -o example-static -Wl,-rpath=$(INSTALL_PREFIX)/lib/ -L$(INSTALL_PREFIX)/lib -lmoar

clean:
	rm -f $(OBJ) $(EXAMPLE_OBJ) libport.a libport.so example-dynamic example-static

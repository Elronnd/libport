BUNDLE_DIR ?= perl6_bundle
HEADER_DIR ?= /usr/include

CFLAGS := -g -O2 -fPIC -Isrc -I$(HEADER_DIR)/moar -I$(HEADER_DIR)/dyncall -DPERL6_INSTALL_PATH='"$(BUNDLE_DIR)"'
OBJ := src/port.o
EXAMPLE_OBJ := src/example.o

default: all

all: static dynamic example-static example-dynamic

dynamic: $(OBJ)
	$(CC) $(OBJ) -shared -o libport.so -Wl,-rpath=$(BUNDLE_DIR)/lib/ -L$(BUNDLE_DIR)/lib -lmoar

static: $(OBJ)
	ar rcs libport.a $(OBJ)

example-dynamic: dynamic $(EXAMPLE_OBJ)
	$(CC) $(EXAMPLE_OBJ) -o example-dynamic -Wl,-rpath=. -L. -lport

example-static: static $(EXAMPLE_OBJ)
	$(CC) $(EXAMPLE_OBJ) libport.a -o example-static -Wl,-rpath=$(BUNDLE_DIR)/lib/ -L$(BUNDLE_DIR)/lib -lmoar

clean:
	rm -f $(OBJ) $(EXAMPLE_OBJ) libport.a libport.so example-dynamic example-static

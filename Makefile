BUNDLE_DIR ?= perl6_bundle
HEADER_DIR ?= /usr/include

# GCC doesn't print warnings for headers in the 'system header path'
CFLAGS += -g -O0 -fPIC
CFLAGS += -std=c11 -D_XOPEN_SOURCE=500 -Wall -Wextra
CFLAGS += -Isrc -I$(HEADER_DIR)/moar -I$(HEADER_DIR)/dyncall -I$(HEADER_DIR)/libtommath -DPERL6_INSTALL_PATH='"$(BUNDLE_DIR)"'

OBJ := src/port.o
EXAMPLE_OBJ := src/example.o

default: all

all: static dynamic example-static example-dynamic

make-fairy-juice:
	perl6 make-fairy-juice.p6

src/port.o: make-fairy-juice

dynamic: $(OBJ)
	$(CC) $(OBJ) -shared -o libport.so -Wl,-rpath=$(BUNDLE_DIR)/lib/ -L$(BUNDLE_DIR)/lib -lmoar

static: $(OBJ)
	ar rcs libport.a $(OBJ)

example-dynamic: dynamic $(EXAMPLE_OBJ)
	$(CC) $(CFLAGS) $(EXAMPLE_OBJ) -o example-dynamic -Wl,-rpath=. -L. -lport

example-static: static $(EXAMPLE_OBJ)
	$(CC) $(CFLAGS) $(EXAMPLE_OBJ) libport.a -o example-static -Wl,-rpath=$(BUNDLE_DIR)/lib/ -L$(BUNDLE_DIR)/lib -lmoar

clean:
	rm -f $(OBJ) $(EXAMPLE_OBJ) libport.a libport.so example-dynamic example-static src/fairy-juice.h

CFLAGS := -I/home/elronnd/.perl6install/include/moar -I/home/elronnd/.perl6install/include/dyncall -g -fPIC
OBJ := port.o

default: all

all: $(OBJ)
	$(CC) $(OBJ) -Wl,-rpath=/home/elronnd/.perl6install/lib/ -Wl,-E -lmoar -L/home/elronnd/.perl6install/lib -o port

clean:
	rm -f $(OBJ) port

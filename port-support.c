#include <unistd.h>
// !F!I!X!M!E!!!
// MASSIVE KLUDGE ALERT
//
// I don't know how to turn integers into pointers in p6, so I use this function
// and say that it's size_t -> pointer, on the p6 side
size_t id(size_t n) { return n; }

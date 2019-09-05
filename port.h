#ifndef PORT_H
#define PORT_H

typedef struct P6State P6State;

P6State *p6_init(void);
void p6_deinit(P6State *state);
void p6eval(P6State *state, char *text);

#endif//PORT_H

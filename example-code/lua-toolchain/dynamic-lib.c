/* Chapter 59: a shared object's TLS cell is distinct for every thread. */
static _Thread_local int counter = 37;
int next_value(void) { return counter++; }

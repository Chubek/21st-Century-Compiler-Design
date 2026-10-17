/* Chapter 59 Linux/POSIX experiment; compile with -ldl -pthread.
   The case-study static linker does not implement this loader protocol. */
#include <assert.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
static int (*next_value)(void);
static void *worker(void *unused)
{
    (void)unused;
    assert(next_value()==37);
    assert(next_value()==38);
    return NULL;
}
int main(int argc,char **argv)
{
    assert(argc==2);
    void *library=dlopen(argv[1],RTLD_NOW|RTLD_LOCAL);
    if (!library) { fprintf(stderr,"%s\n",dlerror()); return 2; }
    (void)dlerror();
    void *address=dlsym(library,"next_value");
    const char *error=dlerror();
    if (error) { fprintf(stderr,"%s\n",error); dlclose(library); return 2; }
    _Static_assert(sizeof next_value==sizeof address,"POSIX x86-64 pointer representation");
    memcpy(&next_value,&address,sizeof next_value);
    assert(next_value()==37);
    pthread_t t;
    assert(pthread_create(&t,NULL,worker,NULL)==0);
    assert(pthread_join(t,NULL)==0);
    assert(next_value()==38);
    assert(dlclose(library)==0);
    puts("independent TLS counters: 37,38");
    return 0;
}

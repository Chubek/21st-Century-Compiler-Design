/* Chapter 48: C11 CLI, graph file and optional BFS visit budget. */
#include "checker.h"

#include <inttypes.h>

static int exit_status(mc_status status)
{
    if (status == MC_SAFE) return 0;
    if (status == MC_UNSAFE) return 1;
    return status == MC_LIMIT ? 3 : 2;
}

int main(int argc, char **argv)
{
    size_t budget = MC_MAX_STATES;
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "usage: %s graph [visit-budget]\n", argv[0]);
        return 2;
    }
    if (argc == 3) {
        budget = 0;
        if (!argv[2][0]) return 2;
        for (size_t i = 0; argv[2][i]; ++i) {
            unsigned char c = (unsigned char)argv[2][i];
            if (c < '0' || c > '9' || budget > (SIZE_MAX - (c - '0')) / 10)
                return 2;
            budget = budget * 10 + (c - '0');
        }
    }
    FILE *in = fopen(argv[1], "r");
    if (!in) { perror(argv[1]); return 2; }
    mc_graph g;
    mc_status status = mc_read(in, &g);
    if (fclose(in) != 0) status = MC_INVALID;
    mc_trace trace;
    if (status == MC_OK) status = mc_safety(&g, 1, budget, &trace);
    puts(mc_status_name(status));
    if (status == MC_UNSAFE) {
        fputs("trace:", stdout);
        for (size_t i = 0; i < trace.length; ++i)
            printf(" %zu", trace.state[i]);
        putchar('\n');
    }
    if (status == MC_SAFE || status == MC_UNSAFE) {
        uint64_t bad = mc_atoms(&g, 1), set;
        const mc_operator ops[] = {MC_EX, MC_EU, MC_EG};
        const char *names[] = {"EX bad", "E[true U bad]", "EG !bad"};
        for (size_t i = 0; i < 3; ++i) {
            uint64_t p = i == 0 ? bad : mc_universe(g.n);
            if (i == 2) p &= ~bad;
            mc_status ctl = mc_ctl(&g, ops[i], p, bad, g.n + 1, &set);
            if (ctl != MC_OK) return exit_status(ctl);
            printf("%s: 0x%016" PRIx64 "\n", names[i], set);
        }
    }
    return exit_status(status);
}

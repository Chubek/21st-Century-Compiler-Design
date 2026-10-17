/* Chapter 48: bounded explicit-state model checking. C11, standard library only. */
#ifndef CHECKER_H
#define CHECKER_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define MC_MAX_STATES 64U
typedef enum { MC_OK, MC_SAFE, MC_UNSAFE, MC_INVALID, MC_LIMIT } mc_status;
typedef enum { MC_EX, MC_EU, MC_EG } mc_operator;
typedef struct {
    size_t n;
    uint64_t initial;
    uint64_t props[MC_MAX_STATES];
    uint64_t edge[MC_MAX_STATES];
} mc_graph;
typedef struct {
    size_t length;
    size_t state[MC_MAX_STATES];
} mc_trace;

uint64_t mc_universe(size_t n);
mc_status mc_validate(const mc_graph *g);
mc_status mc_read(FILE *in, mc_graph *out);
uint64_t mc_atoms(const mc_graph *g, uint64_t mask);
mc_status mc_safety(const mc_graph *g, uint64_t bad_mask,
                    size_t max_visits, mc_trace *out);
mc_status mc_ctl(const mc_graph *g, mc_operator op,
                 uint64_t p, uint64_t q, size_t max_rounds,
                 uint64_t *out);
const char *mc_status_name(mc_status status);

#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vdef.h"
#include "vsb.h"
#include "vapi/vsm.h"


struct iter_priv{
	const struct vrt_ctx *cpy_ctx;
	struct vsb *vsb;
	int jp;
	double delta;
};
struct counter {
        unsigned n, nmax;
        double acc;
};

struct hitrate {
        double tm;
        uint64_t hit, miss;
        struct counter hr;         /* hr stands for hitrate */
};
struct load {
	uint64_t req;
	struct counter rl;         /* rl stands for reqload */
};

uint64_t  beresp_hdr, beresp_body;
uint64_t  bereq_hdr, bereq_body;
uint64_t be_happy;

int n_be, cont;

int run_subroutine(struct iter_priv *iter, struct VSM_data *vd);
int general_info(struct iter_priv *iter);


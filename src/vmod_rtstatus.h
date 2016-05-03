#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vdef.h"
#include "vsb.h"
#include "vapi/vsm.h"


struct rtstatus_priv{
	const struct vrt_ctx *cpy_ctx;
	struct vsb *vsb;
	double delta;
	int jp;
};

struct counter {
        unsigned n, nmax;
        double acc;
};

struct hitrate {
        uint64_t hit, miss;
        struct counter hr;         /* hr stands for hitrate */
        double tm;
} hitrate;

struct load {
	struct counter rl;         /* rl stands for reqload */
	uint64_t req;
} load;

uint64_t  beresp_hdr, beresp_body;
uint64_t  bereq_hdr, bereq_body;
uint64_t be_happy;

int n_be, cont;

int run_subroutine(struct rtstatus_priv *rtstatus, struct VSM_data *vd);
int general_info(struct rtstatus_priv *rtstatus);


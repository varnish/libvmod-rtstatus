#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vsb.h"
#include "vrt.h"
#include "vrt_obj.h"
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


void WS_Release(struct ws *ws, unsigned bytes);
unsigned WS_Reserve(struct ws *ws, unsigned bytes);
int run_subroutine(struct iter_priv *iter, struct VSM_data *vd);
int general_info(struct iter_priv *iter);
int backend(struct iter_priv *iter);

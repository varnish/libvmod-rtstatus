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
	char *p;
	int jp;
};

void WS_Release(struct ws *ws, unsigned bytes);
unsigned WS_Reserve(struct ws *ws, unsigned bytes);
int run_subroutine(struct iter_priv *iter, struct VSM_data *vd);
int general_info(struct iter_priv *iter);
int backend(struct iter_priv *iter);

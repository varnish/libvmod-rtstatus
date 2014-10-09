#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>

#include "vsb.h"
#include "vrt.h"
#include "vrt_obj.h"
#include "cache/cache.h"
#include "vmod_rtstatus.h"
#include "vmod_html.h"

VCL_STRING
vmod_html(const struct vrt_ctx *ctx)
{
	struct vsb *vsb;
	
	WS_Reserve(ctx->ws, 0);
	vsb = VSB_new_auto();
	vsb->s_buf = ctx->ws->f;
	VSB_cat(vsb, html);
	VSB_finish(vsb);
	WS_Release(ctx->ws, strlen(vsb->s_buf) + 1);
	return (vsb->s_buf);
}


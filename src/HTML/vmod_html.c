#include "cache/cache.h"
#include "vmod_rtstatus.h"
#include "vmod_html.h"

VCL_STRING
vmod_html(const struct vrt_ctx *ctx)
{
	struct iter_priv iter = { 0 };
	WS_Reserve(ctx->ws, 0);
	iter.p = ctx->ws->f;
	iter.vsb = VSB_new_auto();
	VSB_cat(iter.vsb, html);
	VSB_finish(iter.vsb);
	if (VSB_error(iter.vsb)) {
	    VSLb(ctx->vsl, SLT_VCL_Error, "VSB error");
	    VSB_delete(iter.vsb);
	    WS_Release(ctx->ws, strlen(iter.vsb->s_buf) + 1);
	    return "{}";
	}
	strcpy(iter.p, iter.vsb->s_buf);
	VSB_delete(iter.vsb);
	WS_Release(ctx->ws, strlen(iter.p) + 1);
	return (iter.p);
}


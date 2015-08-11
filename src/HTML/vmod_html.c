#include "cache/cache.h"
#include "vrt.h"

#include "vmod_rtstatus.h"
#include "vmod_html.h"

VCL_STRING
vmod_html(const struct vrt_ctx *ctx)
{
	struct iter_priv iter = { 0 };
	iter.vsb = VSB_new(NULL, ctx->ws->f, WS_Reserve(ctx->ws, 0), VSB_AUTOEXTEND); 
	VSB_cat(iter.vsb, html);
	VSB_finish(iter.vsb);
	if (VSB_error(iter.vsb)) {
	    VSLb(ctx->vsl, SLT_VCL_Error, "VSB error");
	    WS_Release(ctx->ws, VSB_len(iter.vsb) + 1);
	    return "{}";
	}
	WS_Release(ctx->ws, VSB_len(iter.vsb) + 1);
	return (iter.vsb->s_buf);
}


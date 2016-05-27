#include "cache/cache.h"
#include "vrt.h"
#include "vcl.h"
#include "vsb.h"

#include "vmod_rtstatus.h"
#include "vmod_html.h"

VCL_STRING
vmod_html(const struct vrt_ctx *ctx)
{
	struct vsb *v;

        if (ctx->method != VCL_MET_SYNTH) {
                VSLb(ctx->vsl, SLT_VCL_Error, "rtstatus() can only be used in vcl_synth");
                return "{ \"error\": \"Check Varnishlog for more details\" }";
        }

	v = VSB_new(NULL, ctx->ws->f, WS_Reserve(ctx->ws, 0), VSB_AUTOEXTEND);
	VSB_cat(v, html);
	VSB_finish(v);
	if (VSB_error(v)) {
	    VSLb(ctx->vsl, SLT_VCL_Error, "VSB error");
	    WS_Release(ctx->ws, VSB_len(v) + 1);
	    return "{ \"error\": \"Check Varnishlog for more details\" }";
	}
	WS_Release(ctx->ws, VSB_len(v) + 1);
	return (v->s_buf);
}


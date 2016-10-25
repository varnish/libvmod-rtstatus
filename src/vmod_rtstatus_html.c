#include "cache/cache.h"
#include "vrt.h"
#include "vcl.h"

#include "vmod_rtstatus_vsm.h"
#include "vmod_rtstatus_html.h"

#include "vcc_rtstatus_if.h"

VCL_STRING
vmod_html(const struct vrt_ctx *ctx)
{
	char *p;
	unsigned u,v;

        if (ctx->method != VCL_MET_SYNTH) {
                VSLb(ctx->vsl, SLT_VCL_Error, "rtstatus() can only be used in vcl_synth");
                return "{ \"error\": \"Check Varnishlog for more details\" }";
        }

	u = WS_Reserve(ctx->ws, 0);
	p = ctx->ws->f;
	v = snprintf(p, u, "%s", html);
	v++;
	if (v > u) {
	    WS_Release(ctx->ws, 0);
	    return "{ \"error\": \"Workspace overflow\" }";
	}
	WS_Release(ctx->ws, v);
	return (p);
}


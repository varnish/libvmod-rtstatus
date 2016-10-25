#include "cache/cache.h"

#include "vmod_rtstatus_vsm.h"
#include "vcl.h"
#include "vrt.h"
#include "vrt_obj.h"

#include "vmod_rtstatus_html.h"
#include "vcc_rtstatus_if.h"

VCL_STRING
vmod_rtstatus(VRT_CTX)
{
	struct rtstatus_priv rs;
	struct VSM_data *vd;

	INIT_OBJ(&rs, VMOD_RTSTATUS_MAGIC);

	if (ctx->method != VCL_MET_SYNTH) {
		VSLb(ctx->vsl, SLT_VCL_Error, "rtstatus() can only be used in vcl_synth");
		return "{ \"error\": \"Check Varnishlog for more details\" }";
	}

	vd = VSM_New();
	AN(vd);

	if (!VSM_n_Arg(vd, VRT_r_server_identity(ctx)) ||
			VSM_Open(vd)) {
		VSLb(ctx->vsl, SLT_VCL_Error, "Can't open VSM");
		VSM_Delete(vd);
		return "{ \"error\": \"Check Varnishlog for more details\" }";
	}
	rs.vsb = ctx->specific;

	collect_info(&rs, vd);

	VSM_Delete(vd);
	return "";
}

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


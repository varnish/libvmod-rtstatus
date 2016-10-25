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
		return "{ \"error\": \"rtstatus.rtstatus() can only be used in vcl_synth\" }";
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
vmod_html(VRT_CTX)
{
        if (ctx->method != VCL_MET_SYNTH) {
                VSLb(ctx->vsl, SLT_VCL_Error, "rtstatus.html() can only be used in vcl_synth");
                return "{ \"error\": \"rtstatus() can only be used in vcl_synth\" }\n";
        }

	AN(ctx->specific);
	VSB_cat(ctx->specific, html);
	return "";
}

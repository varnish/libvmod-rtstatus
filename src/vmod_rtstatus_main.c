#include "cache/cache.h"

#include "vmod_rtstatus.h"
#include "vcl.h"
#include "vrt.h"
#include "vrt_obj.h"

int
init_function(VRT_CTX, struct vmod_priv *priv, enum vcl_event_e e)
{
	if (e != VCL_EVENT_LOAD)
		return (0);

        beresp_hdr = beresp_body = 0;
        bereq_hdr = bereq_body = 0;
        be_happy = 0;
        n_be = 0;
        cont = 0;
        return(0);
}


int
general_info(struct rtstatus_priv *rtstatus)
{
	static char vrt_hostname[255] = "";
	VSB_cat(rtstatus->vsb, "\t\"varnish_version\" : \"");
	VSB_cat(rtstatus->vsb, VCS_version);
	VSB_cat(rtstatus->vsb, "\",\n");
	gethostname(vrt_hostname, sizeof(vrt_hostname));
	VSB_cat(rtstatus->vsb, "\t\"server_id\": \"");
	VSB_cat(rtstatus->vsb, vrt_hostname);
	VSB_cat(rtstatus->vsb, "\",\n");
	return(0);
}


VCL_STRING
vmod_rtstatus(VRT_CTX)
{
	struct rtstatus_priv rtstatus = { 0 };
	struct VSM_data *vd;

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

	rtstatus.vsb = ctx->specific;
	rtstatus.cpy_ctx = ctx;
       	rtstatus.jp = 1;
	collect_info(&rtstatus, vd);
	VSM_Delete(vd);
	return "";
}

#include "cache/cache.h"

#include "vmod_rtstatus.h"
#include "vcl.h"
#include "vrt.h"


int
general_info(struct iter_priv *iter)
{
	static char vrt_hostname[255] = "";
	VSB_cat(iter->vsb, "\t\"varnish_version\" : \"");
	VSB_cat(iter->vsb, VCS_version);
	VSB_cat(iter->vsb, "\",\n");
	gethostname(vrt_hostname, sizeof(vrt_hostname));
	VSB_cat(iter->vsb, "\t\"server_id\": \"");
	VSB_cat(iter->vsb, vrt_hostname);
	VSB_cat(iter->vsb, "\",\n");
	return(0);
}

VCL_STRING
vmod_rtstatus(const struct vrt_ctx *ctx, VCL_REAL delta)
{
	struct iter_priv iter = { 0 };
	struct VSM_data *vd;

	if (delta < 1 || delta > 60) {
		VSLb(ctx->vsl, SLT_VCL_Error, "Delta has to be between"
		    "1 and 60 seconds");
		return "{}";
	}
	vd = VSM_New();
	if (VSM_Open(vd)) {
		VSLb(ctx->vsl, SLT_VCL_Error, "Can't open VSM");
		VSM_Delete(vd);
		return "{}";
	}
	iter.vsb = ctx->specific;
	iter.cpy_ctx = ctx;
       	iter.jp = 1;
	iter.delta = delta;
	run_subroutine(&iter, vd);
	return "";
}

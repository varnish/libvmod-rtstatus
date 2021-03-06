#include "cache/cache.h"
#include "vapi/vsm.h"
#include "vcl.h"
#include "cache/cache_backend.h"
#include "vmod_rtstatus.h"

int
backend(struct iter_priv *iter)
{
	const struct vrt_ctx *ctx = iter->cpy_ctx;
	int i;

	VSB_cat(iter->vsb, "\t\"backend\": [");
	for (i = 1; i < iter->cpy_ctx->vcl->ndirector; ++i) {
		CHECK_OBJ_NOTNULL(ctx->vcl->director[i], DIRECTOR_MAGIC);
		if (strcmp("simple", ctx->vcl->director[i]->name) == 0) {
			char buf[1024];
			int j, healthy;
			healthy = VDI_Healthy(ctx->vcl->director[i]);
			j = snprintf(buf, sizeof buf, "{\"director_name\" :"
			    " \"%s\" , \"name\":\"%s\", \"value\": \"%s\"}",
			    ctx->vcl->director[i]->name,
			    ctx->vcl->director[i]->vcl_name,
			    healthy ? "healthy" : "sick");
			assert(j >= 0);
			VSB_cat(iter->vsb, buf);
			if (i < (ctx->vcl->ndirector - 1)) {
			    VSB_cat(iter->vsb, ",\n\t\t");
			}
		}
	}
	VSB_cat(iter->vsb, "],\n");
	return(0);
}

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
	iter.vsb = ctx->req->synth_body;
	iter.cpy_ctx = ctx;
       	iter.jp = 1;
	iter.delta = delta;
	run_subroutine(&iter, vd);
	VSM_Delete(vd);
	return "";

}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vsb.h"
#include "vrt.h"
#include "vrt_obj.h"
#include "cache/cache.h"
#include "vapi/vsm.h"
#include "vcl.h"
#include "cache/cache_backend.h"
#include "vmod_rtstatus.h"

int
backend(struct iter_priv *iter)
{
	int i;

	VSB_cat(iter->vsb, "\t\"backend\": [");
	for (i = 1; i < iter->cpy_ctx->vcl->ndirector; ++i) {
		CHECK_OBJ_NOTNULL(iter->cpy_ctx->vcl->director[i], DIRECTOR_MAGIC);
		if (strcmp("simple", iter->cpy_ctx->vcl->director[i]->name) == 0) {
			char buf[1024];
			int j, healthy;
			healthy = VDI_Healthy(iter->cpy_ctx->vcl->director[i]);
			j = snprintf(buf, sizeof buf, "{\"director_name\" : \"%s\" , \"name\":\"%s\", \"value\": \"%s\"}",
					iter->cpy_ctx->vcl->director[i]->name,
					iter->cpy_ctx->vcl->director[i]->vcl_name,
					healthy ? "healthy" : "sick");
			assert(j >= 0);
			VSB_cat(iter->vsb, buf);
			if (i < (iter->cpy_ctx->vcl->ndirector - 1)) {
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
	VSB_cat(iter->vsb, "\t\"varnish_version\" : \"");
	VSB_cat(iter->vsb, VCS_version);
	VSB_cat(iter->vsb, "\",\n");
	VSB_cat(iter->vsb, "\t\"server_id\": \"");
	VSB_cat(iter->vsb, VRT_r_server_identity(iter->cpy_ctx));
	VSB_cat(iter->vsb, "\",\n");
	VSB_cat(iter->vsb, "\t\"client_id\": \"");
	VSB_cat(iter->vsb, VRT_r_client_identity(iter->cpy_ctx));
	VSB_cat(iter->vsb, "\",\n");
	return(0);
}



VCL_STRING
vmod_rtstatus(const struct vrt_ctx *ctx)
{
	struct iter_priv iter = { 0 };
	struct VSM_data *vd;
	const struct VSC_C_main *VSC_C_main;

	iter.vsb = VSB_new_auto();
	vd = VSM_New();
	if (VSM_Open(vd)) {
		VSLb(ctx->vsl, SLT_VCL_Error, "Can't open VSM");	    
	    VSM_Delete(vd);
	    return "{}";
	}
	WS_Reserve(ctx->ws, 0);
	iter.vsb->s_buf = ctx->ws->f;
	iter.cpy_ctx = ctx;
       	iter.jp = 1;
	run_subroutine(&iter, vd);
	VSB_finish(iter.vsb);
	VSM_Delete(vd);
	WS_Release(ctx->ws, strlen(iter.vsb->s_buf) + 1);
	return(iter.vsb->s_buf);
}

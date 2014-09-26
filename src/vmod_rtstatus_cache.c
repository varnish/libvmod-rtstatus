#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vrt.h"
#include "vrt_obj.h"
#include "cache/cache.h"
#include "vcl.h"
#include "cache/cache_backend.h"
#include "vmod_rtstatus.h"
//////////////////////////////////////////////////////////
char *
wsstrncat(char *dest, const char *src, const struct vrt_ctx *ctx)
{
	if (ctx->ws->r <= ctx->ws->f) {
		return (NULL);
	}
	return strcat(dest, src);
	}
//////////////////////////////////////////////////////////
int
backend(struct iter_priv *iter)
{
	int i;
	int cont = 1;
	
	STRCAT(iter->p, "\t\"backend\": [", iter->cpy_ctx);
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
			STRCAT(iter->p, buf, iter->cpy_ctx);
			if (i < (iter->cpy_ctx->vcl->ndirector - 1)) {
			    STRCAT(iter->p, ",\n\t\t", iter->cpy_ctx);
			}
		}
	}
	STRCAT(iter->p, "],\n", iter->cpy_ctx);
	return(0);
}
/////////////////////////////////////////////////////////
int
general_info(struct iter_priv *iter)
{
    STRCAT(iter->p, "\t\"varnish_version\" : \"", iter->cpy_ctx);
	STRCAT(iter->p, VCS_version, iter->cpy_ctx);
	STRCAT(iter->p, "\",\n", iter->cpy_ctx);
	STRCAT(iter->p, "\t\"server_id\": \"", iter->cpy_ctx);
	STRCAT(iter->p, VRT_r_server_identity (iter->cpy_ctx), iter->cpy_ctx);
	STRCAT(iter->p, "\",\n", iter->cpy_ctx);
	STRCAT(iter->p, "\t\"client_id\": \"", iter->cpy_ctx);
	STRCAT(iter->p, VRT_r_client_identity (iter->cpy_ctx), iter->cpy_ctx);
	STRCAT(iter->p, "\",\n", iter->cpy_ctx);
	return(0);
}

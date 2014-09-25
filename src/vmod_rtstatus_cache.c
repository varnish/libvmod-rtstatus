#include "config.h"
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "miniobj.h"
#include "vrt.h"
#include "vrt_obj.h"
#include "vapi/vsm.h"
#include "cache/cache.h"
#include "vcl.h"
#include "vas.h"
#include "cache/cache_backend.h"

#include "vmod_rtstatus.h"
static char *
wsstrncat(char *dest, const char *src, const struct vrt_ctx *ctx)
{
if (ctx->ws->r <= ctx->ws->f) {
return(NULL);
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
	char tmp[128];
	sprintf(tmp, "\t\"timestamp\": %f,\n", iter->time);
	STRCAT(iter->p, tmp, iter->cpy_ctx);
	STRCAT(iter->p, "\t\"varnish_version\" : \"", iter->cpy_ctx);
	STRCAT(iter->p, VCS_version, iter->cpy_ctx);
	STRCAT(iter->p, "\",\n", iter->cpy_ctx);
	STRCAT(iter->p, "\t\"server_id\": \"", iter->cpy_ctx);
	STRCAT(iter->p, VRT_r_server_identity (iter->cpy_ctx), iter->cpy_ctx);
	STRCAT(iter->p, "\",\n", iter->cpy_ctx);
	STRCAT(iter->p, "\t\"client_id\": \"", iter->cpy_ctx);
	STRCAT(iter->p, VRT_r_client_identity (iter->cpy_ctx), iter->cpy_ctx);
	STRCAT(iter->p, "\",\n", iter->cpy_ctx);

	sprintf(tmp, "\t\"PROVA\": %f,\n",  VRT_r_server_ip(iter->cpy_ctx));
	STRCAT(iter->p, tmp, iter->cpy_ctx);

	return(0);
}
///////////////////////////////////////////////////////
int
vrt_backend( struct iter_priv *iter)
{
    char tmp[128];
    STRCAT(iter->p, "\t\"backend_property\" : \"", iter->cpy_ctx);
    STRCAT(iter->p, iter->cpy_be->vcl_name, iter->cpy_ctx);
    STRCAT(iter->p, "\",\n", iter->cpy_ctx);
    if( iter->cpy_be->ipv6_addr!=NULL){
	STRCAT(iter->p, "\t\"ipv6\" : \"", iter->cpy_ctx);
	STRCAT(iter->p, iter->cpy_be->ipv6_addr, iter->cpy_ctx);
	STRCAT(iter->p, "\",\n", iter->cpy_ctx);
    };
    sprintf(tmp, "\t\"Connection_timeout\": %f,\n", iter->cpy_be->connect_timeout);
    STRCAT(iter->p, tmp, iter->cpy_ctx);
    sprintf(tmp, "\t\"First_byte_to\": %f,\n", iter->cpy_be->first_byte_timeout);
    STRCAT(iter->p, tmp, iter->cpy_ctx);
    sprintf(tmp, "\t\"between_bytes_to\": %f,\n", iter->cpy_be->between_bytes_timeout);
    STRCAT(iter->p, tmp, iter->cpy_ctx);
    sprintf(tmp, "\t\"max_connections\": %u,\n", iter->cpy_be->max_connections);
    STRCAT(iter->p, tmp, iter->cpy_ctx);

   
    

    return(0);
}

///////////////////////////////////////////////////////
int
run_subroutine(struct iter_priv *iter, struct VSM_data *vd)
{
        STRCAT(iter->p, "{\n", iter->cpy_ctx);
	rate(iter, vd);
	general_info(iter);
	backend(iter);
	vrt_backend(iter);
	return(0);
}
////////////////////////////////////////////////////////

#include "config.h"
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "vrt.h"
#include "vrt_obj.h"

#include "vcc_if.h"
//#include "libvarnishapi.h"
//#include "cache/cache.h"
#include "vapi/vsc.h"

#include "vapi/vsm.h"
//#include "vtim.h"
#include "vcl.h"
//#include "cache/cache_backend.h"

struct ws {
	unsigned		magic;
#define WS_MAGIC		0x35fac554
	char			id[4];		/* identity */
	char			*s;		/* (S)tart of buffer */
	char			*f;		/* (F)ree/front pointer */
	char			*r;		/* (R)eserved length */
	char			*e;		/* (E)nd of buffer */
};
#define STRCAT(dst, src, ctx)					\
    do {							\
	dst = wsstrncat(dst, src, ctx);				\
	if (!dst) {						\
	    WS_Release(ctx->ws, 0);				\
	    return 1;						\
	}							\
    } while(0)							\

struct iter_priv{
    char *p;
    const struct vrt_ctx *cpy_ctx;
    char *time_stamp;
    struct vtc_job *jp;
    double time;
};
int
init_function (struct vmod_priv *priv, const struct VCL_conf *conf)
{
return (0);
}
//////////////////////////////////////////////////////////
static char *
wsstrncat (char *dest, const char *src, const struct vrt_ctx *ctx)
{
    if (ctx->ws->r <= ctx->ws->f)
	{
	    return (NULL);
	}
    return strcat (dest, src);
}
/////////////////////////////////////////////////////////
int
general_info (struct iter_priv *iter)
{
char tmp[128];
/*STRCAT (iter->p, "\t\"req_request\": \"", iter->cpy_ctx);
STRCAT (iter->p, VRT_r_req(iter->cpy_ctx), iter->cpy_ctx);
STRCAT (iter->p, "\",\n", iter->cpy_ctx);*/
//STRCAT (iter->p, VRT_r_obj_status (iter->cpy_ctx) , iter->cpy_ctx);
sprintf (tmp, "\t\"timestamp\": %f,\n", iter->time);
STRCAT (iter->p, tmp, iter->cpy_ctx);
STRCAT (iter->p, "\t\"timestamp\" : \"", iter->cpy_ctx);
STRCAT (iter->p, iter->time_stamp, iter->cpy_ctx);
//STRCAT (iter->p, "\",\n\t\"varnish_version\" : \"", iter->cpy_ctx);
//STRCAT (iter->p, VCS_version, iter->cpy_ctx);
//STRCAT (iter->p, "\",\n", iter->cpy_ctx);
/*sprintf (tmp, "\t\"varnish_port\": %d,\n", VRT_r_server_hostname (iter->cpy_ctx));
  STRCAT (iter->p, tmp, iter->cpy_ctx);*/
STRCAT (iter->p, "\t\"server_id\": \"", iter->cpy_ctx);
STRCAT (iter->p, VRT_r_server_identity (iter->cpy_ctx), iter->cpy_ctx);
STRCAT (iter->p, "\",\n", iter->cpy_ctx);
STRCAT (iter->p, "\t\"client_id\": \"", iter->cpy_ctx);
STRCAT (iter->p, VRT_r_client_identity (iter->cpy_ctx), iter->cpy_ctx);
STRCAT (iter->p, "\",\n", iter->cpy_ctx);


//STRCAT (iter->p, VRT_r_beresp_backend_name(iter->cpy_ctx), iter->cpy_ctx);

return (0);
}
//////////////////////////////////////////////////////////
/*int
backend (struct iter_priv *iter)
{
    int i;
    int cont = 1;
    STRCAT (iter->p, "\t\"backend\": [", iter->cpy_ctx);
    for (i = 1; i < iter->cpy_ctx->vcl->ndirector; ++i)
	{
	    CHECK_OBJ_NOTNULL (iter->cpy_ctx->vcl->director[i], DIRECTOR_MAGIC);
	    if (strcmp ("simple", iter->cpy_ctx->vcl->director[i]->name) == 0)
		{
		    char buf[1024];
		    int j, healthy;
		    healthy = VDI_Healthy (iter->cpy_ctx->vcl->director[i]);
		    j = snprintf (buf, sizeof buf, "{\"name\":\"%s\", \"value\": \"%s\"}",
				  iter->cpy_ctx->vcl->director[i]->vcl_name,
				  healthy ? "healthy" : "sick");
		    assert (j >= 0);
		    STRCAT (iter->p, buf, iter->cpy_ctx);
		    if (i < (iter->cpy_ctx->vcl->ndirector - 2))
			{
			    STRCAT (iter->p, ",", iter->cpy_ctx);
			}
		}
	}
    STRCAT (iter->p, "],\n", iter->cpy_ctx);
    return (0);
    }*/
///////////////////////////////////////////////////////
int
rate (struct iter_priv *iter,struct VSM_data *vd )
{
    double tv,dt;
    double  lt, lhit, hit, lmiss, miss, hr, mr, ratio, up;
    char tmp[128];
      const struct VSC_C_main *VSC_C_main;
    tv = VTIM_mono();
    dt = tv - lt;
    lt = tv;
VSC_C_main = VSC_Main(vd,NULL);
    hit = VSC_C_main->cache_hit;
    miss = VSC_C_main->cache_miss;
    hr = (hit - lhit) / lt;
    mr = (miss - lmiss) / lt;
    lhit = hit;
    lmiss = miss;
    if (hr + mr != 0)	{
	    ratio = (hr / (hr + mr)) * 100;
	}
    else ratio = 0;
    up = VSC_C_main->uptime;
    sprintf (tmp, "\t\"hitrate\": %.2f,\n", ratio);
    STRCAT (iter->p, tmp, iter->cpy_ctx);
    sprintf (tmp, "\t\"load\": %.0f,\n", (VSC_C_main->client_req / up));
    STRCAT (iter->p, tmp, iter->cpy_ctx);
    return (0);
}
/////////////////////////////////////////////////////////
int
json_status (void *priv, const struct VSC_point *const pt)
{
    char tmp[128];
    struct iter_priv *iter = priv;
    uint64_t val;
    const struct VSC_section *sec;

    if (pt == NULL)
	return (0);
   
    val = *(const volatile uint64_t *)pt->ptr;
    sec = pt->section;
    
    if (iter->jp)
	iter->jp = 0;
    else
	STRCAT (iter->p, ",\n", iter->cpy_ctx);
    STRCAT (iter->p, "\t\"", iter->cpy_ctx);
    if (strcmp (sec->fantom->type, ""))
	{
	    STRCAT (iter->p, sec->fantom->type, iter->cpy_ctx);
	    STRCAT (iter->p, ".", iter->cpy_ctx);
	}
    if (strcmp (sec->fantom->ident, ""))
	{
	    STRCAT (iter->p, sec->fantom->ident, iter->cpy_ctx);
	    STRCAT (iter->p, ".", iter->cpy_ctx);
	}
    STRCAT (iter->p, pt->desc->name, iter->cpy_ctx);
    STRCAT (iter->p, "\": {", iter->cpy_ctx);
    if (strcmp (sec->fantom->type, ""))
	{
	    STRCAT (iter->p, "\"type\": \"", iter->cpy_ctx);
	    STRCAT (iter->p, sec->fantom->type, iter->cpy_ctx);
	    STRCAT (iter->p, "\", ", iter->cpy_ctx);
	}
    if (strcmp (sec->fantom->ident, ""))
	{
	    STRCAT (iter->p, "\"ident\": \"", iter->cpy_ctx);
	    STRCAT (iter->p, sec->fantom->ident, iter->cpy_ctx);
	    STRCAT (iter->p, "\", ", iter->cpy_ctx);
	}
    STRCAT (iter->p, "\"descr\": \"", iter->cpy_ctx);
    STRCAT (iter->p, pt->desc->sdesc, iter->cpy_ctx);
    STRCAT (iter->p, "\", ", iter->cpy_ctx);
    sprintf (tmp, "\"value\": %" PRIu64 "}", val);
    STRCAT (iter->p, tmp, iter->cpy_ctx);
    if (iter->jp)
    STRCAT (iter->p, "\n", iter->cpy_ctx);
    return (0);
    }
///////////////////////////////////////////////////////
int
run_subroutine (struct iter_priv *iter, struct VSM_data *vd)
{
    STRCAT (iter->p, "{\n", iter->cpy_ctx);
    rate (iter,vd);
    general_info (iter);
    //backend (iter);
    (void)VSC_Iter(vd, NULL, json_status, iter);
    STRCAT (iter->p, "\n}\n", iter->cpy_ctx);
    return (0);
    }
///////////////////////////////////////////////////////
VCL_STRING
vmod_rtstatus (const struct vrt_ctx *ctx)
{
    struct iter_priv iter = { 0 };
    struct tm t_time;
    struct VSM_data *vd;
    // struct VSM_fantom f_main = VSM_FANTOM_NULL;
    const struct VSC_C_main *VSC_C_main;
    vd = VSM_New();
    // VSC_Setup(vd);
    if (VSM_Open(vd)){
	    //WSL (ctx->wrk, SLT_Error, sp->fd, "VSC can't be opened.");
	    VSM_Delete (vd);
	    //return "";
	    exit(1);
	    }
    iter.time_stamp = VRT_TIME_string (ctx, iter.time);
    WS_Reserve (ctx->ws, 0);
    iter.p = ctx->ws->f;
    *(iter.p) = 0;
    iter.cpy_ctx = ctx;
    VSC_C_main =  VSC_Main(vd,NULL);
    run_subroutine (&iter,vd);
   
   VSM_Delete (vd);
    WS_Release (ctx->ws, strlen (iter.p) + 1);
    return (iter.p);
}

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>

#include "vrt.h"
#include "vrt_obj.h"
#include "vapi/vsc.h"
#include "vapi/vsm.h"
#include "vmod_rtstatus.h"

struct ws {
	unsigned		magic;
#define WS_MAGIC		0x35fac554
	char			id[4];		/* identity */
	char			*s;		/* (S)tart of buffer */
	char			*f;		/* (F)ree/front pointer */
	char			*r;		/* (R)eserved length */
	char			*e;		/* (E)nd of buffer */
};


int
init_function(struct vmod_priv *priv, const struct VCL_conf *conf)
{
	return(0);
}
///////////////////////////////////////////////////////
int
rate(struct iter_priv *iter,struct VSM_data *vd )
{
	struct timeval tv;
	double  lt, tt, lhit, hit, lmiss, miss, hr, mr, ratio, up;
	char tmp[128];
	const struct VSC_C_main *VSC_C_main;

	gettimeofday(&tv, NULL);
	tt = tv.tv_usec * 1e-6 + tv.tv_sec;
	lt = tt - lt;
	VSC_C_main = VSC_Main(vd,NULL);
	hit = VSC_C_main->cache_hit;
	miss = VSC_C_main->cache_miss;
	hr = (hit - lhit) / lt;
	mr = (miss - lmiss) / lt;
	lhit = hit;
	lmiss = miss;
	if (hr + mr != 0) {
		ratio = (hr / (hr + mr)) * 100;
	}
	else ratio = 0;

	up = VSC_C_main->uptime;
	sprintf(tmp, "\t\"uptime\" : %.0f,\n", up);
	STRCAT(iter->p, tmp, iter->cpy_ctx);
	sprintf(tmp, "\t\"hitrate\": %.2f,\n", ratio);
	STRCAT(iter->p, tmp, iter->cpy_ctx);
	sprintf(tmp, "\t\"load\": %.0f,\n", (VSC_C_main->client_req / up));
	STRCAT(iter->p, tmp, iter->cpy_ctx);
	return(0);
}
/////////////////////////////////////////////////////////
int
json_status(void *priv, const struct VSC_point *const pt)
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
		STRCAT(iter->p, ",\n", iter->cpy_ctx);
	STRCAT(iter->p, "\t\"", iter->cpy_ctx);
	if (strcmp(sec->fantom->type, "")) {
		STRCAT(iter->p, sec->fantom->type, iter->cpy_ctx);
		STRCAT(iter->p, ".", iter->cpy_ctx);
	}
	if (strcmp(sec->fantom->ident, "")) {
		STRCAT(iter->p, sec->fantom->ident, iter->cpy_ctx);
		STRCAT(iter->p, ".", iter->cpy_ctx);
	}
	STRCAT(iter->p, pt->desc->name, iter->cpy_ctx);
	STRCAT(iter->p, "\": {", iter->cpy_ctx);
	if (strcmp(sec->fantom->type, "")) {
		STRCAT(iter->p, "\"type\": \"", iter->cpy_ctx);
		STRCAT(iter->p, sec->fantom->type, iter->cpy_ctx);
		STRCAT(iter->p, "\", ", iter->cpy_ctx);
	}
	if (strcmp(sec->fantom->ident, "")) {
		STRCAT(iter->p, "\"ident\": \"", iter->cpy_ctx);
		STRCAT(iter->p, sec->fantom->ident, iter->cpy_ctx);
		STRCAT(iter->p, "\", ", iter->cpy_ctx);
	}
	STRCAT(iter->p, "\"descr\": \"", iter->cpy_ctx);
	STRCAT(iter->p, pt->desc->sdesc, iter->cpy_ctx);
	STRCAT(iter->p, "\", ", iter->cpy_ctx);
	sprintf(tmp, "\"value\": %" PRIu64 "}", val);
	STRCAT(iter->p, tmp, iter->cpy_ctx);
	if (iter->jp)
		STRCAT(iter->p, "\n", iter->cpy_ctx);
	return(0);
}
////////////////////////////////////////////////////////
int
run_subroutine(struct iter_priv *iter, struct VSM_data *vd)
{
        STRCAT(iter->p, "{\n", iter->cpy_ctx);
	rate(iter, vd);
	general_info(iter);
	backend(iter);
	(void)VSC_Iter(vd, NULL, json_status, iter);
	STRCAT(iter->p, "\n}\n", iter->cpy_ctx);
	return(0);
}
///////////////////////////////////////////////////////
VCL_STRING
vmod_rtstatus(const struct vrt_ctx *ctx)
{
	struct iter_priv iter = { 0 };
	struct tm t_time;
	struct VSM_data *vd;

	const struct VSC_C_main *VSC_C_main;
	vd = VSM_New();

	if (VSM_Open(vd)) {
	    VSM_Error(vd);
	    VSM_Delete(vd);
	    return "";
	}
	WS_Reserve(ctx->ws, 0);
	iter.p = ctx->ws->f;
	*(iter.p) = 0;
	iter.cpy_ctx = ctx;

	iter.jp = 1;
	VSC_C_main = VSC_Main(vd, NULL);

	run_subroutine(&iter, vd);

	VSM_Delete(vd);
	WS_Release(ctx->ws, strlen(iter.p) + 1);
	return(iter.p);
}

#include <inttypes.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <time.h>

#include "vapi/vsc.h"
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
rate(struct iter_priv *iter, struct VSM_data *vd)
{
	double tt, hr, mr, ratio;
	const struct VSC_C_main *VSC_C_main;
	struct timeval tv;
	time_t up;

	gettimeofday(&tv, NULL);
	tt = tv.tv_usec * 1e-6 + tv.tv_sec;

	VSC_C_main = VSC_Main(vd,NULL);
	hr = VSC_C_main->cache_hit / tt;
	mr = VSC_C_main->cache_miss / tt;
	if (hr + mr != 0) {
		ratio = (hr / (hr + mr)) * 100;
	}
	else ratio = 0.0;
	up = VSC_C_main->uptime;

	VSB_printf(iter->vsb, "\t\"uptime\" : \"%d+%02d:%02d:%02d\",\n",
	    (int)up / 86400, (int)(up % 86400) / 3600,
	    (int)(up % 3600) / 60, (int)up % 60);
	VSB_printf(iter->vsb, "\t\"uptime_sec\": %.2f,\n", (double) up);
	VSB_printf(iter->vsb, "\t\"hitrate\": %.2f,\n", ratio);
	VSB_printf(iter->vsb, "\t\"load\": %.0f,\n",
	    (VSC_C_main->client_req / (double) up));
	return(0);
}

int
json_status(void *priv, const struct VSC_point *const pt)
{
	struct iter_priv *iter = priv;
	const struct VSC_section *sec;
	char tmp[128];
	uint64_t val;

	if (pt == NULL)
		return (0);

	val = *(const volatile uint64_t *)pt->ptr;
	sec = pt->section;

	if (iter->jp)
		iter->jp = 0;
	else
		VSB_cat(iter->vsb, ",\n");
		VSB_cat(iter->vsb, "\t\"");
	if (strcmp(sec->fantom->type, "")) {
		VSB_cat(iter->vsb, sec->fantom->type);
		VSB_cat(iter->vsb, ".");
	}
	if (strcmp(sec->fantom->ident, "")) {
		VSB_cat(iter->vsb, sec->fantom->ident);
		VSB_cat(iter->vsb, ".");
	}
	VSB_cat(iter->vsb, pt->desc->name);
	VSB_cat(iter->vsb, "\": {");
	if (strcmp(sec->fantom->type, "")) {
		VSB_cat(iter->vsb, "\"type\": \"");
		VSB_cat(iter->vsb, sec->fantom->type);
		VSB_cat(iter->vsb, "\", ");
	}
	if (strcmp(sec->fantom->ident, "")) {
		VSB_cat(iter->vsb, "\"ident\": \"");
		VSB_cat(iter->vsb, sec->fantom->ident);
		VSB_cat(iter->vsb, "\", ");
	}
	VSB_cat(iter->vsb, "\"descr\": \"");
	VSB_cat(iter->vsb, pt->desc->sdesc);
	VSB_cat(iter->vsb, "\", ");
	VSB_printf(iter->vsb,"\"value\": %" PRIu64 "}", val);
	if (iter->jp)
		VSB_cat(iter->vsb, "\n");
	return(0);
}

int
run_subroutine(struct iter_priv *iter, struct VSM_data *vd)
{
	VSB_cat(iter->vsb, "{\n");
	rate(iter, vd);
	general_info(iter);
	backend(iter);
	(void)VSC_Iter(vd, NULL, json_status, iter);
	VSB_cat(iter->vsb, "\n}\n");
	return(0);
}

#include <inttypes.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <time.h>

#include "vmod_rtstatus.h"
#include "vapi/vsc.h"
#include "vrt.h"
#include "vtim.h"
#include "vcl.h"

void
rate(struct rtstatus_priv *rtstatus, struct VSM_data *vd)
{
	double ratio;
	struct VSC_C_main *VSC_C_main;
	uint64_t hit, miss;
	time_t up;
	int req;

	VSC_C_main = VSC_Main(vd, NULL);
        if (VSC_C_main == NULL)
                return;

        hit = VSC_C_main->cache_hit;
        miss = VSC_C_main->cache_miss;

        if (hit + miss != 0)
                ratio = (double)hit / (hit + miss);
        else
                ratio = 0;

	up = VSC_C_main->uptime;
	req = VSC_C_main->client_req;

	VSB_printf(rtstatus->vsb, "\t\"uptime\" : \"%d+%02d:%02d:%02d\",\n",
	    (int)up / 86400, (int)(up % 86400) / 3600,
	    (int)(up % 3600) / 60, (int)up % 60);
	VSB_printf(rtstatus->vsb, "\t\"uptime_sec\": %.2f,\n", (double)up);
	VSB_printf(rtstatus->vsb, "\t\"hitrate\": %.2f,\n", ratio * 100);
	VSB_printf(rtstatus->vsb, "\t\"load\": %.2f,\n", (double)req / up);
}

int
json_stats(void *priv, const struct VSC_point *const pt)
{
	struct rtstatus_priv *rtstatus = priv;
	const struct VSC_section *sec;
	uint64_t val;

	if (pt == NULL)
		return (0);

	val = *(const volatile uint64_t *)pt->ptr;
	sec = pt->section;

	if (rtstatus->jp)
		rtstatus->jp = 0;
	else
		VSB_cat(rtstatus->vsb, ",\n");
		VSB_cat(rtstatus->vsb, "\t\"");
	if (strcmp(sec->fantom->type, "")) {
		VSB_cat(rtstatus->vsb, sec->fantom->type);
		VSB_cat(rtstatus->vsb, ".");
	}
	if (strcmp(sec->fantom->ident, "")) {
		VSB_cat(rtstatus->vsb, sec->fantom->ident);
		VSB_cat(rtstatus->vsb, ".");
	}
	VSB_cat(rtstatus->vsb, pt->desc->name);
	VSB_cat(rtstatus->vsb, "\": {");
	if (strcmp(sec->fantom->type, "")) {
		VSB_cat(rtstatus->vsb, "\"type\": \"");
		VSB_cat(rtstatus->vsb, sec->fantom->type);
		VSB_cat(rtstatus->vsb, "\", ");
	}
	if (strcmp(sec->fantom->ident, "")) {
		VSB_cat(rtstatus->vsb, "\"ident\": \"");
		VSB_cat(rtstatus->vsb, sec->fantom->ident);
		VSB_cat(rtstatus->vsb, "\", ");
	}
	VSB_cat(rtstatus->vsb, "\"descr\": \"");
	VSB_cat(rtstatus->vsb, pt->desc->sdesc);
	VSB_cat(rtstatus->vsb, "\", ");
	VSB_printf(rtstatus->vsb,"\"value\": %" PRIu64 "}", val);
	if (rtstatus->jp)
		VSB_cat(rtstatus->vsb, "\n");
	return(0);
}

int
be_info(void *priv, const struct VSC_point *const pt)
{
	struct rtstatus_priv *rtstatus = priv;
	const struct VSC_section *sec;
	uint64_t val;

	if (pt == NULL)
		return (0);

	val = *(const volatile uint64_t *)pt->ptr;
	sec = pt->section;

	if (!strcmp(sec->fantom->type,"MAIN")) {
		if (!strcmp(pt->desc->name, "n_backend"))
			n_be = (int)val;
	}

	if (!strcmp(sec->fantom->type, "VBE")) {
		if (!strcmp(pt->desc->name, "happy"))
			be_happy = val;
		if(!strcmp(pt->desc->name, "bereq_hdrbytes"))
			bereq_hdr = val;
		if(!strcmp(pt->desc->name, "bereq_bodybytes")) {
			bereq_body = val;
			VSB_cat(rtstatus->vsb, "{\"server_name\":\"");
			VSB_cat(rtstatus->vsb, pt->section->fantom->ident);
			VSB_printf(rtstatus->vsb,"\", \"happy\": %" PRIu64,
			    be_happy);
			VSB_printf(rtstatus->vsb,", \"bereq_tot\": %" PRIu64 ",",
			    bereq_body + bereq_hdr);
		}
		if(!strcmp(pt->desc->name, "beresp_hdrbytes"))
			beresp_hdr = val;
		if(!strcmp(pt->desc->name, "beresp_bodybytes")) {
			beresp_body = val;
			VSB_printf(rtstatus->vsb,"\"beresp_tot\": %" PRIu64 "}",
			    beresp_body + beresp_hdr);

			if(cont < (n_be -1)) {
				VSB_cat(rtstatus->vsb, ",\n\t\t");
				cont++;
			}
		}
	}

	return(0);
}
int
collect_info(struct rtstatus_priv *rtstatus, struct VSM_data *vd)
{
	VSB_cat(rtstatus->vsb, "{\n");
	rate(rtstatus, vd);
	general_info(rtstatus);
	VSB_cat(rtstatus->vsb, "\t\"be_info\": [");
	(void)VSC_Iter(vd, NULL, be_info, rtstatus);
	VSB_cat(rtstatus->vsb, "],\n");
	cont = 0;
	(void)VSC_Iter(vd, NULL, json_stats, rtstatus);
	VSB_cat(rtstatus->vsb, "\n}\n");

	return(0);
}


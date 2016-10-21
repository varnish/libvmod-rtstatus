
#include <inttypes.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#include "vmod_rtstatus.h"
#include "vapi/vsc.h"
#include "vrt.h"
#include "vtim.h"
#include "vcl.h"
#include "vcs.h"

#include "vas.h"
#include "miniobj.h"


void
rate(struct rtstatus_priv *rs, struct VSM_data *vd)
{
	struct VSC_C_main *VSC_C_main;
	double ratio;
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

	VSB_printf(rs->vsb, "\"uptime\": \"%d+%02d:%02d:%02d\",\n",
	    (int)up / 86400, (int)(up % 86400) / 3600,
	    (int)(up % 3600) / 60, (int)up % 60);
	VSB_printf(rs->vsb, "\"uptime_sec\": %.2f,\n", (double)up);
        VSB_printf(rs->vsb, "\"absolute_hitrate\": %.2f,\n", ratio * 100);
	VSB_printf(rs->vsb, "\"avg_hitrate\": %.2f,\n", (ratio * 100) / up);
	VSB_printf(rs->vsb, "\"avg_load\": %.2f,\n", (double)req / up);
}


int
stats_cb(void *priv, const struct VSC_point *const pt)
{
	const struct VSC_section *sec;
	struct rtstatus_priv *rs;
	uint64_t val;

	CAST_OBJ_NOTNULL(rs, priv, VMOD_RTSTATUS_MAGIC);

	if (pt == NULL)
		return(0);

	val = *(const volatile uint64_t *)pt->ptr;
	sec = pt->section;

	VSB_cat(rs->vsb, "\"");
	if (strcmp(sec->fantom->type, "")) {
		VSB_cat(rs->vsb, sec->fantom->type);
		VSB_cat(rs->vsb, ".");
	}
	if (strcmp(sec->fantom->ident, "")) {
		VSB_cat(rs->vsb, sec->fantom->ident);
		VSB_cat(rs->vsb, ".");
	}
	VSB_cat(rs->vsb, pt->desc->name);
	VSB_cat(rs->vsb, "\": {");
	VSB_indent(rs->vsb, 4);
	if (strcmp(sec->fantom->type, "")) {
		VSB_cat(rs->vsb, "\"type\": \"");
		VSB_cat(rs->vsb, sec->fantom->type);
		VSB_cat(rs->vsb, "\", ");
	}
	if (strcmp(sec->fantom->ident, "")) {
		VSB_cat(rs->vsb, "\"ident\": \"");
		VSB_cat(rs->vsb, sec->fantom->ident);
		VSB_cat(rs->vsb, "\", ");
	}
	VSB_cat(rs->vsb, "\"descr\": \"");
	VSB_cat(rs->vsb, pt->desc->sdesc);
	VSB_cat(rs->vsb, "\", ");
	VSB_printf(rs->vsb,"\"value\": %" PRIu64 "},\n", val);
	VSB_indent(rs->vsb, -4);

	return(0);
}


int
backend_cb(void *priv, const struct VSC_point *const pt)
{
	const struct VSC_section *sec;
	struct rtstatus_priv *rs;
	uint64_t val;

	if (pt == NULL)
		return(0);

	CAST_OBJ(rs, priv, VMOD_RTSTATUS_MAGIC);

	val = *(const volatile uint64_t *)pt->ptr;
	sec = pt->section;

	if (!strcmp(sec->fantom->type, "VBE")) {
		if (!strcmp(pt->desc->name, "happy"))
			be_happy = (val % 2);
		if (!strcmp(pt->desc->name, "bereq_hdrbytes"))
			bereq_hdr = val;
		if (!strcmp(pt->desc->name, "bereq_bodybytes")) {
			bereq_body = val;
			VSB_cat(rs->vsb, "{\"server_name\": \"");
			VSB_cat(rs->vsb, pt->section->fantom->ident);
			VSB_printf(rs->vsb,"\", \"happy\": \"%s\"" , be_happy ? "healthy" : "sick");
			VSB_printf(rs->vsb,", \"bereq_tot\": %" PRIu64 ", ",
			    bereq_body + bereq_hdr);
		}

                if (!strcmp(pt->desc->name, "beresp_hdrbytes"))
			beresp_hdr = val;
                if (!strcmp(pt->desc->name, "beresp_bodybytes")) {
			beresp_body = val;
			VSB_printf(rs->vsb,"\"beresp_tot\": %" PRIu64 ", ",
			    beresp_body + beresp_hdr);
		}

		if (!strcmp(pt->desc->name, "pipe_hdrbytes"))
			VSB_printf(rs->vsb,"\"pipe_hdrbytes\": %" PRIu64 ", ", val);

		if (!strcmp(pt->desc->name, "pipe_out"))
			VSB_printf(rs->vsb,"\"pipe_out\": %" PRIu64 ", ", val);

                if (!strcmp(pt->desc->name, "pipe_in"))
			VSB_printf(rs->vsb,"\"pipe_in\": %" PRIu64 ", ", val);

                if (!strcmp(pt->desc->name, "conn"))
			VSB_printf(rs->vsb,"\"conn\": %" PRIu64 ", ", val);

		if (!strcmp(pt->desc->name, "req")) {
			VSB_printf(rs->vsb,"\"req\": %" PRIu64 "},\n", val);
		}
	}

	return(0);
}

int
collect_info(struct rtstatus_priv *rs, struct VSM_data *vd)
{
	char vrt_hostname[255];

	CHECK_OBJ_NOTNULL(rs, VMOD_RTSTATUS_MAGIC);
	AN(vd);
	AN(rs->vsb);

	VSB_cat(rs->vsb, "{\n");
	VSB_indent(rs->vsb, 4);

	rate(rs, vd);

	VSB_cat(rs->vsb, "\"varnish_version\": \"");
	VSB_cat(rs->vsb, VCS_version);
	VSB_cat(rs->vsb, "\",\n");
	gethostname(vrt_hostname, sizeof(vrt_hostname));

	VSB_cat(rs->vsb, "\"server_id\": \"");
	VSB_cat(rs->vsb, vrt_hostname);
	VSB_cat(rs->vsb, "\",\n");

	VSB_cat(rs->vsb, "\"be_info\": [\n");
	VSB_indent(rs->vsb, 4);

	(void)VSC_Iter(vd, NULL, backend_cb, rs);
	rs->vsb->s_len -= 2;  /* dirty */
	VSB_indent(rs->vsb, -4);
	VSB_cat(rs->vsb, "\n],\n");

	(void)VSC_Iter(vd, NULL, stats_cb, rs);
	rs->vsb->s_len -= 2;  /* dirty */

	VSB_indent(rs->vsb, -4);
	VSB_cat(rs->vsb, "\n}\n");

	return(0);
}

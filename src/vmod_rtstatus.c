/*
 * Copyright (c) 2017 Varnish Software AS
 * All rights reserved.
 *
 * Author: Arianna Aondio <aondio@varnish-software.com>
 * Author: Dridi Boukelmoune <dridi@varnish-software.com>
 */

#include <sys/time.h>

#include <inttypes.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "cache/cache.h"

#include "miniobj.h"
#include "vas.h"
#include "vcl.h"
#include "vcs.h"
#include "vrt_obj.h"
#include "vsb.h"
#include "vtim.h"

#include "common/heritage.h"
#include "vapi/vsc.h"
#include "vapi/vsm.h"
#include "vsm_priv.h" /* NB: private is OK for bundled VMODs. */

#include "vcc_if.h"
#include "vmod_rtstatus_html.h"

struct rtstatus_priv {
	unsigned 	magic;
#define VMOD_RTSTATUS_MAGIC 0x98b584a
	struct vsb	*vsb;
	const char	*name;
	int		name_len;
	uint64_t	up;
	uint64_t	hit;
	uint64_t	miss;
	uint64_t	req;
	uint64_t	beresp_hdr;
	uint64_t	beresp_body;
	uint64_t	bereq_hdr;
	uint64_t	bereq_body;
	uint64_t	be_happy;
};

static const struct gethdr_s rststatus_content_type =
    { HDR_RESP, "\015Content-Type:"};


/*--------------------------------------------------------------------*/

static int
rtstatus_rate_cb(void *priv, const struct VSC_point * const pt)
{
	struct rtstatus_priv *rs;

	CAST_OBJ_NOTNULL(rs, priv, VMOD_RTSTATUS_MAGIC);

	if (pt == NULL)
		return (0);

	if (!strcmp(pt->name, "MAIN.uptime")) {
		rs->up = *(const volatile uint64_t*)pt->ptr;
		rs->name_len++;
	} else if (!strcmp(pt->name, "MAIN.cache_hit")) {
		rs->hit = *(const volatile uint64_t*)pt->ptr;
		rs->name_len++;
	} else if (!strcmp(pt->name, "MAIN.cache_miss")) {
		rs->miss = *(const volatile uint64_t*)pt->ptr;
		rs->name_len++;
	} else if (!strcmp(pt->name, "MAIN.client_req")) {
		rs->req = *(const volatile uint64_t*)pt->ptr;
		rs->name_len++;
	}

	return (rs->name_len == 4);
}

static void
rtstatus_print_rate(struct rtstatus_priv *rs)
{
	double ratio, total;
	unsigned up;

	total = rs->hit + rs->miss;
        if (total != 0)
                ratio = rs->hit / total;
        else
                ratio = 0;

	up = rs->up;
	VSB_printf(rs->vsb, "\"uptime\": \"%u+%02u:%02u:%02u\",\n",
	    up / 86400, (up % 86400) / 3600, (up % 3600) / 60, up % 60);
	VSB_printf(rs->vsb, "\"uptime_sec\": %u,\n", up);
        VSB_printf(rs->vsb, "\"absolute_hitrate\": %.2f,\n", ratio * 100);

	if (up == 0) {
		VSB_cat(rs->vsb, "\"avg_hitrate\": null,\n");
		VSB_cat(rs->vsb, "\"avg_load\": null,\n");
	} else {
		VSB_printf(rs->vsb, "\"avg_hitrate\": %.2f,\n", (ratio * 100) / up);
		VSB_printf(rs->vsb, "\"avg_load\": %.2f,\n", (double)rs->req / up);
	}
}

static int
rtstatus_stats_cb(void *priv, const struct VSC_point *const pt)
{
	struct rtstatus_priv *rs;
	const char *type, *ident;
	uint64_t val;

	CAST_OBJ_NOTNULL(rs, priv, VMOD_RTSTATUS_MAGIC);

	if (pt == NULL)
		return (0);

	type = pt->name;
	ident = strchr(pt->name, '.');
	XXXAN(ident);
	val = *(const volatile uint64_t *)pt->ptr;

	if (rs->name_len > 0)
		VSB_cat(rs->vsb, ",\n");
	rs->name_len = 1; /* NB: no need to compute an actual strlen. */

	VSB_printf(rs->vsb,
	    "\"%s\": {\"type\": \"%.*s\", \"ident\": \"%s\", "
	    "\"descr\": \"%s\", \"value\": %" PRIu64 "}",
	    pt->name, (int)(ident - type), type, ident + 1, pt->sdesc, val);

	return (0);
}

static int
rtstatus_backend_cb(void *priv, const struct VSC_point *const pt)
{
	struct rtstatus_priv *rs;
	const char *be, *cnt;
	uint64_t val;
	int len;

	CAST_OBJ(rs, priv, VMOD_RTSTATUS_MAGIC);

	if (pt == NULL || strncmp(pt->name, "VBE.", 4))
		return (0);

	val = *(const volatile uint64_t *)pt->ptr;
	be = pt->name + 4;
	cnt = strrchr(be, '.');
	XXXAN(cnt);
	cnt++;
	len = cnt - be;

	if (len != rs->name_len || strncmp(be, rs->name, len)) {
		if (rs->name_len > 0)
			VSB_cat(rs->vsb, "},\n");
		rs->name = be;
		rs->name_len = cnt - be;
		VSB_printf(rs->vsb, "{\"server_name\": \"%.*s\"",
		    len - 1, be);
	}

	VSB_printf(rs->vsb, ", \"%s\": %" PRIu64, cnt, val);
	return (0);
}

static int
rtstatus_collect(struct rtstatus_priv *rs, struct vsm *vd)
{
	char vrt_hostname[255];
	struct vsc *vsc;

	CHECK_OBJ_NOTNULL(rs, VMOD_RTSTATUS_MAGIC);
	AN(rs->vsb);
	AN(vd);

	vsc = VSC_New();
	AN(vsc);

	VSB_cat(rs->vsb, "{\n");
	VSB_indent(rs->vsb, 4);

	rs->name_len = 0;
	(void)VSC_Iter(vsc, vd, rtstatus_rate_cb, rs);
	rtstatus_print_rate(rs);

	VSB_printf(rs->vsb, "\"varnish_version\": \"%s\",\n", VCS_String("V"));

	gethostname(vrt_hostname, sizeof vrt_hostname);
	VSB_printf(rs->vsb, "\"server_id\": \"%s\",\n", vrt_hostname);

	VSB_cat(rs->vsb, "\"be_info\": [\n");
	VSB_indent(rs->vsb, 4);
	rs->name_len = 0;
	(void)VSC_Iter(vsc, vd, rtstatus_backend_cb, rs);
	VSB_cat(rs->vsb, "}\n");
	VSB_indent(rs->vsb, -4);
	VSB_cat(rs->vsb, "],\n");

	rs->name_len = 0;
	(void)VSC_Iter(vsc, vd, rtstatus_stats_cb, rs);

	VSB_indent(rs->vsb, -4);
	VSB_cat(rs->vsb, "\n}\n");

	VSC_Destroy(&vsc, vd);

	return (0);
}

/*--------------------------------------------------------------------*/

VCL_VOID
vmod_synthetic_json(VRT_CTX)
{
	struct rtstatus_priv rs;
	struct vsm *vd;

	if (ctx->method != VCL_MET_SYNTH) {
		VRT_fail(ctx, "rtstatus: can only be used in vcl_synth");
		return;
	}

	INIT_OBJ(&rs, VMOD_RTSTATUS_MAGIC);
	vd = VSM_New();
	AN(vd);

	/* XXX: there is currently no n_arg in heritage */
	if (VSM_Arg(vd, 'n', heritage.identity) < 0 || VSM_Attach(vd, -1)) {
		VSM_Destroy(&vd);
		VRT_fail(ctx, "rtstatus: can't open VSM for %s",
		    heritage.identity);
		return;
	}

	rs.vsb = ctx->specific;
	rtstatus_collect(&rs, vd);
	VSM_Destroy(&vd);
	VRT_SetHdr(ctx, &rststatus_content_type,
	    "application/json; charset=utf-8", vrt_magic_string_end);
}

VCL_VOID
vmod_synthetic_html(VRT_CTX)
{

	if (ctx->method != VCL_MET_SYNTH) {
		VRT_fail(ctx, "rtstatus: can only be used in vcl_synth");
		return;
	}

	AN(ctx->specific);
	VSB_cat(ctx->specific, html);
	VRT_SetHdr(ctx, &rststatus_content_type, "text/html; charset=utf-8",
	    vrt_magic_string_end);
}

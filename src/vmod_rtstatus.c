#include <inttypes.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <time.h>

#include "vapi/vsc.h"
#include "vmod_rtstatus.h"
uint64_t  beresp_hdr, beresp_body;
uint64_t  bereq_hdr, bereq_body;
static struct hitrate hitrate;
static struct load load;
int n_be, cont;


double
VTIM_mono(void)
{
#ifdef HAVE_GETHRTIME
        return (gethrtime() * 1e-9);
#elif  HAVE_CLOCK_GETTIME
        struct timespec ts;

        clock_gettime(CLOCK_MONOTONIC, &ts);
        return (ts.tv_sec + 1e-9 * ts.tv_nsec);
#else
        struct timeval tv;

        gettimeofday(&tv, NULL);
        return (tv.tv_sec + 1e-6 * tv.tv_usec);
#endif
}

int
init_function(struct vmod_priv *priv, const struct VCL_conf *conf)
{
	memset(&hitrate, 0, sizeof(struct hitrate));
	memset(&load, 0, sizeof(struct load));
	beresp_hdr = beresp_body = 0;
	bereq_hdr = bereq_body = 0;
	n_be = 0;
	cont = 0;
	return (0);
}

static void
update_counter(struct counter *counter, double val)
{
        if (counter->n < counter->nmax)
                counter->n++;
        counter->acc += (val - counter->acc) / (double)counter->n;
}

void
rate(struct iter_priv *iter, struct VSM_data *vd)
{
	double hr, mr, ratio, tv, dt, reqload;
	struct VSC_C_main *VSC_C_main;
	uint64_t hit, miss;
	time_t up;
	int req;

	VSC_C_main = VSC_Main(vd, NULL);
        if (VSC_C_main == NULL)
                return;

        tv = VTIM_mono();
        dt = tv - hitrate.tm;
        hitrate.tm = tv;

        hit = VSC_C_main->cache_hit;
        miss = VSC_C_main->cache_miss;
        hr = (hit - hitrate.hit) / dt;
        mr = (miss - hitrate.miss) / dt;
        hitrate.hit = hit;
        hitrate.miss = miss;

        if (hr + mr != 0)
                ratio = hr / (hr + mr);
        else
                ratio = 0;

	up = VSC_C_main->uptime;
	req = VSC_C_main->client_req;
	reqload  =  ((req - load.req) / dt);
	load.req = req;

	update_counter(&hitrate.hr, ratio);
	update_counter(&load.rl, reqload);

	VSB_printf(iter->vsb, "\t\"uptime\" : \"%d+%02d:%02d:%02d\",\n",
	    (int)up / 86400, (int)(up % 86400) / 3600,
	    (int)(up % 3600) / 60, (int)up % 60);
	VSB_printf(iter->vsb, "\t\"uptime_sec\": %.2f,\n", (double)up);
	VSB_printf(iter->vsb, "\t\"hitrate\": %.2f,\n", hitrate.hr.acc * 100);
	VSB_printf(iter->vsb, "\t\"load\": %.2f,\n", load.rl.acc);
	VSB_printf(iter->vsb, "\t\"delta\": %.2f,\n", iter->delta);
}

int
json_status(void *priv, const struct VSC_point *const pt)
{
	struct iter_priv *iter = priv;
	const struct VSC_section *sec;
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
creepy_math(void *priv, const struct VSC_point *const pt)
{
	struct iter_priv *iter = priv;
	const struct VSC_section *sec;
	uint64_t val;

	if (pt == NULL)
		return (0);

	val = *(const volatile uint64_t *)pt->ptr;
	sec = pt->section;
	if(!strcmp(sec->fantom->type,"MAIN")){
		if(!strcmp(pt->desc->name, "n_backend")){
			n_be = (int)val;
		}
	}
	if (!strcmp(sec->fantom->type, "VBE")) {
		if(!strcmp(pt->desc->name, "bereq_hdrbytes"))
			bereq_hdr = val;
		if(!strcmp(pt->desc->name, "bereq_bodybytes")) {
			bereq_body = val;
			VSB_cat(iter->vsb, "{\"ident\":\"");
			VSB_cat(iter->vsb, pt->section->fantom->ident);
			VSB_printf(iter->vsb,"\", \"bereq_tot\": %" PRIu64 ",",
			    bereq_body + bereq_hdr);
		}

		if(!strcmp(pt->desc->name, "beresp_hdrbytes"))
			beresp_hdr = val;
		if(!strcmp(pt->desc->name, "beresp_bodybytes")) {
			beresp_body = val;
			VSB_printf(iter->vsb,"\"beresp_tot\": %" PRIu64 "}",
			    beresp_body + beresp_hdr);
			if(cont < (n_be -1)) {
				VSB_cat(iter->vsb, ",\n\t\t");
				cont++;
			}
		}
	}
	return(0);
}

int
run_subroutine(struct iter_priv *iter, struct VSM_data *vd)
{
	hitrate.hr.nmax = iter->delta;
	load.rl.nmax = iter->delta;
	VSB_cat(iter->vsb, "{\n");
	rate(iter, vd);
	general_info(iter);
	backend(iter);
	VSB_cat(iter->vsb, "\t\"be_bytes\": [");
	(void)VSC_Iter(vd, NULL, creepy_math, iter);
	VSB_cat(iter->vsb, "],\n");
	cont = 0;
	(void)VSC_Iter(vd, NULL, json_status, iter);
	VSB_cat(iter->vsb, "\n}\n");
	return(0);
}


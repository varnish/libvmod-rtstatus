#include <inttypes.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <time.h>

#include "vapi/vsc.h"
#include "vmod_rtstatus.h"

struct counter {
        unsigned n, nmax;
        double acc;
};

struct hitrate {
        double lt;
        uint64_t lhit, lmiss;
        struct counter hr_10;
};
static struct hitrate hitrate;
static struct hitrate load;

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

	hitrate.hr_10.acc = 0;
	hitrate.hr_10.n = 0;
	hitrate.lmiss = 0;
	hitrate.lhit = 0;
	load.hr_10.acc = 0;
	load.hr_10.n = 0;
	load.lhit = 0;
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
	struct VSC_C_main *VSC_C_main;
	double hr, mr, ratio, tv, dt;
	uint64_t hit, miss;
	time_t up;

	VSC_C_main = VSC_Main(vd, NULL);
        if (VSC_C_main == NULL)
                return;

        tv = VTIM_mono();
        dt = tv - hitrate.lt;
        hitrate.lt = tv;

        hit = VSC_C_main->cache_hit;
        miss = VSC_C_main->cache_miss;
        hr = (hit - hitrate.lhit) / dt;
        mr = (miss - hitrate.lmiss) / dt;
        hitrate.lhit = hit;
        hitrate.lmiss = miss;

        if (hr + mr != 0)
                ratio = hr / (hr + mr);
        else
                ratio = 0;

	up = VSC_C_main->uptime;
	int req = VSC_C_main->client_req;
	double reqload  =  ((req - load.lhit) / dt);
	load.lhit = req;

	update_counter(&hitrate.hr_10, ratio);
	update_counter(&load.hr_10, reqload);

	VSB_printf(iter->vsb, "\t\"uptime\" : \"%d+%02d:%02d:%02d\",\n",
	    (int)up / 86400, (int)(up % 86400) / 3600,
	    (int)(up % 3600) / 60, (int)up % 60);
	VSB_printf(iter->vsb, "\t\"uptime_sec\": %.2f,\n", (double) up);
	VSB_printf(iter->vsb, "\t\"hitrate\": %.2f,\n", hitrate.hr_10.acc*100);
	VSB_printf(iter->vsb, "\t\"load\": %.2f,\n", load.hr_10.acc);

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
run_subroutine(struct iter_priv *iter, struct VSM_data *vd)
{
	hitrate.hr_10.nmax = iter->delta;
	load.hr_10.nmax = iter->delta;
	VSB_cat(iter->vsb, "{\n");
	rate(iter, vd);
	general_info(iter);
	backend(iter);
	(void)VSC_Iter(vd, NULL, json_status, iter);
	VSB_cat(iter->vsb, "\n}\n");
	return(0);
}

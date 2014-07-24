#include "config.h"
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "vrt.h"
#include "bin/varnishd/cache.h"
#include "vcc_if.h"
#include "bin/varnishd/cache_backend.h"
#include "varnishapi.h"
#include "vsm.h"
#include "vcl.h"

#define STRCAT(dst, src, sp)					\
    do {							\
	dst = wsstrncat(dst, src, sp);				\
	if (!dst) {						\
	    WS_Release(sp->wrk->ws, 0);				\
	    WSL(sp->wrk, SLT_Error, sp->fd,			\
		"Running out of workspace in vmod_rtstatus."	\
		"Increase sess_workspace to fix this.");	\
	    return 1;						\
	}							\
    } while(0)							\


struct iter_priv
{
  char *p;
  struct sess *cpy_sp;
  char *time_stamp;
  struct vtc_job *jp;
};

int
init_function (struct vmod_priv *priv, const struct VCL_conf *conf)
{
  return (0);
}

//////////////////////////////////////////////////////////
static char *
wsstrncat (char *dest, const char *src, struct sess *sp)
{
  if (sp->wrk->ws->r <= sp->wrk->ws->f)
    {
      return (NULL);
    }
  return strcat (dest, src);
}

/////////////////////////////////////////////////////////
int
general_info (struct iter_priv *iter)
{
  STRCAT (iter->p, "\t\"Timestamp\" : \"", iter->cpy_sp);
  STRCAT (iter->p, iter->time_stamp, iter->cpy_sp);
  STRCAT (iter->p, "\",\n\t\"Varnish_Version\" : \"", iter->cpy_sp);
  STRCAT (iter->p, VCS_version, iter->cpy_sp);
  STRCAT (iter->p, "\",\n", iter->cpy_sp);
  return (0);
}

int
backend (struct iter_priv *iter)
{
  int i;
  int cont = 1;
  for (i = 1; i < iter->cpy_sp->vcl->ndirector; ++i)
    {
      CHECK_OBJ_NOTNULL (iter->cpy_sp->vcl->director[i], DIRECTOR_MAGIC);
      if (strcmp ("simple", iter->cpy_sp->vcl->director[i]->name) == 0)
	{
	  STRCAT (iter->p, "\t\"Backend\": ", iter->cpy_sp);
	  char buf[1024];
	  int j, healthy;

	  healthy =
	    VDI_Healthy (iter->cpy_sp->vcl->director[i], iter->cpy_sp);
	  j =
	    snprintf (buf, sizeof buf, "{\"name\":\"%s\", \"value\": \"%s\"}",
		      iter->cpy_sp->vcl->director[i]->vcl_name,
		      healthy ? "healthy" : "sick");
	  assert (j >= 0);
	  STRCAT (iter->p, buf, iter->cpy_sp);

	}
      STRCAT (iter->p, ",\n", iter->cpy_sp);
    }
  return (0);
}

////////////////////////////////////////////////////////
int
director (struct iter_priv *iter)
{
  STRCAT (iter->p, "\t\"Director\": {\"name\":\"", iter->cpy_sp);
  STRCAT (iter->p, iter->cpy_sp->director->name, iter->cpy_sp);
  STRCAT (iter->p, "\", \"vcl_name\":\"", iter->cpy_sp);
  STRCAT (iter->p, iter->cpy_sp->director->vcl_name, iter->cpy_sp);
  STRCAT (iter->p, "\"}", iter->cpy_sp);
  return (0);
}

////////////////////////////////////////////////////
static void
myexp (double *acc, double val, unsigned *n, unsigned nmax)
{
  if (*n < nmax)
    (*n)++;
  (*acc) += (val - *acc) / (double) *n;
}

///////////////////////////////////////////////////////
int
rate (struct iter_priv *iter)
{
  struct timeval tv;
  double tt, lt, lhit, hit, lmiss, miss, hr, mr, ratio, up;
  char tmp[128];

  AZ (gettimeofday (&tv, NULL));
  tt = tv.tv_usec * 1e-6 + tv.tv_sec;
  lt = tt - lt;

  hit = VSC_C_main->cache_hit;
  miss = VSC_C_main->cache_miss;
  hr = (hit - lhit) / lt;
  mr = (miss - lmiss) / lt;
  lhit = hit;
  lmiss = miss;
  if (hr + mr != 0)
    {
      ratio = (hr / (hr + mr)) * 100;
    }
  up = VSC_C_main->uptime;
  sprintf (tmp, "\t\"hitrate\": \"%8.0f \",\n", ratio);
  STRCAT (iter->p, tmp, iter->cpy_sp);
  sprintf (tmp, "\t\"load\": \"%.0f \",\n", (VSC_C_main->client_req / up));
  STRCAT (iter->p, tmp, iter->cpy_sp);
  return (0);
}

/////////////////////////////////////////////////////////
int
json_status (void *priv, const struct VSC_point *const pt)
{
  char tmp[128];
  struct iter_priv *iter = priv;
  uint64_t val;
  val = *(const volatile uint64_t *) pt->ptr;

  if (iter->jp)
    iter->jp = 0;
  else
    STRCAT (iter->p, ",\n", iter->cpy_sp);
  STRCAT (iter->p, "\t\"", iter->cpy_sp);
  if (strcmp (pt->class, ""))
    {
      STRCAT (iter->p, pt->class, iter->cpy_sp);
      STRCAT (iter->p, ".", iter->cpy_sp);
    }
  if (strcmp (pt->ident, ""))
    {
      STRCAT (iter->p, pt->ident, iter->cpy_sp);
      STRCAT (iter->p, ".", iter->cpy_sp);
    }
  STRCAT (iter->p, pt->name, iter->cpy_sp);
  // STRCAT (iter->p, "counter", iter->cpy_sp);
  STRCAT (iter->p, "\": {", iter->cpy_sp);
  if (strcmp (pt->class, ""))
    {
      STRCAT (iter->p, "\"type\": \"", iter->cpy_sp);
      STRCAT (iter->p, pt->class, iter->cpy_sp);
      STRCAT (iter->p, "\", ", iter->cpy_sp);
    }
  if (strcmp (pt->ident, ""))
    {
      STRCAT (iter->p, "\"ident\": \"", iter->cpy_sp);
      STRCAT (iter->p, pt->ident, iter->cpy_sp);
      STRCAT (iter->p, "\", ", iter->cpy_sp);
    }
  STRCAT (iter->p, "\"descr\": \"", iter->cpy_sp);
  STRCAT (iter->p, pt->desc, iter->cpy_sp);
  STRCAT (iter->p, "\", ", iter->cpy_sp);
  sprintf (tmp, "\"value\": %" PRIu64 "}", val);
  STRCAT (iter->p, tmp, iter->cpy_sp);
  if (iter->jp)
    STRCAT (iter->p, "\n", iter->cpy_sp);
  return (0);
}

///////////////////////////////////////////////////////
int
run_subroutine (struct iter_priv *iter, struct VSM_data *vd)
{
  STRCAT (iter->p, "{\n", iter->cpy_sp);
  rate (iter);
  general_info (iter);
  backend (iter);
  director (iter);
  (void) VSC_Iter (vd, json_status, iter);
  STRCAT (iter->p, "\n}\n", iter->cpy_sp);
  return (0);
}

///////////////////////////////////////////////////////
const char *
vmod_rtstatus (struct sess *sp)
{
  struct iter_priv iter = { 0 };
  struct tm t_time;
  struct VSM_data *vd;
  const struct VSC_C_main *VSC_C_main;

  vd = VSM_New ();
  VSC_Setup (vd);

  if (VSC_Open (vd, 1))
    {
      WSL (sp->wrk, SLT_Error, sp->fd, "VSC can't be opened.");
      VSM_Delete (vd);
      return "";
    }
  iter.time_stamp = VRT_time_string (sp, sp->t_req);
  WS_Reserve (sp->wrk->ws, 0);
  iter.p = sp->wrk->ws->f;
  *(iter.p) = 0;
  iter.cpy_sp = sp;
  VSC_C_main = VSC_Main (vd);
  run_subroutine (&iter, vd);
  VSM_Delete (vd);
  WS_Release (sp->wrk->ws, strlen (iter.p) + 1);

  return (iter.p);
}

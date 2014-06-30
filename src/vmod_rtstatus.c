#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "vrt.h"
#include "bin/varnishd/cache.h"
#include "vcc_if.h"
#include "bin/varnishd/cache_backend.h"
#include "varnishapi.h"
#include "vsm.h"


struct once_priv{
  double up;
  int pad;
};

int
init_function(struct vmod_priv *priv, const struct VCL_conf *conf)
{
  return (0);
}

///////////////////////////////////////////////////////////////////

static char *
wsstrncat(char *dest, const char *src, unsigned max_sz) {
  if (strlen(dest) + strlen(src) >= max_sz) {
    return (NULL);
  }

  return strcat(dest, src);
}

/* This lets us cat to a ws-allocated string and just abandon if we run
   out of space. */
#define STRCAT(dst, src, max)					\
  do {								\
    dst = wsstrncat(dst, src, max);				\
    if (!dst) {							\
      WS_Release(sp->wrk->ws, 0);				\
      WSL(sp->wrk, SLT_Error, sp->fd,				\
	  "Running out of workspace in vmod_backendhealth. "	\
	  "Increase sess_workspace to fix this.");		\
      return "";						\
    }								\
  } while(0)
////////////////////////////////////////////////////////////////////
/*
char*
show_counter_cb(void *priv, const struct VSC_point *const pt)
{
  int i;
  (void)priv;
  struct once_priv *op;
  uint64_t val;
  char *buf=malloc(2000000);

  op = priv;
  val = *(const volatile uint64_t*)pt->ptr;
  i = 0;

  if (strcmp(pt->class, ""))
    i += strcat(buf, pt->class);
  if (strcmp(pt->ident, ""))
    i += strcat(buf, pt->ident);
  /*    i += fprintf(f, "%s", pt->name);
  if (i < 35)
    fprintf(f, "%*s", i - 35, "");
  fprintf(f, " %s:\t %d\n", pt->desc,val);
  
  return buf;
}

////////////////////////////////////////////////////////////////////

char*
write_vsc(struct sess *sp, char *p)
{
  struct VSM_data *vd;
  const struct VSC_C_main *VSC_C_main;

  vd = VSM_New();
  VSC_Setup(vd);

  if (VSC_Open(vd, 1))
    exit(1);

  VSC_C_main = VSC_Main(vd);
  strcat(p, VSC_Iter(vd, show_counter_cb, NULL));

  return p;

}

*/
  //////////////////////////////////////////////////////////////

char*
grace(struct sess *sp, char *p)
{
  char *buf=malloc(2000);
  strcat(p, "GRACE:\n");
 strcat(p, "Grace TTL: ");
  sprintf(buf, "%f\n",sp->exp.ttl);
  strcat(p,buf); 
strcat(p, "Grace grace: ");
  sprintf(buf, "%f\n",sp->exp.grace);
  strcat(p,buf);
 strcat(p, "Grace age: ");
  sprintf(buf, "%f\n",sp->exp.age);
  strcat(p,buf);
 strcat(p, "Grace entered: ");
  sprintf(buf, "%f\n",sp->exp.entered);
  strcat(p,buf);
  free(buf);
  return p;
}



////////////////////////////////////////////////////////////////////

char*
sess(struct sess *sp,char *p)
{
  char *buf=malloc(2000);
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  
  strcat( p, "Varnish-Cache status:\n\n");
  strcat(p, "min: ");
  sprintf(buf, "%d\n",tm.tm_min);
  strcat(p,buf);
  
  /*real interesting counters*/
 strcat(p, "Error code: ");
  sprintf(buf, "%d ",sp->err_code);
  strcat(p,buf);
  strcat(p,sp->err_reason);
  strcat(p,"\n");
  strcat(p,"Client address: ");
  strcat(p,sp->addr);
  strcat(p,"\n");
  strcat(p, "Number restarts: ");
  sprintf(buf, "%d\n",sp->restarts);
  strcat(p,buf);
  strcat(p, "Esi level: ");
  sprintf(buf, "%d\n",sp->esi_level);
  strcat(p,buf);
  strcat(p, "Disable Esi: ");
  sprintf(buf, "%d\n",sp->disable_esi);
  strcat(p,buf);

  if(sp->wrk->is_gzip != 0){
strcat(p, "Gzip: ");
  sprintf(buf, "%d\n",sp->wrk->is_gzip);
  strcat(p,buf);
  }
  free(buf);
  return p;
}

////////////////////////////////////////////////////////

const char *
vmod_rtstatus(struct sess *sp)
{
  char *p;
  unsigned max_sz;
  char buf[2048];
  
  max_sz = WS_Reserve(sp->wrk->ws, 0);
  p = sp->wrk->ws->f;
  *p = 0;
  
  sess(sp,p);
  if(sp->exp.grace){
    grace(sp,p);
  }

  STRCAT(p, buf, max_sz);
  
  WS_Release(sp->wrk->ws, strlen(p));

 
  return (p);
}

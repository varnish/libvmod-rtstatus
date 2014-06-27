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
#include "vcl.h"


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
    i += fprintf(f, "%s", pt->name);
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

}*/



////////////////////////////////////////////////////////////////////

char*
write_sess(struct sess *sp,char *p)
{
  char *buf=malloc(2000);
  char tmp[2048];
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  
  strcat( buf, "Varnish-Cache status:\n\n");
  strcat(buf, "min: ");
  sprintf(tmp, "%d\n",tm.tm_min);
  strcat(buf,tmp);

  strcat(buf, "Session pointer id: ");
  sprintf(tmp, "%d\n", sp->id);
  strcat(buf,tmp);
 
  strcat(buf, "Client id: ");
  sprintf(tmp, "%d\n", sp->client_identity);
  strcat(buf,tmp);

  strcat(buf,"Client address: ");
  strcat(buf,sp->addr);
  strcat(buf,"\n");
  strcat(buf,"Client number port: ");
  strncat(buf,sp->port,sizeof buf);
  strcat(buf,"\n");
  strcpy(p,buf);

  
  // strcpy(p,write_vsc(sp,p));
  return p;
}

////////////////////////////////////////////////////////

const char *
vmod_rtstatus(struct sess *sp)
{
  char *p;
  unsigned max_sz;
  char buf[2048];
  
  max_sz = WS_Reserve(sp->wrk->ws, 2000);
  p = sp->wrk->ws->f;
  *p = 0;
  strcpy(p, write_sess(sp,p));
  STRCAT(p, buf, max_sz);
  
  WS_Release(sp->wrk->ws, strlen(p));

 
  return (p);
}

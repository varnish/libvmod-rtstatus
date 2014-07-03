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
#include "vsl.h"

int
init_function(struct vmod_priv *priv, const struct VCL_conf *conf)
{
  return (0);
}
//////////////////////////////////////////////////////

int
show_counter_cb(void *priv, const struct VSC_point *const pt)
{
  char *tmp=malloc(2048);
  int i;
  
  uint64_t val;

  val=*(const volatile uint64_t*)pt->ptr;
  i = 0;
  // sprintf(tmp,"\t\"");
  //strcat(priv,tmp);
  if (strcmp(pt->class, "")){
    i += strcat(priv, pt->class);
    strcat(priv, ".");
  }
  if (strcmp(pt->ident, "")){
    i += strcat(priv, pt->ident);
    strcat(priv,".");
  }
  i += strcat(priv, pt->name);
  sprintf(tmp,"\": {");
  strcat(priv,tmp);

 if (strcmp(pt->class, "")){
   sprintf(tmp,"type\": \"" );
   strcat(priv,tmp);
   strcat(priv,pt->class);
   strcat(priv,"\", ");
 }
if (strcmp(pt->ident, "")){
   sprintf(tmp,"\"ident\": \"" );
   strcat(priv,tmp);
   strcat(priv,pt->ident);
   strcat(priv,"\", ");
 }
 sprintf(tmp,"\"descr\": \"" );
   strcat(priv,tmp);
   strcat(priv,pt->desc);
   strcat(priv,"\", ");

   sprintf(tmp,"\"value\": \"%d\"},\n",val );
 
 strcat(priv,tmp);
/* if (strcmp(pt->ident, "")){
    i += strcat(priv, pt->ident);
    strcat(priv,".");
    }


  strcat(priv, "\t\t");
  strcat(priv,pt->desc);
  sprintf(tmp,":   %d",val);
  strcat(priv,tmp);
  strcat(priv,"\n");
  // strcat(priv,val);
  // fprintf(f, " %s:\t %d\n", pt->desc,val);*/
  return (0);
   
  }

///////////////////////////////////////////////////////
const char*
vmod_rtstatus(struct sess *sp)
{
  char *p;
  unsigned max_sz;
  char time_stamp[22];
  time_t now;
  char *buf=malloc(2000);
  
  max_sz = WS_Reserve(sp->wrk->ws, 0);
  p = sp->wrk->ws->f;
  *p = 0;
  
  strcat(p,"{\n");
  now = time(NULL);

  (void)strftime(time_stamp, 22, "%Y-%m-%d T %H:%M:%S", localtime(&now));
  strcat(p,time_stamp);
  strcat(p,"\n\n");

  const struct VSC_point *const pt;
  struct VSM_data *vd;
  const struct VSC_C_main *VSC_C_main;
  vd = VSM_New();
  VSC_Setup(vd);

  if (VSC_Open(vd, 1))
    exit(1);
  
  VSC_C_main = VSC_Main(vd);
  
  (void)VSC_Iter(vd, show_counter_cb,(void *)p);

  // VSL(SLT_VCL_Log, 0, "after");
  strcat(p, "}\n");
  WS_Release(sp->wrk->ws, strlen(p));

  return (p);
}

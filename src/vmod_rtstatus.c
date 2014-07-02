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



struct once_priv {
	double	up;
	int pad;
};

///////////////////////////////////////////////////////////////////
/*
char*
grace(struct sess *sp, char *p)
{
  char *buf=malloc(200000);
  strcat(p, "{\n\t\"GRACE\": {\n");
  strcat(p, "\t\t\"Grace TTL\": ");
  sprintf(buf, "\"%f\",\n",sp->exp.ttl);
  strcat(p,buf); 
  strcat(p, "\t\t\"Grace age\": ");
  sprintf(buf, "\"%f\",\n",sp->exp.age);
  strcat(p,buf);
  strcat(p, "\t\t\"Grace entered\": ");
  sprintf(buf, "\"%f\",\n\t}\n}\n\n",sp->exp.entered);
  strcat(p,buf);
  free(buf);
  return p;
}

////////////////////////////////////////////////////////////////////

char*
object(struct sess *sp, char *p)
{
  char *buf=malloc(200);
  strcat(p, "{\n\t\"OBJECT\": {\n");
  strcat(p, "\t\t\"Id\": ");
  sprintf(buf, "\"%d\",\n",sp->obj->xid);
  strcat(p,buf); 
  strcat(p, "\t\t\"Vary\": ");
  sprintf(buf, "\"%d\",\n",sp->obj->vary);
  strcat(p,buf);
  strcat(p, "\t\t\"Hits\": ");
  sprintf(buf, "\"%d\",\n",sp->obj->hits);
  strcat(p,buf);
  strcat(p, "\t\t\"Response\": ");
  sprintf(buf, "\"%d\",\n",sp->obj->response);
  strcat(p,buf); 
  strcat(p, "\t\t\"Gziped\": ");
  sprintf(buf, "\"%d\",\n",sp->obj->gziped);
  strcat(p,buf);
  strcat(p, "\t\t\"Last lru\": ");
  sprintf(buf, "\"%f\",\n",sp->obj->last_lru);
  strcat(p,buf);
  strcat(p, "\t\t\"Last modified\": ");
  sprintf(buf, "\"%f\",\n",sp->obj->last_modified);
  strcat(p,buf);
  strcat(p, "\t\t\"Last use\": ");
  sprintf(buf, "\"%f\",\n\t}\n}\n\n",sp->obj->last_use);
  strcat(p,buf);

  free(buf);
  return p;
}

////////////////////////////////////////////////////////////////////

char*
sess(struct sess *sp,char *p)
{
  char *buf=malloc(200000);

  
  strcat( p, "Varnish-Cache status:\n\n");
    
  /*real interesting counters*/
  /* strcat(p, "{ \"Error code\": ");
  sprintf(buf, "\"%d\" '",sp->err_code);
  strcat(p,buf);
  strcat(p,sp->err_reason);
  strcat(p,"\" }\n");
  strcat(p,"{ \"Client address\":\"");
  strcat(p,sp->addr);
  strcat(p,"\" }\n");
  strcat(p, "{ \"Number restarts\": ");
  sprintf(buf, "\"%d\" }\n",sp->restarts);
  strcat(p,buf);
  strcat(p, "{ \"Esi level\": ");
  sprintf(buf, "\"%d\" }\n",sp->esi_level);
  strcat(p,buf);
  strcat(p, "{ \"Disable Esi\": ");
  sprintf(buf, "\"%d\" }\n\n",sp->disable_esi);
  strcat(p,buf);

  if(sp->wrk->is_gzip != 0){
    strcat(p, "{ \"Gzip\": '");
    sprintf(buf, "\"%d\" }\n\n",sp->wrk->is_gzip);
    strcat(p,buf);
  }
  free(buf);
  return p;
}

////////////////////////////////////////////////////////
int 
director(struct sess*sp,char *p)
{
  char *buf=malloc(200);
  strcat(p, "{\n\t\"DIRECTOR\": {\n");
  strcat(p, "\t\t\"Number directors\": ");
  sprintf(buf, "\"%d\",\n",sp->vcl->ndirector);
  strcat(p,buf);
  strcat(p,"\t\t\"Director name\": \"");
  strcat(p,sp->director->name);
  strcat(p,"\",\n");
  strcat(p,"\t\t\"VCL name\": \"");
  strcat(p,sp->director->vcl_name);
  strcat(p,"\",\n\t}\n}\n\n");
  free(buf);
  return 0;
}
////////////////////////////////////////////////////////
char*
vsc_c_main(struct sess *sp,char *p)
{
  struct VSM_data *vd;
  const struct VSC_C_main *VSC_C_main;
  int i, counter = 1;
  int count = 0;

  vd= VSM_New();
  VSC_Setup(vd);

  if (VSC_Open(vd, 1))
    exit(1);

  VSC_C_main = VSC_Main(vd);
  char buf[1024];
  while (count <1) {
    if (counter){
      (void) VSC_Iter(vd, show_counter_cb, NULL);
      count++;
    }
    else {
      assert(0);
    }
  }
  
  return p;
  }*/
//////////////////////////////////////////////////////

int
show_counter_cb(void *priv, const struct VSC_point *const pt)
  {char tmp[2048];
  int i;
  
  uint64_t val;

  val=*(const volatile uint64_t*)pt->ptr;
  i = 0;
   
  if (strcmp(pt->class, "")){
    i += strcat(priv, pt->class);
    strcat(priv, ".");
  }
  if (strcmp(pt->ident, "")){
    i += strcat(priv, pt->ident);
    strcat(priv,".");
  }
  i += strcat(priv, pt->name);
   
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
  strcat(p,"\n}\n\n");
  /* sess(sp,p);
  object(sp,p);
  if(sp->exp.grace){
    grace(sp,p);
  }
  //vsc_c_main(sp,p);
  director(sp,p);*/

  const struct VSC_point *const pt;
  struct VSM_data *vd;
  const struct VSC_C_main *VSC_C_main;
  vd = VSM_New();
  VSC_Setup(vd);

  if (VSC_Open(vd, 1))
    exit(1);
  
  VSC_C_main = VSC_Main(vd);
  
  (void)VSC_Iter(vd, show_counter_cb,(void *)p);

  VSL(SLT_VCL_Log, 0, "after");

  WS_Release(sp->wrk->ws, strlen(p));

  return (p);
}

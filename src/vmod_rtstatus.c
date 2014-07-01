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
  strcat(p, "{ \"Error code\": ");
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
char*
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
  return p;
}
////////////////////////////////////////////////////////
char*
vsc_c_main(char *p)
{
  char *buf=malloc(200);
  struct VSM_data *vd;
  const struct VSC_C_main *VSC_C_main;
  vd= VSM_New();
  VSC_Setup(vd);

  if (VSC_Open(vd, 1))
    exit(1); 
  VSC_C_main = VSC_Main(vd);
  strcat(p, "Client conn accepted: ");
  sprintf(buf, "%d\n",VSC_C_main->client_conn);
  strcat(p,buf);
  strcat(p, "Client conn dropped: ");
  sprintf(buf, "%d\n",VSC_C_main->client_drop);
  strcat(p,buf);
  strcat(p, "Client requ received: ");
  sprintf(buf, "%d\n",VSC_C_main->client_req);
  strcat(p,buf);
  strcat(p, "Cache hit: ");
  sprintf(buf, "%d\n",VSC_C_main->cache_hit);
  strcat(p,buf);
  strcat(p, "Cache hitpass: ");
  sprintf(buf, "%d\n",VSC_C_main->cache_hitpass);
  strcat(p,buf);
  strcat(p, "Cache miss: ");
  sprintf(buf, "%d\n",VSC_C_main->cache_miss);
  strcat(p,buf);
  strcat(p, "Backend conn success: ");
  sprintf(buf, "%d\n",VSC_C_main->backend_conn);
  strcat(p,buf);
  strcat(p, "Backend conn. not attempted: ");
  sprintf(buf, "%d\n",VSC_C_main->backend_unhealthy);
  strcat(p,buf);
  strcat(p, "Backend busy: ");
  sprintf(buf, "%d\n",VSC_C_main->backend_busy);
  strcat(p,buf);
strcat(p, "Backend conn failures: ");
  sprintf(buf, "%d\n",VSC_C_main->backend_fail);
  strcat(p,buf);
  strcat(p, "Backend reuses: ");
  sprintf(buf, "%d\n",VSC_C_main->backend_reuse);
  strcat(p,buf);
  strcat(p, "Backend conn. was closed: ");
  sprintf(buf, "%d\n",VSC_C_main->backend_toolate);
  strcat(p,buf);
  strcat(p, "Backedn recycles: ");
  sprintf(buf, "%d\n",VSC_C_main->backend_recycle);
  strcat(p,buf);
  strcat(p, "Backend retry: ");
  sprintf(buf, "%d\n",VSC_C_main->backend_retry);
  strcat(p,buf);
   strcat(p, "Num struct sess_mem: ");
  sprintf(buf, "%d\n",VSC_C_main->n_sess_mem);
  strcat(p,buf);
  strcat(p, "Num sess: ");
  sprintf(buf, "%d\n",VSC_C_main->n_sess);
  strcat(p,buf);
  strcat(p, "Num tot worker threads: ");
  sprintf(buf, "%d\n",VSC_C_main->n_wrk);
  strcat(p,buf);
  strcat(p, "Num worker threads created: ");
  sprintf(buf, "%d\n",VSC_C_main->n_wrk_create);
  strcat(p,buf);
  strcat(p, "Num worker threads not created: ");
  sprintf(buf, "%d\n",VSC_C_main->n_wrk_failed);
  strcat(p,buf);
  strcat(p, "Num max worker threads: ");
  sprintf(buf, "%d\n",VSC_C_main->n_wrk_max);
  strcat(p,buf);
  strcat(p, "Num backends: ");
  sprintf(buf, "%d\n",VSC_C_main->n_backend);
  strcat(p,buf);
  
  free(buf); 
 return p;
}

///////////////////////////////////////////////////////
const char *
vmod_rtstatus(struct sess *sp)
{
  char *p;
  unsigned max_sz;
  char time_stamp[22];
  time_t now;
  

  max_sz = WS_Reserve(sp->wrk->ws, 0);
  p = sp->wrk->ws->f;
  *p = 0;
  
  strcat(p,"{\n");
  now = time(NULL);

  (void)strftime(time_stamp, 22, "%Y-%m-%d T %H:%M:%S", localtime(&now));
  strcat(p,time_stamp);
  strcat(p,"\n}\n\n");
  sess(sp,p);
  object(sp,p);
  if(sp->exp.grace){
    grace(sp,p);
  }
  director(sp,p);
  vsc_c_main(p);
 
  WS_Release(sp->wrk->ws, strlen(p));

  return (p);
}

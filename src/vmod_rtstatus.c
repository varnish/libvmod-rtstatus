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


char*
grace(struct sess *sp, char *p)
{
  char *buf=malloc(200000);
  strcat(p, "{\n\t'GRACE': {\n");
  strcat(p, "\t\t'Grace TTL': ");
  sprintf(buf, "'%f',\n",sp->exp.ttl);
  strcat(p,buf); 
  strcat(p, "\t\t'Grace age': ");
  sprintf(buf, "'%f',\n",sp->exp.age);
  strcat(p,buf);
  strcat(p, "\t\t'Grace entered': ");
  sprintf(buf, "'%f',\n\t}\n}\n\n",sp->exp.entered);
  strcat(p,buf);
  free(buf);
  return p;
}

////////////////////////////////////////////////////////////////////

char*
object(struct sess *sp, char *p)
{
  char *buf=malloc(200000);
  strcat(p, "{\n\t'OBJECT': {\n");
  strcat(p, "\t\t'Id': ");
  sprintf(buf, "'%d',\n",sp->obj->xid);
  strcat(p,buf); 
  strcat(p, "\t\t'Vary': ");
  sprintf(buf, "'%d',\n",sp->obj->vary);
  strcat(p,buf);
  strcat(p, "\t\t'Hits': ");
  sprintf(buf, "'%d',\n",sp->obj->hits);
  strcat(p,buf);
  strcat(p, "\t\t'Response': ");
  sprintf(buf, "'%d',\n",sp->obj->response);
  strcat(p,buf); 
  strcat(p, "\t\t'Gziped': ");
  sprintf(buf, "'%d',\n",sp->obj->gziped);
  strcat(p,buf);
  strcat(p, "\t\t'Last lru': ");
  sprintf(buf, "'%f',\n",sp->obj->last_lru);
  strcat(p,buf);
  strcat(p, "\t\t'Last modified': ");
  sprintf(buf, "'%f',\n",sp->obj->last_modified);
  strcat(p,buf);
  strcat(p, "\t\t'Last use': ");
  sprintf(buf, "'%f',\n\t}\n}\n\n",sp->obj->last_use);
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
  strcat(p, "{ 'Error code': ");
  sprintf(buf, "'%d' '",sp->err_code);
  strcat(p,buf);
  strcat(p,sp->err_reason);
  strcat(p,"' }\n");
  strcat(p,"{ 'Client address':'");
  strcat(p,sp->addr);
  strcat(p,"' }\n");
  strcat(p, "{ 'Number restarts': ");
  sprintf(buf, "'%d' }\n",sp->restarts);
  strcat(p,buf);
  strcat(p, "{ 'Esi level': ");
  sprintf(buf, "'%d' }\n",sp->esi_level);
  strcat(p,buf);
  strcat(p, "{ 'Disable Esi': ");
  sprintf(buf, "'%d' }\n\n",sp->disable_esi);
  strcat(p,buf);

  if(sp->wrk->is_gzip != 0){
    strcat(p, "{ 'Gzip': '");
    sprintf(buf, "'%d' }\n\n",sp->wrk->is_gzip);
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
  strcat(p, "{\n\t'DIRECTORS': {\n");
  strcat(p, "\t\t'Number directors': ");
  sprintf(buf, "'%d',\n",sp->vcl->ndirector);
  strcat(p,buf);
  strcat(p,"\t\t'Director name': '");
  strcat(p,sp->director->name);
  strcat(p,"',\n");
  strcat(p,"\t\t'VCL name': '");
  strcat(p,sp->director->vcl_name);
  strcat(p,"',\n\t}\n}\n\n");
  free(buf);
  return p;
}
////////////////////////////////////////////////////////

const char *
vmod_rtstatus(struct sess *sp)
{
  char *p;
  unsigned max_sz;
  //char buf[2048];
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
  // STRCAT(p, buf, max_sz);
  
  WS_Release(sp->wrk->ws, strlen(p));

 
  return (p);
}

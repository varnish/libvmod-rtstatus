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

#define MAX_SZ 1024

int
init_function(struct vmod_priv *priv, const struct VCL_conf *conf)
{
    return (0);
}
//////////////////////////////////////////////////////////
static char *
wsstrncat(char *dest, const char *src, unsigned max_sz) {
    if (strlen(dest) + strlen(src) >= max_sz) {
	return (NULL);
    }

    return strcat(dest, src);
}

/* This lets us cat to a ws-allocated string and just abandon if we run
   out of space. */
#define STRCAT(dst, src, max)						\
    do {								\
	dst = wsstrncat(dst, src, max);					\
	if (!dst) {							\
	    WS_Release(sp->wrk->ws, 0);					\
	    WSL(sp->wrk, SLT_Error, sp->fd,				\
		"Running out of workspace in vmod_backendhealth. "	\
		"Increase sess_workspace to fix this.");		\
	    return "";							\
	}								\
    } while(0)
/////////////////////////////////////////////////////////
char *
director(struct sess *sp, char *p, unsigned max_sz)
{
    STRCAT(p,"\"DIRECTOR\": {\"name\":\"",max_sz);
    STRCAT(p, sp->director->name,max_sz);
    STRCAT(p,"\", \"vcl_name\":\"",max_sz);
    STRCAT(p,sp->director->vcl_name,max_sz);
    STRCAT(p, "\"},\n",max_sz);
    return p;
}
/////////////////////////////////////////////////////////
struct iter_priv{
    char *p;
    char *q;
};

int
json_status(void *iter, const struct VSC_point *const pt)
{
    char tmp[1024];
    int i;
    uint64_t val;
    (struct iter_priv *)iter;
    val=*(const volatile uint64_t*)pt->ptr;
    i = 0;
   
    
    strcat(iter,"\"");
    /* if (strcmp(pt->class, "")) {
	i += strcat(priv, pt->class);
	strcat(priv, ".");
    }
    if (strcmp(pt->ident, "")) {
	i += strcat(priv, pt->ident);
	strcat(priv,".");
    }
    i += strcat(priv, pt->name);
    strcat(priv,"\": {");
    if (strcmp(pt->class, "")) {
	strcat(priv,"type\": \"");
	strcat(priv,pt->class);
	strcat(priv,"\", ");
    }
    if (strcmp(pt->ident, "")) {
	strcat(priv,"\"ident\": \"");
	strcat(priv,pt->ident);
	strcat(priv,"\", ");
    }
    strcat(priv,"\"descr\": \"");
    strcat(priv,pt->desc);
    strcat(priv,"\", ");
    sprintf(tmp,"\"value\": \"%d\"},\n",val );
    strcat(priv,tmp);*/
    return (0);
}

///////////////////////////////////////////////////////


const char*
vmod_rtstatus(struct sess *sp)
{
    struct iter_priv *iter=malloc(sizeof(struct iter_priv));
    //char *p;
    unsigned max_sz;
    char time_stamp[22];
    time_t now;
    struct VSM_data *vd;
    const struct VSC_C_main *VSC_C_main;
    char *buffer=malloc(2048);
    max_sz = WS_Reserve(sp->wrk->ws,0);
    (iter->p) = sp->wrk->ws->f;
    *(iter->p) = 0;
    // *(iter->q)=(char *)WS_Free(sp->wrk->ws);
    
    vd = VSM_New();
    VSC_Setup(vd);
    
     if (VSC_Open(vd, 1)){
	STRCAT(iter->p,"\"Error\" :\"VSC can't be opened\"",max_sz);
	WS_Release(sp->wrk->ws, strlen(iter->p));
	return;
    }
    VSC_C_main = VSC_Main(vd);
    STRCAT(iter->p,"{\n",max_sz);
    now = time(NULL);
    (void)strftime(time_stamp, 22, "%Y-%m-%d T %H:%M:%S", localtime(&now));
    STRCAT(iter->p,"\"Timestamp\" : ",max_sz);
    STRCAT(iter->p,time_stamp,max_sz);
    STRCAT(iter->p,"\n\n",max_sz);
    director(sp,iter->p,max_sz);
    if(MAX_SZ >= max_sz - strlen(iter->p)) {
	WS_Release(sp->wrk->ws, strlen(iter->p));
	WSL(sp->wrk, SLT_Error, sp->fd, "Running out of workspace in vmod_rtstatus. Increase sess_workspace to fix this.");
	return "";
    } else	(void)VSC_Iter(vd, json_status,(void *)buffer);
    STRCAT(iter->p,buffer,max_sz);
    STRCAT(iter->p, "}\n",max_sz);
    free(buffer);
    WS_Release(sp->wrk->ws, strlen(iter->p));
    return (iter->p);
}

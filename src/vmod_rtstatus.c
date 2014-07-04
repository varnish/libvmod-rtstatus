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

#define STRCAT(dst, src, max, sp)					\
    do {								\
	dst = wsstrncat(dst, src, max);					\
	if (!dst) {							\
	    WS_Release(sp->wrk->ws, 0);					\
	    WSL(sp->wrk, SLT_Error, sp->fd,				\
		"Running out of workspace in vmod_backendhealth. "	\
		"Increase sess_workspace to fix this.");		\
	    return "";							\
	}								\
    } while(0)								\


struct iter_priv{
    char *p;
    unsigned ws_sz;
    struct sess *cpy_sp;
};


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
/////////////////////////////////////////////////////////
char *
director(struct sess *sp, char *p, unsigned max_sz)
{
    STRCAT(p,"\"DIRECTOR\": {\"name\":\"",max_sz,sp);
    STRCAT(p, sp->director->name,max_sz,sp);
    STRCAT(p,"\", \"vcl_name\":\"",max_sz,sp);
    STRCAT(p,sp->director->vcl_name,max_sz,sp);
    STRCAT(p, "\"},\n",max_sz,sp);
    return p;
}
/////////////////////////////////////////////////////////
int
json_status(struct iter_priv *iter, const struct VSC_point *const pt)
{
    char *tmp;
    uint64_t val;
    val=*(const volatile uint64_t*)pt->ptr;
       
    STRCAT(iter->p,"\"",iter->ws_sz,iter->cpy_sp);
    if (strcmp(pt->class, "")) {
	STRCAT(iter->p, pt->class,iter->ws_sz,iter->cpy_sp);
	STRCAT(iter->p, ".",iter->ws_sz,iter->cpy_sp);
    }
    if (strcmp(pt->ident, "")) {
	STRCAT(iter->p, pt->ident,iter->ws_sz,iter->cpy_sp);
	STRCAT(iter->p,".",iter->ws_sz,iter->cpy_sp);
    }
    STRCAT(iter->p, pt->name,iter->ws_sz,iter->cpy_sp);
    STRCAT(iter->p,"\": {",iter->ws_sz,iter->cpy_sp);
    if (strcmp(pt->class, "")) {
	STRCAT(iter->p,"type\": \"",iter->ws_sz,iter->cpy_sp);
	STRCAT(iter->p,pt->class,iter->ws_sz,iter->cpy_sp);
	STRCAT(iter->p,"\", ",iter->ws_sz,iter->cpy_sp);
    }
    if (strcmp(pt->ident, "")) {
	STRCAT(iter->p,"\"ident\": \"",iter->ws_sz,iter->cpy_sp);
	STRCAT(iter->p,pt->ident,iter->ws_sz,iter->cpy_sp);
	STRCAT(iter->p,"\", ",iter->ws_sz,iter->cpy_sp);
    }
    STRCAT(iter->p,"\"descr\": \"",iter->ws_sz,iter->cpy_sp);
    STRCAT(iter->p,pt->desc,iter->ws_sz,iter->cpy_sp);
    STRCAT(iter->p,"\", ",iter->ws_sz,iter->cpy_sp);
    asprintf(&tmp,"\"value\": \"%d\"},\n",val );
    STRCAT(iter->p,tmp,iter->ws_sz,iter->cpy_sp);
    free(tmp);
    return (0);
}
///////////////////////////////////////////////////////
const char*
vmod_rtstatus(struct sess *sp)
{
    struct iter_priv *iter=malloc(sizeof(struct iter_priv));
    unsigned max_sz;
    char time_stamp[22];
    time_t now;
    struct tm t_time;
    struct VSM_data *vd;
    const struct VSC_C_main *VSC_C_main;
    vd = VSM_New();
    VSC_Setup(vd);
    
    if (VSC_Open(vd, 1)){
	WSL(sp->wrk, SLT_Error, sp->fd,"VSC can't be opened.");
	return; }
    max_sz = WS_Reserve(sp->wrk->ws,0);
    (iter->p) = sp->wrk->ws->f;
    *(iter->p) = 0;
    iter->ws_sz = WS_Free(sp->wrk->ws);
    iter->cpy_sp = sp;
        
    VSC_C_main = VSC_Main(vd);
    STRCAT(iter->p,"{\n",max_sz, sp);
    now = time(NULL);
    (void)strftime(time_stamp, 22, "%Y-%m-%d T %H:%M:%S", localtime_r(&now,&t_time));
    STRCAT(iter->p,"\"Timestamp\" : ",max_sz, sp);
    STRCAT(iter->p,time_stamp,max_sz,sp);
    STRCAT(iter->p,"\n\n",max_sz,sp);
    director(sp,iter->p,max_sz);
    (void)VSC_Iter(vd, json_status,iter);
    STRCAT(iter->p, "}\n",max_sz,sp);
    //VSC_Delete(vd);
    VSM_Delete(vd);
    WS_Release(sp->wrk->ws, strlen(iter->p));
    return (iter->p);
}

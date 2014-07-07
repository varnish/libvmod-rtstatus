#include "config.h"
#include <inttypes.h>
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
	    return 1;							\
	}								\
    } while(0)								\


struct iter_priv{
    char *p;
    unsigned ws_sz;
    struct sess *cpy_sp;
};
//aggiungi un pointer e ridifinisci wssstrincat

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
int
director(struct sess *sp, char *p, unsigned max_sz)
{
    char *time_stamp;
    time_stamp = VRT_time_string(sp, sp->t_req);
   
    STRCAT(p, "\"Timestamp\" : ",max_sz, sp);
    STRCAT(p, time_stamp,max_sz,sp);
    STRCAT(p, "\n\n",max_sz,sp);
    STRCAT(p,"\"DIRECTOR\": {\"name\":\"",max_sz,sp);
    STRCAT(p, sp->director->name,max_sz,sp);
    STRCAT(p,"\", \"vcl_name\":\"",max_sz,sp);
    STRCAT(p,sp->director->vcl_name,max_sz,sp);
    STRCAT(p, "\"},\n",max_sz,sp);
    return 0;
}
/////////////////////////////////////////////////////////
int
json_status(void *priv, const struct VSC_point *const pt)
{
    char tmp[128];
    struct iter_priv *iter = priv;
    uint64_t val;
    val = *(const volatile uint64_t*)pt->ptr;

    STRCAT(iter->p,"\"", iter->ws_sz, iter->cpy_sp);
    if (strcmp(pt->class, "")) {
	STRCAT(iter->p, pt->class,iter->ws_sz, iter->cpy_sp);
	STRCAT(iter->p, ".", iter->ws_sz, iter->cpy_sp);
    }
    if (strcmp(pt->ident, "")) {
	STRCAT(iter->p, pt->ident, iter->ws_sz, iter->cpy_sp);
	STRCAT(iter->p, ".", iter->ws_sz, iter->cpy_sp);
    }
    STRCAT(iter->p, pt->name, iter->ws_sz, iter->cpy_sp);
    STRCAT(iter->p, "\": {", iter->ws_sz, iter->cpy_sp);
    if (strcmp(pt->class, "")) {
	STRCAT(iter->p, "type\": \"", iter->ws_sz, iter->cpy_sp);
	STRCAT(iter->p, pt->class, iter->ws_sz, iter->cpy_sp);
	STRCAT(iter->p, "\", ", iter->ws_sz, iter->cpy_sp);
    }
    if (strcmp(pt->ident, "")) {
	STRCAT(iter->p, "\"ident\": \"", iter->ws_sz, iter->cpy_sp);
	STRCAT(iter->p, pt->ident, iter->ws_sz, iter->cpy_sp);
	STRCAT(iter->p, "\", ", iter->ws_sz, iter->cpy_sp);
    }
    STRCAT(iter->p, "\"descr\": \"", iter->ws_sz, iter->cpy_sp);
    STRCAT(iter->p, pt->desc, iter->ws_sz, iter->cpy_sp);
    STRCAT(iter->p, "\", ", iter->ws_sz, iter->cpy_sp);
    sprintf(tmp, "\"value\": \"%" PRIu64 "},\n", val );
    STRCAT(iter->p, tmp, iter->ws_sz, iter->cpy_sp);

    return 0;
}
///////////////////////////////////////////////////////
const char*
vmod_rtstatus(struct sess *sp)
{
    struct iter_priv iter = { 0 };
    unsigned max_sz;
    struct tm t_time;
    struct VSM_data *vd;
    const struct VSC_C_main *VSC_C_main;
 
    
    vd = VSM_New();
    VSC_Setup(vd);
    
    if (VSC_Open(vd, 1)) {
	WSL(sp->wrk, SLT_Error, sp->fd,"VSC can't be opened.");
	return ""; 
    }
    max_sz = WS_Reserve(sp->wrk->ws, 0);
    iter.p = sp->wrk->ws->f;
    *(iter.p) = 0;
    iter.ws_sz = WS_Free(sp->wrk->ws);
    iter.cpy_sp = sp;
    VSC_C_main = VSC_Main(vd);
    director(sp, iter.p, max_sz);
    (void)VSC_Iter(vd, json_status, &iter);
    
    VSM_Delete(vd);
    WS_Release(sp->wrk->ws, strlen(iter.p) + 1);
    
    return (iter.p);
}

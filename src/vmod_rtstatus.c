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

#define STRCAT(dst, src, sp)						\
    do {								\
	dst = wsstrncat(dst, src, sp);					\
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
    struct sess *cpy_sp;
    char *time_stamp;
};

int
init_function(struct vmod_priv *priv, const struct VCL_conf *conf)
{
    return (0);
}
//////////////////////////////////////////////////////////
static char *
wsstrncat(char *dest, const char *src, struct sess *sp) {
    if (sp->wrk->ws->r - sp->wrk->ws->f < strlen(src)) {
	return (NULL);
    }
    return strcat(dest, src);
}
/////////////////////////////////////////////////////////
int
director(struct iter_priv *iter)
{
     STRCAT(iter->p, "\"Timestamp\" : ", iter->cpy_sp);
    STRCAT(iter->p,iter->time_stamp,iter->cpy_sp);
    STRCAT(iter->p, "\n\n", iter->cpy_sp);
    STRCAT(iter->p,"\"DIRECTOR\": {\"name\":\"",iter->cpy_sp);
    STRCAT(iter->p, iter->cpy_sp->director->name, iter->cpy_sp);
    STRCAT(iter->p,"\", \"vcl_name\":\"", iter->cpy_sp);
    STRCAT(iter->p,iter->cpy_sp->director->vcl_name, iter->cpy_sp);
    STRCAT(iter->p, "\"},\n", iter->cpy_sp);
    
    return (0); 
}
/////////////////////////////////////////////////////////
int
json_status(void *priv, const struct VSC_point *const pt)
{
    char tmp[128];
    struct iter_priv *iter = priv;
    uint64_t val;
    val = *(const volatile uint64_t*)pt->ptr;

    STRCAT(iter->p,"\"", iter->cpy_sp);
    if (strcmp(pt->class, "")) {
	STRCAT(iter->p, pt->class, iter->cpy_sp);
	STRCAT(iter->p, ".", iter->cpy_sp);
    }
    if (strcmp(pt->ident, "")) {
	STRCAT(iter->p, pt->ident, iter->cpy_sp);
	STRCAT(iter->p, ".", iter->cpy_sp);
    }
    STRCAT(iter->p, pt->name, iter->cpy_sp);
    STRCAT(iter->p, "\": {", iter->cpy_sp);
    if (strcmp(pt->class, "")) {
	STRCAT(iter->p, "type\": \"", iter->cpy_sp);
	STRCAT(iter->p, pt->class, iter->cpy_sp);
	STRCAT(iter->p, "\", ", iter->cpy_sp);
    }
    if (strcmp(pt->ident, "")) {
	STRCAT(iter->p, "\"ident\": \"", iter->cpy_sp);
	STRCAT(iter->p, pt->ident, iter->cpy_sp);
	STRCAT(iter->p, "\", ", iter->cpy_sp);
    }
    STRCAT(iter->p, "\"descr\": \"", iter->cpy_sp);
    STRCAT(iter->p, pt->desc, iter->cpy_sp);
    STRCAT(iter->p, "\", ", iter->cpy_sp);
    sprintf(tmp, "\"value\": \"%" PRIu64 "},\n", val );
    STRCAT(iter->p, tmp, iter->cpy_sp);
    
    return (0);
}
///////////////////////////////////////////////////////
const char*
vmod_rtstatus(struct sess *sp)
{
    struct iter_priv iter = { 0 };
    struct tm t_time;
    struct VSM_data *vd;
    const struct VSC_C_main *VSC_C_main;
    //char *time_stamp;
    
    vd = VSM_New();
    VSC_Setup(vd);
    
    if (VSC_Open(vd, 1)) {
	WSL(sp->wrk, SLT_Error, sp->fd, "VSC can't be opened.");
	VSM_Delete(vd);
	return ""; 
    }
    iter.time_stamp = VRT_time_string(sp,sp->t_req);
   
    WS_Reserve(sp->wrk->ws, 0);
    iter.p = sp->wrk->ws->f;
    *(iter.p) = 0;
    iter.cpy_sp = sp;
    VSC_C_main = VSC_Main(vd);
    director(&iter);
    (void)VSC_Iter(vd, json_status, &iter);
    VSM_Delete(vd);
    WS_Release(sp->wrk->ws, strlen(iter.p) + 1);
    
    return (iter.p);
}

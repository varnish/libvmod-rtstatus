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
/////////////////////////////////////////////////////////
char *
director(struct sess *sp, char *p)
{
	strcat(p,"\"DIRECTOR\": {\"name\":\"");
	strcat(p, sp->director->name);
	strcat(p,"\", \"vcl_name\":\"");
	strcat(p,sp->director->vcl_name);
	strcat(p, "\"},\n");
	return p;
}
/////////////////////////////////////////////////////////
int
json_status(void *priv, const struct VSC_point *const pt)
{
	char tmp[1024];
	int i;
	uint64_t val;
	val=*(const volatile uint64_t*)pt->ptr;
	i = 0;
	strcat(priv,"\"");
	if (strcmp(pt->class, "")) {
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
	strcat(priv,tmp);
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
	struct VSM_data *vd;
	const struct VSC_C_main *VSC_C_main;

	max_sz = WS_Reserve(sp->wrk->ws, 0);
	p = sp->wrk->ws->f;
	*p = 0;

	vd = VSM_New();
	VSC_Setup(vd);

	if (VSC_Open(vd, 1))
		strcat(p,"\"Error\" :\"VSC can't be opened\"");
	VSC_C_main = VSC_Main(vd);
	strcat(p,"{\n");
	now = time(NULL);
	(void)strftime(time_stamp, 22, "%Y-%m-%d T %H:%M:%S", localtime(&now));
	strcat(p,"\"Timestamp\" : ");
	strcat(p,time_stamp);
	strcat(p,"\n\n");
	director(sp,p);
	(void)VSC_Iter(vd, json_status,(void *)p);
	strcat(p, "}\n");
	if (strlen(p)>=max_sz) {
		WS_Release(sp->wrk->ws, 0);
		WSL(sp->wrk, SLT_Error, sp->fd, "Running out of workspace in vmod_rtstatus. Increase sess_workspace to fix this.");
		return ""; 
	}
	WS_Release(sp->wrk->ws, strlen(p));
	return (p);
}

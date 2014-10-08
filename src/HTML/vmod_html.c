#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>

#include "vrt.h"
#include "vrt_obj.h"
#include "cache/cache.h"
#include "vmod_rtstatus.h"
#include "vmod_html.h"

int
print_html( char *p, const struct vrt_ctx *ctx)
{
	STRCAT(p, html, ctx);
	return(0);
}
///////////////////////////////////////////////
VCL_STRING
vmod_html(const struct vrt_ctx *ctx)
{
	char *p;
	WS_Reserve(ctx->ws, 0);
	p = ctx->ws->f;
	print_html(p, ctx);
	WS_Release(ctx->ws, strlen(p) + 1);
	return (p);
}


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vdef.h"
#include "vsb.h"
#include "vapi/vsm.h"

struct rtstatus_priv {
	unsigned 	magic;
#define VMOD_RTSTATUS_MAGIC 0x98b584a
	struct vsb 	*vsb;
	uint64_t 	beresp_hdr;
	uint64_t	beresp_body;
	uint64_t	bereq_hdr;
	uint64_t	bereq_body;
	uint64_t	be_happy;
};

int collect_info(struct rtstatus_priv *rtstatus, struct VSM_data *vd);

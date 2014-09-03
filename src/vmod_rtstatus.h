

struct iter_priv{
    char *p;
    const struct vrt_ctx *cpy_ctx;
    char *time_stamp;
    struct vtc_job *jp;
    double time;
};


#define STRCAT(dst, src, ctx)					\
    do {							\
	dst = wsstrncat(dst, src, ctx);				\
	if (!dst) {						\
	    WS_Release(ctx->ws, 0);				\
	    return 1;						\
	}							\
    } while(0)							\



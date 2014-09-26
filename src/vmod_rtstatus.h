struct iter_priv{
	char *p;
	const struct vrt_ctx *cpy_ctx;
	int jp;
};

#define STRCAT(dst, src, ctx)					\
	do {							\
		dst = wsstrncat(dst, src, ctx);			\
			if (!dst) {				\
				WS_Release(ctx->ws, 0);		\
				return 1;			\
			}					\
	} while(0)						\

void WS_Release(struct ws *ws, unsigned bytes);
unsigned WS_Reserve(struct ws *ws, unsigned bytes);
char *wsstrncat(char *dest, const char *src, const struct vrt_ctx *ctx);
int general_info(struct iter_priv *iter);
int backend(struct iter_priv *iter);

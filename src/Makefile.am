AM_CPPFLAGS = @VMOD_INCLUDES@ -Wall -Werror

vmoddir = @VMOD_DIR@
vmod_LTLIBRARIES = libvmod_rtstatus.la

libvmod_rtstatus_la_LDFLAGS = -module -export-dynamic -avoid-version -shared $(VARNISHAPI_LIBS)
libvmod_rtstatus_la_LIBADD = -lvarnishapi

libvmod_rtstatus_la_SOURCES = \
	vmod_rtstatus.h \
	vmod_rtstatus.c \
	vmod_rtstatus_main.c \
	HTML/vmod_html.h \
	HTML/vmod_html.c

nodist_libvmod_rtstatus_la_SOURCES = \
	vcc_if.c \
	vcc_if.h

vmod_rtstatus.lo: vcc_if.c vcc_if.h

vcc_if.c: vcc_if.h

vcc_if.h: @VMODTOOL@ $(top_srcdir)/src/vmod_rtstatus.vcc
	@VMODTOOL@ $(top_srcdir)/src/vmod_rtstatus.vcc

VMOD_TESTS = $(top_srcdir)/src/tests/*.vtc
.PHONY: $(VMOD_TESTS)

$(top_srcdir)/src/tests/*.vtc:
	@VARNISHTEST@ -Dvarnishd=@VARNISHD@ -Dvmod_topbuild=$(abs_top_builddir) $@


check: $(VMOD_TESTS)

EXTRA_DIST = \
	vmod_rtstatus.vcc\
	$(VMOD_TESTS)

CLEANFILES = \
	$(builddir)/vcc_if.c \
	$(builddir)/vcc_if.h \
	$(builddir)/vmod_rtstatus.rst \
	$(builddir)/vmod_rtstatus.man.rst

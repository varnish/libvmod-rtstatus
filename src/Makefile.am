AM_CPPFLAGS = @VMOD_INCLUDES@ -Wall -Wextra -Werror -Iforeign

vmoddir = @VMOD_DIR@
vmod_LTLIBRARIES = libvmod_rtstatus.la

libvmod_rtstatus_la_LDFLAGS = -module -export-dynamic -avoid-version -shared -lvarnishapi

libvmod_rtstatus_la_SOURCES = \
	vmod_rtstatus.c

nodist_libvmod_rtstatus_la_SOURCES = \
	vcc_if.c \
	vcc_if.h

vmod_rtstatus.lo: vcc_if.c vcc_if.h

vcc_if.c: vcc_if.h

vcc_if.h: @VMODTOOL@ $(top_srcdir)/src/vmod_rtstatus.vcc
	@VMODTOOL@ $(top_srcdir)/src/vmod_rtstatus.vcc

VMOD_TESTS = $(top_srcdir)/src/tests/*.vtc
.PHONY: $(VMOD_TESTS)

$(top_srcdir)/src/tests/*.vtc: libvmod_rtstatus.la
	@VARNISHTEST@ -Dvarnishd=@VARNISHD@ -Dvmod_topbuild=$(abs_top_builddir) $@

check: $(VMOD_TESTS)

EXTRA_DIST = \
	vmod_rtstatus.vcc \
	$(VMOD_TESTS)

CLEANFILES = \
	$(builddir)/vcc_if.c \
	$(builddir)/vcc_if.h \
	$(builddir)/vmod_rtstatus.man.rst \
	$(builddir)/vmod_rtstatus.rst

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

dist_man_MANS = vmod_rtstatus.3
MAINTAINERCLEANFILES = $(dist_man_MANS)

vcc_if.c: vcc_if.h

vcc_if.h vmod_rtstatus.man.rst: @VMODTOOL@ $(top_srcdir)/src/vmod_rtstatus.vcc
	@VMODTOOL@ $(top_srcdir)/src/vmod_rtstatus.vcc

BUILT_SOURCES = HTML/vmod_html.h
HTML/vmod_html.h: HTML/rtstatus.html HTML/cstring.py
	@PYTHON@ $(abs_top_srcdir)/src/HTML/cstring.py $(abs_top_srcdir)/src/HTML/rtstatus.html > $@

vmod_rtstatus.3: vmod_rtstatus.man.rst

%.1 %.2 %.3 %.4 %.5 %.6 %.7 %.8 %.9:
if HAVE_RST2MAN
	${RST2MAN} $< $@
else
	@echo "========================================"
	@echo "You need rst2man installed to make dist"
	@echo "========================================"
	@false
endif

VMOD_TESTS = $(top_srcdir)/src/tests/*.vtc
.PHONY: $(VMOD_TESTS)

$(top_srcdir)/src/tests/*.vtc:
	@VARNISHTEST@ -Dvarnishd=@VARNISHD@ -Dvmod_topbuild=$(abs_top_builddir) $@


check: $(VMOD_TESTS)

EXTRA_DIST = \
	vmod_rtstatus.vcc \
	HTML/rtstatus.html \
	HTML/cstring.py \
	$(VMOD_TESTS)

CLEANFILES = \
	$(builddir)/vcc_if.c \
	$(builddir)/vcc_if.h \
	$(builddir)/HTML/vmod_html.h \
	$(builddir)/vmod_rtstatus.rst \
	$(builddir)/vmod_rtstatus.man.rst \
	$(dist_man_MANS)

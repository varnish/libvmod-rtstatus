ACLOCAL_AMFLAGS = -I m4 -I ${LIBVARNISHAPI_DATAROOTDIR}/aclocal

SUBDIRS = src

DISTCHECK_CONFIGURE_FLAGS = \
        VMOD_DIR='$${libdir}/varnish/vmods'


EXTRA_DIST = README.rst LICENSE vmod-rtstatus.spec debian

doc_DATA = README.rst LICENSE

#dist_man_MANS = vmod_rtstatus.3
#MAINTAINERCLEANFILES = $(dist_man_MANS)

#vmod_rtstatus.3: src/vmod_rtstatus.man.rst

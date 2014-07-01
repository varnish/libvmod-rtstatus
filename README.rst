============
vmod_rtstatus
============

-------------------------------
Varnish Real-Time Status Module
-------------------------------

:Author: Arianna Aondio
:Date: 2014-07-01
:Version: 1.0

SYNOPSIS
========

import rtstatus;

DESCRIPTION
===========

rtstatus vmod fetches(in realtime)from backend some counters
for Varnish 3.0 and later.

FUNCTIONS
=========

rtstatus
-----

Prototype
        ::

                rtstatus( )
Return value
	STRING

INSTALLATION
============
The source tree is based on autotools to configure the building, and
does also have the necessary bits in place to do functional unit tests
using the varnishtest tool.

Usage::

 ./configure VARNISHSRC=DIR [VMODDIR=DIR]

`VARNISHSRC` is the directory of the Varnish source tree for which to
compile your vmod. Both the `VARNISHSRC` and `VARNISHSRC/include`
will be added to the include search paths for your module.

Optionally you can also set the vmod install directory by adding
`VMODDIR=DIR` (defaults to the pkg-config discovered directory from your
Varnish installation).

Make targets:

* make - builds the vmod
* make install - installs your vmod in `VMODDIR`
* make check - runs the unit tests in ``src/tests/*.vtc``

In your VCL you could then use this vmod along the following lines::
        
        import rtstatus;

        sub vcl_recv {
    		if (req.url ~ "/rtstatus") {
        		error 800 "OK";
    		}
	}

	sub vcl_error {
    		if(obj.status == 800){
        		set obj.status = 200;
        		synthetic rtstatus.rtstatus();
        	return (deliver);
    		}
	}


COPYRIGHT
=========

This document is licensed under the same license as the
libvmod-rtstatus project. See LICENSE for details.

* Copyright (c) 2014 Varnish Software

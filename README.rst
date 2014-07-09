=============
vmod_rtstatus
=============

-------------------------------
Varnish Real-Time Status Module
-------------------------------

:Author: Arianna Aondio
:Date: 2014-07-03
:Version: 1.0

SYNOPSIS
========

import rtstatus;

DESCRIPTION
===========

A vmod that lets you query your Varnish server for a JSON object the
counters. With the accompanied VCL code,
visiting the URL /rtstatus on the Varnish server will produce an
application/json response of the following format:

{
	"Timestamp" : Wed, 09 Jul 2014 10:52:28 GMT,
	"Varnish Version" : varnish-3.0.5 revision 8213a0b,
	"Director": {"name":"simple", "vcl_name":"default"},
	"client_conn": {"descr": "Client connections accepted", "value": "1},
	"LCK.cli.locks": {type": "LCK", "ident": "cli", "descr": "Lock Operations", "value": "15},
	"VBE.default(127.0.0.1,,8080).happy": {type": "VBE", "ident": "default(127.0.0.1,,8080)", "descr": "Happy health probes", "value": "0},
}

FUNCTIONS
=========

rtstatus
--------

Prototype::

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

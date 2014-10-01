=============
vmod_rtstatus
=============


.. image:: https://secure.travis-ci.org/aondio/libvmod-rtstatus.png
   :alt: Travis CI badge
   :target: http://travis-ci.org/libvmod-rtstatus


-------------------------------
Varnish Real-Time Status Module
-------------------------------

:Author: Arianna Aondio
:Date: 2014-09-30
:Version: 1.0

SYNOPSIS
========

import rtstatus;

DESCRIPTION
===========

A vmod that lets you query your Varnish server for a JSON object the
counters. With the accompanied VCL code,

visiting the URL /rtstatus.json on the Varnish server will produce an
application/json response of the following format::

    {
	"Timestamp" : "Thu, 24 Jul 2014 10:27:10 GMT",
	"Varnish_Version" : "varnish-3.0.5 revision 8213a0b",
	"Backend": {"name":"default", "value": "healthy"},
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

Make targets:

* make - builds the vmod
* make install - installs your vmod in `VMODDIR`
* make check - runs the unit tests in ``src/tests/*.vtc``

In your VCL you could then use this vmod along the following lines::
        
        vcl 4.0;
	import std;
	import rtstatus;

	sub vcl_recv {
		if (req.url ~ "/rtstatus.json") {
        		return(synth(700, "OK"));        	}
		if (req.url ~ "/rtstatus") {
			return(synth(800, "OK"));
		}
	}
	sub vcl_synth {	
		if (resp.status == 700){
			set resp.status = 200;
			synthetic(rtstatus.rtstatus());
			return (deliver);
		}
		if (resp.status == 800) {
			set resp.http.Content-Type = "text/html; charset=utf-8";
			synthetic(std.fileread("/home/arianna/libvmod-rtstatus/src/rtstatus.html"));
			return (deliver);
			}
	}

COPYRIGHT
=========

This document is licensed under the same license as the
libvmod-rtstatus project. See LICENSE for details.

* Copyright (c) 2014 Varnish Software

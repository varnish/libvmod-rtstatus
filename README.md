
###vmod_rtstatus

####Varnish Real-Time Status Module

|Author   	|Date   	|Version   	|
|---	|---	|---	|
|Arianna Aondio   	|2015-12-09   	|1.1.1   	|

####Synopsis

```python
import rtstatus;
```

####Description

A vmod that lets you query your Varnish server for a JSON object containing
counters.

Visiting the URL ``/rtstatus.json`` on the Varnish server will produce an
application/json response of the following format::

```json
    {
	"Uptime" : "0+00:09:38",
	"hitrate": "0.00",
	"load": "1",
	"varnish_version" : "varnish-4.0.1 revision c6f20e4",
	"server_id": "arianna-ThinkPad-X230",
	"client_id": "127.0.0.1",
	"backend": [{"director_name" : "simple" , "name":"default", "value": "healthy"},
		{"director_name" : "simple" , "name":"server1", "value": "healthy"},
		{"director_name" : "simple" , "name":"server2", "value": "healthy"}],
	"MAIN.uptime": {"type": "MAIN", "descr": "Child process uptime", "value": 578},
	"VBE.server1(192.168.0.10,,8081).vcls": {"type": "VBE", "ident": "server1(192.168.0.10,,8081)", "descr": "VCL references", "value": 1},
	"VBE.server1(192.168.0.10,,8081).happy": {"type": "VBE", "ident": "server1(192.168.0.10,,8081)", "descr": "Happy health probes", "value": 0},
	"VBE.server1(192.168.0.10,,8081).bereq_hdrbytes": {"type": "VBE", "ident": "server1(192.168.0.10,,8081)", "descr": "Request header bytes", "value": 0},
    }
```

Visiting the URL `/rtstatus` on the Varnish server will produce an
application/javascript response of the following format:

![RTstatus frontend](varnishstatus.png)

####Functions

|Prototype   	| Return  	| Description  	|
|---	|---	|---	|
|  rtstatus(REAL delta) 	|   string	|  *delta* is the interval of seconds used for hitrate and load calculations. It has to be > 0 and < 60 seconds. 	|
| html()  	|   string	|   	|


####Installation

The source tree is based on autotools to configure the building, and does also have the necessary bits in place to do functional unit tests using the varnishtest tool.

Make targets:

* make - builds the vmod
* make install - installs your vmod in `VMODDIR`
* make check - runs the unit tests in ``src/tests/*.vtc``

In your VCL you could then use this vmod along the following lines::
        
```c
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
		set resp.http.Content-Type = "text/plain; charset=utf-8";
		synthetic(rtstatus.rtstatus(5));
		return (deliver);
	}
	if (resp.status == 800) {
		set resp.status = 200;
		set resp.http.Content-Type = "text/html; charset=utf-8";
		synthetic(rtstatus.html());
		return (deliver);
		}
}
```

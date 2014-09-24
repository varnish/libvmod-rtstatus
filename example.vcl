vcl 4.0;
import std;
import directors;
import rtstatus;

backend default {
	.host = "127.0.0.1";
	.port = "8080";
}
backend server1 {
	.host = "192.168.0.10";
	.port ="8081";
}
backend server2 {
	.host = "192.168.0.10";
	.port = "8082";
}

sub vcl_init {
	new bar = directors.round_robin();
	bar.add_backend(server1);
	bar.add_backend(server2);
	bar.add_backend(default);
}

sub vcl_recv {
	if (req.url ~ "/rtstatus.json") {
		return(synth(200, "OK"));
	}
	if (req.url ~ "/rtstatus") {
		return(synth(800, "OK"));
	}
}

sub vcl_synth {
	if(resp.status == 200) {
		synthetic(rtstatus.rtstatus());
		return (deliver);
	}
	if(resp.status == 800) {
		set resp.http.Content-Type = "text/html; charset=utf-8";
		synthetic(std.fileread("/home/arianna/libvmod-rtstatus/src/rtstatus.html"));
		return (deliver);
	}
}





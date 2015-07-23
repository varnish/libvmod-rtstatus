Before reading this, please read the official README. 
This file is meant for a better understanding of the VMOD.

The VMOD has two functions:

rtstatus(REAL delta): this is the important one, it generates a JSON object containing a lot of varnish-counters.They are mostly raw, so you can use them as you prefer. 
The parameter *delta* is requested because is used for calculating hitrate and request load, the idea behind this is something like "I'd really like to know how my hitrate and my load have been doing in the last 13 seconds". *delta* has to be greater than 0 and smaller than 60 seconds; please remember the VMOD is called rtstatus where rt stands for real-time, values greater than one minutes just don't make sense for this vmod.
I suggest to set delta to 5 or 10 seconds. 
 
html(): this function has been written for presenting the counters in a nice way. It is a c wrapper around some HTML/css and javascript. Basically I just wrote everything into a big string and then I used this string to feed the vmod function.
Please feel free customize your frontend and if your idea is good, feel free to suggest it :)

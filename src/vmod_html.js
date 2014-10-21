<script type="text/javascript" src="http://code.jquery.com/jquery-1.4.2.min.js">


  function openSpoiler(id){
  var a = document.getElementById(id).style;
  a.display = a.display=='block'?'none':'block'}

    function openSpoiler(id){
    var a = document.getElementById(id).style;
    a.display = a.display=='block'?'none':'block'}

  function info(){
  $.getJSON( "/rtstatus.json", function( data ) {
  var items = [];
  var index = 0;
  items.push("<p>"+ "<b>Varnish version: </b> " + data.varnish_version + "</p>");
  items.push("<p>"+ "<b>Server identity: </b> " + data.server_id + "</p>");
  items.push("<p>"+ "<b>Varnish uptime: </b> " + data.uptime + "</p>");
  items.push("<p>"+ "<b>Hitrate: </b>"+ " " + data.hitrate + " %"+ "</p>");
  items.push("<p>"+ "<b>Load: </b>"+ " " + data.load + " req/s"+ "</p>");
  $("#info").html("");
  $( "<p/>", { "class": "my-new-list", html: items.join( "" )}).appendTo( "#info" );


  var items = [];
  var cont = 0;
  var i = 1;
  $.each( data, function( key, val ) {
  if( val.type === "MAIN" && key.match("backend")){                                    
  items.push( "<p>" + "<b>" + val.descr + "</b>"  +" :  " + "</p>" );
  }});
  $("#be").html("");
  $( "<p/>", {"class": "my-new-list",html: items.join( "" )}).appendTo( "#be" );


   var items = [];
  var cont = 0;
  var i = 1;
  $.each( data, function( key, val ) {
  if( val.type === "MAIN" && key.match("backend")){
  items.push( "<p>" +  val.value + "</p>" );
  }});
  $("#bevalue").html("");
  $( "<p/>", {"class": "my-new-list",html: items.join( "" )}).appendTo( "#bevalue" );


  var items = [];
  var i,j;
  var cont;
  $.each( data, function( key, val ) {
  if(key === "backend"){
  var tmp = data.backend;
  }
  if(tmp) {
  for(i = 0, cont = 0; i < tmp.length;i++){
  items.push( "<p>" + "<b>Name: </b> " + tmp[i].name + " : " + tmp[i].value + "</p>" );
  }}});
  $("div.backend").html("");
  $( "<p/>", {"class": "my-new-list", html: items.join( "" )}).appendTo( "div.backend" );



  var cont = 0;
  $('table#tblbe TBODY').html("");
  $.each(data, function(key, val){
  if( val.type === "VBE"){
  if(cont % 9 === 0){
  $('table#tblbe TBODY').append('<tr><td>'+"\t"+"<b>BACKEND : "+ val.ident+'</td><td>'+"\t\t"+"</b>" +'</td><td></tr>');
  }
			   $('table#tblbe TBODY').append('<tr><td>'+"\t"+val.descr+'</td><td>'+val.value+'</td><td></tr>');
			   cont++;
			   }
			   });



			   var cont = 0;
			   $('table#tbl TBODY').html("");
			   $.each(data, function(key, val){
if(cont > 7){
    $('table#tbl TBODY').append('<tr><td>'+"\t"+key+'</td><td>'+val.descr+'</td><td>'+val.value+'</td><td></tr>');
    }
    cont++;
    });
    });
    };
    var myVar = setInterval(function(){info()}, 1000);

</script>

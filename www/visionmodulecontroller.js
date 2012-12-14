var xmlHttp;

function stateCallback() {
  var stat, rstate;
  if( !xmlHttp ) return;

  try {
    rstate = xmlHttp.readyState;
  } catch (err) {
    alert(err);
  }

  switch( rstate )
  {
    // uninitialized
    case 0:
    // loading
    case 1:
    // loaded
    case 2:
    // interactive
    case 3:
    break;
    // complete, so act on response
    case 4:
    // check http status
      try {
        stat = xmlHttp.status;
      }
      catch (err) {
        stat = "xmlHttp.status does not exist";
      }
      if( stat == 200 )    // success
      {
          AJAX_response(xmlHttp.responseText);
      }
      // loading not successfull, e.g. page not available
      else { }
  }
}

function init_AJAX() 
{
  var new_xmlHttp;

  try
  {
    // Internet Explorer
    if( window.ActiveXObject )
    {
      for( var i = 5; i; i-- )
      {
        try
        {
          // loading of a newer version of msxml dll (msxml3 - msxml5) failed
          // use fallback solution
          // old style msxml version independent, deprecated
          if( i == 2 ) {
            new_xmlHttp = new ActiveXObject("Microsoft.XMLHTTP");
          }
          // try to use the latest msxml dll
          else {
            new_xmlHttp = new ActiveXObject( "Msxml2.XMLHTTP." + i + ".0" );
          }
          break;
        }
        catch( excNotLoadable ) {
          new_xmlHttp = false;
        }
      }
    }
    // Mozilla, Opera und Safari
    else if( window.XMLHttpRequest ) {
      new_xmlHttp = new XMLHttpRequest();
    }
  }
  catch( excNotLoadable ) {
    new_xmlHttp = false;
  }

  new_xmlHttp.onreadystatechange = stateCallback;

  xmlHttp = new_xmlHttp;
}

function AJAX_get(url) {
  if( xmlHttp ) {
    xmlHttp.abort();
    xmlHttp = false;
  }

  init_AJAX();
  xmlHttp.open("GET", url, true);
  xmlHttp.send(null);
}

var pretab = "";
var pretd = "";
function extendmenu(mid, aobject) {
    var tab_name = "sc"+mid;
    var td_name = "td"+mid;
    
    if(pretab != "") {
        document.getElementById(pretab).style.display = "none"
	document.getElementById(pretd).style.backgroundColor="#FFFFFF";
	document.getElementById(pretd).style.fontWeight="normal";
	document.getElementById(pretd).style.color="#000000";
    }
    
    document.getElementById(tab_name).style.display = "block"
    document.getElementById(td_name).style.backgroundColor="navy";
    document.getElementById(td_name).style.fontWeight="bold";
    document.getElementById(td_name).style.color="#FFFFFF";
    pretab = tab_name;
    pretd = td_name;
}

function send_command(cmd) {
    document.getElementById('hints').firstChild.nodeValue = "Send command: " + cmd;
    AJAX_get('/?action=command&command='+ cmd)
}

function AJAX_response(text) {
    document.getElementById('hints').firstChild.nodeValue = "Got response: " + text;
}

function KeyDown(ev) {
    ev = ev || window.event;
    pressed = ev.which || ev.keyCode;
    
    switch (pressed) {
    case 37:
        //send_command('pan_plus');
        break;
    case 39:
        //send_command('pan_minus');
        break;
    case 38:
        //send_command('tilt_minus');
        break;
    case 40:
        //send_command('tilt_plus');
        break;
    case 32:
        //send_command('reset_pan_tilt');
        break;
    default:
        break;
    }
}

document.onkeydown = KeyDown;

/* Copyright (C) 2007 Richard Atterer, richard©atterer.net
   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License, version 2. See the file
   COPYING for details. */

var imageNr = 0; // Serial number of current image
var finished = new Array(); // References to img objects which have finished downloading
var paused = false;

function createImageLayer() {
    var img = new Image();
    img.style.position = "absolute";
    img.style.zIndex = -1;
    img.onload = imageOnload;
    //  img.onclick = imageOnclick;
    img.src = "/?action=snapshot&n=" + (++imageNr);
    var video = document.getElementById("video");
    var ctx=document.getElementById("videocanvas").getContext("2d");    
    ctx.drawImage(img, 0,0);
    video.insertBefore(img, video.firstChild);
}

// Two layers are always present (except at the very beginning), to avoid flicker
function imageOnload() {
  this.style.zIndex = imageNr; // Image finished, bring to front!
  while (1 < finished.length) {
    var del = finished.shift(); // Delete old image(s) from document
    del.parentNode.removeChild(del);
  }
  finished.push(this);
  if (!paused) createImageLayer();
}

// function createImageLayer() {
//     var img = new Image();
//     img.style.position = "absolute";
//     //img.style.zIndex = -1;
//     img.onload = imageOnload;
//     //  img.onclick = imageOnclick;
//     img.src = "/?action=snapshot&n=" + (++imageNr);
//     var video = document.getElementById("video");
//     var ctx=video.getContext("2d");    
//     ctx.drawImage(img, 0,0);
// }

// // Two layers are always present (except at the very beginning), to avoid flicker
// function imageOnload() {
//   this.style.zIndex = imageNr; // Image finished, bring to front!
//   while (1 < finished.length) {
//     var del = finished.shift(); // Delete old image(s) from document
//     //del.parentNode.removeChild(del);
//   }
//   finished.push(this);
//   if (!paused) createImageLayer();
// }


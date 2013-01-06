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
      if( stat == 200 ) {   // success
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

function SendCommand(cmd) {
    document.getElementById('hints').firstChild.nodeValue = "Send command: " + cmd;
    AJAX_get('/?action=command&command='+ cmd)
}

function AJAX_response(text) {
    document.getElementById('hints').firstChild.nodeValue = "Got response: " + text;
    console.log("|%s|",text.substring(0,32));
    if ( text.substring(0,15) == "processingmode=") {
	ResponseProcessingMode( text.substring(15) );
    } else if ( text.substring(0,8) == "control=" ) {
	ResponseControl(text.substring(8));
    }
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

var imageNr = 0; // Serial number of current image
var paused = false;  

function onclickPausePlayButton(b) {
    paused = !paused;
    if ( paused ) {
	b.value = "Play";
	b.innerHTML = "Play";
    } else {
	b.value = "Pause";
	b.innerHTML = "Pause";
    }
    if (! paused ) CreateImageLayer();
}

function CreateImageLayer() {
    var img = new Image();
    img.style.position = "absolute";
    img.style.zIndex = 0;
    img.onload = imageOnload;
    img.src = "/?action=snapshot&n=" + (++imageNr);
//    var ctx=document.getElementById("videocanvas").getContext("2d");    
    ctx.drawImage(img, 0,0);
}

function imageOnload() {
//    var ctx=document.getElementById("videocanvas").getContext("2d");    
    ctx.drawImage(this, 0,0);
    if (!paused) CreateImageLayer();
}

function OnChangeProcessingMode() {
    sb = document.getElementById('processingMode');
    SendCommand("processingmode" + "&" + "mode=" + sb.value) 
    
    return false;
}

var count = 0;
function InitCanvas() {
    var imgd = ctx.getImageData(0, 0, width, height);
    var pix = imgd.data;
    var bpp = 4;
    var bpl = width * bpp;

    count++;

// Loop over each pixel and invert the color.
    for (var i = 0; i < height; i++ ) {
        for( var j = 0; j < width; j++ ) {
            pix[i * bpl + j * bpp + 0 ] = i & 0xff; // red
            pix[i * bpl + j * bpp + 1 ] = j & 0xff; // green
            pix[i * bpl + j * bpp + 2 ] = count & 0xff; // blue
	    pix[i * bpl + j * bpp + 3 ] = 255;
            // i+3 is alpha (the fourth element)
        }
    }
//    console.log("Image data %d %d %d %d %d %d", imgd.data[0], imgd.data[1], imgd.data[2], imgd.data[3], imgd.data[4], imgd.data[5]);
    // Draw the ImageData at the given (x,y) coordinates.
    ctx.putImageData(imgd, 0, 0);
}

function onclickResetColour() {
    document.getElementById("redminButton").value = 255;
    document.getElementById("greenminButton").value = 255;
    document.getElementById("blueminButton").value = 255;

    document.getElementById("redmaxButton").value = 0;
    document.getElementById("greenmaxButton").value = 0;
    document.getElementById("bluemaxButton").value = 0;

    document.getElementById("redgreenminButton").value = 255;
    document.getElementById("redblueminButton").value = 255;
    document.getElementById("greenblueminButton").value = 255;

    document.getElementById("redgreenmaxButton").value = -255;
    document.getElementById("redbluemaxButton").value = -255;
    document.getElementById("greenbluemaxButton").value = -255;

    onchangeColourDefinition();
}

function onchangeColourDefinition( ) {
    console.log("Sendign new colour definition");
    SendCommand("updatecolour" + "&" + "{" +
		document.getElementById("colourName").value + "&" + 
		document.getElementById("redminButton").value + "&" +
		document.getElementById("greenminButton").value + "&" + 
		document.getElementById("blueminButton").value + "&" +

		document.getElementById("redmaxButton").value + "&" + 
		document.getElementById("greenmaxButton").value + "&" + 
		document.getElementById("bluemaxButton").value + "&" + 

		document.getElementById("redgreenminButton").value + "&" + 
		document.getElementById("redblueminButton").value + "&" + 
		document.getElementById("greenblueminButton").value + "&" + 

		document.getElementById("redgreenmaxButton").value + "&" + 
		document.getElementById("redbluemaxButton").value + "&" + 
		document.getElementById("greenbluemaxButton").value + "&" + 

		0 + "&" + 
		0 + "&" + 
		0 + "&" + 

		255 + "&" + 
		255 + "&" + 
		255 + "}"
	       );
}

function onmouseupVideo( event ) {
    mouseDown = false;
}

function onmousedownVideo( event ) {
    mouseDown = true;
}

function onmouseclickVideo( event ) {
    if ( mouseDown ) {
	var x = event.clientX - clientRect.left;
	var y = event.clientY - clientRect.top;
    
	//console.log("Reading position (" + x + "," + y + ")");

	var imgd = ctx.getImageData( x, y, 1, 1 );
	var pix = imgd.data;
	
	//console.log("Pixel " + pix[0] + "," + pix[1] + "," + pix[2] );
	AddPixelToColourDefinition( pix );
    }
}

var video;
var ctx;
var width;
var height;
var clientRect;
var mouseDown;

function InitVisionModule() {
    video=document.getElementById("videocanvas");
    ctx=video.getContext("2d");
    width=video.width;
    height=video.height;
    clientRect = video.getBoundingClientRect();

    video.addEventListener('mousedown',onmousedownVideo, false);
    video.addEventListener('mousemove',onmouseclickVideo, false);
    video.addEventListener('mouseup',onmouseupVideo, false);
    video.addEventListener('mouseout',onmouseupVideo, false);

    mouseDown = false;

    SendCommand("processingmode" + "&" + "mode=query");
    
    CreateImageLayer();
}

function AddPixelToColourDefinition( pix ) {
    var red = pix[0];
    var green = pix[1];
    var blue = pix[2];

    if ( ( ( red > 0 ) || ( green > 0 ) || ( blue > 0 ) ) && ( ( red != 255 ) || ( green != 0 ) || ( blue != 0 ) ) ) {
	var redgreen = red - green;
	var redblue = red - blue;
	var greenblue = green - blue;
	var sum = 1 + red + green + blue;
	var redratio = red / sum;
	var greenratio = green / sum;
	var blueratio = blue / sum;
	var b;
	var changed = false;
	
//	console.log("adding pixel " + red + "," + green + "," + blue + "," + redgreen + "," + redblue + "," + greenblue + "," + redratio + "," + greenratio + "," + blueratio );
	
	b = document.getElementById("redminButton");
//	console.log("Typeof: b.value=" + typeof(b.value) );
//	console.log("Typeof: red=" + typeof(red) );
	
	if ( parseInt(b.value,10) > red ) {
	    b.value = red;
	    changed = true;
	}
	
	b = document.getElementById("greenminButton");
	if ( parseInt(b.value,10) > green ) {
	    b.value = green;
	    changed = true;
	}
	
	b = document.getElementById("blueminButton");
	if ( parseInt(b.value,10) > blue ) {
	    b.value = blue;
	    changed = true;
	}
	
	b = document.getElementById("redmaxButton");
	if ( parseInt(b.value,10) < red ) {
	    b.value = red;
	    changed = true;
	}
	
	b = document.getElementById("greenmaxButton");
	if ( parseInt(b.value,10) < green ) {
	    b.value = green;
	    changed = true;
	}
	
	b = document.getElementById("bluemaxButton");
	if ( parseInt(b.value,10) < blue ) {
	    b.value = blue;
	    changed = true;
	}
	
	b = document.getElementById("redgreenminButton");
	if ( parseInt(b.value,10) > redgreen ) {
	    b.value = redgreen;
	    changed = true;
	}
	
	b = document.getElementById("redblueminButton");
	if ( parseInt(b.value,10) > redblue ) {
	    b.value = redblue;
	    changed = true;
	}
	
	b = document.getElementById("greenblueminButton");
	if ( parseInt(b.value,10) > greenblue ) {
	    b.value = greenblue;
	    changed = true;
	}
	
	b = document.getElementById("redgreenmaxButton");
	if ( parseInt(b.value,10) < redgreen ) {
	    b.value = redgreen;
	    changed = true;
	}
	
	b = document.getElementById("redbluemaxButton");
	if ( parseInt(b.value,10) < redblue ) {
	    b.value = redblue;
	    changed = true;
	}
	
	b = document.getElementById("greenbluemaxButton");
	if ( parseInt(b.value,10) < greenblue ) {
	    b.value = greenblue;
	    changed = true;
	}
	console.log("Changed = " + changed );
	if ( changed ) {
	    onchangeColourDefinition();
	}
    }
}

function ResponseProcessingMode( text ) {
    var s = document.getElementById("processingMode");

    if ( text == "raw" ) {
	s.value = "raw";
    } else if ( text == "showcolours" ) {
	s.value = "showcolours";
    } else if ( text == "segmentcolours" ) {
	s.value = "segmentcolours";
    }	
} 

function ResponseControl( text ) {
    var index = text.indexOf("&");
    var control;
    var value = ""

    if ( index > 0 ) {
	control = text.substring(0,index);
    }
    else {
	control = text;
    }
    
    if ( text.substring(index+1,index+1+6) == "value=") {
	value = text.substring(index+1+6);
    }
    var ctrl = document.getElementById(control + "Button");
    ctrl.value = parseInt(value,10);
}

function OnChangeVideoControl( ctrl ) {
    SendCommand("videocontrol" + "&" + "control=" + ctrl.name + "&value=" + ctrl.value); 
}
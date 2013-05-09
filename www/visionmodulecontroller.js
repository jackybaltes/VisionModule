// Javascript functions for the VisionModule
// Jacky Baltes <jacky@cs.umanitoba.ca> Sun Jan  6 21:19:09 CST 2013
//

var resetColour;

function AJAX_stateCallback( req ) {
  var stat, rstate;
  if( ! req ) return;

  try {
    rstate = req.readyState;
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
          stat = req.status;
      }
      catch (err) {
        stat = "req.status does not exist";
      }
      if( stat == 200 ) {   // success
          AJAX_response(req.responseText);
      }
      // loading not successfull, e.g. page not available
      else { 
      }
  }
}

function AJAX_init() {
    var new_xmlHttp = null;
    
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
	new_xmlHttp = null;
    }
 
    if (new_xmlHttp != null ) {
	new_xmlHttp.onreadystatechange = function() {
	    AJAX_stateCallback( new_xmlHttp );
	}
    }
    return new_xmlHttp;
}

function AJAX_get(req,url) {
    req.open("GET", url, true);
    req.send(null);
}

function AJAX_put(req, url, file) {
    req.open("POST", url, true);
    var fd = new FormData();
    fd.append(url,file);
    req.send(fd);
}

function SendCommand(cmd) {
    document.getElementById('hints').firstChild.nodeValue = "Send command: " + cmd;
    req = AJAX_init();
    if ( req != null )
    {
	AJAX_get(req, '/?action=command&command='+ cmd);
    }
}

function AJAX_response(text) {
    document.getElementById('hints').firstChild.nodeValue = "Got response: " + text;
    console.log("|%s|",text.substring(0,72));
    if ( text.substring(0,15) == "processingmode=") {
	ResponseProcessingMode( text.substring(15) );
    } else if ( text.substring(0,7) == "colour=" ) {
	ResponseColour(text.substring(7));
    } else if ( text.substring(0,8) == "control=" ) {
	ResponseControl(text.substring(8));
    } else if ( text.substring(0,11) == "colourlist=" ) {
	ResponseColourList(text.substring(11));
    } else if ( text.substring(0,13) == "colour added " ) {
	ResponseColourAdded( text.substring(13) );
    } else if ( text.substring(0,15) == "colour deleted " ) {
	ResponseColourDeleted( text.substring(15) );
    } else if ( text.substring(0,16) == "colour selected " ) {
	ResponseColourSelected( text.substring(16) );
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
    ctx.drawImage(img, 0,0);
}

function imageOnload() {
    ctx.drawImage(this, 0,0);
    if (!paused) CreateImageLayer();
}

function OnChangeProcessingMode() {
    var sb = document.getElementById('processingMode');
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
    UpdateColourParameters( resetColour );
}

function UpdateColourParameters( col ) {
    document.getElementById("redminButton").value = col.red_min;
    document.getElementById("greenminButton").value = col.green_min;
    document.getElementById("blueminButton").value = col.blue_min;

    document.getElementById("redmaxButton").value = col.red_max;
    document.getElementById("greenmaxButton").value = col.green_max;
    document.getElementById("bluemaxButton").value = col.blue_max;

    document.getElementById("redgreenminButton").value = col.redgreen_min;
    document.getElementById("redblueminButton").value = col.redblue_min;
    document.getElementById("greenblueminButton").value = col.greenblue_min;

    document.getElementById("redgreenmaxButton").value = col.redgreen_max;
    document.getElementById("redbluemaxButton").value = col.redblue_max;
    document.getElementById("greenbluemaxButton").value = col.greenblue_max;

    // document.getElementById("redratiominButton").value = col.redratio_min;
    // document.getElementById("greenratiominButton").value = col.greenratio_min;
    // document.getElementById("blueratiominButton").value = col.blueratio_min;

    // document.getElementById("redratiomaxButton").value = col.redratio_max;
    // document.getElementById("greenratiomaxButton").value = col.greenratio_max;
    // document.getElementById("blueratiomaxButton").value = col.blueratio_max;

    OnChangeColourDefinition();
}

function OnChangeColourDefinition( ) {
    console.log("Sending new colour definition");
    SendCommand("updatecolour" + "&" + "{" +
		colourNameSelector.value + "&" + 

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

// Need to replace with ratio input parameter holders
		0 + "&" + 
		0 + "&" + 
		0 + "&" + 

		255 + "&" + 
		255 + "&" + 
		255 + "}"
	       );
}

function OnChangeSelectedColour( sel ) {
    var c = sel.value;
    SendCommand("querycolour" + "&" + "colour=" + c);
    SendCommand("selectcolour" + "&" + "name=" + c);
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
	
	console.log("Pixel(" + x + "," + y + ")" + "=" + pix[0] + "," + pix[1] + "," + pix[2] );
	AddPixelToColourDefinition( pix );
    }
}

var video;
var ctx;
var width;
var height;
var clientRect;
var mouseDown;
var colourNameSelector;

function InitVisionModule() {
    video=document.getElementById("videocanvas");
    ctx=video.getContext("2d");
    colourNameSelector =  document.getElementById( "colourNameSelector" );

    width=video.width;
    height=video.height;
    clientRect = video.getBoundingClientRect();

    SendCommand("querycolourlist");

    video.addEventListener('mousedown',onmousedownVideo, false);
    video.addEventListener('mousemove',onmouseclickVideo, false);
    video.addEventListener('mouseup',onmouseupVideo, false);
    video.addEventListener('mouseout',onmouseupVideo, false);

    mouseDown = false;

    resetColour = ArrayToColour(["reset","255","255","255","0","0","0","255","255","255","-255","-255","-255","255","255","255","0","0","0"]);

    SendCommand("processingmode" + "&" + "mode=query");
    
    SendCommand("videocontrol" + "&" + "control=" + "brightness" + "&" + "value=" + "query");

    SendCommand("videocontrol" + "&" + "control=" + "contrast" + "&" + "value=" + "query");

    SendCommand("videocontrol" + "&" + "control=" + "saturation" + "&" + "value=" + "query");

    SendCommand("videocontrol" + "&" + "control=" + "sharpness" + "&" + "value=" + "query");

    SendCommand("videocontrol" + "&" + "control=" + "gain" + "&" + "value=" + "query");

//    UpdateColourSelection();
    $( "#add-colour-dialog-modal" ).dialog("close"); 

    CreateImageLayer();
}

function RemoveChildren( sel ) {
    var n = sel.firstChild;
    while ( n ) {
	var n2 = n.nextSibling;
	sel.removeChild(n);
	n = n2;
    }
}

function AddArrayToSelection(sel, arr ) {
    for(var i=0, len=arr.length; i < len; i++) {
	var c = arr[i];
	var e = new Option( c, c );

	sel.appendChild(e);
    }    
}

function UpdateColourSelection( colours ) {
    RemoveChildren( colourNameSelector );
    AddArrayToSelection( colourNameSelector, colours );
    UpdateCurrentColourParameters( );
}

function UpdateCurrentColourParameters() {
    OnChangeSelectedColour( colourNameSelector );
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
	    OnChangeColourDefinition();
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

function ResponseColour( text ) {
    var col = TextToColour( text );
    UpdateColourParameters( col );
}

function ResponseColourList( text ) {
    var colours = TextToColourList( text );
    UpdateColourSelection( colours );
}

function ResponseColourAdded() {
    SendCommand("querycolourlist");
}

function ResponseColourDeleted() {
    SendCommand("querycolourlist");
}

function ResponseColourSelected() {
}

function ArrayToColour( t ) {
    var col;

    if ( t.length == 19 ) {
	col = {};

	col.name = t[0];

	col.red_min = parseInt( t[1] );
	col.green_min = parseInt( t[2] );
	col.blue_min = parseInt( t[3] );

	col.red_max = parseInt( t[4] );
	col.green_max = parseInt( t[5] );
	col.blue_max = parseInt( t[6] );

	col.redgreen_min = parseInt( t[7] );
	col.redblue_min = parseInt( t[8] );
	col.greenblue_min = parseInt( t[9] );

	col.redgreen_max = parseInt( t[10] );
	col.redblue_max = parseInt( t[11] );
	col.greenblue_max = parseInt( t[12] );

	col.redratio_min = parseInt( t[13] );
	col.greenratio_min = parseInt( t[14] );
	col.blueratio_min = parseInt( t[15] );

	col.redratio_max = parseInt( t[16] );
	col.greenratio_max = parseInt( t[17] );
	col.blueratio_max = parseInt( t[18] );	
    }
    return col;
}

function TextToColour( text ) {
    var s;
    var col;

    if ( ( text[0] == "{" ) && ( text[text.length-1] == "}" ) ) {
	text = text.substring(1,text.length-1);
    }

    var t = text.split("&");

    if ( t.length == 19 ) {
	col = ArrayToColour( t );
    }
    return col;
}

function TextToColourList( text ) {

    if ( ( text[0] == "{" ) && ( text[text.length-1] == "}" ) ) {
	text = text.substring(1,text.length-1);
    }

    var colours = text.split("&");

    return colours;
}

function OnChangeVideoControl( ctrl ) {
    SendCommand("videocontrol" + "&" + "control=" + ctrl.name + "&value=" + ctrl.value); 
}

function OnClickAddColour( ) {
    $( "#add-colour-dialog-modal" ).dialog("open"); 
}

function AddColour( ) {
    var inp = document.getElementById("add-colour-id");
    var name = inp.value;
    if ( name != "" ) {
	var n = colourNameSelector.firstChild;
	while ( n ) {
	    if ( n.value == name ) {
		alert("Colour " + name + " already defined");
		break;
	    }
	    n = n.nextSibling;
	}
	if ( ! n ) {
	    SendCommand( "addcolour" + "&" + "name=" + name );
	    //	    e = new Option( name, name );
	    //colourNameSelector.appendChild( e );
	}
    }
}

function OnClickDeleteColour( ) {
    SendCommand("deletecolour" + "&" + "name=" + colourNameSelector.value );
}

function DownloadConfiguration( ) {
    window.open( "__config__.cfg", "Download Configuration" );
}

function UploadConfiguration( inp ) {
    if ( inp.files.length == 1 ) {
	var file = inp.files[0];

	var xhr = new XMLHttpRequest();
	xhr.file = file; // not necessary if you create scopes like this
	xhr.addEventListener('progress', function(e) {
            var done = e.position || e.loaded, total = e.totalSize || e.total;
            console.log('xhr progress: ' + (Math.floor(done/total*1000)/10) + '%');
	}, false);
	
	if ( xhr.upload ) {
            xhr.upload.onprogress = function(e) {
		var done = e.position || e.loaded, total = e.totalSize || e.total;
		console.log('xhr.upload progress: ' + done + ' / ' + total + ' = ' + (Math.floor(done/total*1000)/10) + '%');
            };
	}
	
	xhr.onreadystatechange = function(e) {
            if ( 4 == this.readyState ) {
		console.log(['xhr upload complete', e]);
            }
	};

	AJAX_put( xhr, '/__config__.cfg', file );
    }
}

function OnClickShutdown( ) {
    SendCommand( "shutdown" );
}
static const char canvas_htm[] PROGMEM = "<!-- Author : Mudassar Tamboli --> 
 
<!DOCTYPE html> 
<html> 
<title> ESP32-OV7670 - Websocket Demo By Mudassar Tamboli </title> 
<script type = "text/javascript"> 
    var ws = null; 
    var r = 0; 
    var capturecount = 1; 
    var ln = 0; 
    var flag = 0; 
    var xres = 0; 
    var yres = 0; 
    var canvas; 
    var ctx; 
    var imgData; 
    var gcanvasid = "canvas-QQQ-VGA";     
    var camera_ip = "192.168.4.1"; 
 
    function initWebSocket() { 
     
        if ("WebSocket" in window) { 
            if (ws != null) { 
                ws.close(); 
            }  
           
            ws = new WebSocket('ws://' +  camera_ip + ':81/', ['arduino']); 
            if (ws == null) { 
                document.getElementById("connecting").innerText = "Failed to connect to camera [ " + camera_ip + " ]"; 
                return;     
            } 
            ws.binaryType = 'arraybuffer'; 
 
 
            // open websocket  
            ws.onopen = function() { 
                document.getElementById("canvas7670").style.visibility = "visible"; 
                document.getElementById("connecting").style.visibility = "hidden"; 
                document.getElementById("constatus").innerText = "Connected to " + ws.url; 
                if (gcanvasid != null && gcanvasid != "") { 
                    capture(gcanvasid); 
                } 
            };//ws.onopen 
            
            // receive message  
            ws.onmessage = function (evt) {  
                var arraybuffer = evt.data; 
                if (arraybuffer.byteLength == 1) { 
                    flag  = new Uint8Array(evt.data); // Start Flag 
                    if (flag == 0xAA) { 
                        ln = 0;                    
                    } 
                    if (flag == 0xFF) { 
                       //alert("Last Block"); 
                    } 
 
                    if (flag == 0x11) { 
                        //alert("Camera IP"); 
                    } 
 
                } else { 
 
                    if (flag == 0x11) { 
                        //alert("Camera IP " + evt.data); 
                        camera_ip = evt.data; 
                        document.getElementById("wifi-ip").innerText = camera_ip; 
                        flag = 0;          
                    } else { 
                        var bytearray = new Uint8Array(evt.data); 
                        display(bytearray, arraybuffer.byteLength, flag); 
                    } 
                 } 
 
               }; //ws.onmessage 
             
            // close websocket 
               ws.onclose = function() {  
                   document.getElementById("canvas7670").style.visibility = "hidden"; 
                   document.getElementById("connecting").style.visibility = "visible"; 
               }; //ws.onclose 
 
               // websocket error handling 
               ws.onerror = function(evt) { 
                   document.getElementById("canvas7670").style.visibility = "hidden"; 
                   document.getElementById("connecting").style.visibility = "visible"; 
                   document.getElementById("connecting").innerText = "Error " + evt.data; 
                   document.getElementById("constatus").innerText = ""; 
               }; 
      
            } else { 
            // The browser doesn't support WebSocket 
                alert("WebSocket NOT supported by your Browser!"); 
            } 
        } // WebSocketCamera 
 
 
function onloadinit() { 
   camera_ip = location.hostname; 
   //alert(camera_ip); 
   init(); 
} 
function init() { 
    document.getElementById("canvas7670").style.visibility = "hidden"; 
    document.getElementById("connecting").style.visibility = "visible"; 
    initCanvas(); 
    initWebSocket();  
} 
 
function initCanvas() { 
    var canvas = document.getElementById("canvas-QQQ-VGA"); 
    var ctx = canvas.getContext("2d"); 
    ctx.font = "10px Comic Sans MS"; 
    ctx.fillStyle = "#FF0000"; 
    ctx.textAlign = "center"; 
    ctx.fillText("80 x 60", canvas.width/2, canvas.height/2);     
 
    canvas = document.getElementById("canvas-QQ-VGA"); 
    ctx = canvas.getContext("2d"); 
    ctx.font = "10px Comic Sans MS"; 
    ctx.fillStyle = "#FF0000"; 
    ctx.textAlign = "center"; 
    ctx.fillText("160 x 120", canvas.width/2, canvas.height/2);     
 
    canvas = document.getElementById("canvas-Q-VGA"); 
    ctx = canvas.getContext("2d"); 
    ctx.font = "10px Comic Sans MS"; 
    ctx.fillStyle = "#FF0000"; 
    ctx.textAlign = "center"; 
    ctx.fillText("320 x 240", canvas.width/2, canvas.height/2);     
 
/* 
    canvas = document.getElementById("canvas-VGA"); 
    ctx = canvas.getContext("2d"); 
    ctx.font = "10px Comic Sans MS"; 
    ctx.fillStyle = "#FF0000"; 
    ctx.textAlign = "center"; 
    ctx.fillText("640 x 480", canvas.width/2, canvas.height/2);     
*/     
 
} 
 
//https://github.com/ThingPulse/minigrafx/issues/8 
function display(pixels, pixelcount, flag) { 
    //alert('display');  
    var i = 0; 
    for(y=0; y < yres; y++) { 
       for(x=0; x < xres; x++) 
       {  
           i = (y * xres + x) << 1; 
           pixel16 = (0xffff & pixels[i]) | ((0xffff & pixels[i+1]) << 8); 
           imgData.data[ln+0] = ((((pixel16 >> 11) & 0x1F) * 527) + 23) >> 6; 
           imgData.data[ln+1] = ((((pixel16 >> 5) & 0x3F) * 259) + 33) >> 6; 
           imgData.data[ln+2] = (((pixel16 & 0x1F) * 527) + 23) >> 6;   
           imgData.data[ln+3] = (0xFFFFFFFF) & 255; 
           ln += 4; 
       } 
    } 
     
    if (flag == 0xFF) { // last block 
       ln = 0;         
       ctx.putImageData(imgData,0,0); 
       ws.send("next-frame");     
    } 
     
 } 
 
function reset() 
{ 
   r = 0; 
   capturecount = 1; 
   ln = 0; 
   flag = 0; 
   xres = 0; 
   yres = 0; 
   //gcanvasid = "";     
   initCanvas(); 
} 
 
function setip(whichip) { 
    if (gcanvasid != null && gcanvasid != "") {   
       camera_ip = document.getElementById(whichip).innerText; 
       init(); 
       initCanvas(); 
       capture(gcanvasid); 
    } 
} 
 
function capture(canvasid) 
{ 
    if (ws.readyState != 1) { 
    //    alert("ws.readyState " + ws.readyState);     
        return;       
    } 
  
    reset();     
    gcanvasid = canvasid;     
    canvas = document.getElementById(canvasid); 
    ctx = canvas.getContext('2d'); 
    if (canvasid.indexOf("canvas-VGA", 0) != -1) { 
        xres = 640; 
        yres = 60; 
    } else if (canvasid.indexOf("canvas-Q-VGA", 0) != -1) { 
        xres = 320; 
        yres = 120; 
    } else if (canvasid.indexOf("canvas-QQ-VGA", 0) != -1) { 
        xres = 160; 
        yres = 120; 
    } else if (canvasid.indexOf("canvas-QQQ-VGA", 0) != -1) { 
        xres = 80; 
        yres = 60; 
    } 
     
    imgData = ctx.createImageData(canvas.width, canvas.height); 
    for (var i=0;i<imgData.data.length;i+=4) 
    { 
        imgData.data[i+0]=0xCC; 
        imgData.data[i+1]=0xCC; 
        imgData.data[i+2]=0xCC; 
        imgData.data[i+3]=255; 
    } 
    ctx.putImageData(imgData, canvas.width, canvas.height); 
    ws.send(canvasid);     
} 
 
</script> 
 
<body onload="onloadinit()"> 
 
<div valign=center align=center style=" 
    display: inline-block; 
    position: fixed; 
    top: 0; 
    bottom: 0; 
    left: 0; 
    right: 0; 
    width: 200px; 
    height: 100px; 
    margin: auto; 
    background-color: #FFFFFF;" id="connecting">Connecting Camera ... 
</div> 
 
<center> <h1> ESP32-OV7670 Websocket Video Camera </h1> </center> 
 
 
<table align=center valign=center id="canvas7670"> 
    <tr>  
        <td valign="bottom">     
<canvas id="canvas-QQQ-VGA" width="80" height="60" style="cursor:crosshair;border:1px solid #FFFF00;" onclick="capture('canvas-QQQ-VGA')"> 
Your browser does not support the HTML5 canvas tag. 
</canvas> 
        </td> 
 
        <td valign="bottom">     
<canvas id="canvas-QQ-VGA" width="160" height="120" style="cursor:crosshair;border:1px solid #00FF00;" onclick="capture('canvas-QQ-VGA')"> 
Your browser does not support the HTML5 canvas tag. 
</canvas> 
        </td> 
 
        <td valign="bottom"> 
<canvas id="canvas-Q-VGA" width="320" height="240" style="cursor:crosshair;border:1px solid #0000FF;" onclick="capture('canvas-Q-VGA')"> 
Your browser does not support the HTML5 canvas tag. 
</canvas> 
        </td> 
<!-- 
        <td valign="bottom"> 
<canvas id="canvas-VGA" width="640" height="480" style="cursor:crosshair;border:1px solid #FF00FF;" onclick="capture('canvas-VGA')"> 
Your browser does not support the HTML5 canvas tag. 
</canvas> 
        </td> 
-->   
    </tr> 
 
    <tr>  
        <td align=center>     
            QQQ-VGA     <BR> 
        </td> 
 
        <td align=center>     
            QQ-VGA     <BR> 
        </td> 
     
        <td align=center> 
            Q-VGA     <BR> 
        </td> 
     
<!-- 
        <td align=center>     
            VGA     <BR> 
        </td> 
-->   
    </tr> 
 
 
</table> 
<BR><BR> 
<table width=30% align=center bgcolor="#FFFF00" > 
 <tr align=center style="color: #fff; background: black;">  
   <td id="constatus" colspan=2>Websocket not connected</td>  
 </tr> 
 
 <tr align=center style="color: #fff; background: black;">  
   <td>AP IP </td>  
   <td>WiFi IP</td>  
 </tr> 
 
 <tr align=center bgcolor="#FFF">  
   <td id="ap-ip" style="cursor:pointer" onclick="setip('ap-ip')">192.168.4.1</td>  
   <td id="wifi-ip" style="cursor:pointer" onclick="setip('wifi-ip')"></td>  
 </tr> 
          
</table>  
 
 
</body> 
</html> 
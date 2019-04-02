
//Author : Mudassar Tamboli

bool SendDataFrame = false;
bool MoveMotor  = true;
bool MotorInCourse;

#include "OV7670.h"

#include <WebSockets.h>
#include <WebSocketsClient.h>
#include <WebSocketsServer.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClient.h>
#include "canvas_htm.h"

const char *ap_ssid     = "Esp32AP";
const char *ap_password = "thereisnospoon";

const char *ssid_AP_1 = "WZ4_2.4Ghz";
const char *pwd_AP_1  = "wz4@vslmgtc$";

const char *ssid_AP_2 = "kramm2@148";
const char *pwd_AP_2  = "5134992001";

const char *ssid_AP_3 = "Gaia-Proj";
const char *pwd_AP_3  = "G@ia3_2018"; 

   
//------------------------------------------------
OV7670 *camera;
const int SIOD = 21; //SDA
const int SIOC = 22; //SCL

const int VSYNC = 34;
const int HREF = 35;

const int XCLK = 32;
const int PCLK = 33;

const int D0 = 27;
const int D1 = 17;
const int D2 = 16;
const int D3 = 15; //TD0
const int D4 = 14; //TMS
const int D5 = 13; //TCK
const int D6 = 12; //TDI
const int D7 = 4; 

const int TFT_DC = 2;
const int TFT_CS = 5;
//DIN <- MOSI 23
//CLK <- SCK 18
const int ChangeFormatDelay = 2000;
//------------------------------------------------
#include <AccelStepper.h> //http://www.airspayce.com/mikem/arduino/AccelStepper/

// Motor pin definitions
const int motorPin1 = 26;     // IN1 on the ULN2003 driver 1
const int motorPin2 = 18;    // IN2 on the ULN2003 driver 1
const int motorPin3 = 19;   // IN3 on the ULN2003 driver 1
const int motorPin4 = 23;     // IN4 on the ULN2003 driver 1
//------------------------------------------------

//MH-ET Live

//            _________________
//           /    __________   \
//  GND-RST |    |          |  | 1  -GND
//  NC - 36 |    |  ESP-32  |  | 3  - 27
//  39 - 26 |    |          |  | 22 - 25
//  35 - 18 |    |          |  | 21 - 32
//  33 - 19 |    |          |  | 17 - 12
//  34 - 23 |    |__________|  | 16 -  4
//  14 -  5 |                  | GND-  0
//  NC -3V3 |                  | VCC-  2
//  9  - 13 |                  | 15 -  8
//  11 - 10 |_                 | 7  -  6
//            |      ___       |
//            |_____|USB|______|

//            _________________              //            _________________
//           /                 \             //           /                 \
//  GND-TXD |* * MH-ET Live * *| RST-GND     //  GND-  1 |* * MH-ET Live * *| RST-GND
// .27 -RXD |* *            * *| SVP- NC     //  27 -  3 |* *            * *| 36 - NC
//  25 - 22.|* *            * *| 26 -SVN     //  25 - 22 |* *            * *| 26 - 39
// .32 - 21.|* *            * *| 18   35.    //  32 - 21 |* *            * *| 18   35
// .TDI- 17.|* *            * *| 19 - 33.    //  12 - 17 |* *            * *| 19 - 33
// .4  - 16.|* *            * *| 23 - 34.    //  4  - 16 |* *            * *| 23 - 34
//  0  -GND |* *            * *|.5  -TMS.    //  0  -GND |* *            * *| 5  - 14
// .2  -VCC |* *            * *| 3V3- NC     //  2  -VCC |* *            * *| 3V3- NC
//  SD1-TD0.|* *            * *|.TCK-SD2     //  8  - 15 |* *            * *| 13 -  9
//  CLK-SD0 |* *            * *| SD3-CMD     //  6  -  7 |* *            * *| 10 - 11
//           \       ___       |             //           \       ___       |             
//            |_____|USB|______|             //            |_____|USB|______|      

//------------------------------------------------
// Initialize with pin sequence IN1-IN3-IN2-IN4 for using the AccelStepper with 28BYJ-48
#define HALFSTEP 8
AccelStepper stepper1(HALFSTEP, motorPin1, motorPin3, motorPin2, motorPin4);
#define movement  4097//*10
#define velocity  1500



//------------------------------------------------
#define ssid        "whiz4"
#define password    "salvation1234"
#define ssid2        ""
#define password2    ""



WiFiMulti wifiMulti;
WiFiServer server(80);

unsigned char pix = 0;

//unsigned char bmpHeader[BMP::headerSize];

unsigned char start_flag = 0xAA;
unsigned char end_flag = 0xFF;
unsigned char ip_flag = 0x11;

WebSocketsServer webSocket(81);    // create a websocket server on port 81
//------------------------------------------------

//------------------------------------------------
void startWebSocket() { // Start a WebSocket server
  webSocket.begin();                          // start the websocket server
  webSocket.onEvent(webSocketEvent);          // if there's an incomming websocket message, go to function 'webSocketEvent'
  Serial.println("WebSocket server started.");
}
//------------------------------------------------

//------------------------------------------------
void startWebServer()
{
   server.begin();
   Serial.println("Http web server started.");
}
//------------------------------------------------

//------------------------------------------------
void serve()
{
  WiFiClient client = server.available();
  if (client) 
  {
    //Serial.println("New Client.");
    String currentLine = "";
    while (client.connected()) 
    {
      if (client.available()) 
      {
        char c = client.read();
        //Serial.write(c);
        if (c == '\n') 
        {
          if (currentLine.length() == 0) 
          {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.print(canvas_htm);
            client.println();
            break;
          } 
          else 
          {
            currentLine = "";
          }
        } 
        else if (c != '\r') 
        {
          currentLine += c;
        }
        
      }
    }
    // close the connection:
    client.stop();

  }  
}
//------------------------------------------------

//------------------------------------------------
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t payloadlength) { // When a WebSocket message is received
 
  int blk_count = 0;

  char canvas_VGA[] = "canvas-VGA";
  char canvas_Q_VGA[] = "canvas-Q-VGA";
  char canvas_QQ_VGA[] = "canvas-QQ-VGA";
  char canvas_QQQ_VGA[] = "canvas-QQQ-VGA";
  char ipaddr[26] ;
  IPAddress localip;
  
  switch (type) {
    case WStype_DISCONNECTED:             // if the websocket is disconnected
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {              // if a new websocket connection is established
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
           webSocket.sendBIN(0, &ip_flag, 1);
           localip = WiFi.localIP();
           sprintf(ipaddr, "%d.%d.%d.%d", localip[0], localip[1], localip[2], localip[3]);
           webSocket.sendTXT(0, (const char *)ipaddr);
           
      }
      break;
    case WStype_TEXT:                     // if new text data is received
      if (payloadlength == sizeof(canvas_QQQ_VGA)-1) {
        if (memcmp(canvas_QQQ_VGA, payload, payloadlength) == 0) {
              Serial.printf("canvas_QQQ_VGA");
              webSocket.sendBIN(0, &end_flag, 1);
              camera = new OV7670(OV7670::Mode::QQQVGA_RGB565, SIOD, SIOC, VSYNC, HREF, XCLK, PCLK, D0, D1, D2, D3, D4, D5, D6, D7);
        }
      } else if (payloadlength == sizeof(canvas_QQ_VGA)-1) {
        if (memcmp(canvas_QQ_VGA, payload, payloadlength) == 0) {
              Serial.printf("canvas_QQ_VGA");
              webSocket.sendBIN(0, &end_flag, 1);
              camera = new OV7670(OV7670::Mode::QQVGA_RGB565, SIOD, SIOC, VSYNC, HREF, XCLK, PCLK, D0, D1, D2, D3, D4, D5, D6, D7);
        }
      } else if (payloadlength == sizeof(canvas_Q_VGA)-1) {
        if (memcmp(canvas_Q_VGA, payload, payloadlength) == 0) {
              Serial.printf("canvas_Q_VGA");
              webSocket.sendBIN(0, &end_flag, 1);
              camera = new OV7670(OV7670::Mode::QVGA_RGB565, SIOD, SIOC, VSYNC, HREF, XCLK, PCLK, D0, D1, D2, D3, D4, D5, D6, D7);
        }
      } else if (payloadlength == sizeof(canvas_VGA)-1) {
        if (memcmp(canvas_VGA, payload, payloadlength) == 0) {
              Serial.printf("canvas_VGA");
              webSocket.sendBIN(0, &end_flag, 1);
              camera = new OV7670(OV7670::Mode::VGA_RGB565, SIOD, SIOC, VSYNC, HREF, XCLK, PCLK, D0, D1, D2, D3, D4, D5, D6, D7);
        }
      } 

      
      blk_count = camera->yres/I2SCamera::blockSlice;//30, 60, 120
      for (int i=0; i<blk_count; i++) 
      {

          if (i == 0) {
              camera->startBlock = 1;
              camera->endBlock = I2SCamera::blockSlice;
              webSocket.sendBIN(0, &start_flag, 1);
          }

          if (i == blk_count-1) {
              webSocket.sendBIN(0, &end_flag, 1);
          }
        
          camera->oneFrame();
          webSocket.sendBIN(0, camera->frame, camera->xres * I2SCamera::blockSlice * 2);
          camera->startBlock += I2SCamera::blockSlice;
          camera->endBlock   += I2SCamera::blockSlice;
      }
      
      break;
    case WStype_ERROR:                     // if new text data is received
      Serial.printf("Error \n");
    default:
      Serial.printf("WStype %x not handled \n", type);

  }
}
//------------------------------------------------

//------------------------------------------------
void initWifiStation() {

    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(ssid, password);    
    Serial.print("\nConnecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) 
    {
       delay(5000);        
       Serial.print(".");
    }
    Serial.println(String("\nConnected to the WiFi network (") + ssid + ")" );

    Serial.print("\nStation IP address: ");
    Serial.println(WiFi.localIP()); 

}
//------------------------------------------------

//------------------------------------------------
void initWifiMulti()
{
    wifiMulti.addAP(ssid_AP_1, pwd_AP_1);
    wifiMulti.addAP(ssid_AP_2, pwd_AP_2);
    wifiMulti.addAP(ssid_AP_3, pwd_AP_3);

    Serial.println("Connecting Wifi...");
    while(wifiMulti.run() != WL_CONNECTED) 
    {
       delay(5000);        
       Serial.print(".");
    }
    
    Serial.print("\n");
    Serial.print("WiFi connected : ");
    Serial.print("IP address : ");
    Serial.println(WiFi.localIP());
}
//------------------------------------------------

//------------------------------------------------
void setupMotor()
{
  stepper1.setMaxSpeed(velocity);
  stepper1.setAcceleration(500.0);
  stepper1.setCurrentPosition(0);
  stepper1.setSpeed(velocity);
  stepper1.moveTo(movement);
}
//------------------------------------------------
//Change direction when the stepper reaches the target position
//------------------------------------------------
void runMotor()
{
	int blk_count = 0;
	bool anyImage = false;
	if(MoveMotor)
	{
		static int videoMode = 0;
		int typeFrame = 0;
		long divisor;
		long startF = 0;
		long endF;
		long _frameBytes = 0;
		_frameBytes = OV7670::frameBytes;
		Serial.printf("\n%l\n", _frameBytes); 
		switch(_frameBytes)
		{
			case 9600:
			//80x60
			typeFrame = 0;
			Serial.printf("%d [\n", typeFrame); 
			divisor = 80*2;
			startF = _frameBytes/2;
			endF = _frameBytes;
			break;
			
			case 38400:
			//160x120
			typeFrame = 1;
			Serial.printf("%d [\n", typeFrame); 
			divisor = 160*2;
			startF = _frameBytes/2;
			endF = _frameBytes;
			break;
			
			case 76800:
			//320x240
			typeFrame = 2;
			Serial.printf("%d [\n", typeFrame); 
			divisor = 320*2;
			 //Divide por dois, pois vem só metade do frame em  nessa resolução
			//_frameBytes /= 2;
			startF = 0;
			endF = _frameBytes;
			break;

		}
		if(_frameBytes)
		{
			int pixel16;
			
			for(long i = startF; i < endF; i+=2)
			{
				//Monta o pixel RGB 565
				pixel16 = (OV7670::frame[i+1]<<8) + OV7670::frame[i];
				//Imprime o pixel
				Serial.printf("%04X\t",pixel16);
				//Serial.printf("0x%04X,\t",pixel16);
				//Serial.printf("%04X\t",i);
				
				//Quebra a linha de acordo com o tamanho da imagem
				if(!((i+2)%divisor))Serial.printf("\n");
				//Verifica se a imagem não é totalmente preta
				if(pixel16 > 0x0040)anyImage = true;
			}
			Serial.printf("\n]\n");
		}
		if(_frameBytes && anyImage)
		{
			switch(videoMode)
			{
				case 0:
					Serial.printf("canvas_QQQ_VGA");
					webSocket.sendBIN(0, &end_flag, 1);
					camera = new OV7670(OV7670::Mode::QQQVGA_RGB565, SIOD, SIOC, VSYNC, HREF, XCLK, PCLK, D0, D1, D2, D3, D4, D5, D6, D7);
					delay(ChangeFormatDelay);	//Delay para estabilizar a próxima imagem
					
					//Configura um novo giro
					MoveMotor = false;
					MotorInCourse = true;
					stepper1.setCurrentPosition(movement);
					stepper1.moveTo(0);
				break;
				
				case 1:
					Serial.printf("canvas_QQ_VGA");
					webSocket.sendBIN(0, &end_flag, 1);
					camera = new OV7670(OV7670::Mode::QQVGA_RGB565, SIOD, SIOC, VSYNC, HREF, XCLK, PCLK, D0, D1, D2, D3, D4, D5, D6, D7);
					delay(ChangeFormatDelay);	//Delay para estabilizar a próxima imagem
				break;
				
				case 2:
					Serial.printf("canvas_Q_VGA");
					webSocket.sendBIN(0, &end_flag, 1);
					camera = new OV7670(OV7670::Mode::QVGA_RGB565, SIOD, SIOC, VSYNC, HREF, XCLK, PCLK, D0, D1, D2, D3, D4, D5, D6, D7);
					delay(ChangeFormatDelay);	//Delay para estabilizar a próxima imagem
				break;
			}
			if(videoMode < 2) videoMode++;
			else videoMode = 0;


			blk_count = camera->yres/I2SCamera::blockSlice;//30, 60, 120
			for (int i=0; i<blk_count; i++) 
			{
				if (i == 0) {
					camera->startBlock = 1;
					camera->endBlock = I2SCamera::blockSlice;
					webSocket.sendBIN(0, &start_flag, 1);
				}

				if (i == blk_count-1) {
					webSocket.sendBIN(0, &end_flag, 1);
				}
				camera->oneFrame();
				webSocket.sendBIN(0, camera->frame, camera->xres * I2SCamera::blockSlice * 2);
				camera->startBlock += I2SCamera::blockSlice;
				camera->endBlock   += I2SCamera::blockSlice;
			}
		}
		
		if(!_frameBytes)// || anyImage)
		{
			MoveMotor = false;
			MotorInCourse = true;
			stepper1.setCurrentPosition(movement);
			stepper1.moveTo(0);
			//Serial.println("Mandou rodar");
		}
	}

	if ( (stepper1.distanceToGo() == 0) && !SendDataFrame)
	{
		MotorInCourse = false;
		SendDataFrame = true;
		//Serial.println("Parou o motor");
	}
	stepper1.run();
}
//------------------------------------------------
void initWifiAP()
{
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ap_ssid, ap_password);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
}
//------------------------------------------------

//------------------------------------------------
void setup()
{
 // Serial.begin(115200);
  Serial.begin(1000000);
  initWifiMulti();
  initWifiAP();
  startWebSocket();
  startWebServer();
  setupMotor();
}
//------------------------------------------------

//------------------------------------------------
void loop()
{
  if(!MotorInCourse)
  {
    webSocket.loop();
    serve();
  }
  runMotor();
  //delay(10);
}
//------------------------------------------------



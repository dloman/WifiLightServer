#include "WifiNetworkData.h"
#include <user_config.h>
#include <SmingCore/SmingCore.h>

#define LED_PIN 2 // GPIO number

Timer gTimer;
HttpServer gServer;
bool gStatus = false;
unsigned gUdpPort = 42069;
IPAddress gBroadcast(10, 18, 15, 255);


void onUdpReceive(
  UdpConnection& connection,
  char *data,
  int size,
  IPAddress remoteIP,
  uint16_t remotePort);

void sendStatus();

UdpConnection gUdpConnection(onUdpReceive);

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void onUdpReceive(
  UdpConnection& connection,
  char *data,
  int size,
  IPAddress remoteIP,
  uint16_t remotePort)
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Status(HttpRequest& request, HttpResponse& response)
{
  digitalWrite(LED_PIN, gStatus);
  Serial.print("pin 0 = ");
  Serial.println(gStatus);
	JsonObjectStream* pStream = new JsonObjectStream();
	JsonObject& json = pStream->getRoot();
	json["lights"] = (bool)gStatus;
	response.sendJsonObject(pStream);
	gUdpConnection.sendStringTo(gBroadcast, gUdpPort, static_cast<String>(gStatus));
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Help(HttpRequest& request, HttpResponse& response)
{
	response.setContentType(ContentType::TEXT);
	response.sendString("Available commands: \non \noff \ntoggle \nstatus");
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void On(HttpRequest& request, HttpResponse& response)
{
  gStatus = true;
  Status(request, response);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Off(HttpRequest& request, HttpResponse& response)
{
  gStatus = false;
  Status(request, response);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Toggle(HttpRequest& request, HttpResponse& response)
{
  gStatus = !gStatus;
  Status(request, response);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void sendStatus()
{
	gUdpConnection.sendStringTo(gBroadcast, gUdpPort, static_cast<String>(gStatus));
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void startWebServer()
{
	gServer.listen(80);
	gServer.addPath("/", Help);
	gServer.addPath("/help", Help);
	gServer.addPath("/on", On);
	gServer.addPath("/off", Off);
	gServer.addPath("/toggle", Toggle);
	gServer.addPath("/status", Status);
	gServer.setDefaultHandler(Help);

	Serial.println("\r\n=== WEB SERVER STARTED ===");
	Serial.println(WifiStation.getIP());
	Serial.println("==============================\r\n");
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void init()
{
	pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, gStatus);

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	WifiStation.enable(true);
  WifiStation.config(ssid, password);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP
	WifiStation.waitConnection(startWebServer);

	gTimer.initializeMs(10000, sendStatus).start();
}

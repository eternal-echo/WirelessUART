/*
 Wireless UART Bridge
 UART to TCP/IP Bridge
 */


#include <ESP8266WiFi.h>
// WiFi network name and password:
const char* ssid     = "Wireless_UART";
const char* password = "33445566";
// WiFi Client and Server
WiFiServer server(8000);
WiFiClient client;
void setup()
{
    Serial.begin(115200);
	Serial.setTimeout(10);
	// 开启热点
	WiFi.softAP(ssid, password);
	IPAddress myIP = WiFi.softAPIP();
	// Serial.print("AP IP address: ");
	// Serial.println(myIP);
	// 开启服务器
	server.begin();
	// Serial.println("Server started");
	// Serial.println("Connect to " + myIP.toString());
	// 接收客户端连接
	while (client.connected() == false) {
		client = server.available();
		delay(100);
  }
}

void loop(){
	char buf[100];
	int i = 0;
	// 当客户端连接时，完成串口和TCP/IP的桥接
	if (client.connected()) {
		// 当串口有数据时
		if (Serial.available()) {
			// 按字节依次读取串口数据
			delay(2);
			while (Serial.available()) {
				buf[i++] = Serial.read();
			}
			buf[i] = '\0';
			// 一次性发送串口数据到客户端
			client.write(buf);
		}
		if (client.available()) {
			// 按字节依次读取客户端数据
			delay(2);
			while (client.available()) {
				buf[i++] = client.read();
			}
			buf[i] = '\0';
			// 一次性发送客户端数据到串口
			Serial.write(buf);
		}
	}
	// 当客户端断开连接时，等待客户端重新连接
	else {
		client.stop();
		while (client.connected() == false) {
			// 接收客户端的连接
			client = server.available();
			delay(100);
		}
	}
}

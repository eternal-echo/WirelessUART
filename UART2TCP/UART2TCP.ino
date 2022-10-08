/*
 Wireless UART Bridge
 UART to TCP/IP Bridge
 */


#include <ESP8266WiFi.h>
// WiFi network name and password:
const char* ssid     = "OpenWrt-620";
const char* password = "33445566";
// WiFi Client and Server
WiFiClient client;
char servername[] = "192.168.1.142";
int port = 8000;

void setup()
{
    Serial.begin(115200);
	Serial.setTimeout(10);
    // 连接WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    // 连接到服务器 192.168.4.1:8000
    while (!client.connect(servername, port)) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("Server connected");
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
    // 当服务器断开连接时，重新连接服务器
	else {
        while (!client.connect(servername, port)) {
            delay(500);
            Serial.print(".");
        }
        Serial.println("");
        Serial.println("Server connected");
    }
}

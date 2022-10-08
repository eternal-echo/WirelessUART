# 无线串口

Created: October 7, 2022 4:34 PM

常见的无线串口淘宝上一般是40块，刚好手头有几个esp8266模块，十块钱一个。esp8266具有一个串口，也支持wifi相关的无线通信，因此我们可以编写简单的arduino程序实现无线串口的功能，就不用买专门的无线串口模组了。

## 硬件

需要一个ESP8266模块下载器和两个ESP8266模块，大约三十元以内。并且ESP8266重新刷回AT固件可以当作普通串口WiFi模块使用。

![Untitled](%E6%97%A0%E7%BA%BF%E4%B8%B2%E5%8F%A3%202b6fb60de8734d728a95120cf9bcf2ff/Untitled.jpeg)

![Untitled](%E6%97%A0%E7%BA%BF%E4%B8%B2%E5%8F%A3%202b6fb60de8734d728a95120cf9bcf2ff/Untitled%201.jpeg)

## 程序

```mermaid
flowchart LR
	Client--TCP---Server
	Client--WiFi-->Server
	Board--uart---Client
	Server--uart---PC
```

服务器ESP8266模块通过烧写座子的串口连接到电脑端，客户端ESP8266模块通过串口ttl线与单片机进行连接，而两个ESP8266模块已经通过TCP实现串口和网口的桥接。

### 服务器

服务器的程序主要如下，`setup()` 中完成WiFi热点初始化，TCP服务器初始化、接收客户端连接、串口初始化等操作。串口需要设置等待时间为10ms，默认是1000ms，这里减小了等待时间。

```cpp
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
```

服务器的循环入下所示。首先监测当前是否有客户端连接，如果没有就继续等待客户端的连接。连接成功后就监测串口和网口的缓冲区是否有数据，当有数据时，一个字节一个字节地读取串口缓冲区地所有数据，再一次性写入客户端中，尽可能地保证消息连续。这里在检测到有数据后延时了2ms，是为了避免系统反应太快，刚接收到一个字节就发出去了，如果是汉字这种多字节的很容易造成乱码，延时2ms基本就能保证该一起发过去的数据都已经在缓冲区里了，最后一次性读出来，转发到客户端。

```cpp
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
```

### 客户端

客户端与服务器代码类似。测试无误可以把调试信息注释掉。

```cpp
/*
 Wireless UART Bridge
 UART to TCP/IP Bridge
 */

#include <ESP8266WiFi.h>
// WiFi network name and password:
const char* ssid     = "Wireless_UART";
const char* password = "33445566";
// WiFi Client and Server
WiFiClient client;
char servername[] = "192.168.4.1";
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
```

## 测试

![Untitled](%E6%97%A0%E7%BA%BF%E4%B8%B2%E5%8F%A3%202b6fb60de8734d728a95120cf9bcf2ff/Untitled.png)

最后两边能够成功透传，小模块制作成功。
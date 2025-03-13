#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include "DFRobot_HumanDetection.h"

// WiFi配置
const char* ssid = "OnePlus 10 Pro";
const char* password = "55132768";

// 创建人体检测传感器对象
DFRobot_HumanDetection hu(&Serial1);

// 邮箱配置
String smtpEmail = "a19426125871@163.com";
const char* smtpCode = "UTuCrjtKsEjAVpct";
const char* smtpCodeType = "163";

// 连接WiFi网络
void connectWiFi()
{
	Serial.println("Connecting to WiFi...");
	WiFi.begin(ssid,password);
	while(WiFi.status()!=WL_CONNECTED)
	{
		//通过循环不断检测是否连接成功
		delay(500);
		Serial.print(".");
	}
	Serial.println("Connected to WiFi");
}

//发送邮件的函数，收件人，标题，主题，内容
void sendEmailMessage(String recipient, String fromTitle, String subject, String messageContent)
{
	if(WiFi.status() != WL_CONNECTED)
	{
		//首先检查WiFi是否已经连接、如果没
		Serial.println("WiFi not connected");
		return;
	}
	HTTPClient http;
	//构建POST请求的数据，拼接各项参数,创建一个HTTPClient对象，用于后续发送
	String postPayload = "ColaKey=2JEDHbiLFdWGGM1741874318631OHygaFSkmw&tomail=" + recipient + "&fromTitle=" + fromTitle + "&subject=" + subject + "&content=" + messageContent + "&smtpCode=" + smtpCode + "&smtpEmail=" + smtpEmail + "&smtpCodeType=" + smtpCodeType + "&isTextContent=true";
	//开始连接指定的邮件发送服务URL??
	http.begin("https://luckycola.com.cn/tools/customMail");
	//设置请求头，指定内容类型为表单数据格式,为了让邮件发送服务能正确解析接收
	http.addHeader("Content-Type","application/x-www-form-urlencoded");
	//发送POST请求
	int responseCode = http.POST(postPayload);
	http.end();
}

void setup()
{
	Serial.begin(115200);
	Serial1.begin(115200, SERIAL_8N1, 4, 5);
	
	// 初始化人体检测传感器
	Serial.println("Start initialization");
	while (hu.begin() != 0) {
		Serial.println("Sensor init error!");
		delay(1000);
	}
	Serial.println("Sensor initialization successful");

	// 配置工作模式
	while (hu.configWorkMode(hu.eSleepMode) != 0) {
		Serial.println("Work mode config error!");
		delay(1000);
	}
	
	// 配置LED指示灯
	hu.configLEDLight(hu.eHPLed, 1);
	hu.sensorRet();

	connectWiFi();
}

void loop()
{
	String presence, movement;
	
	// 获取人体存在状态
	switch (hu.smHumanData(hu.eHumanPresence)) {
		case 0:
			presence = "No one present";
			break;
		case 1:
			presence = "Someone present";
			break;
		default:
			presence = "Read error";
	}

	// 获取运动状态
	switch (hu.smHumanData(hu.eHumanMovement)) {
		case 0:
			movement = "None";
			break;
		case 1:
			movement = "Still";
			break;
		case 2:
			movement = "Active";
			break;
		default:
			movement = "Read error";
	}

	// 获取呼吸和心率数据
	int breathRate = hu.getBreatheValue();
	int heartRate = hu.getHeartRate();
	
	// 构建邮件内容
	String mailContent = "Status: " + presence + 
						"\nMovement: " + movement +
						"\nBreathing Rate: " + String(breathRate) +
						"\nHeart Rate: " + String(heartRate);
	
	// 发送邮件
	sendEmailMessage("1942612587@qq.com", "esp32 data", "Human Detection Data", mailContent);
	
	delay(120000);  // 2分钟发送间隔
}

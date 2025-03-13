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
		Serial.println("WiFi not connected");
		return;
	}
	
	Serial.println("正在发送邮件...");
	HTTPClient http;
	http.setTimeout(15000); // 增加到15秒超时
	
	// 设置更多参数以提高稳定性
	http.setReuse(false); // 不复用连接
	
	String postPayload = "ColaKey=2JEDHbiLFdWGGM1741874318631OHygaFSkmw&tomail=" + recipient + "&fromTitle=" + fromTitle + "&subject=" + subject + "&content=" + messageContent + "&smtpCode=" + smtpCode + "&smtpEmail=" + smtpEmail + "&smtpCodeType=" + smtpCodeType + "&isTextContent=true";
	
	bool requestSent = false;
	
	// 尝试两次连接
	for(int i=0; i<2 && !requestSent; i++) {
		http.begin("https://luckycola.com.cn/tools/customMail");
		http.addHeader("Content-Type","application/x-www-form-urlencoded");
		
		// 发送POST请求并检查响应
		int responseCode = http.POST(postPayload);
		Serial.print("HTTP响应代码: ");
		Serial.println(responseCode);
		
		if(responseCode > 0) {
			String payload = http.getString();
			Serial.println("服务器响应: " + payload);
			requestSent = true;
			Serial.println("邮件发送成功");
		} else {
			// 区分不同的错误情况
			if(responseCode == -11) {
				Serial.println("连接错误，但邮件可能已发送。");
				// 通常这种情况邮件还是能送达的
				requestSent = true; 
			} else {
				Serial.println("HTTP错误: " + String(responseCode));
				delay(1000); // 等待1秒后重试
			}
		}
		http.end();
	}
	
	Serial.println("邮件处理完成");
}

// 使用非阻塞方式控制发送间隔
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 120000; // 2分钟

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
	unsigned long currentTime = millis();
	
	// 检查是否到了发送间隔
	if(currentTime - lastSendTime >= sendInterval) {
		Serial.println("开始读取传感器数据...");
		
		// 设置读取超时
		unsigned long startTime = millis();
		bool readSuccess = false;
		String presence = "Unknown", movement = "Unknown";
		int breathRate = 0, heartRate = 0;
		
		// 尝试读取传感器数据，最多等待3秒
		while(millis() - startTime < 3000 && !readSuccess) {
			// 获取人体存在状态
			int presenceValue = hu.smHumanData(hu.eHumanPresence);
			if(presenceValue >= 0) {
				readSuccess = true;
				
				switch (presenceValue) {
					case 0: presence = "无人在场"; break;
					case 1: presence = "有人在场"; break;
					default: presence = "读取错误";
				}
				
				// 获取运动状态
				switch (hu.smHumanData(hu.eHumanMovement)) {
					case 0: movement = "无动作"; break;
					case 1: movement = "静止"; break;
					case 2: movement = "活动"; break;
					default: movement = "读取错误";
				}
				
				// 获取呼吸和心率数据
				breathRate = hu.getBreatheValue();
				heartRate = hu.getHeartRate();
			}
			delay(100); // 短暂延迟后重试
		}
		
		if(readSuccess) {
			// 构建邮件内容
			String mailContent = "状态: " + presence + 
							"\n动作: " + movement +
							"\n呼吸频率: " + String(breathRate) +
							"\n心率: " + String(heartRate);
			
			// 发送邮件
			sendEmailMessage("1942612587@qq.com", "esp32 data", "人体检测数据", mailContent);
			
			// 更新发送时间
			lastSendTime = currentTime;
		} else {
			Serial.println("传感器数据读取超时");
			// 如果读取失败，延迟后重试
			delay(5000);
		}
	}
	
	// 短暂延迟避免CPU占用过高
	delay(1000);
	
	// 检查WiFi连接状态
	if(WiFi.status() != WL_CONNECTED) {
		Serial.println("WiFi断开，正在重连...");
		connectWiFi();
	}
}

#include <ESP8266HTTPClient.h>
#include <string>
#include <Arduino.h>
#include <FS.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <U8g2lib.h>
#include "image.cpp"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "ActionControl.h"
#define BUTTON_PIN 2 // GPIO2 引脚 (D4)
#define BUTTON_PIN2 15

int batteryPercentage = 0;//电池电量
static bool initweather = false; // 天气初始化
AsyncWebServer server(80);
volatile bool buttonPressed = false;     // 按键标志
volatile bool buttonPressed2 = false;    // 按键标志
unsigned long lastPressTime = 0;         // 上次按键时间
const unsigned long debounceDelay = 50;  // 消抖时间 (ms)
unsigned long lastPressTime2 = 0;        // 上次按键时间
const unsigned long debounceDelay2 = 50; // 消抖时间 (ms)
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R2, /* reset=*/U8X8_PIN_NONE, /* clock=*/5, /* data=*/4); 

const char* ssid = "ZhuNiu_Birthday";
const char* password = "20050826";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ntp.aliyun.com", 8 * 3600, 60000);
// 参数: UDP对象, NTP服务器, 时区偏移(秒), 更新间隔(毫秒)
const char *weatherAPI = "http://api.seniverse.com/v3/weather/daily.json?key=";
String temperature = "";
String humidity = "";
String weather = "";
String useruid = "";
String cityname = "";
String weatherapi = "";
const char *ssidFile = "/ssid.json";
int emojiState = 0; // 表情状态
int prevEmojiState = -1;//跟踪前一个表情,判断表情是否变化 
const int EmojiNum =9;
void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}
void fetchWeather() {
    if (initweather == false && WiFi.status() == WL_CONNECTED) {
        WiFiClient client;
        HTTPClient http;
        String url = String(weatherAPI) + weatherapi + "&location=" + cityname + "&language=zh-Hans&unit=c&start=0&days=1";
        
        if (http.begin(client, url)) {
            int httpCode = http.GET();
            if (httpCode > 0) {
                String payload = http.getString();
                DynamicJsonDocument doc(1024);
                deserializeJson(doc, payload);
                temperature = doc["results"][0]["daily"][0]["high"].as<String>();
                humidity = doc["results"][0]["daily"][0]["humidity"].as<String>();
                weather = doc["results"][0]["daily"][0]["text_day"].as<String>();
                initweather = true;
                
                Serial.println("Weather data received:");
                Serial.println("  Temp: " + temperature);
                Serial.println("  Humidity: " + humidity);
                Serial.println("  Weather: " + weather);
            }
            http.end();
        }
    }
}
// 表情绘制函数
void drawHi() { u8g2.drawXBMP(0, 0, 128, 64, hi); }
void drawAngry() { u8g2.drawXBMP(0, 0, 128, 64, angry); }
void drawError() { u8g2.drawXBMP(0, 0, 128, 64, error); }
void drawDowhat() { u8g2.drawXBMP(0, 0, 128, 64, dowhat); }
void drawLove() { u8g2.drawXBMP(0, 0, 128, 64, love); }
void drawSick() { u8g2.drawXBMP(0, 0, 128, 64, sick); }
void drawYun() { u8g2.drawXBMP(0, 0, 128, 64, yun); }
void drawWeather() {if (weather == "阴" || weather == "多云") {
        u8g2.drawXBMP(0, 0, 64, 64, cloud);
    } else if (weather == "小雨" || weather == "大雨" || weather == "暴雨" || weather == "雨"||weather=="中雨") {
        u8g2.drawXBMP(0, 0, 64, 64, rain);
    } else {
        u8g2.drawXBMP(0, 0, 64, 64, sun);
    }
    
    u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.drawStr(64, 10, cityname.c_str());
    u8g2.drawStr(64, 20, "Temp");
    u8g2.drawStr(64, 30, (temperature + " C").c_str());
    u8g2.drawStr(64, 50, "Humidity");
    u8g2.drawStr(64, 60, (humidity + " %").c_str());
}
void drawTime() { 
    if(WiFi.status()==WL_CONNECTED)
    {timeClient.update();
    
    int hours = timeClient.getHours();
    int minutes = timeClient.getMinutes();
    int seconds = timeClient.getSeconds();
    
    u8g2.setFont(u8g2_font_ncenB14_tr);
    
    // 标题 - 居中
    u8g2.setCursor(5, 15);
    u8g2.print("CurrentTime");
    
    // 时间 - 居中
    char timeStr[9];
    sprintf(timeStr, "%02d:%02d:%02d", hours, minutes, seconds);
    int textWidth = u8g2.getStrWidth(timeStr);
    int x = (128 - textWidth) / 2;
    u8g2.setCursor(x, 50);
    u8g2.print(timeStr);
    }
}

// 函数指针数组
void (*drawEmoji[])() = { drawHi, drawAngry, drawError, drawDowhat, drawLove, drawSick, drawYun, drawWeather,drawTime };
//监听网页函数
void OpenHtml(const char* path, const char* filename) {
    if (!SPIFFS.exists(filename)) {
        server.on(path, HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(404, "text/plain", "404");
        });
        return;
    }
    
    server.on(path, HTTP_GET, [filename](AsyncWebServerRequest *request) {
        request->send(SPIFFS, filename, "text/html");
    });
}
//表情管理
void expression() {
    const char* paths[] = {"/histate","/angrystate","/errorstate","/dowhatstate",
                           "/lovestate","/sickstate","/yunstate","/weather","/time"};
  
    for (int i = 0; i < EmojiNum; i++) {
        server.on(paths[i], HTTP_GET, [i](AsyncWebServerRequest *request) {
            emojiState = i;
            request->send(200, "text/plain", "OK");
        });
    }
}
//按键中断函数
void ICACHE_RAM_ATTR handleButtonPress()
{
    unsigned long currentTime = millis();
    if (currentTime - lastPressTime > debounceDelay)
    {
        buttonPressed = true;
        lastPressTime = currentTime;
    }
   
}
void ICACHE_RAM_ATTR handleButtonPress2()
{
   
    unsigned long currentTime2 = millis();
    if (currentTime2 - lastPressTime2 > debounceDelay2)
    {
        buttonPressed2 = true;
        lastPressTime2 = currentTime2;
    }
}
//显示项目信息
void showProjectInfo() {
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_ncenB08_tr);
        
        // 第一行：项目名称
        u8g2.setCursor(10, 12);
        u8g2.print("EDA Robot Dog v1.0");
        
        // 第二行：作者
        u8g2.setCursor(10, 26);
        u8g2.print("MadeBy:WenXuWang");
        
        // 第三行：日期
        u8g2.setCursor(10, 40);
        u8g2.print("Date:2026.04.17");
        
        // 第四行：WiFi 状态
        u8g2.setCursor(10, 54);
        if (WiFi.status() == WL_CONNECTED) {
            u8g2.print("WiFi Connected");
            u8g2.setCursor(10, 64);
            u8g2.print("IP: ");
            u8g2.print(WiFi.localIP().toString());
        } else {
            u8g2.print("WiFi Not Connected");
            u8g2.setCursor(10, 64);
            u8g2.print("AP IP: ");
            u8g2.print(WiFi.softAPIP().toString());
        }
    } while (u8g2.nextPage());
}
//计算剩余电量
int getBattery() {
    // 采样10次取平均值
    long total = 0;
    for (int i = 0; i < 10; i++) {
        total += analogRead(A0);
        delay(5);
    }
    float avgAdc = total / 10.0;
    
    // 转换为电压（ESP8266 ADC参考电压1.0V）
    float voltage = (avgAdc / 1023.0) * 1.0 * 8.4;  // 8.4是分压比
    
    // 转换为百分比（6.4V = 0%, 8.4V = 100%）
    if (voltage >= 8.4) return 100;
    if (voltage <= 6.4) return 0;
    return (int)((voltage - 6.4) / (8.4 - 6.4) * 100);
}
void handleWiFiConfig() {
    server.on("/connect", HTTP_POST, [](AsyncWebServerRequest *request) {
        // 1. 检查所有参数是否存在
        if (!request->hasParam("ssid", true) || 
            !request->hasParam("pass", true) ||
            !request->hasParam("uid", true) ||
            !request->hasParam("city", true) ||
            !request->hasParam("api", true)) {
            request->send(400, "text/plain", "Missing parameters");
            return;
        }
        
        // 2. 获取参数值
        String ssid = request->getParam("ssid", true)->value();
        String pass = request->getParam("pass", true)->value();
        String uid = request->getParam("uid", true)->value();
        String city = request->getParam("city", true)->value();
        String api = request->getParam("api", true)->value();
        
        // 3. 验证参数不能为空
        if (ssid.length() == 0 || pass.length() == 0) {
            request->send(400, "text/plain", "SSID and Password required");
            return;
        }
        
        // 4. 打印调试信息
        Serial.println("=== WiFi Config Received ===");
        Serial.printf("SSID: %s\n", ssid.c_str());
        Serial.printf("Password: %s\n", pass.c_str());
        Serial.printf("UID: %s\n", uid.c_str());
        Serial.printf("City: %s\n", city.c_str());
        Serial.printf("API: %s\n", api.c_str());
    
        
        // 6. 保存配置到JSON文件
        DynamicJsonDocument doc(2048);  // 增大到2KB
        doc["ssid"] = ssid;
        doc["pass"] = pass;
        doc["uid"] = uid;
        doc["city"] = city;
        doc["api"] = api;
        
        fs::File file = SPIFFS.open("/ssid.json", "w");
        if (file) {
            if (serializeJson(doc, file) == 0) {
                Serial.println("JSON serialization failed!");
                request->send(500, "text/plain", "Save failed");
                file.close();
                return;
            }
            file.close();
            Serial.println("Config saved to SPIFFS");
        } else {
            Serial.println("Failed to open file for writing");
            request->send(500, "text/plain", "Cannot save config");
            return;
        }
        
        // 7. 更新全局变量（需要先声明）
        extern String useruid, cityname, weatherapi;
        useruid = uid;
        cityname = city;
        weatherapi = api;
        WiFi.begin(ssid,pass);
        // 8. 发送响应（在连接WiFi之前，避免超时）
        request->send(200, "text/html", "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Connecting</title><meta http-equiv='refresh' content='10;url=/'></head><body><h1>正在连接WiFi...</h1><p>SSID: " + ssid + "</p><p>10秒后将自动跳转</p></body></html>");
        
        
       
    });
}
void loadWiFiConfig()
{
    if (SPIFFS.exists(ssidFile))
    {
        fs::File file = SPIFFS.open(ssidFile, "r");
        if (file)
        {
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, file);
            if (!error)
            {
                String ssid = doc["ssid"];
                String pass = doc["pass"];
                String uid = doc["uid"];
                String city = doc["city"];
                String api = doc["api"];
                useruid = uid;
                cityname = city;
                weatherapi = api;
                WiFi.begin(ssid.c_str(), pass.c_str());
               
                // 尝试连接WiFi，最多等待10秒
                unsigned long startAttemptTime = millis();
                while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 5000)
                {
                    delay(500);
                }
                // 如果连接失败，打印状态
                if (WiFi.status() != WL_CONNECTED)
                {
                    Serial.println("WiFi connection failed, starting captive portal...");
                    handleWiFiConfig(); // 启动强制门户
                }
                else
                {
                    Serial.println("WiFi connected");
                    timeClient.begin();
                }
            }
            file.close();
        }
    }
}


//启动配置函数///////////////////////////////////////////////////////////////
void setup() {
    servo_Setup();
    SPIFFS.begin();
    Serial.begin(115200);
    u8g2.begin(); 
    timeClient.begin();
       u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_ncenB12_tr);
            u8g2.setCursor(10, 35);
            u8g2.print("I am starting...");
        } while (u8g2.nextPage());
        
   int attempts;
     // 注册配网路由
        handleWiFiConfig();
        loadWiFiConfig();  // 这个函数内部会尝试连接保存的WiFi
        
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\nDefault WiFi failed, loading saved config...");
        // 再等待5秒
        attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 10) {
            delay(500);
            attempts++;
        }
    }
   
    // 如果还是失败，启动AP配网模式
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\nStarting AP config mode...");
        WiFi.mode(WIFI_AP);
        WiFi.softAP("ZHNNIU-Robot","");
    } else {
        Serial.println("\nWiFi Connected!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    }
    
    expression();
    Action(server);
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
//主界面
     OpenHtml("/","/index.html");
//遥控器界面
     OpenHtml("/control.html","/control.html");
//配置界面
     OpenHtml("/setting.html","/setting.html");
//电量显示   
     server.on("/batteryPercentage", HTTP_GET, [](AsyncWebServerRequest *request)
    { request->send(200, "text/plain", String(batteryPercentage)); });
    server.onNotFound(notFound);
//信息显示
server.on("/info", HTTP_GET, [](AsyncWebServerRequest *request) {
      showProjectInfo(); // 调用你的信息显示函数
    request->send(200, "text/plain", "OK");
});
    server.begin();


//按键配置
     pinMode(BUTTON_PIN, INPUT_PULLUP); // GPIO2 设置为输入并启用内部上拉电阻
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonPress, FALLING);
    pinMode(BUTTON_PIN2, INPUT); // GPIO15 设置为输入（无需内部上拉）
    // 监听上升沿触发中断
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN2), handleButtonPress2, RISING); // 设置下降沿中断
}


////////////////////////////////////////////////////////////////////////////
//主循环函数
void loop() 
{   
    
    //更新电量
    batteryPercentage=getBattery();
    //表情处理
   if (emojiState != prevEmojiState||emojiState==8) {
    if(emojiState==7){fetchWeather();}
    
        u8g2.firstPage();
        do { if (emojiState >= 0 && emojiState < EmojiNum) drawEmoji[emojiState](); } 
        while (u8g2.nextPage());
        prevEmojiState = emojiState;
    
    }
      //动作处理
   if (ActionState>=0&& ActionState < ACTION_NUM) {
    
       doAction[ActionState]();
       ActionState=-1;
    
    }
    //按键中断处理
    if(buttonPressed)
    {
        buttonPressed=false;
        showProjectInfo();//显示信息
    }
    //复位按键
     if (buttonPressed2) {
        buttonPressed2 = false;
        
        // 显示重启提示
        u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_ncenB12_tr);
            u8g2.setCursor(10, 35);
            u8g2.print("I am Restarting...");
        } while (u8g2.nextPage());
        
        delay(1000);  // 等待显示完成
        ESP.restart();  // 软复位
    }
}
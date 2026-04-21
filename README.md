# ESP8266 智能四足机器狗

## 项目简介
基于 ESP8266 设计的智能四足机器人，支持 Wi-Fi 远程控制、4 路舵机驱动及 OLED 信息显示。  
项目包含硬件电路设计、PCB 布局布线、焊接装配及嵌入式软件全流程开发。

## 功能特性
- 📱 手机/PC 端 WiFi 远程控制（HTTP + WebSocket）
- 🦾 4 路数字舵机独立平滑控制（PWM 驱动）
- 🌤️ OLED 实时显示天气、时间及交互表情
- 🔋 7.4V 电源管理（转 5V / 3.3V）
- 🌐 通过 HTTP 获取天气 JSON 数据，NTP 网络校时

## 硬件架构
| 组件 | 型号 | 数量 |
|------|------|------|
| 主控 | ESP8266 (NodeMCU) | 1 |
| 舵机 | MG996R | 4 |
| 屏幕 | 0.96 寸 OLED (SSD1306, I2C) | 1 |
| 电源 | 7.4V 锂电池 → 5V/3.3V 稳压模块 | 1 |
| PCB | 双层板，手工焊接 | 1 |

## 舵机分配
| 位置 | 舵机编号 |
|------|----------|
| 左前腿 | 1 |
| 右前腿 | 2 |
| 左后腿 | 3 |
| 右后腿 | 4 |

## 软件开发
### 开发环境
- VS Code（Visual Studio Code）
- PlatformIO IDE 插件
- ESP8266 开发板包

### 关键技术点
- 基于定时器生成高精度 PWM 波（50Hz，占空比 2.5%~12.5%）
- OLED（SSD1306）I2C 驱动，支持字符显示及动态表格绘制
- HTTP 请求解析 JSON 天气数据
- NTP 协议实现网络时间同步
- Web 端实时控制页面（HTML + JavaScript）

### 项目结构
```
├── src/
│ ├── main.cpp # 主程序入口
│ ├── ActionControl.cpp # 舵机动作控制
│ ├── ActionControl.h
│ └── image.cpp # OLED 图像数据
├── data/
│ ├── index.html # 控制页面
│ ├── control.html # 舵机手动控制
│ └── setting.html # WiFi/API 配置
├── platformio.ini
└── .gitignore
```

## 快速开始
1. 安装 VS Code 和 PlatformIO 插件
2. 用 VS Code 打开项目文件夹
3. PlatformIO 会自动安装 ESP8266 依赖
4. 修改 `src/main.cpp` 中的 WiFi SSID 和密码
5. 点击 PlatformIO 工具栏的 **Upload** 按钮编译并上传
6. 串口查看 IP 地址，浏览器访问该 IP 即可控制

## 作者
王文旭  
- 学校：西安电子科技大学 通信工程  
- GitHub：[github.com/wwx798](https://github.com/wwx798)  
- 邮箱：3221816668@qq.com

## 许可证
本项目仅供学习交流使用
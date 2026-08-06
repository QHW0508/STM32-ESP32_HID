
 * ESP32-S3 连接 JDY-16 接收 + USB OTG HID 键盘语音反馈

#include "USB.h"
#include "USBHIDKeyboard.h"
#include <BLEDevice.h>
#include <BLEClient.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>


#define ENABLE_VOICE        true


#define I2S_BCLK_PIN        4
#define I2S_WS_PIN          5
#define I2S_DOUT_PIN        6
#define VOICE_VOLUME        1.0f   // 0.0 ~ 2.0，1.0 为原始音量

#if ENABLE_VOICE
#include "driver/i2s_std.h"
#include "voice_clips.h"
#endif

// ==================== 配置 ====================

#define JDY16_BLE_NAME    "JDY-16"


#define JDY16_BLE_ADDR    ""

// JDY-16 透传服务 UUID
#define SERVICE_UUID        "0000FFE0-0000-1000-8000-00805F9B34FB"
#define CHAR_NOTIFY_UUID    "0000FFE2-0000-1000-8000-00805F9B34FB"  // STM32 → ESP32 
#define CHAR_WRITE_UUID     "0000FFE1-0000-1000-8000-00805F9B34FB"  // ESP32 → STM32 


#define ENABLE_ACK_TO_STM32 false


#define ENABLE_HEX_TOKEN_PARSE true


#define SCAN_TIME           5


#define RECONNECT_DELAY     1000


#define RX_BUFFER_SIZE      512


#define LED_PIN           48
#define LED_ACTIVE_LOW    true


#define SERIAL_BAUD       115200

// ==================== 全局变量 ====================

USBHIDKeyboard Keyboard;

BLEClient* pClient = nullptr;
BLERemoteCharacteristic* pNotifyChar = nullptr;
BLERemoteCharacteristic* pWriteChar = nullptr;
BLEAdvertisedDevice* targetDevice = nullptr;

bool bleConnected = false;     // 连接 JDY-16
bool doConnect = false;        // 扫描设备
bool usbConnected = false;

volatile bool newDataReceived = false;
char rxBuffer[RX_BUFFER_SIZE];
volatile size_t rxLength = 0;

static char pendingBuf[4];
static size_t pendingLen = 0;

// ==================== 语音播报状态 ====================
#if ENABLE_VOICE
i2s_chan_handle_t i2sTx = nullptr;

#define VOICE_QUEUE_SIZE  8
uint8_t voiceQueue[VOICE_QUEUE_SIZE];
volatile int vqHead = 0;   // 写入位置
volatile int vqTail = 0;   // 读取位置

// 当前播放
const uint8_t* playData = nullptr;
uint32_t playLen = 0;
uint32_t playPos = 0;
bool voicePlaying = false;
#endif



void ledOn() {
    if (LED_PIN < 0) return;
    digitalWrite(LED_PIN, LED_ACTIVE_LOW ? LOW : HIGH);
}

void ledOff() {
    if (LED_PIN < 0) return;
    digitalWrite(LED_PIN, LED_ACTIVE_LOW ? HIGH : LOW);
}

void ledBlink(int times, int delayMs) {
    for (int i = 0; i < times; i++) {
        ledOn();
        delay(delayMs);
        ledOff();
        delay(delayMs);
    }
}

// 打印状态信息

void printEscaped(const char* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        char c = data[i];
        if (c >= 32 && c <= 126) {
            Serial.print(c);
        } else if (c == '\n') {
            Serial.print("\\n");
        } else if (c == '\r') {
            Serial.print("\\r");
        } else if (c == '\t') {
            Serial.print("\\t");
        } else {
            Serial.printf("\\x%02X", (uint8_t)c);
        }
    }
}

//蓝牙通知回调 JDY-16 → ESP32

static void notifyCallback(BLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify) {
    if (length == 0) return;

    noInterrupts();
    size_t copyLen = length;
    if (copyLen >= RX_BUFFER_SIZE) copyLen = RX_BUFFER_SIZE - 1;
    memcpy(rxBuffer, pData, copyLen);
    rxBuffer[copyLen] = '\0';
    rxLength = copyLen;
    newDataReceived = true;
    interrupts();

    Serial.print("[BLE] 收到 JDY-16 数据 (");
    Serial.print(length);
    Serial.print(" 字节): ");
    printEscaped((const char*)pData, length);
    Serial.println();
}

// BLE 客户端回调

class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient* pClient) {
        bleConnected = true;
        Serial.println("[BLE] 已连接 JDY-16 ✓");
    }

    void onDisconnect(BLEClient* pClient) {
        bleConnected = false;
        pNotifyChar = nullptr;
        pWriteChar = nullptr;
        Serial.println("[BLE] JDY-16 已断开 ✗，将重新扫描...");
    }
};

// 扫描回调

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        // 按名称匹配
        bool nameMatch = advertisedDevice.haveName() &&
                         advertisedDevice.getName() == JDY16_BLE_NAME;

        // 按地址匹配
        bool addrMatch = false;
        if (strlen(JDY16_BLE_ADDR) > 0) {
            addrMatch = advertisedDevice.getAddress().toString() == JDY16_BLE_ADDR;
        }

        if (nameMatch || addrMatch) {
            Serial.print("[扫描] 发现目标: ");
            Serial.print(advertisedDevice.haveName() ? advertisedDevice.getName().c_str() : "(无名)");
            Serial.print("  地址: ");
            Serial.print(advertisedDevice.getAddress().toString().c_str());
            Serial.print("  RSSI: ");
            Serial.println(advertisedDevice.getRSSI());

            BLEDevice::getScan()->stop();

            if (targetDevice != nullptr) {
                delete targetDevice;
            }
            targetDevice = new BLEAdvertisedDevice(advertisedDevice);
            doConnect = true;
        }
    }
};

// 连接 JDY-16 

bool connectToJDY16() {
    Serial.print("[BLE] 正在连接 ");
    Serial.print(targetDevice->getAddress().toString().c_str());
    Serial.println(" ...");

    
    if (pClient != nullptr) {
        delete pClient;
        pClient = nullptr;
    }

    pClient = BLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallback());

    if (!pClient->connect(targetDevice)) {
        Serial.println("[BLE] 连接失败 ✗");
        return false;
    }

    Serial.println("[BLE] 连接成功，枚举所有服务与特征:");

    
    int subscribedCount = 0;
    pWriteChar = nullptr;

    std::map<std::string, BLERemoteService*>* services = pClient->getServices();
    for (auto& svc : *services) {
        Serial.print("  Service: ");
        Serial.println(svc.first.c_str());

        std::map<std::string, BLERemoteCharacteristic*>* chars = svc.second->getCharacteristics();
        for (auto& ch : *chars) {
            BLERemoteCharacteristic* rc = ch.second;

            // 打印特征
            Serial.print("    Char: ");
            Serial.print(ch.first.c_str());
            Serial.print("  [");
            if (rc->canRead())           Serial.print("READ ");
            if (rc->canWrite())          Serial.print("WRITE ");
            if (rc->canWriteNoResponse()) Serial.print("WRITE_NR ");
            if (rc->canNotify())         Serial.print("NOTIFY ");
            if (rc->canIndicate())       Serial.print("INDICATE ");
            Serial.println("]");

            // 订阅所有可通知
            if (rc->canNotify()) {
                rc->registerForNotify(notifyCallback, true);
                subscribedCount++;
                Serial.print("      -> 已订阅 (Notify) ✓\n");
            } else if (rc->canIndicate()) {
                rc->registerForNotify(notifyCallback, false);
                subscribedCount++;
                Serial.print("      -> 已订阅 (Indicate) ✓\n");
            }

            // 记录 FFE1 写特征
            if (pWriteChar == nullptr &&
                ch.first == "0000ffe1-0000-1000-8000-00805f9b34fb" &&
                rc->canWrite()) {
                pWriteChar = rc;
            }
        }
    }

    Serial.printf("[BLE] 共订阅 %d 个可通知特征\n", subscribedCount);

    if (subscribedCount == 0) {
        Serial.println("[BLE] 没有任何可订阅的特征 ✗");
        pClient->disconnect();
        return false;
    }

    if (pWriteChar != nullptr) {
        Serial.println("[BLE] 写特征 FFE1 可用（可回传数据给 STM32）");
    }

    return true;
}

// 键盘发送函数 

// 十六进制字符判断与取值
static bool isHexChar(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}
static uint8_t hexVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return c - 'A' + 10;
}

// 语音播报函数 
#if ENABLE_VOICE


void queueVoice(uint8_t c) {
    VoiceClip clip;
    if (!getVoiceClip(c, clip)) return;
    int next = (vqHead + 1) % VOICE_QUEUE_SIZE;
    if (next == vqTail) return;   // 队列满，丢弃（避免越积越多）
    voiceQueue[vqHead] = c;
    vqHead = next;
}

// 打印当前播报内容
static void printVoiceName(uint8_t c) {
    Serial.print("[语音] 播报: ");
    if (c >= 'a' && c <= 'z') Serial.println((char)(c - 'a' + 'A'));
    else if (c >= 'A' && c <= 'Z') Serial.println((char)c);
    else if (c == ' ') Serial.println("space");
    else if (c == 0x0A) Serial.println("enter");
    else if (c == 0x08) Serial.println("backspace");
    else if (c == 0x09) Serial.println("tab");
    else if (c == 0x1B) Serial.println("escape");
    else if (c == '.') Serial.println("dot");
    else Serial.printf("0x%02X\n", c);
}


void pumpVoice() {
    // ---------- 取下一段 ----------
    if (!voicePlaying) {
        if (vqTail != vqHead) {
            uint8_t c = voiceQueue[vqTail];
            vqTail = (vqTail + 1) % VOICE_QUEUE_SIZE;
            VoiceClip clip;
            if (getVoiceClip(c, clip)) {
                playData = clip.data;
                playLen = clip.len;
                playPos = 0;
                voicePlaying = true;
                printVoiceName(c);
            }
        }
        return;
    }

    // ---------- 播放完毕 ----------
    if (playPos >= playLen) {
        voicePlaying = false;
        playData = nullptr;
        return;
    }

    
    int16_t stereoBuf[1024];
    const uint32_t CHUNK = 512;

    while (playPos < playLen) {
        uint32_t remaining = playLen - playPos;
        uint32_t monoSamples = remaining / 2;
        if (monoSamples > CHUNK) monoSamples = CHUNK;

        // 用 memcpy 取单声道数据
        int16_t monoBuf[CHUNK];
        memcpy(monoBuf, playData + playPos, monoSamples * 2);

        for (uint32_t i = 0; i < monoSamples; i++) {
            int32_t s = (int32_t)(monoBuf[i] * VOICE_VOLUME);
            if (s > 32767) s = 32767;
            if (s < -32768) s = -32768;
            stereoBuf[i * 2]     = (int16_t)s;
            stereoBuf[i * 2 + 1] = (int16_t)s;
        }

        size_t written = 0;
        // 缓冲满(written==0)就先返回，下一轮 pump 再写
        i2s_channel_write(i2sTx, stereoBuf, monoSamples * 4, &written, 0);
       
        uint32_t framesWritten = written / 4;
        playPos += framesWritten * 2;

        if (framesWritten < monoSamples) {
            
            break;
        }
        
    }

    if (playPos >= playLen) {
        voicePlaying = false;
        playData = nullptr;
    }
}

#endif // ENABLE_VOICE


static void typeOneByte(uint8_t c) {
    if (c == 0x0A) {
        // 0x0A (LF/换行) → 回车 ENTER
        if (usbConnected) Keyboard.write(KEY_RETURN);
        Serial.print("[Enter]");
    } else if (c == 0x08) {
        // 0x08 (BS/退格) → 删除 Backspace
        if (usbConnected) Keyboard.write(KEY_BACKSPACE);
        Serial.print("[Backspace]");
    } else if (c == 0x0D) {
        // 忽略 \r，避免 \r\n 产生两次回车
    } else if (c == 0x09) {
        
        if (usbConnected) Keyboard.write(KEY_TAB);
        Serial.print("[Tab]");
    } else if (c == 0x1B) {
       
        if (usbConnected) Keyboard.write(KEY_ESC);
        Serial.print("[Esc]");
    } else if (c >= 32 && c <= 126) {
        
        if (usbConnected) Keyboard.write(c);
        Serial.print((char)c);
    } else {
        
        Serial.printf("[0x%02X]", c);
    }

#if ENABLE_VOICE
    
    queueVoice(c);
#endif
}

void sendToKeyboard(const char* text, size_t len) {
    if (!usbConnected) {
        Serial.println("[USB] USB 未连接，键盘输出暂停（语音仍播报）");
    }

    Serial.print("[键盘] 发送: ");

    
    char buf[RX_BUFFER_SIZE + 4];
    size_t total = 0;
    if (pendingLen > 0) {
        memcpy(buf, pendingBuf, pendingLen);
        total = pendingLen;
        pendingLen = 0;
    }
    if (total + len > sizeof(buf)) len = sizeof(buf) - total;
    memcpy(buf + total, text, len);
    total += len;

    size_t i = 0;
    while (i < total) {
#if ENABLE_HEX_TOKEN_PARSE
        // 解析 
        if (buf[i] == '0' && (i + 1) < total && (buf[i + 1] == 'x' || buf[i + 1] == 'X')) {
            if ((i + 3) < total && isHexChar(buf[i + 2]) && isHexChar(buf[i + 3])) {
                // 完整令牌: 转译成对应字节处理
                uint8_t val = hexVal(buf[i + 2]) * 16 + hexVal(buf[i + 3]);
                typeOneByte(val);
                i += 4;
                continue;
            }
            
            bool couldBeTokenTail = ((i + 2) >= total) ||
                                    ((i + 3) >= total && isHexChar(buf[i + 2]));
            if (couldBeTokenTail) {
                pendingLen = total - i;
                if (pendingLen > sizeof(pendingBuf)) pendingLen = sizeof(pendingBuf);
                memcpy(pendingBuf, buf + i, pendingLen);
                break;
            }
            
        } else if (buf[i] == '0' && (i + 1) >= total) {
            
            pendingLen = 1;
            pendingBuf[0] = '0';
            break;
        }
#endif
        typeOneByte((uint8_t)buf[i]);
        i++;
    }

    Serial.println();

#if ENABLE_VOICE
    
#else
    // LED 闪烁
    ledBlink(1, 50);
#endif
}



void setup() {
    // 初始化 LED
    if (LED_PIN >= 0) {
        pinMode(LED_PIN, OUTPUT);
        ledOff();
    }

    // 初始化串口
    Serial.begin(SERIAL_BAUD);
    delay(500);

    Serial.println();
    Serial.println("╔══════════════════════════════════════════╗");
    Serial.println("║  ESP32-S3 连接 JDY-16 + USB HID 键盘转发 ║");
    Serial.println("║  STM32→JDY-16→ESP32→USB OTG→手机输入框  ║");
    Serial.println("╚══════════════════════════════════════════╝");
    Serial.println();

    //  初始化 USB HID 键盘 
    USB.begin();
    Keyboard.begin();
    Serial.println("[USB] HID 键盘已初始化");

    // 初始化语音播报
#if ENABLE_VOICE
    i2s_chan_config_t chanCfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chanCfg.auto_clear = true;   
    chanCfg.dma_desc_num = 8;        
    chanCfg.dma_frame_num = 2400;    // 8×2400=19200帧 ≈ 435ms@44100Hz ，抗抖动
    i2s_new_channel(&chanCfg, &i2sTx, nullptr);

    i2s_std_config_t stdCfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(VOICE_SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = (gpio_num_t)I2S_BCLK_PIN,
            .ws   = (gpio_num_t)I2S_WS_PIN,
            .dout = (gpio_num_t)I2S_DOUT_PIN,
            .din  = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };
    i2s_channel_init_std_mode(i2sTx, &stdCfg);
    i2s_channel_enable(i2sTx);
    Serial.printf("[语音] I2S 已初始化 (BCLK=%d, WS=%d, DOUT=%d, %dHz)\n",
                  I2S_BCLK_PIN, I2S_WS_PIN, I2S_DOUT_PIN, VOICE_SAMPLE_RATE);
#endif

    //初始BLE 主机模式
    BLEDevice::init("");

    
    BLEDevice::setPower(ESP_PWR_LVL_N3);
    Serial.println("[BLE] 发射功率已设为 -3dBm（省电防欠压）");//电脑USB驱动能力不够，缩短线OK，线太长

    BLEScan* pScan = BLEDevice::getScan();
    pScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pScan->setActiveScan(true);      // 主动扫描
    pScan->setInterval(100);
    pScan->setWindow(99);

    Serial.println("[BLE] 主机模式启动");
    Serial.print("[BLE] 目标设备名称: ");
    Serial.println(JDY16_BLE_NAME);
    Serial.println();

    Serial.println("════════════════════════════════════════════");
    Serial.println("  启动");
    Serial.println("════════════════════════════════════════════");
    Serial.println();

    ledBlink(3, 150);
}


void loop() {
    
#if ENABLE_VOICE
    pumpVoice();
#endif

    // 检测 USB
    bool currentUsb = (bool)USB;
    if (currentUsb != usbConnected) {
        usbConnected = currentUsb;
        if (usbConnected) {
            Serial.println("[USB] 设备已连接 ✓");
        } else {
            Serial.println("[USB] 设备已断开 ✗");
        }
    }

    // 状态机
    if (!bleConnected) {
        if (doConnect) {
            
            doConnect = false;
            if (connectToJDY16()) {
                Serial.println("[BLE] JDY-16 就绪，等待数据...");
            } else {
                Serial.println("[BLE] 连接失败，稍后重试...");
                delay(RECONNECT_DELAY);
            }
        } else {
            // 继续扫描
            BLEScan* pScan = BLEDevice::getScan();
            pScan->start(SCAN_TIME, false);
            pScan->clearResults();
            delay(200);
        }
    }

    // 收到的数据
    if (newDataReceived) {

        noInterrupts();
        char data[RX_BUFFER_SIZE];
        size_t len = rxLength;
        memcpy(data, rxBuffer, len);
        data[len] = '\0';
        rxLength = 0;
        newDataReceived = false;
        interrupts();

        // 通过 USB 键盘发送
        sendToKeyboard(data, len);

        // 可选：回传 ACK 给 STM32
        if (ENABLE_ACK_TO_STM32 && bleConnected && pWriteChar != nullptr) {
            pWriteChar->writeValue("OK", 2);
        }
    }

    // LED 状态指示
    static unsigned long lastLedUpdate = 0;
    unsigned long now = millis();

    if (!usbConnected) {
        // 快闪 = USB 未连接
        if (now - lastLedUpdate > 200) {
            static bool ledState = false;
            ledState = !ledState;
            if (ledState) ledOn(); else ledOff();
            lastLedUpdate = now;
        }
    } else if (!bleConnected) {
        // 慢闪 = USB 已连接，等待 JDY-16
        if (now - lastLedUpdate > 500) {
            static bool ledState = false;
            ledState = !ledState;
            if (ledState) ledOn(); else ledOff();
            lastLedUpdate = now;
        }
    } else {
        // 常亮 = 全部连接
        ledOn();
    }

    delay(2);   // 缩短延时
}

#include <WiFi.h>
#include <HTTPClient.h>
#include <driver/i2s.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <TFT_eSPI.h>

// WiFi credentials
const char* ssid = "Nothing Phone (2a)";
const char* password = "12345678901";

// n8n webhook URL
const char* webhookURL = "http://192.168.68.107:5678/webhook/voice-transcribe";

// Pin definitions for ESP32-S3
#define BUTTON_PIN 7
#define I2S_WS_PIN 5
#define I2S_BCK_PIN 4
#define I2S_SD_PIN 6
#define SD_CS 10
#define SD_MOSI 11
#define SD_SCK 12
#define SD_MISO 13

// ST7789 TFT Display Pin Configuration for ESP32-S3
#define TFT_MOSI 35       // SDA pin (Serial Data)
#define TFT_SCLK 36       // SCL pin (Serial Clock)  
#define TFT_CS   -1       // CS not used (connect to GND or 3.3V)
#define TFT_DC   37       // DC pin (Data/Command)
#define TFT_RST  38       // RES pin (Reset)
#define TFT_BL   39       // BLK pin (Backlight control)

// Audio settings
#define SAMPLE_RATE 16000
#define RECORD_TIME_MS 8000
#define I2S_READ_SIZE 512
#define WAV_HEADER_SIZE 44
#define MAX_RECORD_SIZE (SAMPLE_RATE * 2 * RECORD_TIME_MS / 1000)

// File system
#define RECORDINGS_FOLDER "/codemerge"
#define FILENAME_PREFIX "recordings_"

// Display settings
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240

// Global variables
bool isRecording = false;
bool buttonPressed = false;
unsigned long lastButtonPress = 0;
bool sdCardInitialized = false;
bool wifiConnected = false;
int recordingCounter = 1;

int16_t* i2sBuffer = nullptr;
uint8_t* audioBuffer = nullptr;
uint32_t recordedSamples = 0;
uint32_t bufferIndex = 0;
String currentFilename = "";

// Display object
TFT_eSPI tft = TFT_eSPI();

// Display colors
#define COLOR_BG       TFT_BLACK
#define COLOR_TEXT     TFT_WHITE
#define COLOR_HEADER   TFT_CYAN
#define COLOR_RECORDING TFT_RED
#define COLOR_READY    TFT_GREEN
#define COLOR_WIFI     TFT_BLUE
#define COLOR_SD       TFT_YELLOW
#define COLOR_RED      TFT_RED
#define COLOR_GREEN    TFT_GREEN

// WAV header structure
struct WavHeader {
    char riff[4];
    uint32_t fileSize;
    char wave[4];
    char fmt[4];
    uint32_t fmtSize;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    char data[4];
    uint32_t dataSize;
};

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Voice Recorder System v4.1 with Display - ESP32-S3");
    Serial.println("Recording to: /codemerge/recordings_XXX.wav");
    Serial.println("\n=== ESP32-S3 PIN CONFIGURATION ===");
    Serial.printf("Button: GPIO %d\n", BUTTON_PIN);
    Serial.printf("I2S WS: GPIO %d, BCK: GPIO %d, SD: GPIO %d\n", I2S_WS_PIN, I2S_BCK_PIN, I2S_SD_PIN);
    Serial.printf("SD CS: GPIO %d, MOSI: GPIO %d, SCK: GPIO %d, MISO: GPIO %d\n", 
                  SD_CS, SD_MOSI, SD_SCK, SD_MISO);
    Serial.printf("TFT SDA: GPIO %d, SCL: GPIO %d, DC: GPIO %d, RES: GPIO %d, BLK: GPIO %d\n", 
                  TFT_MOSI, TFT_SCLK, TFT_DC, TFT_RST, TFT_BL);
    Serial.println("==================================\n");
    
    // Initialize components
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    setupDisplay();
    setupSDCard();
    setupI2S();
    connectToWiFi();
    
    // Allocate memory
    i2sBuffer = (int16_t*)heap_caps_malloc(I2S_READ_SIZE * sizeof(int16_t), MALLOC_CAP_DMA);
    audioBuffer = (uint8_t*)heap_caps_malloc(MAX_RECORD_SIZE + WAV_HEADER_SIZE, MALLOC_CAP_8BIT);
    
    if (!i2sBuffer || !audioBuffer) {
        Serial.println("ERROR: Memory allocation failed");
        displayError("Memory allocation failed!");
        while(1) delay(1000);
    }
    
    recordingCounter = findNextRecordingNumber();
    displayMainScreen();
    Serial.println("System ready. Press button to record.");
}

void loop() {
    handleButton();
    
    if (isRecording) {
        recordAudio();
        updateRecordingDisplay();
        
        // Auto-stop at max time
        if (recordedSamples >= (SAMPLE_RATE * RECORD_TIME_MS / 1000)) {
            stopRecording();
        }
    }
    
    delay(10);
}

void setupDisplay() {
    Serial.println("Initializing display...");
    
    // Initialize backlight first
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH); // Turn on backlight
    delay(100);
    
    // Add error handling for display initialization
    try {
        tft.init();
        delay(100);
        tft.setRotation(0); // Adjust rotation as needed (0, 1, 2, 3)
        tft.fillScreen(COLOR_BG);
        
        // Display startup message
        tft.setTextColor(COLOR_HEADER);
        tft.setTextSize(2);
        tft.drawCentreString("Voice Recorder", SCREEN_WIDTH/2, 20, 2);
        tft.setTextColor(COLOR_TEXT);
        tft.setTextSize(1);
        tft.drawCentreString("v4.1 with Display", SCREEN_WIDTH/2, 50, 2);
        
        Serial.println("Display initialized successfully");
    } catch (...) {
        Serial.println("Display initialization failed");
        // Continue without display
    }
}

void setupSDCard() {
    displayAutomationStatus("Initializing SD Card...", COLOR_TEXT);
    // Initialize shared SPI bus
    SPI.begin(SHARED_SCK, SHARED_MISO, SHARED_MOSI, SD_CS);
    
    if (!SD.begin(SD_CS)) {
        Serial.println("SD Card initialization failed");
        sdCardInitialized = false;
        displayAutomationStatus("SD Card FAILED", COLOR_RED);
        delay(2000);
        return;
    }
    
    sdCardInitialized = true;
    Serial.println("SD Card initialized");
    displayAutomationStatus("SD Card OK", COLOR_GREEN);
    delay(1000);
    
    // Create directory if needed
    if (!SD.exists(RECORDINGS_FOLDER)) {
        SD.mkdir(RECORDINGS_FOLDER);
    }
}

void setupI2S() {
    displayStatus("Initializing I2S...", COLOR_TEXT);
    
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = I2S_READ_SIZE,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };
    
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCK_PIN,
        .ws_io_num = I2S_WS_PIN,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD_PIN
    };
    
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    
    Serial.println("I2S initialized");
    displayAutomationStatus("I2S OK", COLOR_GREEN);
    delay(1000);
}

void connectToWiFi() {
    displayStatus("Connecting to WiFi...", COLOR_TEXT);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
        displayStatus("WiFi connecting...", COLOR_WIFI);
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.println("\nWiFi connected");
        displayAutomationStatus("WiFi Connected", COLOR_GREEN);
    } else {
        wifiConnected = false;
        Serial.println("\nWiFi connection failed");
        displayAutomationStatus("WiFi FAILED", COLOR_RED);
    }
    delay(2000);
}

void handleButton() {
    int buttonState = digitalRead(BUTTON_PIN);
    unsigned long currentTime = millis();
    
    if (buttonState == LOW && !buttonPressed && 
        (currentTime - lastButtonPress) > 100) {
        
        buttonPressed = true;
        lastButtonPress = currentTime;
        
        if (!isRecording) {
            startRecording();
        } else {
            stopRecording();
        }
    } else if (buttonState == HIGH) {
        buttonPressed = false;
    }
}

void startRecording() {
    Serial.println("Recording started...");
    
    currentFilename = generateFilename();
    recordedSamples = 0;
    bufferIndex = WAV_HEADER_SIZE;
    
    // Create WAV header placeholder
    WavHeader header;
    createWavHeader(&header, 0);
    memcpy(audioBuffer, &header, sizeof(header));
    
    isRecording = true;
    displayRecordingScreen();
}

void stopRecording() {
    if (!isRecording) return;
    
    Serial.println("Recording stopped");
    isRecording = false;
    
    displayAutomationStatus("Processing...", COLOR_TEXT);
    
    // Update WAV header
    uint32_t dataSize = recordedSamples * 2;
    WavHeader header;
    createWavHeader(&header, dataSize);
    memcpy(audioBuffer, &header, sizeof(header));
    
    saveAudioToSD();
    sendFileToN8N();
    recordingCounter++;
    
    displayMainScreen();
    Serial.println("Ready for next recording");
}

void recordAudio() {
    size_t bytesRead;
    esp_err_t result = i2s_read(I2S_NUM_0, i2sBuffer, I2S_READ_SIZE * sizeof(int16_t), &bytesRead, portMAX_DELAY);
    
    if (result == ESP_OK && bytesRead > 0) {
        size_t samplesRead = bytesRead / sizeof(int16_t);
        
        // Check buffer space
        if (bufferIndex + (samplesRead * 2) > MAX_RECORD_SIZE + WAV_HEADER_SIZE) {
            stopRecording();
            return;
        }
        
        // Store samples
        for (size_t i = 0; i < samplesRead; i++) {
            int16_t sample = i2sBuffer[i];
            audioBuffer[bufferIndex++] = sample & 0xFF;
            audioBuffer[bufferIndex++] = (sample >> 8) & 0xFF;
        }
        
        recordedSamples += samplesRead;
    }
}

void createWavHeader(WavHeader* header, uint32_t dataSize) {
    memcpy(header->riff, "RIFF", 4);
    header->fileSize = dataSize + WAV_HEADER_SIZE - 8;
    memcpy(header->wave, "WAVE", 4);
    memcpy(header->fmt, "fmt ", 4);
    header->fmtSize = 16;
    header->audioFormat = 1;
    header->numChannels = 1;
    header->sampleRate = SAMPLE_RATE;
    header->byteRate = SAMPLE_RATE * 1 * 16 / 8;
    header->blockAlign = 1 * 16 / 8;
    header->bitsPerSample = 16;
    memcpy(header->data, "data", 4);
    header->dataSize = dataSize;
}

void saveAudioToSD() {
    if (!sdCardInitialized) {
        Serial.println("SD Card not available");
        displayAutomationStatus("SD Card Error", COLOR_RED);
        delay(2000);
        return;
    }
    
    displayAutomationStatus("Saving recording...", COLOR_WIFI);
    
    uint32_t totalSize = (recordedSamples * 2) + WAV_HEADER_SIZE;
    
    File audioFile = SD.open(currentFilename, FILE_WRITE);
    if (!audioFile) {
        Serial.println("Failed to create file");
        displayAutomationStatus("Save Error", COLOR_RED);
        delay(2000);
        return;
    }
    
    size_t bytesWritten = audioFile.write(audioBuffer, totalSize);
    audioFile.close();
    
    if (bytesWritten == totalSize) {
        Serial.println("Audio saved to SD card");
        displayAutomationStatus("Recording saved", COLOR_GREEN);
    } else {
        Serial.println("File write incomplete");
        displayAutomationStatus("Save Error", COLOR_RED);
    }
    delay(1000);
}

void sendFileToN8N() {
    if (!sdCardInitialized || !wifiConnected) {
        Serial.println("Cannot send file - SD Card or WiFi not available");
        if (!wifiConnected) {
            displayAutomationStatus("Automation Offline", COLOR_RED);
        } else {
            displayAutomationStatus("SD Card Error", COLOR_RED);
        }
        delay(2000);
        return;
    }
    
    displayAutomationStatus("Sending to automation...", COLOR_WIFI);
    Serial.print("Sending file to webhook... ");
    
    File audioFile = SD.open(currentFilename);
    if (!audioFile) {
        Serial.println("Failed to open file");
        displayAutomationStatus("File Error", COLOR_RED);
        delay(2000);
        return;
    }
    
    size_t fileSize = audioFile.size();
    Serial.print("(");
    Serial.print(fileSize / 1024.0);
    Serial.print(" KB) ");
    
    uint8_t* fileBuffer = (uint8_t*)malloc(fileSize);
    if (!fileBuffer) {
        Serial.println("Memory allocation failed");
        audioFile.close();
        displayAutomationStatus("Memory Error", COLOR_RED);
        delay(2000);
        return;
    }
    
    audioFile.readBytes((char*)fileBuffer, fileSize);
    audioFile.close();
    
    HTTPClient http;
    http.begin(webhookURL);
    http.addHeader("Content-Type", "audio/wav");
    http.setTimeout(30000);
    
    int httpResponseCode = http.POST(fileBuffer, fileSize);
    
    free(fileBuffer);
    
    if (httpResponseCode == 200 || httpResponseCode == 202) {
        Serial.println("File sent successfully");
        displayAutomationStatus("Sent to automation!", COLOR_GREEN);
    } else {
        Serial.print("Send failed (HTTP ");
        Serial.print(httpResponseCode);
        Serial.println(")");
        displayAutomationStatus("Automation Failed", COLOR_RED);
    }
    
    http.end();
    delay(3000); // Show result longer
}

// Display Functions

void displayMainScreen() {
    tft.fillScreen(COLOR_BG);
    
    // WiFi Status - Top Priority
    tft.setTextSize(1);
    if (wifiConnected) {
        tft.setTextColor(COLOR_GREEN);
        tft.drawString("WiFi Connected", 10, 10, 2);
        tft.drawString("Automation Ready", 10, 30, 2);
    } else {
        tft.setTextColor(COLOR_RED);
        tft.drawString("WiFi Disconnected", 10, 10, 2);
        tft.drawString("Local Save Only", 10, 30, 2);
    }
    
    // Recording counter - compact
    tft.setTextColor(COLOR_TEXT);
    char counterStr[20];
    sprintf(counterStr, "Recording #%03d", recordingCounter);
    tft.drawCentreString(counterStr, SCREEN_WIDTH/2, 70, 2);
    
    // Main status
    tft.setTextColor(COLOR_READY);
    tft.setTextSize(3);
    tft.drawCentreString("READY", SCREEN_WIDTH/2, 110, 2);
    
    // Simple instruction
    tft.setTextColor(COLOR_TEXT);
    tft.setTextSize(1);
    tft.drawCentreString("Press to Record", SCREEN_WIDTH/2, 160, 2);
    
    // Quick status line at bottom
    String statusLine = "";
    if (!sdCardInitialized) statusLine += "SD ERR ";
    if (wifiConnected) statusLine += "AUTO ON";
    else statusLine += "AUTO OFF";
    
    tft.setTextColor(COLOR_TEXT);
    tft.drawCentreString(statusLine, SCREEN_WIDTH/2, 210, 1);
}

void displayRecordingScreen() {
    tft.fillScreen(COLOR_BG);
    
    // Recording indicator
    tft.setTextColor(COLOR_RECORDING);
    tft.setTextSize(3);
    tft.drawCentreString("RECORDING", SCREEN_WIDTH/2, 40, 2);
    
    // Current file
    tft.setTextColor(COLOR_TEXT);
    tft.setTextSize(1);
    char fileStr[20];
    sprintf(fileStr, "#%03d", recordingCounter);
    tft.drawCentreString(fileStr, SCREEN_WIDTH/2, 90, 2);
    
    // Automation status
    if (wifiConnected) {
        tft.setTextColor(COLOR_GREEN);
        tft.drawCentreString("Will send to automation", SCREEN_WIDTH/2, 190, 1);
    } else {
        tft.setTextColor(COLOR_RED);  
        tft.drawCentreString("Local save only", SCREEN_WIDTH/2, 190, 1);
    }
    
    tft.setTextColor(COLOR_TEXT);
    tft.drawCentreString("Press to stop", SCREEN_WIDTH/2, 210, 1);
}

void updateRecordingDisplay() {
    static unsigned long lastUpdate = 0;
    static int blinkState = 0;
    
    if (millis() - lastUpdate > 1000) { // Update every 1000ms
        lastUpdate = millis();
        
        // Blink recording dot
        blinkState = !blinkState;
        tft.setTextColor(blinkState ? COLOR_RECORDING : COLOR_BG);
        tft.setTextSize(2);
        tft.drawCentreString("●", SCREEN_WIDTH/2, 115, 4);
        
        // Show elapsed time
        float elapsed = (float)recordedSamples / SAMPLE_RATE;
        char timeStr[20];
        sprintf(timeStr, "%.1fs", elapsed);
        
        tft.setTextColor(COLOR_TEXT);
        // Clear previous time
        tft.fillRect(0, 140, SCREEN_WIDTH, 25, COLOR_BG);
        tft.setTextSize(2);
        tft.drawCentreString(timeStr, SCREEN_WIDTH/2, 140, 2);
    }
}

void displayStatus(const char* message, uint16_t color) {
    // Clear status area
    tft.fillRect(0, SCREEN_HEIGHT - 40, SCREEN_WIDTH, 30, COLOR_BG);
    
    tft.setTextColor(color);
    tft.setTextSize(1);
    tft.drawCentreString(message, SCREEN_WIDTH/2, SCREEN_HEIGHT - 30, 2);
}

void displayAutomationStatus(const char* message, uint16_t color) {
    // Clear center area for automation messages (using working reference style)
    tft.fillRect(0, 100, 240, 60, TFT_BLACK);
    
    // Convert color to TFT colors
    uint16_t tftColor = TFT_WHITE;
    if (color == COLOR_GREEN) tftColor = TFT_GREEN;
    else if (color == COLOR_RED) tftColor = TFT_RED;
    else if (color == COLOR_WIFI) tftColor = TFT_BLUE;
    
    tft.setTextColor(tftColor, TFT_BLACK);
    tft.setTextSize(1);
    tft.drawString(message, 120, 120);
    
    // Add automation icon/indicator
    if (color == COLOR_GREEN) {
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.drawString("✓", 120, 140);
    } else if (color == COLOR_RED) {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString("✗", 120, 140);
    } else {
        // Processing indicator
        tft.setTextColor(TFT_BLUE, TFT_BLACK);
        tft.drawString("●", 120, 140);
    }
}

void displayError(const char* error) {
    tft.fillScreen(COLOR_BG);
    tft.setTextColor(COLOR_RED);
    tft.setTextSize(2);
    tft.drawCentreString("ERROR", SCREEN_WIDTH/2, 80, 2);
    
    tft.setTextColor(COLOR_TEXT);
    tft.setTextSize(1);
    tft.drawCentreString(error, SCREEN_WIDTH/2, 120, 2);
}

String generateFilename() {
    char filename[50];
    sprintf(filename, "%s/%s%03d.wav", RECORDINGS_FOLDER, FILENAME_PREFIX, recordingCounter);
    return String(filename);
}

int findNextRecordingNumber() {
    if (!sdCardInitialized) return 1;
    
    int maxNumber = 0;
    File root = SD.open(RECORDINGS_FOLDER);
    if (!root) return 1;
    
    File file = root.openNextFile();
    while (file) {
        String filename = file.name();
        if (filename.startsWith(FILENAME_PREFIX) && filename.endsWith(".wav")) {
            int startPos = strlen(FILENAME_PREFIX);
            int endPos = filename.lastIndexOf(".wav");
            if (endPos > startPos) {
                String numberStr = filename.substring(startPos, endPos);
                int number = numberStr.toInt();
                if (number > maxNumber) {
                    maxNumber = number;
                }
            }
        }
        file = root.openNextFile();
    }
    root.close();
    
    return maxNumber + 1;
}
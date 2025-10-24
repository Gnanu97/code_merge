Here's a streamlined, essential-details-only README for your voice-controlled healthcare automation system:

```markdown
# 🎙️ Voice-Controlled Healthcare Assistant

Voice-activated patient database management and automated email system using ESP32-S3, n8n, and AI transcription.

![System Overview](WhatsApp-Image-2025-09-11-at-10.06.37_768842e7.jpg)

## 🎯 What It Does

Press a button, speak a command, and the system automatically:
- Transcribes your voice using **Google Gemini AI**
- Searches patient database with natural language
- Sends automated emails to patients
- Handles appointment reminders and medical queries

**Example Commands:**
- "Find all thyroid patients"
- "Send appointment reminder to overdue patients"
- "List heart disease patients"
- "Email diabetes patients about their checkup"

## 🏗️ System Architecture

```
ESP32-S3 (I2S Mic + SD Card) → n8n Webhook → Google Gemini (Transcription)
    → Groq LLM (Intent Classification) → Supabase (Patient DB) → Gmail API
```

## 🔧 Hardware

### Components
- **ESP32-S3** - Main controller
- **I2S MEMS Microphone** - Audio capture
- **SD Card Module** - Local audio storage
- **Push Button** - Recording trigger

### Pin Configuration

| Component | ESP32-S3 Pin |
|-----------|--------------|
| Button | GPIO 7 |
| I2S Mic (WS/BCK/SD) | GPIO 5/4/6 |
| SD Card (CS/MOSI/SCK/MISO) | GPIO 10/11/12/13 |

## ⚙️ Configuration

### Arduino Code (`n8n_automation_codemerge.ino`)
```
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_PASSWORD";
const char* webhookURL = "http://YOUR_IP:5678/webhook/voice-transcribe";
```

### n8n Workflow (`n8n-automation.json`)

**Required API Keys:**
1. **Google Gemini** - Voice transcription ([Get key](https://makersuite.google.com/app/apikey))
2. **Groq** - LLM processing ([Get key](https://console.groq.com/keys))
3. **Gmail OAuth2** - Email sending
4. **Supabase** - Patient database

**Import:** Load `n8n-automation.json` into n8n (http://localhost:5678)

## 🗄️ Database Schema (Supabase)

```
CREATE TABLE code_merge (
  id SERIAL PRIMARY KEY,
  name VARCHAR(255),
  email VARCHAR(255),
  condition VARCHAR(255),
  status VARCHAR(100),
  appointment_date DATE
);
```

## 🚀 How It Works

1. **Press button** → Records 8 seconds of audio
2. **Audio saved** → WAV file stored to SD card (`/codemerge/recordings_XXX.wav`)
3. **Uploaded** → HTTP POST to n8n webhook
4. **Transcribed** → Google Gemini converts speech to text
5. **Analyzed** → Groq LLM determines intent (email or database query)
6. **Executed** → Either queries Supabase or sends Gmail

### n8n Workflow Flow

```
Webhook → Gemini Transcription → Groq Intent Classifier
    ↓
Switch Node (Routes by function)
    ↓
├─ Email Path → Extract Parameters → Gmail Send
└─ Query Path → Vector Search → Patient Results → Batch Email
```

## 📊 Technical Specs

- **Audio:** 16 kHz, 16-bit mono WAV
- **Recording:** 8 seconds max
- **Storage:** SD card (FAT32)
- **WiFi:** 2.4 GHz only
- **Processing:** Real-time AI transcription

## 🔐 Security Notes

- Contains PHI (Protected Health Information) - ensure HIPAA compliance
- Webhook should use HTTPS + authentication in production
- Audio files contain sensitive medical data
- Consider encrypting SD card storage

## 📁 Files

- `n8n_automation_codemerge.ino` - ESP32 firmware
- `n8n-automation.json` - Workflow automation
- `README.md` - This documentation

## ⚡ Key Features

✅ **Hands-free operation** - Voice-controlled interface  
✅ **Natural language** - No rigid command syntax  
✅ **AI-powered** - Gemini transcription + Groq classification  
✅ **Semantic search** - Vector database for intelligent queries  
✅ **Batch automation** - Send emails to multiple patients  
✅ **Local storage** - SD card backup of all recordings  
✅ **Context memory** - Maintains conversation history  
✅ **WiFi enabled** - Wireless data transmission  

## 🎤 Voice Command Examples

### Database Queries
```
"Find thyroid patients"
"Show overdue appointments"
"List all diabetes patients"
```

### Email Actions
```
"Email all heart patients about checkup"
"Send reminder to overdue patients"
"Message jane@example.com about results"
```

## 🔄 Workflow Nodes Explained

1. **Webhook** - Receives audio from ESP32
2. **Gemini Flash** - Speech-to-text transcription
3. **Groq Chat Model** - Intent + parameter extraction
4. **Switch** - Routes to email or database path
5. **Supabase Vector Store** - Semantic patient search
6. **Gmail** - Automated email dispatch
7. **Loop Over Items** - Batch email processing

## 🛠️ Important Settings

**Arduino:**
- Sample rate: `SAMPLE_RATE 16000`
- Recording time: `RECORD_TIME_MS 8000`
- Button pin: `BUTTON_PIN 7`

**n8n:**
- Gemini model: `gemini-1.5-flash`
- Groq model: `openai/gpt-oss-120b`
- Context window: 10 messages

---

**Built for healthcare automation | ESP32-S3 + n8n + AI**
```

This streamlined version focuses only on the critical technical details, configuration requirements, and system architecture without basic installation steps.

[1](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
[2](https://leadsintec.com/in-depth-analysis-of-the-esp32-s3-module-performance-security-and-ecosystem/)
[3](https://www.elprocus.com/esp32-s3-development-board/)
[4](https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/product-overview.html)
[5](https://www.uccindu.com/news/uccnews32s3.html)
[6](https://norvi.lk/advantages-of-esp32-s3-in-building-hmi/)
[7](https://www.sciencedirect.com/science/article/pii/S2772671124002468)
[8](https://circuitdigest.com/esp32-projects)

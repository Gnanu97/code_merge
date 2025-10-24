# 🎙️ Voice-Controlled Healthcare Assistant

> **AI-powered voice automation for patient management and email workflows**

Voice-activated patient database queries and automated email system using **ESP32-S3**, **n8n**, and **AI transcription**.

---
<img width="354" height="468" alt="image" src="https://github.com/user-attachments/assets/c6b1b972-0688-4a0b-9f65-244194fa667e" />

## 🚀 Overview

Press a button, speak naturally, and let AI handle the rest:

- 🎤 **Voice Recognition** — Google Gemini AI transcription  
- 🔍 **Smart Search** — Natural language patient database queries  
- 📧 **Auto Emails** — Batch email sending via Gmail  
- 🤖 **Intent Detection** — Groq LLM classifies commands automatically  

---

## 💡 Example Commands

### 📋 Patient Database Queries

🗣️ "Find all thyroid patients"
🗣️ "Send appointment reminder to overdue patients"
🗣️ "List heart disease patients"
🗣️ "Email diabetes patients about their checkup"



## 🏗️ Architecture

```text
┌──────────────┐     ┌──────────────┐     ┌──────────────┐
│   ESP32-S3   │────▶│  n8n Webhook │────▶│ Google Gemini│
│  + I2S Mic   │     │              │     │ Transcription│
└──────────────┘     └──────────────┘     └──────┬───────┘
                                                  │
                                                  ▼
┌──────────────┐     ┌──────────────┐     ┌──────────────┐
│  Gmail API   │◀────│   Supabase   │◀────│  Groq LLM    │
│  (Emails)    │     │  (Patient DB)│     │ (Intent AI)  │
└──────────────┘     └──────────────┘     └──────────────┘
```

---

## 🔧 Hardware Components

| Component          | Pin              | Purpose              |
| ------------------ | ---------------- | -------------------- |
| **ESP32-S3**       | -                | Main microcontroller |
| **I2S Microphone** | GPIO 5/4/6       | Audio recording      |
| **SD Card Module** | GPIO 10/11/12/13 | Local storage        |
| **Push Button**    | GPIO 7           | Recording trigger    |

### Wiring

```text
ESP32-S3          Component
--------          ---------
GPIO 7     ───►   Push Button
GPIO 5/4/6 ───►   I2S Mic (WS/BCK/SD)
GPIO 10-13 ───►   SD Card (CS/MOSI/SCK/MISO)
```

---

## ⚙️ Configuration

### 📝 Arduino Setup

Edit **`n8n_automation_codemerge.ino`**:

```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_PASSWORD";
const char* webhookURL = "http://YOUR_IP:5678/webhook/voice-transcribe";
```

### 🔗 API Keys Required

| Service           | Purpose             |
| ----------------- | ------------------- |
| **Google Gemini** | Voice transcription |
| **Groq**          | LLM intent analysis |
| **Gmail OAuth2**  | Email automation    |
| **Supabase**      | Patient database    |

### 🗄️ Database Schema

```sql
CREATE TABLE code_merge (
  id SERIAL PRIMARY KEY,
  name VARCHAR(255),
  email VARCHAR(255),
  condition VARCHAR(255),
  status VARCHAR(100),
  appointment_date DATE
);
```

---

## 🎯 How It Works

| Step | Action                                      |
| ---- | ------------------------------------------- |
| 1    | Press button → Records 8 seconds            |
| 2    | Saves WAV to SD card                        |
| 3    | Uploads to n8n webhook via HTTP POST        |
| 4    | Google Gemini transcribes speech            |
| 5    | Groq LLM classifies intent (email or query) |
| 6    | Executes: Supabase search OR Gmail send     |

<img width="1697" height="568" alt="image" src="https://github.com/user-attachments/assets/7de0bf33-49d9-4ed5-8030-5a8fc8d9b40e" />


### 🧩 n8n Workflow Path

```text
Webhook → Gemini → Groq Intent Classifier
                         │
                    ┌────┴────┐
                    ▼         ▼
                Email     Database
                Path       Query
                    │         │
                    ▼         ▼
                 Gmail    Supabase
                         Vector Search
                              │
                              ▼
                         Batch Email
```

---

## 📊 Technical Specifications

```text
Audio Format:    WAV (16kHz, 16-bit, Mono)
Recording Time:  8 seconds max
Storage:         SD Card (FAT32)
WiFi:            2.4 GHz only
Transcription:   Google Gemini 1.5 Flash
LLM Model:       Groq GPT-OSS-120B
Database:        Supabase with vector search
```

---

## 🎤 Voice Command Categories

### 🔍 Database Queries

```bash
✓ "Find thyroid patients"
✓ "Show overdue appointments"
✓ "List all diabetes patients"
✓ "Get heart disease patients"
```

### 📧 Email Commands

```bash
✓ "Email all heart patients about checkup"
✓ "Send reminder to overdue patients"
✓ "Message jane@example.com about results"
```

---

## 🔐 Security Considerations

⚠️ **This system handles Protected Health Information (PHI)**

* ✅ Use HTTPS for n8n webhook
* ✅ Enable webhook authentication
* ✅ Encrypt SD card storage
* ✅ Implement HIPAA compliance measures
* ✅ Use WPA3 WiFi encryption
* ✅ Enable Supabase Row Level Security

---

## 📦 Project Files

```text
📁 Project Root
├── 📄 n8n_automation_codemerge.ino    # ESP32 firmware
├── 📄 n8n-automation.json              # Workflow automation
└── 📄 README.md                        # Documentation
```

---

## ✨ Key Features

| Feature                | Description                |
| ---------------------- | -------------------------- |
| 🎤 **Voice Control**   | Hands-free operation       |
| 🧠 **AI-Powered**      | Gemini + Groq intelligence |
| 🔍 **Semantic Search** | Vector database queries    |
| 📧 **Batch Emails**    | Multiple patient messaging |
| 💾 **Local Backup**    | SD card audio archive      |
| 🌐 **Wireless**        | WiFi data transmission     |
| 🧩 **Context Memory**  | Conversation history       |

---

## 🛠️ Important Settings

### Arduino Constants

```cpp
SAMPLE_RATE 16000       // Audio quality
RECORD_TIME_MS 8000     // Recording duration
BUTTON_PIN 7            // Trigger pin
```

### n8n Configuration

```text
Gemini Model: gemini-1.5-flash
Groq Model: openai/gpt-oss-120b
Memory: 10 message context window
```

---



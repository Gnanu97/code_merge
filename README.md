# 🎙️ Voice-Controlled Healthcare Assistant

An intelligent voice-activated healthcare management system that combines **ESP32-S3 hardware**, **n8n automation**, and **AI-powered voice transcription** to manage patient records and send automated emails through natural voice commands.

## ✨ Overview

This project enables healthcare professionals to interact with patient databases and send emails using voice commands. Simply press a button, speak your command, and the system handles the rest—from transcription to database queries to automated email dispatch.

## 🏗️ System Architecture

### Hardware Layer
- **ESP32-S3** microcontroller with WiFi capability
- **I2S MEMS Microphone** for high-quality audio recording
- **ST7789 TFT Display** (240x240) for real-time status feedback
- **SD Card Module** for local audio storage
- **Push Button** for recording control

### Software Stack
- **Arduino/ESP32** firmware for audio capture and transmission
- **n8n** workflow automation platform
- **Google Gemini 1.5 Flash** for voice transcription
- **Groq LLM** (GPT-OSS-120B) for intent classification
- **Supabase** vector database for patient records
- **Gmail API** for automated email dispatch

## 🎯 Key Features

### Voice Commands Supported
- **Email Commands**: "Send an email to [recipient] about [subject]"
- **Patient Queries**: "Find thyroid patients", "Show overdue appointments", "List heart condition patients"
- **Database Searches**: Natural language queries for specific medical conditions, patient statuses, or demographics

### Automation Capabilities
- Real-time voice-to-text transcription using Google Gemini
- Intelligent intent classification (email vs. database query)
- Semantic search through patient database using vector embeddings
- Batch email sending to multiple patients
- Conversation memory for context-aware interactions

## 🔧 Hardware Setup

### Pin Configuration (ESP32-S3)

| Component | Pin | Function |
|-----------|-----|----------|
| Button | GPIO 7 | Recording trigger |
| I2S Microphone | WS: GPIO 5<br>BCK: GPIO 4<br>SD: GPIO 6 | Audio input |
| SD Card | CS: GPIO 10<br>MOSI: GPIO 11<br>SCK: GPIO 12<br>MISO: GPIO 13 | Local storage |
| TFT Display | SDA: GPIO 35<br>SCL: GPIO 36<br>DC: GPIO 37<br>RST: GPIO 38<br>BL: GPIO 39 | Visual feedback |

### Wiring Diagram

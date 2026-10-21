#pragma once

// Project Wokwi 477223713128462337: da lap mach, chay ket noi day du.
#define LAB_STAGE 3

#define BLYNK_TEMPLATE_ID "TMPL6HbyluXzh"
#define BLYNK_TEMPLATE_NAME "Dieukhiengiongnoi"
#define BLYNK_AUTH_TOKEN "DIEN_DEVICE_AUTH_TOKEN"

// Dung hostname cua thiet bi trong Blynk, khong co https://.
#define BLYNK_SERVER "sgp1.blynk.cloud"
#define THINGSPEAK_CHANNEL_ID 3525538
#define THINGSPEAK_WRITE_API_KEY "DIEN_WRITE_API_KEY"

// Kiem tra relay o phan 1. Doi thanh HIGH neu relay mo phong
// dong COM-NO khi IN=HIGH. LED tai phai sang khi lenh quat=ON.
#define RELAY_ON_LEVEL LOW

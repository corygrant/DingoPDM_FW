#pragma once

#include "port.h"

extern float *pVarMap[VAR_MAP_SIZE];

struct NeoPixelColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

inline constexpr NeoPixelColor NeoPixel_Off          = {  0,   0,   0};
inline constexpr NeoPixelColor NeoPixel_Red          = {255,   0,   0};
inline constexpr NeoPixelColor NeoPixel_Green        = {  0, 255,   0};
inline constexpr NeoPixelColor NeoPixel_Blue         = {  0,   0, 255};
inline constexpr NeoPixelColor NeoPixel_Yellow       = {255, 255,   0};
inline constexpr NeoPixelColor NeoPixel_Cyan         = {  0, 255, 255};
inline constexpr NeoPixelColor NeoPixel_Magenta      = {255,   0, 255};
inline constexpr NeoPixelColor NeoPixel_White        = {255, 255, 255};
inline constexpr NeoPixelColor NeoPixel_Orange       = {255, 165,   0};
inline constexpr NeoPixelColor NeoPixel_Purple       = {128,   0, 128};
inline constexpr NeoPixelColor NeoPixel_Pink         = {255, 192, 203};
inline constexpr NeoPixelColor NeoPixel_Brown        = {165,  42,  42};
inline constexpr NeoPixelColor NeoPixel_LightBlue    = {173, 216, 230};
inline constexpr NeoPixelColor NeoPixel_LightGreen   = {144, 238, 144};
inline constexpr NeoPixelColor NeoPixel_LightYellow  = {255, 255, 224};
inline constexpr NeoPixelColor NeoPixel_LightCyan    = {224, 255, 255};
inline constexpr NeoPixelColor NeoPixel_LightMagenta = {255, 224, 255};
inline constexpr NeoPixelColor NeoPixel_LightOrange  = {255, 200, 150};
inline constexpr NeoPixelColor NeoPixel_LightPurple  = {200, 150, 255};

/*
* Individual NeoPixel
*/
class NeoPixel {
public:
    NeoPixel() = default;
    
    void SetColor(NeoPixelColor color) {
        stScaledColor.r = (uint8_t)(((uint16_t)color.r * nBrightness) / 255);
        stScaledColor.g = (uint8_t)(((uint16_t)color.g * nBrightness) / 255);
        stScaledColor.b = (uint8_t)(((uint16_t)color.b * nBrightness) / 255);
        stActiveColor = color;
    };

    void SetBrightness(uint8_t brightness) { 
        stScaledColor.r = (uint8_t)(((uint16_t)stActiveColor.r * brightness) / 255);
        stScaledColor.g = (uint8_t)(((uint16_t)stActiveColor.g * brightness) / 255);
        stScaledColor.b = (uint8_t)(((uint16_t)stActiveColor.b * brightness) / 255);
        nBrightness = brightness;
    };

    NeoPixelColor GetColor() { return stScaledColor; };
    uint8_t GetRed() { return stScaledColor.r; };
    uint8_t GetGreen() { return stScaledColor.g; };
    uint8_t GetBlue() { return stScaledColor.b; };

    uint32_t GetBytes() { return ((uint32_t)stScaledColor.g << 16) | 
                                 ((uint32_t)stScaledColor.r << 8) | 
                                  (uint32_t)stScaledColor.b; };
    
private:
    NeoPixelColor stActiveColor;
    NeoPixelColor stScaledColor;
    uint8_t nBrightness = 255;
};

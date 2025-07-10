#pragma once

#include <jni.h>
#include <imgui.h>

struct Context {
    JavaVM* vm = nullptr;
    JNIEnv* env = nullptr;
};

// -- Config Structure --
struct WatermarkConfig
{
    char text[256];
    bool showBg;
    bool roundedBg;
    float bgTransparency;
    ImVec4 bgColor;
    bool fancyText;
    ImVec4 fadeStart;
    ImVec4 fadeEnd;
    int glowLayers;
    float glowSpread;
    float glowFadeSpeed;
    bool glowGradient;
    ImVec4 glowInner;
    ImVec4 glowOuter;
    float fadeSpeed;
    bool fadeLeftToRight;
    float scale;
    bool glowEnabled;
    bool useOldGlow;
	bool useStripeGlow;
};

// -- Global Variable --
inline WatermarkConfig g_config;


extern Context ct;

extern bool g_showUI;
extern bool g_unloadRequested;
extern bool g_showWatermark;

inline bool g_watermarkShowBackground = true;
inline bool g_watermarkRoundedBackground = true;
inline char g_watermarkText[128] = "Combust Private | v1.0";
inline ImVec2 g_watermarkPos = ImVec2(50, 50);
inline bool g_draggingWatermark = false;
inline float g_watermarkScale = 1.0f;
inline float g_watermarkBgAlpha = 180.0f; // Default alpha (out of 255)
inline float g_watermarkBgTransparency = 1.0f; // default 70% opacity (0 = fully transparent, 1 = fully opaque)
inline ImVec4 g_watermarkBgColor = ImVec4(0.f, 0.f, 0.f, 1.f); // default black, fully opaque

inline bool g_glowEnabled = true; // MIGHT HAS TO BE RENAMED
inline ImVec4 g_glowColorInner = ImVec4(1.0f, 0.3f, 0.0f, 1.0f); // Orange inner
inline ImVec4 g_glowColorOuter = ImVec4(1.0f, 0.0f, 0.0f, 0.0f); // Red outer, transparent
extern int g_glowLayers;
extern float g_glowSpread;
extern bool g_glowUseBrushGradient;
inline float g_glowFadeSpeed = 1.0f; // default speed

// fuckass glow
inline bool g_useOldGlow = true; // New toggle for old vs moving glow
inline bool g_useStripeGlow = false;
inline float g_NewglowLineLength = 0.1f; // 10% of perimeter

// New for fancy text
extern bool g_fancyTextEnabled;
extern ImVec4 g_fadeColorStart;
extern ImVec4 g_fadeColorEnd;
extern float g_fadeSpeed;
extern bool g_fadeDirectionLeftToRight;

#include "globals.h"
#include <imgui.h>

//bool g_glowEnabled = false;

Context ct;

bool g_showUI = true;
bool g_unloadRequested = false;
bool g_showWatermark = true;

int g_glowLayers = 10;
float g_glowSpread = 10.0f;
bool g_glowUseBrushGradient = false;

// New for fancy text
bool g_fancyTextEnabled = false;
ImVec4 g_fadeColorStart = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red
ImVec4 g_fadeColorEnd = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);   // Cyan
float g_fadeSpeed = 0.5f;
bool g_fadeDirectionLeftToRight = true;


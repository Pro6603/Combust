// opengl_hook.cpp
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <gl/GL.h>
#include <thread>

#include "opengl_hook.h"
#include "globals.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl3.h"
#include "MinHook.h"
#include "utils.h"
#include <iostream>
#include <imgui_internal.h>
#include <fstream>
#include <vector>

//
// Config System (partial)
//

char g_configNameInput[64] = "my_config";
std::vector<std::string> g_availableConfigs;
int g_selectedConfigIndex = -1;

std::string configDir = "configs/";

void SyncToConfig()
{
    strcpy_s(g_config.text, sizeof(g_config.text), g_watermarkText);
    g_config.showBg = g_watermarkShowBackground;
    g_config.roundedBg = g_watermarkRoundedBackground;
    g_config.bgTransparency = g_watermarkBgTransparency;
    g_config.bgColor = g_watermarkBgColor;
    g_config.fancyText = g_fancyTextEnabled;
    g_config.fadeStart = g_fadeColorStart;
    g_config.fadeEnd = g_fadeColorEnd;
    g_config.glowLayers = g_glowLayers;
    g_config.glowSpread = g_glowSpread;
    g_config.glowFadeSpeed = g_glowFadeSpeed;
    g_config.glowGradient = g_glowUseBrushGradient;
    g_config.glowInner = g_glowColorInner;
    g_config.glowOuter = g_glowColorOuter;
    g_config.glowEnabled = g_glowEnabled; // ← new line
    g_config.fadeSpeed = g_fadeSpeed;
    g_config.fadeLeftToRight = g_fadeDirectionLeftToRight;
    g_config.scale = g_watermarkScale;
    g_config.useOldGlow = g_useOldGlow;
    g_config.useStripeGlow = g_useStripeGlow;

}


void SyncFromConfig()
{
    strcpy_s(g_watermarkText, sizeof(g_watermarkText), g_config.text);
    g_watermarkShowBackground = g_config.showBg;
    g_watermarkRoundedBackground = g_config.roundedBg;
    g_watermarkBgTransparency = g_config.bgTransparency;
    g_watermarkBgColor = g_config.bgColor;
    g_fancyTextEnabled = g_config.fancyText;
    g_fadeColorStart = g_config.fadeStart;
    g_fadeColorEnd = g_config.fadeEnd;
    g_glowLayers = g_config.glowLayers;
    g_glowSpread = g_config.glowSpread;
    g_glowFadeSpeed = g_config.glowFadeSpeed;
    g_glowUseBrushGradient = g_config.glowGradient;
    g_glowColorInner = g_config.glowInner;
    g_glowColorOuter = g_config.glowOuter;
    g_glowEnabled = g_config.glowEnabled; // ← new line
    g_fadeSpeed = g_config.fadeSpeed;
    g_fadeDirectionLeftToRight = g_config.fadeLeftToRight;
    g_watermarkScale = g_config.scale;
    g_useOldGlow = g_config.useOldGlow;
    g_useStripeGlow = g_config.useStripeGlow;

}


void SaveConfig(const std::string& filename)
{
    CreateDirectoryA(configDir.c_str(), nullptr); // ensure directory exists
    std::ofstream out(configDir + filename + ".bin", std::ios::binary);
    if (out)
    {
        SyncToConfig();
        out.write(reinterpret_cast<char*>(&g_config), sizeof(g_config));
        out.close();
    }
}

void LoadConfig(const std::string& filename)
{
    std::ifstream in(configDir + filename + ".bin", std::ios::binary);
    if (in)
    {
        in.read(reinterpret_cast<char*>(&g_config), sizeof(g_config));
        in.close();
        SyncFromConfig();
    }
}

void RefreshConfigList()
{
    g_availableConfigs.clear();

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA("configs\\*.bin", &findData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do {
            std::string name = findData.cFileName;
            size_t dot = name.rfind(".bin");
            if (dot != std::string::npos)
                name = name.substr(0, dot);

            g_availableConfigs.push_back(name);
        } while (FindNextFileA(hFind, &findData));

        FindClose(hFind);
    }
}



typedef BOOL(WINAPI* wglSwapBuffers_t)(HDC hdc);
wglSwapBuffers_t o_wglSwapBuffers = nullptr;

HWND g_hwnd = nullptr;
bool g_imgui_initialized = false;

const char* glsl_version = "#version 130";

// ** Global font pointer **
ImFont* g_mainFont = nullptr;

// Forward declaration
void SetupCustomDarkStyle();

inline int clamp(int val, int minVal, int maxVal)
{
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}


BOOL WINAPI hk_wglSwapBuffers(HDC hdc)
{
    static bool triedInit = false;

    if (!g_imgui_initialized && !triedInit)
    {
        triedInit = true;

        g_hwnd = WindowFromDC(hdc);
        if (!g_hwnd)
            g_hwnd = GetActiveWindow();

        if (wglGetCurrentContext() != nullptr)
        {
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();

            io.Fonts->AddFontDefault();
            g_mainFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf", 35.0f);

            if (!g_mainFont)
            {
                MessageBoxA(nullptr, "Failed to load font", "Font Error", MB_ICONERROR);
                return o_wglSwapBuffers(hdc);
            }

            if (!ImGui_ImplWin32_Init(g_hwnd))
            {
                MessageBoxA(nullptr, "ImGui_ImplWin32_Init failed", "ImGui Init Error", MB_ICONERROR);
                return o_wglSwapBuffers(hdc);
            }

            if (!ImGui_ImplOpenGL3_Init(glsl_version))
            {
                MessageBoxA(nullptr, "ImGui_ImplOpenGL3_Init failed", "ImGui Init Error", MB_ICONERROR);
                return o_wglSwapBuffers(hdc);
            }

            SetupCustomDarkStyle();

            RefreshConfigList();

            g_imgui_initialized = true;
        }
    }


    // Handle unload request cleanly
    if (g_unloadRequested)
    {
        RemoveHook();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        // Free DLL and exit thread
        FreeLibraryAndExitThread((HMODULE)GetModuleHandle(nullptr), 0);
        return TRUE; // technically won't get here
    }

    // Toggle UI visibility with Right Ctrl
    static bool wasPressed = false;
    bool isPressed = (GetAsyncKeyState(VK_RCONTROL) & 0x8000) != 0;
    if (isPressed && !wasPressed)
        g_showUI = !g_showUI;
    wasPressed = isPressed;

    // Setup frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Watermark (if enabled)
    // Relevant part inside hk_wglSwapBuffers where watermark is drawn...

    if (g_showWatermark)
    {
        ImGui::PushFont(g_mainFont);
        const char* watermarkText = g_watermarkText;

        // Clamp scale to prevent extreme values
        float scale = ImClamp(g_watermarkScale, 0.1f, 3.0f);

        ImVec2 textSize = ImGui::CalcTextSize(watermarkText);
        textSize.x *= scale;
        textSize.y *= scale;

        ImVec2 position = g_watermarkPos;
        const float paddingX = 8.0f;
        const float paddingY = 3.0f;
        const float rounding = g_watermarkRoundedBackground ? 8.0f : 0.0f;

        ImDrawList* drawList = ImGui::GetForegroundDrawList();

        if (g_watermarkShowBackground)
        {
            ImVec2 bgMin = ImVec2(position.x - paddingX, position.y - paddingY);
            ImVec2 bgMax = ImVec2(position.x + textSize.x + paddingX, position.y + textSize.y + paddingY);

            int alpha = static_cast<int>(255 * g_watermarkBgTransparency);
            alpha = clamp(alpha, 0, 255);

            ImU32 bgCol = IM_COL32(
                (int)(g_watermarkBgColor.x * 255),
                (int)(g_watermarkBgColor.y * 255),
                (int)(g_watermarkBgColor.z * 255),
                alpha
            );

            // Optional: glow parameters
            // TODO: implement the function to edit those values in the ui DONE
            const int glowLayers = 10;
            const float glowSpread = 10.0f;

            if (g_glowEnabled)
            {
                float time = ImGui::GetTime();

                for (int i = 0; i < g_glowLayers; ++i)
                {
                    float t = static_cast<float>(i) / g_glowLayers;
                    float alpha = (1.0f - t) * 0.15f;

                    ImVec2 offset = ImVec2(g_glowSpread * t, g_glowSpread * t);
                    float roundingAmount = rounding + t * 2.0f;

                    ImVec2 glowMin = ImVec2(bgMin.x - offset.x, bgMin.y - offset.y);
                    ImVec2 glowMax = ImVec2(bgMax.x + offset.x, bgMax.y + offset.y);

                    if (g_useOldGlow)
                    {
                        // ✅ OLD GLOW: full gradient fading outward
                        ImVec4 glowColor;

                        if (g_glowUseBrushGradient)
                        {
                            float scroll = fmodf(time * g_glowFadeSpeed, 2.0f);
                            if (scroll > 1.0f)
                                scroll = 2.0f - scroll;

                            if (!g_fadeDirectionLeftToRight)
                                scroll = 1.0f - scroll;

                            glowColor = ImLerp(g_glowColorInner, g_glowColorOuter, scroll);
                        }
                        else
                        {
                            glowColor = ImLerp(g_glowColorInner, g_glowColorOuter, t);
                        }

                        glowColor.w = alpha;
                        ImU32 glowCol = ImGui::ColorConvertFloat4ToU32(glowColor);

                        drawList->AddRectFilled(glowMin, glowMax, glowCol, roundingAmount);
                    }
                    else
                    {
                        // Draw static outer glow layer first
                        ImVec2 innerOffset = ImVec2(g_glowSpread * t, g_glowSpread * t);
                        ImVec2 glowMin = ImVec2(bgMin.x - innerOffset.x, bgMin.y - innerOffset.y);
                        ImVec2 glowMax = ImVec2(bgMax.x + innerOffset.x, bgMax.y + innerOffset.y);

                        ImU32 outerCol = ImGui::ColorConvertFloat4ToU32(ImVec4(g_glowColorOuter.x, g_glowColorOuter.y, g_glowColorOuter.z, alpha));
                        drawList->AddRectFilled(glowMin, glowMax, outerCol, rounding + t * 2.0f);

                        // Moving glow inner line
                        float time = ImGui::GetTime();
                        float perimeter = 2.0f * ((glowMax.x - glowMin.x) + (glowMax.y - glowMin.y));
                        float scroll = fmodf(time * g_glowFadeSpeed * perimeter, perimeter);
                        if (!g_fadeDirectionLeftToRight)
                            scroll = perimeter - scroll;

                        float lineLen = ImClamp(g_NewglowLineLength, 1.0f, perimeter * 0.5f); // Avoid full overlap
                        float start = scroll;
                        float end = scroll + lineLen;

                        ImU32 innerCol = ImGui::ColorConvertFloat4ToU32(ImVec4(g_glowColorInner.x, g_glowColorInner.y, g_glowColorInner.z, alpha * 2.0f));

                        auto drawLineSegment = [&](float& pos, float len, ImVec2 a, ImVec2 b) {
                            float dx = b.x - a.x;
                            float dy = b.y - a.y;
                            float segLen = sqrtf(dx * dx + dy * dy);

                            if (segLen <= 0.0f) return false; // prevent crash on invalid segment

                            if (pos >= segLen) {
                                pos -= segLen;
                                return false;
                            }

                            float l = ImMin(len, segLen - pos);
                            ImVec2 p1 = ImLerp(a, b, pos / segLen);
                            ImVec2 p2 = ImLerp(a, b, (pos + l) / segLen);
                            drawList->AddLine(p1, p2, innerCol, 2.0f);
                            pos = 0.0f;
                            return (len - l > 0.0f);
                        };


                        float segPos = start;
                        float remain = lineLen;

                        ImVec2 corners[5] = {
                            ImVec2(glowMin.x, glowMin.y), // top-left
                            ImVec2(glowMax.x, glowMin.y), // top-right
                            ImVec2(glowMax.x, glowMax.y), // bottom-right
                            ImVec2(glowMin.x, glowMax.y), // bottom-left
                            ImVec2(glowMin.x, glowMin.y)  // back to top-left
                        };

                        for (int j = 0; j < 4 && remain > 0.0f; ++j) {
                            float posCopy = segPos;
                            if (drawLineSegment(posCopy, remain, corners[j], corners[j + 1])) {
                                float dx2 = corners[j + 1].x - corners[j].x;
                                float dy2 = corners[j + 1].y - corners[j].y;
                                remain -= sqrtf(dx2 * dx2 + dy2 * dy2) - posCopy;
                                segPos = 0.0f;
                            }
                        }
                    }


                }
            }


            drawList->AddRectFilled(bgMin, bgMax, bgCol, rounding);
        }


        ImVec2 mousePos = ImGui::GetIO().MousePos;
        ImVec2 textMin = position;
        ImVec2 textMax = ImVec2(position.x + textSize.x, position.y + textSize.y);
        bool hovering = mousePos.x >= textMin.x && mousePos.x <= textMax.x &&
            mousePos.y >= textMin.y && mousePos.y <= textMax.y;

        static bool dragging = false;
        static ImVec2 dragOffset;

        if (hovering && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            dragging = true;
            dragOffset = ImVec2(mousePos.x - g_watermarkPos.x, mousePos.y - g_watermarkPos.y);
        }
        else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            dragging = false;
        }

        if (dragging)
        {
            g_watermarkPos = ImVec2(mousePos.x - dragOffset.x, mousePos.y - dragOffset.y);
        }

        if (g_fancyTextEnabled && g_mainFont)
        {
            ImVec2 charPos = position;
            float time = ImGui::GetTime();
            float scroll = fmodf(time * g_fadeSpeed, 1.0f);
            if (!g_fadeDirectionLeftToRight)
                scroll = 1.0f - scroll;

            float totalWidth = g_mainFont->CalcTextSizeA(g_mainFont->FontSize * scale, FLT_MAX, -1.0f, watermarkText).x;

            for (int i = 0; watermarkText[i] != '\0'; ++i)
            {
                char c[2] = { watermarkText[i], 0 };
                ImVec2 charSize = g_mainFont->CalcTextSizeA(g_mainFont->FontSize * scale, FLT_MAX, -1.0f, c);
                float charXCenter = (charPos.x - position.x + charSize.x * 0.5f) / totalWidth;
                float gradientPos = fmodf(charXCenter + scroll, 1.0f);
                if (gradientPos < 0.0f)
                    gradientPos += 1.0f;

                float blend = 1.0f - fabsf(gradientPos * 2.0f - 1.0f);
                ImVec4 interp = ImLerp(g_fadeColorStart, g_fadeColorEnd, blend);
                ImU32 col = ImGui::ColorConvertFloat4ToU32(interp);
                ImU32 shadowCol = IM_COL32(0, 0, 0, 160);

                drawList->AddText(g_mainFont, g_mainFont->FontSize * scale, ImVec2(charPos.x + 1, charPos.y + 1), shadowCol, c);
                drawList->AddText(g_mainFont, g_mainFont->FontSize * scale, charPos, col, c);
                charPos.x += charSize.x;
            }
        }
        else
        {
            drawList->AddText(g_mainFont, g_mainFont->FontSize * scale, ImVec2(position.x + 1, position.y + 1), IM_COL32(0, 0, 0, 160), watermarkText);
            drawList->AddText(g_mainFont, g_mainFont->FontSize * scale, position, IM_COL32(255, 255, 255, 200), watermarkText);
        }

        ImGui::PopFont();
    }

    if (g_showUI)
    {
        ImGui::PushFont(g_mainFont);
        ImGui::SetNextWindowSize(ImVec2(1050, 720), ImGuiCond_Always);
        ImGui::Begin("Combust Private", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

        if (ImGui::BeginTabBar("MainTabs"))
        {
            if (ImGui::BeginTabItem("Combat"))
            {
                ImGui::TextWrapped("Combat settings go here.");
                // TODO: Add combat controls
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Movement"))
            {
                ImGui::TextWrapped("Movement settings go here.");
                // TODO: Add movement controls
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Render"))
            {
                ImGui::TextWrapped("Render settings go here.");

                ImGui::Checkbox("Show Watermark", &g_showWatermark);

                // TODO: Add other render controls here
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Settings"))
            {
                ImGui::TextWrapped("Settings:");

                ImGui::SeparatorText("Edit Watermark");
                ImGui::InputText("Watermark Text", g_watermarkText, IM_ARRAYSIZE(g_watermarkText));
                ImGui::Checkbox("Show Background", &g_watermarkShowBackground);
                ImGui::Checkbox("Rounded Corners", &g_watermarkRoundedBackground);
                ImGui::SliderFloat("Background Transparency", &g_watermarkBgTransparency, 0.0f, 1.0f, "%.2f");
                ImGui::ColorEdit4("Watermark Background Color", (float*)&g_watermarkBgColor);
                ImGui::Text("Drag the watermark directly with your mouse!");
                ImGui::Checkbox("Enable Fancy Text", &g_fancyTextEnabled);
                ImGui::ColorEdit4("Fade Color Start", (float*)&g_fadeColorStart);
                ImGui::ColorEdit4("Fade Color End", (float*)&g_fadeColorEnd);

                ImGui::SeparatorText("Glow Settings");
                ImGui::Checkbox("Enable Glow Effect", &g_glowEnabled);
                ImGui::Checkbox("Use Old Glow Style", &g_useOldGlow);
                ImGui::SliderFloat("Glow Line Length", &g_NewglowLineLength, 0.01f, 1.0f, "%.2f");
                ImGui::Checkbox("Glow Uses Brush Gradient", &g_glowUseBrushGradient);
                ImGui::ColorEdit4("Glow Inner Color", (float*)&g_glowColorInner);
                ImGui::ColorEdit4("Glow Outer Color", (float*)&g_glowColorOuter);
                ImGui::SliderInt("Glow Layers", &g_glowLayers, 1, 50);
                ImGui::SliderFloat("Glow Spread", &g_glowSpread, 1.0f, 50.0f, "%.1f");
                ImGui::SliderFloat("Glow Fade Speed", &g_glowFadeSpeed, 0.1f, 10.0f, "%.1f");

                ImGui::SeparatorText("General Element Editing");
                ImGui::SliderFloat("Fade Speed", &g_fadeSpeed, 0.1f, 10.0f, "%.1f");
                const char* dirs[] = { "Left to Right", "Right to Left" };
                int dir = g_fadeDirectionLeftToRight ? 0 : 1;
                if (ImGui::Combo("Fade Direction", &dir, dirs, IM_ARRAYSIZE(dirs)))
                    g_fadeDirectionLeftToRight = (dir == 0);
                ImGui::SliderFloat("Watermark Scale", &g_watermarkScale, 0.1f, 3.0f, "%.1f");

                ImGui::SeparatorText("Config Saving/Loading");
                ImGui::InputText("Config Name", g_configNameInput, sizeof(g_configNameInput));

                if (ImGui::Button("Save Config"))
                {
                    SaveConfig(g_configNameInput);
                    RefreshConfigList(); // update list after save
                }

                if (ImGui::Button("Refresh Config List"))
                {
                    RefreshConfigList();
                }

                if (!g_availableConfigs.empty())
                {
                    std::vector<const char*> items;
                    for (const auto& cfg : g_availableConfigs)
                        items.push_back(cfg.c_str());

                    ImGui::Combo("Select Config", &g_selectedConfigIndex, items.data(), static_cast<int>(items.size()));

                    if (ImGui::Button("Load Selected Config") && g_selectedConfigIndex >= 0)
                    {
                        LoadConfig(g_availableConfigs[g_selectedConfigIndex]);
                    }
                }
                else
                {
                    ImGui::TextDisabled("No configs found.");
                }


                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::PopFont();

        ImGui::End();
    }

    ImGui::Render();

    RECT rect;
    if (GetClientRect(g_hwnd, &rect))
    {
        glViewport(0, 0, rect.right - rect.left, rect.bottom - rect.top);
    }

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glPopAttrib();

    return o_wglSwapBuffers(hdc);
}


void SetupCustomDarkStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();

    // Rounded corners
    style.WindowRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 6.0f;

    // Colors
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.95f); // dark gray, almost black
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.25f, 0.25f, 0.25f, 0.8f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);

    // You can customize more colors here as needed
}

void SetupHook()
{
    MH_Initialize();

    HMODULE hOpenGL = GetModuleHandleA("opengl32.dll");
    void* target = GetProcAddress(hOpenGL, "wglSwapBuffers");

    MH_CreateHook(target, &hk_wglSwapBuffers, reinterpret_cast<void**>(&o_wglSwapBuffers));
    MH_EnableHook(target);
    std::wcout << L"Successfully injected Combust\n";
}

void RemoveHook()
{
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
}

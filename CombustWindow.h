/*
#pragma once
#include <imgui.h>
#include <imgui_internal.h> // Ensure internal types like ImVec3 are included

class CombustWindow {
public:
   CombustWindow();
   void Render(bool inGame);

private:
   int currentTab = 0;
   void DrawTabs();
   void DrawCheatTab();
   void DrawSettingsTab();

   bool someWallhackState = false;
   bool someAimbotState = false;
   float uiOpacity = 0.9f;
   ImVec4 accentColor = ImVec4(0.2f, 0.6f, 0.9f, 1.0f);
};
*/
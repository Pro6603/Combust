/*
#include "CombustWindow.h"
#include "CombustStyle.h"

CombustWindow::CombustWindow() {
    ApplyCombustStyle();
}

void CombustWindow::Render(bool inGame) {
    // Apply window transparency based on uiOpacity slider
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg].w = uiOpacity;

    ImGui::SetNextWindowSize(ImVec2(720, 420), ImGuiCond_FirstUseEver);
    ImGui::Begin("Combust", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

    DrawTabs();
    ImGui::Separator();

    if (currentTab == 0) DrawCheatTab();
    if (currentTab == 1) DrawSettingsTab();

    ImGui::Spacing();
    ImGui::SliderFloat("Opacity", &uiOpacity, 0.1f, 1.0f, "%.2f");

    ImGui::ColorEdit4("Accent Color", (float*)&accentColor);

    ImGui::End();
}


void CombustWindow::DrawTabs() {
    if (ImGui::Button("Cheats")) currentTab = 0;
    ImGui::SameLine();
    if (ImGui::Button("Settings")) currentTab = 1;
}

void CombustWindow::DrawCheatTab() {
    ImGui::Text("Toggle your cheats here!");
    ImGui::Checkbox("Wallhack", &someWallhackState);
    ImGui::Checkbox("Aimbot", &someAimbotState);
}

void CombustWindow::DrawSettingsTab() {
    ImGui::Text("Customize appearance here.");
    ImGui::SliderFloat("Opacity", &uiOpacity, 0.0f, 1.0f);
    ImGui::ColorEdit3("Accent Color", (float*)&accentColor);
}
*/


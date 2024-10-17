#include "graph.h"

#include <ImGui/imgui.h>
#include <ImGui/implot.h>
#include <client/client.h>
#include <utils/misc.h>
#include <hooks/maxscore.h>
#include <vector>
#include <string>

void gui::graph::Render() {
	if (ImGui::Begin("Graph", &showGraph, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
        std::vector<unsigned int> values; // It's basically a diagonal matrix because I'm abusing PlotBarGroups to show different colors
        std::vector<const char*> labels;
        std::vector<double> positions;

        std::vector<double> rankPos;
        const char* rankLabels[] = {"A", "AA", "AAA"};

        int i = 0;

        if (client::state.peers.size() > 0) {
            for (const auto& [key, value] : client::state.peers) {
                for (int j = 0; j < i; j++)
                    values.push_back(0);
                values.push_back(utils::CalculateExScore(value.score));
                for (int j = i; j < client::state.peers.size(); j++)
                    values.push_back(0);
                labels.push_back(value.username.c_str());
                positions.push_back(i);
                i++;
            }

            rankPos = { std::ceil(hooks::max_score::maxScore * 0.666), std::ceil(hooks::max_score::maxScore * 0.777), std::ceil(hooks::max_score::maxScore * 0.888) };

            ImGui::BeginChild("GraphDisp", ImVec2(0, 0), ImGuiChildFlags_ResizeX | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
            {
                if (ImPlot::BeginPlot("##GraphPlot", ImVec2(-1, 400), ImPlotFlags_NoFrame | ImPlotFlags_NoInputs | ImPlotFlags_NoTitle | ImPlotFlags_NoLegend)) {
                    ImPlot::SetupAxes("Players", "Score", ImPlotAxisFlags_NoLabel, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoLabel);
                    ImPlot::SetupAxisTicks(ImAxis_X1, positions.data(), labels.size(), labels.data());
                    ImPlot::SetupAxisTicks(ImAxis_Y1, rankPos.data(), rankPos.size(), rankLabels);
                    ImPlot::SetupAxisLimits(ImAxis_Y1, 0, hooks::max_score::maxScore, ImPlotCond_Always);
                    ImPlot::PlotBarGroups(labels.data(), values.data(), labels.size(), labels.size(), 0.67f, 0, ImPlotBarGroupsFlags_Stacked);
                    ImPlot::EndPlot();
                }
                ImGui::EndChild();
            }
            ImGui::SameLine();
            ImGui::BeginChild("ScoreDetails", ImVec2(0, 0), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
            {
                for (const auto& [key, value] : client::state.peers) {
                    ImGui::BulletText((value.username + ": ").c_str());
                    ImGui::SameLine();
                    ImGui::Text(std::to_string(utils::CalculateExScore(value.score)).c_str());

                    ImGui::TextColored(ImVec4(0.765f, 0.976f, 0.824f, 1.0f), ("PG: " + std::to_string(value.score.p_great)).c_str());
                    ImGui::SameLine();
                    ImGui::Text("/");
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(1, 0.824f, 0, 1.0f), ("GR: " + std::to_string(value.score.great)).c_str());
                    ImGui::SameLine();
                    ImGui::Text("/");
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(1, 0.659f, 0, 1.0f), ("GD: " + std::to_string(value.score.good)).c_str());
                    ImGui::SameLine();
                    ImGui::Text("/");
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(1, 0.412f, 0, 1.0f), ("BD: " + std::to_string(value.score.bad)).c_str());
                    ImGui::SameLine();
                    ImGui::Text("/");
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(1, 0.129f, 0, 1.0f), ("PR: " + std::to_string(value.score.poor)).c_str());
                    ImGui::SameLine();
                    ImGui::Text("  "); // Very ugly way to do padding aaaaa
                }
                ImGui::EndChild();
            }
        }
        else {
            ImGui::Text("Not connected to any server...");
        }
	}
    ImGui::End();
}
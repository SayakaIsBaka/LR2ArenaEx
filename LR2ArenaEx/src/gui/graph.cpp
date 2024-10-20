#include "graph.h"

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

        if (client::connected && client::state.peers.size() > 0) {
            for (const auto& [key, value] : client::state.peers) {
                for (int j = 0; j < i; j++)
                    values.push_back(0);
                values.push_back(utils::CalculateExScore(value.score));
                for (int j = i + 1; j < client::state.peers.size(); j++)
                    values.push_back(0);
                labels.push_back(value.username.c_str());
                positions.push_back(i);
                i++;
            }

            rankPos = { std::ceil(hooks::max_score::maxScore * 0.666), std::ceil(hooks::max_score::maxScore * 0.777), std::ceil(hooks::max_score::maxScore * 0.888) };
            auto adjGraphDim = graphDim[overlay::lr2type];
            if (client::state.peers.size() > 2)
                adjGraphDim.x *= (client::state.peers.size() * 0.5);

            ImGui::BeginChild("GraphDisp", ImVec2(0, 0), ImGuiChildFlags_ResizeX | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
            {
                if (ImPlot::BeginPlot("##GraphPlot", adjGraphDim, ImPlotFlags_NoFrame | ImPlotFlags_NoInputs | ImPlotFlags_NoTitle | ImPlotFlags_NoLegend)) {
                    ImPlot::SetupAxes("Players", "Score", ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoLabel, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoLabel);
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
                    ImGui::BulletText("%s: ", value.username.c_str());
                    ImGui::SameLine();
                    ImGui::Text("%d (%.2f%%)", utils::CalculateExScore(value.score), utils::CalculateRate(value.score, hooks::max_score::maxScore));

                    auto opt = utils::GetOptionName(value.option);
                    if (!opt.empty()) {
                        opt = "[" + opt + "]";
                        auto windowWidth = ImGui::GetWindowSize().x;
                        auto textWidth = ImGui::CalcTextSize(opt.c_str()).x;
                        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
                        ImGui::TextDisabled("%s", opt.c_str());
                    }

                    if (ImGui::BeginTable((key.host + std::to_string(key.port)).c_str(), 5))
                    {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("PG");
                        ImGui::TableNextColumn();
                        ImGui::Text("GR");
                        ImGui::TableNextColumn();
                        ImGui::Text("GD");
                        ImGui::TableNextColumn();
                        ImGui::Text("BD");
                        ImGui::TableNextColumn();
                        ImGui::Text("PR  "); // Ugly way to do padding aaaaaaa

                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::TextColored(ImVec4(0.765f, 0.976f, 0.824f, 1.0f), "%d", value.score.p_great);
                        ImGui::TableNextColumn();
                        ImGui::TextColored(ImVec4(1, 0.824f, 0, 1.0f), "%d", value.score.great);
                        ImGui::TableNextColumn();
                        ImGui::TextColored(ImVec4(1, 0.659f, 0, 1.0f), "%d", value.score.good);
                        ImGui::TableNextColumn();
                        ImGui::TextColored(ImVec4(1, 0.412f, 0, 1.0f), "%d", value.score.bad);
                        ImGui::TableNextColumn();
                        ImGui::TextColored(ImVec4(1, 0.129f, 0, 1.0f), "%d", value.score.poor);

                        ImGui::EndTable();
                    }
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
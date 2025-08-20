#include "graph.h"
#include "items.h"

#include <ImGui/implot.h>
#include <client/client.h>
#include <utils/misc.h>
#include <hooks/maxscore.h>
#include <vector>
#include <string>
#include <sstream>

void PrepareData(std::vector<unsigned int> &values, std::vector<const char*> &labels, std::vector<double> &positions, const std::vector<network::Peer> &peers, gui::graph::graphType type) {
    int i = 0;

    for (const auto& value : peers) {
        for (int j = 0; j < i; j++)
            values.push_back(0);
        if (type == gui::graph::graphType::SCORE) {
            values.push_back(utils::CalculateExScore(value.score));
        }
        else if (type == gui::graph::graphType::BP) {
            values.push_back(value.score.bad + value.score.poor);
        }
        else if (type == gui::graph::graphType::MAX_COMBO) {
            values.push_back(value.score.max_combo);
        }
        for (int j = i + 1; j < peers.size(); j++)
            values.push_back(0);
        labels.push_back(value.username.c_str());
        positions.push_back(i);
        i++;
    }
}

void DisplayGraph(std::vector<unsigned int>& values, std::vector<const char*>& labels, std::vector<double>& positions, gui::graph::graphType type) {
    auto adjGraphDim = gui::graph::graphDim[overlay::lr2type];
    if (client::state.peers.size() > 2)
        adjGraphDim.x *= (client::state.peers.size() * 0.5);

    std::string yLabel = "";

    switch (type) {
    case gui::graph::graphType::SCORE:
        yLabel = "Score";
        break;
    case gui::graph::graphType::BP:
        yLabel = "BP";
        break;
    case gui::graph::graphType::MAX_COMBO:
        yLabel = "Max combo";
        break;
    }

    if (ImPlot::BeginPlot("##GraphPlot", adjGraphDim, ImPlotFlags_NoFrame | ImPlotFlags_NoInputs | ImPlotFlags_NoTitle | ImPlotFlags_NoLegend)) {
        ImPlot::SetupAxes("Players", yLabel.c_str(), ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoLabel, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoLabel);
        ImPlot::SetupAxisTicks(ImAxis_X1, positions.data(), labels.size(), labels.data());
        if (type == gui::graph::graphType::SCORE) {
            std::vector<double> rankPos;
            const char* rankLabels[] = { "A", "AA", "AAA" };

            rankPos = { std::ceil(hooks::max_score::maxScore * 0.666), std::ceil(hooks::max_score::maxScore * 0.777), std::ceil(hooks::max_score::maxScore * 0.888) };
            ImPlot::SetupAxisTicks(ImAxis_Y1, rankPos.data(), rankPos.size(), rankLabels);
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0, hooks::max_score::maxScore, ImPlotCond_Always);
        }
        else if (type == gui::graph::graphType::MAX_COMBO) {
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0, hooks::max_score::maxScore / 2, ImPlotCond_Always);
        }
        ImPlot::PlotBarGroups(labels.data(), values.data(), labels.size(), labels.size(), 0.67f, 0, ImPlotBarGroupsFlags_Stacked);
        ImPlot::EndPlot();
    }
}

void gui::graph::Render() {
	if (ImGui::Begin("Graph", &showGraph, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
        items::Render();
        if (client::connected && client::state.peers.size() > 0) {
            std::vector<network::Peer> sortedPeers;
            sortedPeers.reserve(client::state.peers.size());
            for (const auto& [key, value] : client::state.peers) {
                sortedPeers.push_back(value);
            }

            std::sort(sortedPeers.begin(), sortedPeers.end(), [type(sortType)](network::Peer first, network::Peer second) {
                switch (type) {
                case gui::graph::graphType::SCORE: return utils::CalculateExScore(first.score) >= utils::CalculateExScore(second.score);
                case gui::graph::graphType::BP: return first.score.bad + first.score.poor >= second.score.bad + second.score.poor;
                case gui::graph::graphType::MAX_COMBO: return first.score.max_combo >= second.score.max_combo;
                default: return true;
                }
            });

            ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(gui::graph::graphDim[overlay::lr2type].x * 2.0f, FLT_MAX));
            ImGui::BeginChild("GraphDisp", ImVec2(0, 0), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_HorizontalScrollbar);
            {
                std::vector<unsigned int> values; // It's basically a diagonal matrix because I'm abusing PlotBarGroups to show different colors
                std::vector<const char*> labels;
                std::vector<double> positions;

                if (ImGui::BeginTabBar("##TabsGraph", ImGuiTabBarFlags_None))
                {
                    if (ImGui::BeginTabItem("Score")) {
                        sortType = graphType::SCORE;
                        PrepareData(values, labels, positions, sortedPeers, graphType::SCORE);
                        DisplayGraph(values, labels, positions, graphType::SCORE);
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("BP")) {
                        sortType = graphType::BP;
                        PrepareData(values, labels, positions, sortedPeers, graphType::BP);
                        DisplayGraph(values, labels, positions, graphType::BP);
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Max combo")) {
                        sortType = graphType::MAX_COMBO;
                        PrepareData(values, labels, positions, sortedPeers, graphType::MAX_COMBO);
                        DisplayGraph(values, labels, positions, graphType::MAX_COMBO);
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
                ImGui::EndChild();
            }
            ImGui::SameLine();
            ImGui::BeginChild("ScoreDetails", ImVec2(0, 0), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
            {
                int entryIdx = 0;
                for (const auto& value : sortedPeers) {
                    std::stringstream tableIdx("ScoreTable##");
                    tableIdx << entryIdx;
                    ImGui::BulletText("%s: ", value.username.c_str());
                    ImGui::SameLine();
                    ImGui::Text("%d (%.2f%%)", utils::CalculateExScore(value.score), utils::CalculateRate(value.score, hooks::max_score::maxScore));

                    auto opt = utils::GetOptionName(value.option);
                    auto gauge = utils::GetGaugeName(value.gauge);

                    auto str = "[" + gauge + "]";
                    if (!opt.empty()) {
                        str += " [" + opt + "]";
                    }

                    auto windowWidth = ImGui::GetWindowSize().x;
                    auto textWidth = ImGui::CalcTextSize(str.c_str()).x;
                    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
                    ImGui::TextDisabled("%s", str.c_str());

                    if (ImGui::BeginTable(tableIdx.str().c_str(), 5))
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
                    entryIdx++;
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
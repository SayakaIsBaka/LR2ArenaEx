#include "graph.h"

#include <ImGui/imgui.h>
#include <ImGui/implot.h>
#include <client/client.h>
#include <utils/misc.h>
#include <vector>
#include <string>

void gui::graph::Render() { // TODO: different color per bar, set scale, display more details about each player?
	if (ImGui::Begin("Graph", &showGraph, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
        std::vector<unsigned int> values;
        std::vector<const char*> labels;
        std::vector<double> positions;

        int i = 0;

        if (client::state.peers.size() > 0) {
            for (const auto& [key, value] : client::state.peers) {
                values.push_back(utils::CalculateExScore(value.score));
                labels.push_back(value.username.c_str());
                positions.push_back(i);
                i++;
            }

            if (ImPlot::BeginPlot("##GraphPlot", ImVec2(-1, 400), ImPlotFlags_NoFrame | ImPlotFlags_NoInputs | ImPlotFlags_NoTitle | ImPlotFlags_NoLegend)) {
                ImPlot::SetupAxes("Players", "Score", ImPlotAxisFlags_NoLabel, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoLabel);
                ImPlot::SetupAxisTicks(ImAxis_X1, positions.data(), labels.size(), labels.data());
                ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 10000, ImPlotCond_Always);
                ImPlot::PlotBars("", values.data(), values.size(), 0.67f, 0, 0);
                ImPlot::EndPlot();
            }
        }
        else {
            ImGui::Text("Not connected to any server...");
        }
	}
    ImGui::End();
}
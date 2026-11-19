#pragma once

#include "Simulation/FociSimTypes.h"

namespace FociSim
{

enum class GraphicsPreset : uint8_t
{
	MacBook2018_IrisPerformance = 0,
	MacBook2018_RadeonBalanced = 1,
	PhotorealBroadcastReplay = 2
};

struct GraphicsConfig
{
	int32_t TargetFPS = 60;
	float ScreenPercentage = 85.0f;
	int32_t MSAACount = 2;
	int32_t CSMMaxCascades = 2;
	int32_t MaxCSMResolution = 1024;
	bool bEnableCapsuleShadows = true;
	int32_t SSRQuality = 2;
	float GrassCullDistanceMultiplier = 1.0f;
};

inline GraphicsConfig GetGraphicsConfigForPreset(GraphicsPreset Preset)
{
	switch (Preset)
	{
	case GraphicsPreset::MacBook2018_IrisPerformance:
		return GraphicsConfig{60, 75.0f, 2, 1, 1024, true, 1, 0.75f};
	case GraphicsPreset::MacBook2018_RadeonBalanced:
		return GraphicsConfig{60, 85.0f, 2, 2, 1024, true, 2, 1.0f};
	case GraphicsPreset::PhotorealBroadcastReplay:
		return GraphicsConfig{30, 100.0f, 4, 3, 2048, true, 3, 1.5f};
	default:
		return GraphicsConfig{60, 85.0f, 2, 2, 1024, true, 2, 1.0f};
	}
}

} // namespace FociSim

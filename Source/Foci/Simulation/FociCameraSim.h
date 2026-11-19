#pragma once

#include "Simulation/FociSimTypes.h"
#include <algorithm>
#include <vector>

namespace FociSim
{

inline void ComputeBroadcastCameraFrame3D(
	const Vec3& BallLocation,
	const std::vector<Vec3>& HumanPlayerLocations,
	float BallWeight,
	float MinArmLength,
	float MaxArmLength,
	Vec3& OutTargetFocusLocation,
	float& OutTargetArmLength)
{
	Vec3 HumanCentroid = BallLocation;
	if (!HumanPlayerLocations.empty())
	{
		Vec3 Sum(0.0f, 0.0f, 0.0f);
		for (const Vec3& Loc : HumanPlayerLocations)
		{
			Sum += Loc;
		}
		const float InvCount = 1.0f / static_cast<float>(HumanPlayerLocations.size());
		HumanCentroid = Sum * InvCount;
	}

	const float ClampedWeight = Clamp(BallWeight, 0.0f, 1.0f);
	OutTargetFocusLocation = Lerp(HumanCentroid, BallLocation, ClampedWeight);
	OutTargetFocusLocation.Z = 60.0f;

	float MaxSpreadCm = Vec3::Dist2D(OutTargetFocusLocation, BallLocation);
	for (const Vec3& Loc : HumanPlayerLocations)
	{
		MaxSpreadCm = std::max(MaxSpreadCm, Vec3::Dist2D(OutTargetFocusLocation, Loc));
	}

	OutTargetArmLength = GetMappedRangeValueClamped(800.0f, 3800.0f, MinArmLength, MaxArmLength, MaxSpreadCm);
}

} // namespace FociSim

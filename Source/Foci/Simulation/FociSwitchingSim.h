#pragma once

#include "Simulation/FociSimTypes.h"
#include <algorithm>
#include <limits>
#include <vector>

namespace FociSim
{

inline int32_t SelectBestTeammateToSwitch3D(
	int32_t RequestingHumanPlayerIndex,
	int32_t CurrentControlledCandidateIndex,
	const Vec3& CurrentFootballerLocation,
	const Vec3& BallLocation,
	const Vec2& RightStickFlickDirection,
	const std::vector<Vec3>& TeammateLocations,
	const std::vector<int32_t>& TeammateControllingHumanIndices,
	const std::vector<bool>& TeammateIsGoalkeeper)
{
	(void)RequestingHumanPlayerIndex;

	const int32_t Count = static_cast<int32_t>(std::min({
		TeammateLocations.size(),
		TeammateControllingHumanIndices.size(),
		TeammateIsGoalkeeper.size()
	}));

	const bool bDirectionalSwitch = (RightStickFlickDirection.SizeSquared() >= 0.2f);
	const Vec3 FlickDir3D = bDirectionalSwitch ? StickToBroadcastWorldDir(RightStickFlickDirection) : Vec3();

	int32_t BestIndex = -1;
	float BestDist = std::numeric_limits<float>::max();
	float BestScore = -std::numeric_limits<float>::max();

	for (int32_t i = 0; i < Count; ++i)
	{
		const size_t Idx = static_cast<size_t>(i);
		if (i == CurrentControlledCandidateIndex)
		{
			continue;
		}
		if (TeammateIsGoalkeeper[Idx])
		{
			continue;
		}
		if (TeammateControllingHumanIndices[Idx] != -1)
		{
			continue;
		}

		if (!bDirectionalSwitch)
		{
			const float Dist = Vec3::Dist3D(TeammateLocations[Idx], BallLocation);
			if (Dist < BestDist)
			{
				BestDist = Dist;
				BestIndex = i;
			}
		}
		else
		{
			const Vec3 ToCand = TeammateLocations[Idx] - CurrentFootballerLocation;
			const float Dist = ToCand.Size2D();
			if (Dist < 1.0f)
			{
				continue;
			}
			const Vec3 Dir = ToCand.GetSafeNormal2D();
			const float Dot = Vec3::Dot(FlickDir3D, Dir);
			if (Dot < 0.342f)
			{
				continue;
			}
			const float Score = (Dot * 2000.0f) - (Dist * 0.35f);
			if (Score > BestScore)
			{
				BestScore = Score;
				BestIndex = i;
			}
		}
	}

	return BestIndex;
}

} // namespace FociSim

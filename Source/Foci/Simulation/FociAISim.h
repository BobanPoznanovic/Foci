#pragma once

#include "Simulation/FociSimTypes.h"
#include <algorithm>
#include <cmath>

namespace FociSim
{

inline Vec3 ComputeDynamicFormationTarget3D(
	const Vec3& BaseFormationAnchor,
	const Vec3& BallLocation,
	TeamSide MyTeam,
	bool bMyTeamHasPossession,
	float HalfPitchLengthCm = 5250.0f,
	float HalfPitchWidthCm = 3400.0f)
{
	const float AttackSign = (MyTeam == TeamSide::Home) ? 1.0f : ((MyTeam == TeamSide::Away) ? -1.0f : 0.0f);
	const float BallShiftX = BallLocation.X * 0.25f;
	const float PossessionBiasX = bMyTeamHasPossession ? (650.0f * AttackSign) : (-450.0f * AttackSign);
	const float TargetX = Clamp(BaseFormationAnchor.X + BallShiftX + PossessionBiasX, -HalfPitchLengthCm + 300.0f, HalfPitchLengthCm - 300.0f);
	const float TargetY = Clamp(BaseFormationAnchor.Y + (BallLocation.Y - BaseFormationAnchor.Y) * 0.20f, -HalfPitchWidthCm + 250.0f, HalfPitchWidthCm - 250.0f);
	return Vec3(TargetX, TargetY, BaseFormationAnchor.Z);
}

inline bool ComputeGoalkeeperIntercept3D(
	const Vec3& BallLocation,
	const Vec3& BallVelocity,
	float GoalLineX,
	float GoalHalfWidthCm,
	float CrossbarHeightCm,
	Vec3& OutInterceptPoint3D,
	float& OutTimeToGoalLine)
{
	if (std::fabs(BallVelocity.X) <= 150.0f)
	{
		return false;
	}

	OutTimeToGoalLine = (GoalLineX - BallLocation.X) / BallVelocity.X;
	if (OutTimeToGoalLine < 0.02f || OutTimeToGoalLine > 1.8f)
	{
		return false;
	}

	const float YPred = BallLocation.Y + BallVelocity.Y * OutTimeToGoalLine;
	const float ZPred = std::max(11.0f, BallLocation.Z + BallVelocity.Z * OutTimeToGoalLine - 0.5f * 980.0f * OutTimeToGoalLine * OutTimeToGoalLine);

	if (std::fabs(YPred) <= (GoalHalfWidthCm + 60.0f) && ZPred <= (CrossbarHeightCm + 40.0f))
	{
		OutInterceptPoint3D = Vec3(
			GoalLineX,
			Clamp(YPred, -GoalHalfWidthCm, GoalHalfWidthCm),
			Clamp(ZPred, 11.0f, CrossbarHeightCm)
		);
		return true;
	}

	return false;
}

} // namespace FociSim

#pragma once

#include "Simulation/FociSimTypes.h"
#include <algorithm>
#include <functional>
#include <vector>

namespace FociSim
{

enum class PitchEvent : uint8_t
{
	InPlay = 0,
	GoalScoredHome = 1,
	GoalScoredAway = 2,
	ThrowInHome = 3,
	ThrowInAway = 4,
	CornerKickHome = 5,
	CornerKickAway = 6,
	GoalKickHome = 7,
	GoalKickAway = 8
};

inline PitchEvent EvaluateBallBoundary3D(
	const Vec3& BallCenterLocation,
	float BallRadiusCm,
	TeamSide LastTouchTeam,
	float HalfLengthCm = 5250.0f,
	float HalfWidthCm = 3400.0f,
	float GoalHalfWidth = 366.0f,
	float CrossbarHeight = 244.0f)
{
	// 1. Crossed +X goal line
	if (BallCenterLocation.X - BallRadiusCm > HalfLengthCm)
	{
		if (std::fabs(BallCenterLocation.Y) <= (GoalHalfWidth - BallRadiusCm) &&
			BallCenterLocation.Z >= 0.0f &&
			(BallCenterLocation.Z + BallRadiusCm) <= CrossbarHeight)
		{
			return PitchEvent::GoalScoredHome;
		}
		return (LastTouchTeam == TeamSide::Home) ? PitchEvent::GoalKickAway : PitchEvent::CornerKickHome;
	}

	// 2. Crossed -X goal line
	if (BallCenterLocation.X + BallRadiusCm < -HalfLengthCm)
	{
		if (std::fabs(BallCenterLocation.Y) <= (GoalHalfWidth - BallRadiusCm) &&
			BallCenterLocation.Z >= 0.0f &&
			(BallCenterLocation.Z + BallRadiusCm) <= CrossbarHeight)
		{
			return PitchEvent::GoalScoredAway;
		}
		return (LastTouchTeam == TeamSide::Away) ? PitchEvent::GoalKickHome : PitchEvent::CornerKickAway;
	}

	// 3. Crossed +-Y touchline
	if (std::fabs(BallCenterLocation.Y) - BallRadiusCm > HalfWidthCm)
	{
		return (LastTouchTeam == TeamSide::Home) ? PitchEvent::ThrowInAway : PitchEvent::ThrowInHome;
	}

	// 4. Otherwise in play
	return PitchEvent::InPlay;
}

inline bool IsPlayerInOffsidePosition3D(
	float ReceiverX,
	float BallXAtPass,
	TeamSide AttackingTeam,
	const std::vector<float>& DefenderXPositions,
	float HalfLengthCm = 5250.0f)
{
	if (AttackingTeam == TeamSide::Neutral)
	{
		return false;
	}

	// Own-half immunity
	if (AttackingTeam == TeamSide::Home && ReceiverX <= 0.0f)
	{
		return false;
	}
	if (AttackingTeam == TeamSide::Away && ReceiverX >= 0.0f)
	{
		return false;
	}

	std::vector<float> Sorted = DefenderXPositions;
	if (AttackingTeam == TeamSide::Home)
	{
		std::sort(Sorted.begin(), Sorted.end(), std::greater<float>());
		const float SecondLastDefenderX = (Sorted.size() >= 2) ? Sorted[1] : HalfLengthCm;
		return (ReceiverX > BallXAtPass + 5.0f) && (ReceiverX > SecondLastDefenderX + 5.0f);
	}
	else
	{
		std::sort(Sorted.begin(), Sorted.end(), std::less<float>());
		const float SecondLastDefenderX = (Sorted.size() >= 2) ? Sorted[1] : -HalfLengthCm;
		return (ReceiverX < BallXAtPass - 5.0f) && (ReceiverX < SecondLastDefenderX - 5.0f);
	}
}

inline Vec3 ComputeRestartLocation3D(
	PitchEvent Event,
	const Vec3& LastOutBallLocation,
	float HalfLengthCm = 5250.0f,
	float HalfWidthCm = 3400.0f,
	float BallRadiusCm = 11.0f)
{
	switch (Event)
	{
	case PitchEvent::CornerKickHome:
		return Vec3(
			HalfLengthCm - 50.0f,
			(LastOutBallLocation.Y >= 0.0f) ? (HalfWidthCm - 50.0f) : -(HalfWidthCm - 50.0f),
			BallRadiusCm
		);
	case PitchEvent::CornerKickAway:
		return Vec3(
			-(HalfLengthCm - 50.0f),
			(LastOutBallLocation.Y >= 0.0f) ? (HalfWidthCm - 50.0f) : -(HalfWidthCm - 50.0f),
			BallRadiusCm
		);
	case PitchEvent::GoalKickHome:
		return Vec3(-(HalfLengthCm - 550.0f), 0.0f, BallRadiusCm);
	case PitchEvent::GoalKickAway:
		return Vec3(HalfLengthCm - 550.0f, 0.0f, BallRadiusCm);
	case PitchEvent::ThrowInHome:
	case PitchEvent::ThrowInAway:
		return Vec3(
			Clamp(LastOutBallLocation.X, -(HalfLengthCm - 50.0f), HalfLengthCm - 50.0f),
			(LastOutBallLocation.Y >= 0.0f) ? (HalfWidthCm - 20.0f) : -(HalfWidthCm - 20.0f),
			BallRadiusCm
		);
	case PitchEvent::GoalScoredHome:
	case PitchEvent::GoalScoredAway:
	case PitchEvent::InPlay:
	default:
		return Vec3(0.0f, 0.0f, BallRadiusCm);
	}
}

} // namespace FociSim

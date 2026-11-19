#pragma once

#include "Simulation/FociSimTypes.h"

namespace FociSim
{

inline void ComputeKickVelocityAndSpin3D(
	const Vec3& StartLocation,
	const Vec3& RawAimDirection2D,
	float PowerRatio,
	bool bIsShot,
	bool bIsLobOrCross,
	bool bIsThroughBall,
	bool bIsFinesse,
	bool bIsChip,
	AssistLevel Assist,
	const Vec3& IdealAssistedTargetLocation,
	Vec3& OutLaunchVelocity,
	Vec3& OutSpinVectorRadPerSec)
{
	const float P = Clamp(PowerRatio, 0.0f, 1.0f);

	Vec3 DirRaw = RawAimDirection2D.GetSafeNormal2D();
	if (DirRaw.SizeSquared2D() < kSmallNumber)
	{
		DirRaw = Vec3(1.0f, 0.0f, 0.0f);
	}

	const Vec3 ToIdeal = IdealAssistedTargetLocation - StartLocation;
	const Vec3 DirIdeal = (ToIdeal.Size2D() >= 1.0f) ? ToIdeal.GetSafeNormal2D() : DirRaw;

	const float AssistWeight = (Assist == AssistLevel::Assisted) ? 0.75f : ((Assist == AssistLevel::Semi) ? 0.35f : 0.0f);
	Vec3 FinalDir2D = Lerp(DirRaw, DirIdeal, AssistWeight).GetSafeNormal2D();
	if (FinalDir2D.SizeSquared2D() < kSmallNumber)
	{
		FinalDir2D = DirRaw;
	}

	auto MakeBackspin = [&](float Mag) -> Vec3
	{
		return Vec3(Mag * FinalDir2D.Y, -Mag * FinalDir2D.X, 0.0f);
	};

	float PitchDeg = 2.0f;
	float Speed = 900.0f;

	if (bIsShot && bIsChip)
	{
		PitchDeg = 42.0f;
		Speed = Lerp(1100.0f, 1750.0f, P);
		OutSpinVectorRadPerSec = MakeBackspin(16.0f);
	}
	else if (bIsShot && bIsFinesse)
	{
		PitchDeg = Lerp(6.0f, 16.0f, P);
		Speed = Lerp(1800.0f, 3400.0f, P) * 0.86f;
		const float SideSign = ((StartLocation.Y >= 0.0f) == (FinalDir2D.X >= 0.0f)) ? -1.0f : 1.0f;
		OutSpinVectorRadPerSec = Vec3(0.0f, 0.0f, SideSign * 38.0f);
	}
	else if (bIsShot)
	{
		PitchDeg = Lerp(4.0f, 19.0f, P);
		Speed = Lerp(1800.0f, 3400.0f, P);
		OutSpinVectorRadPerSec = MakeBackspin(6.0f);
	}
	else if (bIsLobOrCross)
	{
		PitchDeg = Lerp(24.0f, 38.0f, P);
		Speed = Lerp(1400.0f, 2700.0f, P);
		OutSpinVectorRadPerSec = MakeBackspin(18.0f);
	}
	else if (bIsThroughBall)
	{
		PitchDeg = bIsChip ? 32.0f : 4.0f;
		Speed = Lerp(1200.0f, 2600.0f, P);
		OutSpinVectorRadPerSec = bIsChip ? MakeBackspin(14.0f) : Vec3(0.0f, 0.0f, 0.0f);
	}
	else
	{
		PitchDeg = 2.0f;
		Speed = Lerp(900.0f, 2400.0f, P);
		OutSpinVectorRadPerSec = Vec3(0.0f, 0.0f, 0.0f);
	}

	const float PitchRad = PitchDeg * (kPi / 180.0f);
	const float HorizSpeed = Speed * std::cos(PitchRad);
	const float VertSpeed = Speed * std::sin(PitchRad);

	OutLaunchVelocity = Vec3(FinalDir2D.X * HorizSpeed, FinalDir2D.Y * HorizSpeed, VertSpeed);
}

} // namespace FociSim

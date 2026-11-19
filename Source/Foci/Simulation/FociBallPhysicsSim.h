#pragma once

#include "Simulation/FociSimTypes.h"

namespace FociSim
{

struct BallPhysicsParams
{
	float BallRadiusCm = 11.0f;
	float BallMassKg = 0.43f;
	float TurfRestitution = 0.68f;
	float TurfRollingFriction = 320.0f;
	float AirDragCoefficient = 0.00022f;
	float MagnusCoefficient = 0.010f;
	float SpinDecayRate = 1.4f;
	float GravityCmPerSecSq = 980.0f;
};

inline void StepBallPhysics3D(
	Vec3& InOutPosition,
	Vec3& InOutVelocity,
	Vec3& InOutAngularVelocity,
	float DeltaTime,
	float GroundZ = 0.0f,
	float RadiusCm = 11.0f,
	float Restitution = 0.68f,
	float RollingFriction = 320.0f,
	float DragCoeff = 0.00022f,
	float MagnusCoeff = 0.010f,
	float SpinDecay = 1.4f)
{
	if (DeltaTime <= 0.0f)
	{
		return;
	}

	// Step 1: Air Drag
	float Speed = InOutVelocity.Size();
	InOutVelocity += InOutVelocity * (-DragCoeff * Speed * DeltaTime);

	// Step 2: Magnus Lift / Curve
	Speed = InOutVelocity.Size();
	const float SpinMag = InOutAngularVelocity.Size();
	if (SpinMag > kSmallNumber && Speed > 20.0f)
	{
		const Vec3 MagnusAccel = Vec3::Cross(InOutAngularVelocity, InOutVelocity) * MagnusCoeff;
		InOutVelocity += MagnusAccel * DeltaTime;
	}

	// Step 3: Gravity vs. Ground Roll
	const float ZMin = GroundZ + RadiusCm;
	if (InOutPosition.Z <= ZMin + 0.5f && std::fabs(InOutVelocity.Z) < 35.0f)
	{
		InOutPosition.Z = ZMin;
		InOutVelocity.Z = 0.0f;
		const float HorizSpeed = InOutVelocity.Size2D();
		if (HorizSpeed > 0.0f)
		{
			const float NewHoriz = std::max(0.0f, HorizSpeed - RollingFriction * DeltaTime);
			const float Scale = NewHoriz / HorizSpeed;
			InOutVelocity.X *= Scale;
			InOutVelocity.Y *= Scale;
		}
	}
	else
	{
		InOutVelocity.Z -= 980.0f * DeltaTime;
	}

	// Step 4: Position Integration
	InOutPosition += InOutVelocity * DeltaTime;

	// Step 5: Turf Bounce
	if (InOutPosition.Z < ZMin)
	{
		InOutPosition.Z = ZMin;
		if (InOutVelocity.Z < 0.0f)
		{
			InOutVelocity.Z = -InOutVelocity.Z * Restitution;
			InOutVelocity.X *= 0.88f;
			InOutVelocity.Y *= 0.88f;
			if (InOutVelocity.Z < 25.0f)
			{
				InOutVelocity.Z = 0.0f;
			}
		}
	}

	// Step 6: Spin Decay
	const float DecayFactor = std::max(0.0f, 1.0f - SpinDecay * DeltaTime);
	InOutAngularVelocity = InOutAngularVelocity * DecayFactor;
}

inline Vec3 PredictBallPositionAtTime(
	const Vec3& StartPos,
	const Vec3& StartVel,
	const Vec3& StartAngVel,
	float FutureSeconds,
	float GroundZ = 0.0f,
	const BallPhysicsParams& Params = {})
{
	Vec3 Pos = StartPos;
	Vec3 Vel = StartVel;
	Vec3 AngVel = StartAngVel;

	constexpr float StepDt = 1.0f / 60.0f;
	float Elapsed = 0.0f;
	while (Elapsed < FutureSeconds)
	{
		const float Dt = std::min(StepDt, FutureSeconds - Elapsed);
		StepBallPhysics3D(
			Pos,
			Vel,
			AngVel,
			Dt,
			GroundZ,
			Params.BallRadiusCm,
			Params.TurfRestitution,
			Params.TurfRollingFriction,
			Params.AirDragCoefficient,
			Params.MagnusCoefficient,
			Params.SpinDecayRate
		);
		Elapsed += Dt;
	}
	return Pos;
}

} // namespace FociSim

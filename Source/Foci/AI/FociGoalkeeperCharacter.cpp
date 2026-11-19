#include "AI/FociGoalkeeperCharacter.h"
#include "Gameplay/FociBallActor.h"
#include "Simulation/FociAISim.h"

AFociGoalkeeperCharacter::AFociGoalkeeperCharacter()
{
	bIsGoalkeeper = true;
}

void AFociGoalkeeperCharacter::UpdateGoalkeeperAI(AFociBallActor* Ball, float DeltaTime)
{
	if (Ball == nullptr)
	{
		return;
	}

	const float GoalLineX = (TeamSide == EFociTeamSide::Home) ? -5250.0f : 5250.0f;
	const float StandX = (TeamSide == EFociTeamSide::Home) ? -5050.0f : 5050.0f;

	DiveCooldownSeconds = FMath::Max(0.0f, DiveCooldownSeconds - DeltaTime);
	if (DiveCooldownSeconds <= 0.0f)
	{
		bIsDiving = false;
	}

	FVector InterceptPt = FVector::ZeroVector;
	float TimeToLine = 0.0f;
	if (ComputeGoalkeeperIntercept3D(Ball->GetActorLocation(), Ball->LinearVelocity, GoalLineX, 366.0f, 244.0f, InterceptPt, TimeToLine))
	{
		if (DiveCooldownSeconds <= 0.0f)
		{
			TriggerDiveToward(InterceptPt);
		}
	}

	const float DistToBall = FVector::Dist(GetActorLocation(), Ball->GetActorLocation());
	if (DistToBall <= DiveReachCm && !Ball->PossessingFootballer.IsValid())
	{
		if (Ball->LinearVelocity.Size() <= CatchMaxShotSpeedCmPerSec)
		{
			Ball->SetPossessingFootballer(this, TeamSide);
		}
		else
		{
			const FVector ParryDir(
				(TeamSide == EFociTeamSide::Home) ? 1.0f : -1.0f,
				(GetActorLocation().Y >= 0.0f) ? 0.6f : -0.6f,
				0.35f
			);
			Ball->KickBall(ParryDir.GetSafeNormal() * 1100.0f, FVector::ZeroVector, this, TeamSide);
		}
		return;
	}

	const FVector TargetPos(StandX, FMath::Clamp(Ball->GetActorLocation().Y * 0.45f, -280.0f, 280.0f), GetActorLocation().Z);
	const FVector ToTarget = TargetPos - GetActorLocation();
	if (ToTarget.Size2D() > 25.0f)
	{
		const FVector Dir = ToTarget.GetSafeNormal2D();
		SetMovementInput3D(FVector2D(Dir.X, -Dir.Y), 0.4f, false);
	}
	else
	{
		SetMovementInput3D(FVector2D::ZeroVector, 0.0f, false);
	}
}

void AFociGoalkeeperCharacter::TriggerDiveToward(const FVector& InterceptPoint3D)
{
	ActiveDiveTarget3D = InterceptPoint3D;
	bIsDiving = true;
	DiveCooldownSeconds = 0.85f;
	LaunchCharacter((InterceptPoint3D - GetActorLocation()).GetSafeNormal() * DiveSpeedCmPerSec, true, true);
}

bool AFociGoalkeeperCharacter::ComputeGoalkeeperIntercept3D(
	const FVector& BallLocation,
	const FVector& BallVelocity,
	float GoalLineX,
	float GoalHalfWidthCm,
	float CrossbarHeightCm,
	FVector& OutInterceptPoint3D,
	float& OutTimeToGoalLine)
{
	FociSim::Vec3 OutPt;
	const bool bHit = FociSim::ComputeGoalkeeperIntercept3D(
		FociSim::Vec3(BallLocation.X, BallLocation.Y, BallLocation.Z),
		FociSim::Vec3(BallVelocity.X, BallVelocity.Y, BallVelocity.Z),
		GoalLineX,
		GoalHalfWidthCm,
		CrossbarHeightCm,
		OutPt,
		OutTimeToGoalLine
	);
	OutInterceptPoint3D = FVector(OutPt.X, OutPt.Y, OutPt.Z);
	return bHit;
}

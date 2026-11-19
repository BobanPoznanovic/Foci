#include "AI/FociAIController.h"
#include "AI/FociGoalkeeperCharacter.h"
#include "Gameplay/FociFootballerCharacter.h"
#include "Gameplay/FociBallActor.h"
#include "Simulation/FociAISim.h"
#include "EngineUtils.h"

AFociAIController::AFociAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

FVector AFociAIController::ComputeDynamicFormationTarget3D(
	const FVector& BaseFormationAnchor,
	const FVector& BallLocation,
	EFociTeamSide MyTeam,
	bool bMyTeamHasPossession,
	float HalfPitchLengthCm)
{
	const FociSim::Vec3 Target = FociSim::ComputeDynamicFormationTarget3D(
		FociSim::Vec3(BaseFormationAnchor.X, BaseFormationAnchor.Y, BaseFormationAnchor.Z),
		FociSim::Vec3(BallLocation.X, BallLocation.Y, BallLocation.Z),
		static_cast<FociSim::TeamSide>(static_cast<uint8>(MyTeam)),
		bMyTeamHasPossession,
		HalfPitchLengthCm
	);
	return FVector(Target.X, Target.Y, Target.Z);
}

void AFociAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TackleCooldown = FMath::Max(0.0f, TackleCooldown - DeltaTime);

	AFociFootballerCharacter* Controlled = Cast<AFociFootballerCharacter>(GetPawn());
	if (!CachedBall.IsValid() && GetWorld())
	{
		for (TActorIterator<AFociBallActor> It(GetWorld()); It; ++It)
		{
			CachedBall = *It;
			break;
		}
	}

	if (Controlled == nullptr || !CachedBall.IsValid() || Controlled->ControllingHumanPlayerIndex >= 0)
	{
		return;
	}

	if (AFociGoalkeeperCharacter* GK = Cast<AFociGoalkeeperCharacter>(Controlled))
	{
		GK->UpdateGoalkeeperAI(CachedBall.Get(), DeltaTime);
		return;
	}

	auto SteerTo = [&](const FVector& Target, float SprintFactor)
	{
		const FVector ToTarget = Target - Controlled->GetActorLocation();
		if (ToTarget.Size2D() > 75.0f)
		{
			const FVector Dir = ToTarget.GetSafeNormal2D();
			Controlled->SetMovementInput3D(FVector2D(Dir.X, -Dir.Y), SprintFactor, false);
		}
		else
		{
			Controlled->SetMovementInput3D(FVector2D::ZeroVector, 0.0f, false);
		}
	};

	const FVector BallLoc = CachedBall->GetActorLocation();

	// 1. AI holds ball
	if (CachedBall->PossessingFootballer.Get() == Controlled)
	{
		const FVector OpponentGoal = (Controlled->TeamSide == EFociTeamSide::Home)
			? FVector(5250.0f, 0.0f, 110.0f)
			: FVector(-5250.0f, 0.0f, 110.0f);

		if (FVector::Dist2D(Controlled->GetActorLocation(), OpponentGoal) <= 2400.0f)
		{
			const FVector Aim = (OpponentGoal - Controlled->GetActorLocation()).GetSafeNormal2D();
			Controlled->ExecuteShootAction(CachedBall.Get(), FVector2D(Aim.X, -Aim.Y), 0.75f, EFociAssistLevel::Assisted);
		}
		else
		{
			SteerTo(OpponentGoal, 0.65f);
		}
		return;
	}

	// 2. AI teammate holds ball
	if (CachedBall->PossessingFootballer.IsValid() && CachedBall->LastTouchTeam == Controlled->TeamSide)
	{
		SteerTo(ComputeDynamicFormationTarget3D(Controlled->FormationHomeLocation, BallLoc, Controlled->TeamSide, true), 0.55f);
		return;
	}

	// 3. Opponent holds ball or loose ball
	const float MyDist = FVector::Dist2D(Controlled->GetActorLocation(), BallLoc);
	bool bIsClosest = (MyDist <= 1600.0f);
	if (bIsClosest && GetWorld())
	{
		for (TActorIterator<AFociFootballerCharacter> It(GetWorld()); It; ++It)
		{
			AFociFootballerCharacter* Other = *It;
			if (Other && Other != Controlled && Other->TeamSide == Controlled->TeamSide && !Other->bIsGoalkeeper && Other->ControllingHumanPlayerIndex < 0)
			{
				if (FVector::Dist2D(Other->GetActorLocation(), BallLoc) < MyDist)
				{
					bIsClosest = false;
					break;
				}
			}
		}
	}

	if (bIsClosest)
	{
		SteerTo(BallLoc, 0.85f);
		if (MyDist <= 150.0f && TackleCooldown <= 0.0f)
		{
			Controlled->ExecuteTackleAction(CachedBall.Get(), false);
			TackleCooldown = 1.2f;
		}
	}
	else
	{
		SteerTo(ComputeDynamicFormationTarget3D(Controlled->FormationHomeLocation, BallLoc, Controlled->TeamSide, false), 0.45f);
	}
}

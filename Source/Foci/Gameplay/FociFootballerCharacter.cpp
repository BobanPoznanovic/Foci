#include "Gameplay/FociFootballerCharacter.h"
#include "Gameplay/FociBallActor.h"
#include "Simulation/FociKickSolverSim.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"

AFociFootballerCharacter::AFociFootballerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
	bUseControllerRotationYaw = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->RotationRate = FRotator(0.0f, 640.0f, 0.0f);
		MoveComp->MaxWalkSpeed = 420.0f;
	}
}

void AFociFootballerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsSprinting && GetVelocity().Size2D() > 100.0f)
	{
		const float Drain = 0.08f * (1.3f - Stats.Physical / 150.0f) * DeltaTime;
		CurrentStamina = FMath::Clamp(CurrentStamina - Drain, 0.0f, 1.0f);
	}
	else
	{
		CurrentStamina = FMath::Clamp(CurrentStamina + 0.05f * DeltaTime, 0.0f, 1.0f);
	}
}

FVector AFociFootballerCharacter::GetDribbleSocketWorldLocation() const
{
	FVector Pos = GetActorLocation() + GetActorForwardVector() * 55.0f;
	Pos.Z = 11.0f;
	return Pos;
}

float AFociFootballerCharacter::GetMaxSprintSpeed() const
{
	return FMath::Lerp(520.0f, 780.0f, FMath::Clamp((Stats.Pace - 1.0f) / 98.0f, 0.0f, 1.0f));
}

void AFociFootballerCharacter::SetMovementInput3D(const FVector2D& StickInput, float AnalogSprintFactor, bool bJockeyHeld)
{
	bIsJockeying = bJockeyHeld;
	const float EffectiveSprint = (CurrentStamina > 0.05f && !bJockeyHeld) ? FMath::Clamp(AnalogSprintFactor, 0.0f, 1.0f) : 0.0f;
	bIsSprinting = (EffectiveSprint > 0.25f);

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = bJockeyHeld ? 320.0f : FMath::Lerp(420.0f, GetMaxSprintSpeed(), EffectiveSprint);
	}

	const FVector MoveDir(StickInput.X, -StickInput.Y, 0.0f);
	if (MoveDir.SizeSquared() > 0.01f)
	{
		AddMovementInput(MoveDir.GetSafeNormal(), FMath::Clamp(MoveDir.Size(), 0.0f, 1.0f));
	}
}

void AFociFootballerCharacter::ExecutePassAction(AFociBallActor* Ball, const FVector2D& AimInput, float PowerRatio, bool bThroughBall, bool bLobCross, EFociAssistLevel Assist)
{
	if (Ball == nullptr)
	{
		return;
	}

	const FVector RawAim = (AimInput.SizeSquared() > 0.04f)
		? FVector(AimInput.X, -AimInput.Y, 0.0f).GetSafeNormal()
		: GetActorForwardVector().GetSafeNormal2D();

	AFociFootballerCharacter* BestTeammate = nullptr;
	float BestScore = -FLT_MAX;
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AFociFootballerCharacter> It(World); It; ++It)
		{
			AFociFootballerCharacter* Other = *It;
			if (Other == nullptr || Other == this || Other->TeamSide != TeamSide)
			{
				continue;
			}
			const FVector ToOther = Other->GetActorLocation() - GetActorLocation();
			const float Dist = ToOther.Size2D();
			if (Dist > 4500.0f || Dist < 1.0f)
			{
				continue;
			}
			const float Dot = FVector::DotProduct(RawAim, ToOther.GetSafeNormal2D());
			if (Dot > 0.25f)
			{
				const float Score = Dot * 1500.0f - Dist * 0.25f;
				if (Score > BestScore)
				{
					BestScore = Score;
					BestTeammate = Other;
				}
			}
		}
	}

	const FVector IdealTarget = BestTeammate
		? (BestTeammate->GetActorLocation() + (bThroughBall ? BestTeammate->GetVelocity() * 0.65f : FVector::ZeroVector))
		: (GetActorLocation() + RawAim * 1500.0f);

	FVector LaunchVel = FVector::ZeroVector;
	FVector SpinVec = FVector::ZeroVector;
	ComputeKickVelocityAndSpin3D(
		Ball->GetActorLocation(),
		RawAim,
		PowerRatio,
		false,
		bLobCross,
		bThroughBall,
		bIsFinesseModifierHeld,
		bIsChipModifierHeld,
		Assist,
		IdealTarget,
		LaunchVel,
		SpinVec
	);

	Ball->KickBall(LaunchVel, SpinVec, this, TeamSide);
}

void AFociFootballerCharacter::ExecuteShootAction(AFociBallActor* Ball, const FVector2D& AimInput, float PowerRatio, EFociAssistLevel ShotAssist)
{
	if (Ball == nullptr)
	{
		return;
	}

	const FVector RawAim = (AimInput.SizeSquared() > 0.04f)
		? FVector(AimInput.X, -AimInput.Y, 0.0f).GetSafeNormal()
		: GetActorForwardVector().GetSafeNormal2D();

	const float GoalX = (TeamSide == EFociTeamSide::Home) ? 5250.0f : -5250.0f;
	const FVector IdealGoalTarget(GoalX, FMath::Clamp(-AimInput.Y * 300.0f, -310.0f, 310.0f), 110.0f);

	FVector LaunchVel = FVector::ZeroVector;
	FVector SpinVec = FVector::ZeroVector;
	ComputeKickVelocityAndSpin3D(
		Ball->GetActorLocation(),
		RawAim,
		PowerRatio,
		true,
		false,
		false,
		bIsFinesseModifierHeld,
		bIsChipModifierHeld,
		ShotAssist,
		IdealGoalTarget,
		LaunchVel,
		SpinVec
	);

	Ball->KickBall(LaunchVel, SpinVec, this, TeamSide);
}

bool AFociFootballerCharacter::ExecuteTackleAction(AFociBallActor* Ball, bool bSlideTackle)
{
	if (Ball == nullptr)
	{
		return false;
	}

	if (bSlideTackle)
	{
		LaunchCharacter(GetActorForwardVector().GetSafeNormal2D() * 550.0f, true, false);
	}

	const float ReachCm = bSlideTackle ? 260.0f : 165.0f;
	const FVector ToBall = Ball->GetActorLocation() - GetActorLocation();
	const float Dist2D = ToBall.Size2D();
	const float Dot = FVector::DotProduct(GetActorForwardVector().GetSafeNormal2D(), ToBall.GetSafeNormal2D());

	if (Dist2D <= ReachCm && Dot >= 0.35f && Ball->PossessingFootballer.Get() != this)
	{
		if (bSlideTackle)
		{
			Ball->KickBall(GetActorForwardVector().GetSafeNormal2D() * 650.0f + FVector(0.0f, 0.0f, 80.0f), FVector::ZeroVector, this, TeamSide);
		}
		else
		{
			Ball->SetPossessingFootballer(this, TeamSide);
		}
		return true;
	}

	return false;
}

void AFociFootballerCharacter::ComputeKickVelocityAndSpin3D(
	const FVector& StartLocation,
	const FVector& RawAimDirection2D,
	float PowerRatio,
	bool bIsShot,
	bool bIsLobOrCross,
	bool bIsThroughBall,
	bool bIsFinesse,
	bool bIsChip,
	EFociAssistLevel AssistLevel,
	const FVector& IdealAssistedTargetLocation,
	FVector& OutLaunchVelocity,
	FVector& OutSpinVectorRadPerSec)
{
	FociSim::Vec3 OutVel;
	FociSim::Vec3 OutSpin;
	FociSim::ComputeKickVelocityAndSpin3D(
		FociSim::Vec3(StartLocation.X, StartLocation.Y, StartLocation.Z),
		FociSim::Vec3(RawAimDirection2D.X, RawAimDirection2D.Y, RawAimDirection2D.Z),
		PowerRatio,
		bIsShot,
		bIsLobOrCross,
		bIsThroughBall,
		bIsFinesse,
		bIsChip,
		static_cast<FociSim::AssistLevel>(static_cast<uint8>(AssistLevel)),
		FociSim::Vec3(IdealAssistedTargetLocation.X, IdealAssistedTargetLocation.Y, IdealAssistedTargetLocation.Z),
		OutVel,
		OutSpin
	);

	OutLaunchVelocity = FVector(OutVel.X, OutVel.Y, OutVel.Z);
	OutSpinVectorRadPerSec = FVector(OutSpin.X, OutSpin.Y, OutSpin.Z);
}

#include "Gameplay/FociPitchVolume.h"
#include "Simulation/FociPitchSim.h"
#include "Components/BoxComponent.h"

AFociPitchVolume::AFociPitchVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	PitchBoundsVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("PitchBoundsVolume"));
	PitchBoundsVolume->SetBoxExtent(FVector(HalfPitchLengthCm, HalfPitchWidthCm, 1500.0f));
	PitchBoundsVolume->SetCollisionProfileName(TEXT("NoCollision"));
	RootComponent = PitchBoundsVolume;
}

FVector AFociPitchVolume::GetKickOffSpot() const
{
	return GetActorLocation() + FVector(0.0f, 0.0f, 11.0f);
}

FVector AFociPitchVolume::ComputeRestartLocation3D(EFociPitchEvent Event, const FVector& LastOutBallLocation) const
{
	const FociSim::Vec3 Restart = FociSim::ComputeRestartLocation3D(
		static_cast<FociSim::PitchEvent>(static_cast<uint8>(Event)),
		FociSim::Vec3(LastOutBallLocation.X, LastOutBallLocation.Y, LastOutBallLocation.Z),
		HalfPitchLengthCm,
		HalfPitchWidthCm,
		11.0f
	);
	return FVector(Restart.X, Restart.Y, Restart.Z);
}

EFociPitchEvent AFociPitchVolume::EvaluateBallBoundary3D(
	const FVector& BallCenterLocation,
	float BallRadiusCm,
	EFociTeamSide LastTouchTeam,
	float HalfLengthCm,
	float HalfWidthCm,
	float GoalHalfWidth,
	float CrossbarHeight)
{
	const FociSim::PitchEvent Event = FociSim::EvaluateBallBoundary3D(
		FociSim::Vec3(BallCenterLocation.X, BallCenterLocation.Y, BallCenterLocation.Z),
		BallRadiusCm,
		static_cast<FociSim::TeamSide>(static_cast<uint8>(LastTouchTeam)),
		HalfLengthCm,
		HalfWidthCm,
		GoalHalfWidth,
		CrossbarHeight
	);
	return static_cast<EFociPitchEvent>(static_cast<uint8>(Event));
}

bool AFociPitchVolume::IsPlayerInOffsidePosition3D(
	float ReceiverX,
	float BallXAtPass,
	EFociTeamSide AttackingTeam,
	const TArray<float>& DefenderXPositions)
{
	std::vector<float> Defenders;
	Defenders.reserve(DefenderXPositions.Num());
	for (float X : DefenderXPositions)
	{
		Defenders.push_back(X);
	}

	return FociSim::IsPlayerInOffsidePosition3D(
		ReceiverX,
		BallXAtPass,
		static_cast<FociSim::TeamSide>(static_cast<uint8>(AttackingTeam)),
		Defenders
	);
}

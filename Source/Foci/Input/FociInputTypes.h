#pragma once

#include "CoreMinimal.h"
#include "FociInputTypes.generated.h"

UENUM(BlueprintType)
enum class EFociPS5Button : uint8
{
	Cross = 0,
	Circle = 1,
	Square = 2,
	Triangle = 3,
	L1 = 4,
	R1 = 5,
	L2 = 6,
	R2 = 7,
	L3 = 8,
	R3 = 9,
	DPadUp = 10,
	DPadDown = 11,
	DPadLeft = 12,
	DPadRight = 13,
	Touchpad = 14,
	Options = 15
};

UENUM(BlueprintType)
enum class EFociControlContext : uint8
{
	OnBall = 0,
	OffBall = 1,
	System = 2
};

UENUM(BlueprintType)
enum class EFociGameplayAction : uint8
{
	GroundPass = 0,
	ThroughBall = 1,
	LobPassCross = 2,
	Shoot = 3,
	Sprint = 4,
	FinesseModifier = 5,
	ChipModifier = 6,
	ShieldBall = 7,

	SwitchPlayer = 8,
	StandingTackle = 9,
	SlidingTackle = 10,
	TeammateContain = 11,
	Jockey = 12,
	GoalkeeperRush = 13,

	TacticalMentalityUp = 14,
	TacticalMentalityDown = 15,
	OffsideTrap = 16,
	PauseMatch = 17
};

UENUM(BlueprintType)
enum class EFociControlPreset : uint8
{
	Classic = 0,
	Alternate = 1,
	TwoButton = 2,
	Custom = 3
};

UENUM(BlueprintType)
enum class EFociAssistLevel : uint8
{
	Assisted = 0,
	Semi = 1,
	Manual = 2
};

UENUM(BlueprintType)
enum class EFociAutoSwitchMode : uint8
{
	AirAndLooseBalls = 0,
	AirBallsOnly = 1,
	ManualOnly = 2
};

UENUM(BlueprintType)
enum class EFociTeamSide : uint8
{
	Neutral = 0,
	Home = 1,
	Away = 2
};

USTRUCT(BlueprintType)
struct FOCI_API FFociPlayerInputProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Input")
	int32 PlayerIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Input")
	FString ProfileName = TEXT("Player 1");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Input")
	EFociTeamSide TeamSide = EFociTeamSide::Home;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Input")
	EFociControlPreset Preset = EFociControlPreset::Classic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Input")
	TMap<EFociGameplayAction, EFociPS5Button> ActionBindings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Input")
	EFociAssistLevel PassAssistance = EFociAssistLevel::Assisted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Input")
	EFociAssistLevel ShotAssistance = EFociAssistLevel::Assisted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Input")
	EFociAssistLevel CrossAssistance = EFociAssistLevel::Assisted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Input")
	EFociAutoSwitchMode AutoSwitchMode = EFociAutoSwitchMode::AirAndLooseBalls;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Input")
	bool bAnalogSprint = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Input")
	bool bEnableVibration = true;
};

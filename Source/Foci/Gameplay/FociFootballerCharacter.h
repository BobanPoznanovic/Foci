#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Input/FociInputTypes.h"
#include "FociFootballerCharacter.generated.h"

class AFociBallActor;

USTRUCT(BlueprintType)
struct FOCI_API FFociFootballerStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Stats")
	float Pace = 82.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Stats")
	float Shooting = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Stats")
	float Passing = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Stats")
	float Dribbling = 82.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Stats")
	float Defending = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Stats")
	float Physical = 78.0f;
};

UCLASS()
class FOCI_API AFociFootballerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFociFootballerCharacter();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Footballer")
	EFociTeamSide TeamSide = EFociTeamSide::Home;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Footballer")
	int32 SquadNumber = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Footballer")
	bool bIsGoalkeeper = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Footballer")
	FVector FormationHomeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Footballer")
	FFociFootballerStats Stats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Footballer")
	float CurrentStamina = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Footballer")
	int32 ControllingHumanPlayerIndex = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Footballer")
	bool bIsSprinting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Footballer")
	bool bIsJockeying = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Footballer")
	bool bIsFinesseModifierHeld = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Footballer")
	bool bIsChipModifierHeld = false;

	UFUNCTION(BlueprintPure, Category = "Foci|Footballer")
	FVector GetDribbleSocketWorldLocation() const;

	UFUNCTION(BlueprintPure, Category = "Foci|Footballer")
	float GetMaxSprintSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "Foci|Footballer")
	void SetMovementInput3D(const FVector2D& StickInput, float AnalogSprintFactor, bool bJockeyHeld);

	UFUNCTION(BlueprintCallable, Category = "Foci|Footballer")
	void ExecutePassAction(AFociBallActor* Ball, const FVector2D& AimInput, float PowerRatio, bool bThroughBall, bool bLobCross, EFociAssistLevel Assist);

	UFUNCTION(BlueprintCallable, Category = "Foci|Footballer")
	void ExecuteShootAction(AFociBallActor* Ball, const FVector2D& AimInput, float PowerRatio, EFociAssistLevel ShotAssist);

	UFUNCTION(BlueprintCallable, Category = "Foci|Footballer")
	bool ExecuteTackleAction(AFociBallActor* Ball, bool bSlideTackle);

	UFUNCTION(BlueprintCallable, Category = "Foci|Footballer")
	static void ComputeKickVelocityAndSpin3D(
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
		FVector& OutSpinVectorRadPerSec
	);
};

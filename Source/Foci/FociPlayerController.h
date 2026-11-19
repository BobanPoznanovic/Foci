#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Input/FociInputTypes.h"
#include "FociPlayerController.generated.h"

class UInputMappingContext;
class AFociFootballerCharacter;
class AFociBallActor;

UCLASS()
class FOCI_API AFociPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AFociPlayerController();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UFUNCTION(BlueprintPure, Category = "Foci|Input")
	int32 GetHumanPlayerIndex() const;

	UFUNCTION(BlueprintCallable, Category = "Foci|Camera")
	void SetSharedBroadcastCamera(AActor* CameraActor);

	UFUNCTION(BlueprintCallable, Category = "Foci|Input")
	void SetControlledFootballer(AFociFootballerCharacter* NewFootballer);

	UFUNCTION(BlueprintCallable, Category = "Foci|Input")
	bool RequestTeammateSwitch(const FVector2D& RightStickFlick);

	UFUNCTION(BlueprintPure, Category = "Foci|Input")
	float GetCurrentActionChargeRatio() const;

	UFUNCTION(BlueprintPure, Category = "Foci|Input")
	static int32 SelectBestTeammateToSwitch3D(
		int32 RequestingHumanPlayerIndex,
		int32 CurrentControlledCandidateIndex,
		const FVector& CurrentFootballerLocation,
		const FVector& BallLocation,
		const FVector2D& RightStickFlickDirection,
		const TArray<FVector>& TeammateLocations,
		const TArray<int32>& TeammateControllingHumanIndices,
		const TArray<bool>& TeammateIsGoalkeeper
	);

protected:
	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;

	TWeakObjectPtr<AActor> SharedBroadcastCamera;
	TWeakObjectPtr<AFociBallActor> CachedBall;
	bool bIsChargingAction = false;
	EFociGameplayAction ChargingAction = EFociGameplayAction::GroundPass;
	float CurrentActionChargeTime = 0.0f;
	float RightStickSwitchCooldown = 0.0f;
};

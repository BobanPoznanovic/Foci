#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FociBroadcastCameraActor.generated.h"

class USceneComponent;
class USpringArmComponent;
class UCameraComponent;
class AFociBallActor;

UCLASS()
class FOCI_API AFociBroadcastCameraActor : public AActor
{
	GENERATED_BODY()

public:
	AFociBroadcastCameraActor();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foci|Camera")
	TObjectPtr<USceneComponent> FocusRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foci|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foci|Camera")
	TObjectPtr<UCameraComponent> BroadcastCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Camera")
	float MinArmLengthCm = 2400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Camera")
	float MaxArmLengthCm = 4600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Camera")
	float CameraLagSpeed = 5.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Camera")
	float BallFocusWeight = 0.60f;

	TWeakObjectPtr<AFociBallActor> TrackedBall;

	UFUNCTION(BlueprintCallable, Category = "Foci|Camera")
	void SetTrackedBall(AFociBallActor* InBall);

	UFUNCTION(BlueprintPure, Category = "Foci|Camera")
	static void ComputeBroadcastCameraFrame3D(
		const FVector& BallLocation,
		const TArray<FVector>& HumanPlayerLocations,
		float BallWeight,
		float MinArmLength,
		float MaxArmLength,
		FVector& OutTargetFocusLocation,
		float& OutTargetArmLength
	);
};

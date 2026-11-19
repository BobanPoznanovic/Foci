#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Input/FociInputTypes.h"
#include "FociBallActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class ACharacter;

UCLASS()
class FOCI_API AFociBallActor : public AActor
{
	GENERATED_BODY()

public:
	AFociBallActor();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foci|Ball")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foci|Ball")
	TObjectPtr<UStaticMeshComponent> BallMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Ball|Physics")
	float BallRadiusCm = 11.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Ball|Physics")
	float BallMassKg = 0.43f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Ball|Physics")
	float TurfRestitution = 0.68f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Ball|Physics")
	float TurfRollingFriction = 320.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Ball|Physics")
	float AirDragCoefficient = 0.00022f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Ball|Physics")
	float MagnusCoefficient = 0.010f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Ball|Physics")
	float SpinDecayRate = 1.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Ball|Physics")
	float GroundZ = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Foci|Ball|State")
	FVector LinearVelocity = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Foci|Ball|State")
	FVector AngularVelocity = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Foci|Ball|State")
	EFociTeamSide LastTouchTeam = EFociTeamSide::Neutral;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Foci|Ball|State")
	float PossessionLockoutTimer = 0.0f;

	TWeakObjectPtr<ACharacter> PossessingFootballer;
	TWeakObjectPtr<ACharacter> LastTouchFootballer;

	UFUNCTION(BlueprintCallable, Category = "Foci|Ball")
	void KickBall(const FVector& NewLinearVelocity, const FVector& NewAngularVelocity, ACharacter* Kicker, EFociTeamSide KickerTeam);

	UFUNCTION(BlueprintCallable, Category = "Foci|Ball")
	void SetPossessingFootballer(ACharacter* NewOwner, EFociTeamSide OwnerTeam);

	UFUNCTION(BlueprintCallable, Category = "Foci|Ball")
	void ReleasePossession();

	UFUNCTION(BlueprintCallable, Category = "Foci|Ball")
	void ResetBallToLocation(const FVector& NewWorldLocation);

	UFUNCTION(BlueprintPure, Category = "Foci|Ball")
	bool CanBeCapturedBy(const ACharacter* Candidate) const;

	UFUNCTION(BlueprintPure, Category = "Foci|Ball")
	FVector PredictBallPositionAtTime(float FutureSeconds) const;

	UFUNCTION(BlueprintCallable, Category = "Foci|Ball|Physics")
	static void StepBallPhysics3D(
		FVector& InOutPosition,
		FVector& InOutVelocity,
		FVector& InOutAngularVelocity,
		float DeltaTime,
		float InGroundZ,
		float RadiusCm,
		float Restitution,
		float RollingFriction,
		float DragCoeff,
		float MagnusCoeff,
		float SpinDecay
	);
};

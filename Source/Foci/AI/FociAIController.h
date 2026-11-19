#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Input/FociInputTypes.h"
#include "FociAIController.generated.h"

class AFociBallActor;

UCLASS()
class FOCI_API AFociAIController : public AAIController
{
	GENERATED_BODY()

public:
	AFociAIController();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure, Category = "Foci|AI")
	static FVector ComputeDynamicFormationTarget3D(
		const FVector& BaseFormationAnchor,
		const FVector& BallLocation,
		EFociTeamSide MyTeam,
		bool bMyTeamHasPossession,
		float HalfPitchLengthCm = 5250.0f
	);

private:
	TWeakObjectPtr<AFociBallActor> CachedBall;
	float TackleCooldown = 0.0f;
};

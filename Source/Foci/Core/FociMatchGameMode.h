#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Input/FociInputTypes.h"
#include "Core/FociMatchGameState.h"
#include "FociMatchGameMode.generated.h"

class AFociBallActor;
class AFociPitchVolume;
class AFociBroadcastCameraActor;
class AFociFootballerCharacter;
class AFociGoalkeeperCharacter;

UCLASS()
class FOCI_API AFociMatchGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFociMatchGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Match")
	bool bFiveASideMode = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Match")
	int32 NumLocalHumanPlayers = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Match")
	float GoalCelebrationDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Match")
	float HalfTimeDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Match")
	float KickOffDelaySeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Match")
	TSubclassOf<AFociFootballerCharacter> FootballerClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Match")
	TSubclassOf<AFociGoalkeeperCharacter> GoalkeeperClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foci|Match")
	TObjectPtr<AFociBallActor> MatchBall;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foci|Match")
	TObjectPtr<AFociPitchVolume> PitchVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foci|Match")
	TObjectPtr<AFociBroadcastCameraActor> BroadcastCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foci|Match")
	TArray<TObjectPtr<AFociFootballerCharacter>> HomeSquad;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foci|Match")
	TArray<TObjectPtr<AFociFootballerCharacter>> AwaySquad;

	UFUNCTION(BlueprintCallable, Category = "Foci|Match")
	void EnsureFourLocalHumanPlayers(UWorld* World);

	UFUNCTION(BlueprintCallable, Category = "Foci|Match")
	void SpawnSquadsAndAssignHumanPlayers();

	UFUNCTION(BlueprintCallable, Category = "Foci|Match")
	void ResetSquadsForKickOff();

	UFUNCTION(BlueprintCallable, Category = "Foci|Match")
	bool CheckAndApplyOffsideOnPass(AFociFootballerCharacter* Passer, AFociFootballerCharacter* Receiver);

private:
	void ResolveLooseBallCapture();

	float PhaseStateTimer = 0.0f;
};

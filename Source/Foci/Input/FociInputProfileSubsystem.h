#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InputCoreTypes.h"
#include "Input/FociInputTypes.h"
#include "FociInputProfileSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFociOnPlayerProfileChanged, int32, PlayerIndex, const FFociPlayerInputProfile&, UpdatedProfile);

UCLASS()
class FOCI_API UFociInputProfileSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable, Category = "Foci|Input")
	FFociOnPlayerProfileChanged OnPlayerProfileChanged;

	UFUNCTION(BlueprintCallable, Category = "Foci|Input")
	void EnsureDefaultProfilesInitialized();

	UFUNCTION(BlueprintCallable, Category = "Foci|Input")
	bool LoadProfilesFromDisk();

	UFUNCTION(BlueprintCallable, Category = "Foci|Input")
	bool SaveProfilesToDisk();

	UFUNCTION(BlueprintPure, Category = "Foci|Input")
	FFociPlayerInputProfile GetPlayerProfile(int32 PlayerIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Foci|Input")
	void ApplyControlPreset(int32 PlayerIndex, EFociControlPreset Preset);

	UFUNCTION(BlueprintCallable, Category = "Foci|Input")
	void RemapPlayerAction(int32 PlayerIndex, EFociGameplayAction Action, EFociPS5Button NewButton, bool bSwapConflictingInSameContext = true);

	UFUNCTION(BlueprintCallable, Category = "Foci|Input")
	void SetPlayerTeamSide(int32 PlayerIndex, EFociTeamSide NewTeamSide);

	UFUNCTION(BlueprintCallable, Category = "Foci|Input")
	void SetPlayerAssistSettings(int32 PlayerIndex, EFociAssistLevel PassAssist, EFociAssistLevel ShotAssist, EFociAssistLevel CrossAssist, EFociAutoSwitchMode AutoSwitchMode);

	UFUNCTION(BlueprintPure, Category = "Foci|Input")
	EFociPS5Button GetBoundButtonForAction(int32 PlayerIndex, EFociGameplayAction Action) const;

	UFUNCTION(BlueprintPure, Category = "Foci|Input")
	bool FindActionForButtonInContext(int32 PlayerIndex, EFociPS5Button Button, EFociControlContext Context, EFociGameplayAction& OutAction) const;

	UFUNCTION(BlueprintPure, Category = "Foci|Input")
	static EFociControlContext GetActionContext(EFociGameplayAction Action);

	UFUNCTION(BlueprintPure, Category = "Foci|Input")
	static bool AreActionsInSameContext(EFociGameplayAction ActionA, EFociGameplayAction ActionB);

	UFUNCTION(BlueprintPure, Category = "Foci|Input")
	static TMap<EFociGameplayAction, EFociPS5Button> BuildPresetBindings(EFociControlPreset Preset);

	UFUNCTION(BlueprintPure, Category = "Foci|Input")
	static FKey ConvertPS5ButtonToUnrealKey(EFociPS5Button Button);

	UFUNCTION(BlueprintPure, Category = "Foci|Input")
	static EFociPS5Button ConvertUnrealKeyToPS5Button(const FKey& Key, bool& bOutIsValid);

private:
	UPROPERTY()
	TArray<FFociPlayerInputProfile> PlayerProfiles;
};

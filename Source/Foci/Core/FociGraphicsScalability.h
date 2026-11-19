#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FociGraphicsScalability.generated.h"

UENUM(BlueprintType)
enum class EFociGraphicsPreset : uint8
{
	MacBook2018_IrisPerformance = 0,
	MacBook2018_RadeonBalanced = 1,
	PhotorealBroadcastReplay = 2
};

USTRUCT(BlueprintType)
struct FOCI_API FFociGraphicsConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Graphics")
	int32 TargetFPS = 60;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Graphics")
	float ScreenPercentage = 85.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Graphics")
	int32 MSAACount = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Graphics")
	int32 CSMMaxCascades = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Graphics")
	int32 MaxCSMResolution = 1024;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Graphics")
	bool bEnableCapsuleShadows = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Graphics")
	int32 SSRQuality = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foci|Graphics")
	float GrassCullDistanceMultiplier = 1.0f;
};

UCLASS()
class FOCI_API UFociGraphicsScalability : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Foci|Graphics")
	static FFociGraphicsConfig GetConfigForPreset(EFociGraphicsPreset Preset);

	UFUNCTION(BlueprintCallable, Category = "Foci|Graphics", meta = (WorldContext = "WorldContextObject"))
	static void ApplyGraphicsPreset(const UObject* WorldContextObject, EFociGraphicsPreset Preset);
};

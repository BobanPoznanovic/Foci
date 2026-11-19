#include "Core/FociGraphicsScalability.h"
#include "Simulation/FociScalabilitySim.h"
#include "HAL/IConsoleManager.h"

FFociGraphicsConfig UFociGraphicsScalability::GetConfigForPreset(EFociGraphicsPreset Preset)
{
	const FociSim::GraphicsConfig SimConfig = FociSim::GetGraphicsConfigForPreset(
		static_cast<FociSim::GraphicsPreset>(static_cast<uint8>(Preset))
	);

	FFociGraphicsConfig OutConfig;
	OutConfig.TargetFPS = SimConfig.TargetFPS;
	OutConfig.ScreenPercentage = SimConfig.ScreenPercentage;
	OutConfig.MSAACount = SimConfig.MSAACount;
	OutConfig.CSMMaxCascades = SimConfig.CSMMaxCascades;
	OutConfig.MaxCSMResolution = SimConfig.MaxCSMResolution;
	OutConfig.bEnableCapsuleShadows = SimConfig.bEnableCapsuleShadows;
	OutConfig.SSRQuality = SimConfig.SSRQuality;
	OutConfig.GrassCullDistanceMultiplier = SimConfig.GrassCullDistanceMultiplier;
	return OutConfig;
}

void UFociGraphicsScalability::ApplyGraphicsPreset(const UObject* WorldContextObject, EFociGraphicsPreset Preset)
{
	(void)WorldContextObject;

	const FFociGraphicsConfig Config = GetConfigForPreset(Preset);

	auto SetIntCVar = [](const TCHAR* Name, int32 Value)
	{
		if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(Name))
		{
			CVar->Set(Value, ECVF_SetByCode);
		}
	};

	auto SetFloatCVar = [](const TCHAR* Name, float Value)
	{
		if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(Name))
		{
			CVar->Set(Value, ECVF_SetByCode);
		}
	};

	const int32 CapsuleValue = Config.bEnableCapsuleShadows ? 1 : 0;

	SetFloatCVar(TEXT("t.MaxFPS"), static_cast<float>(Config.TargetFPS));
	SetFloatCVar(TEXT("r.ScreenPercentage"), Config.ScreenPercentage);
	SetIntCVar(TEXT("r.AntiAliasingMethod"), 3);
	SetIntCVar(TEXT("r.MSAACount"), Config.MSAACount);
	SetIntCVar(TEXT("r.Shadow.CSM.MaxCascades"), Config.CSMMaxCascades);
	SetIntCVar(TEXT("r.Shadow.MaxCSMResolution"), Config.MaxCSMResolution);
	SetIntCVar(TEXT("r.CapsuleShadows"), CapsuleValue);
	SetIntCVar(TEXT("r.CapsuleDirectShadows"), CapsuleValue);
	SetIntCVar(TEXT("r.CapsuleIndirectShadows"), CapsuleValue);
	SetIntCVar(TEXT("r.SSR.Quality"), Config.SSRQuality);
	SetFloatCVar(TEXT("foliage.CullDistanceScale"), Config.GrassCullDistanceMultiplier);
	SetIntCVar(TEXT("r.DynamicGlobalIlluminationMethod"), 0);
	SetIntCVar(TEXT("r.Shadow.Virtual.Enable"), 0);
}

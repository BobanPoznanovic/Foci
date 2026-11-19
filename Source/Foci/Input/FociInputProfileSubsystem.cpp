#include "Input/FociInputProfileSubsystem.h"
#include "Input/FociControlsSaveGame.h"
#include "Simulation/FociInputSim.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	FociSim::PlayerInputProfile ToSimProfile(const FFociPlayerInputProfile& UEProfile)
	{
		FociSim::PlayerInputProfile Sim;
		Sim.PlayerIndex = UEProfile.PlayerIndex;
		Sim.ProfileName = TCHAR_TO_UTF8(*UEProfile.ProfileName);
		Sim.PlayerTeamSide = static_cast<FociSim::TeamSide>(static_cast<uint8>(UEProfile.TeamSide));
		Sim.Preset = static_cast<FociSim::ControlPreset>(static_cast<uint8>(UEProfile.Preset));
		for (const auto& Pair : UEProfile.ActionBindings)
		{
			Sim.ActionBindings[static_cast<FociSim::GameplayAction>(static_cast<uint8>(Pair.Key))] =
				static_cast<FociSim::PS5Button>(static_cast<uint8>(Pair.Value));
		}
		Sim.PassAssistance = static_cast<FociSim::AssistLevel>(static_cast<uint8>(UEProfile.PassAssistance));
		Sim.ShotAssistance = static_cast<FociSim::AssistLevel>(static_cast<uint8>(UEProfile.ShotAssistance));
		Sim.CrossAssistance = static_cast<FociSim::AssistLevel>(static_cast<uint8>(UEProfile.CrossAssistance));
		Sim.AutoSwitch = static_cast<FociSim::AutoSwitchMode>(static_cast<uint8>(UEProfile.AutoSwitchMode));
		Sim.bAnalogSprint = UEProfile.bAnalogSprint;
		Sim.bEnableVibration = UEProfile.bEnableVibration;
		return Sim;
	}

	FFociPlayerInputProfile FromSimProfile(const FociSim::PlayerInputProfile& Sim)
	{
		FFociPlayerInputProfile UEProfile;
		UEProfile.PlayerIndex = Sim.PlayerIndex;
		UEProfile.ProfileName = UTF8_TO_TCHAR(Sim.ProfileName.c_str());
		UEProfile.TeamSide = static_cast<EFociTeamSide>(static_cast<uint8>(Sim.PlayerTeamSide));
		UEProfile.Preset = static_cast<EFociControlPreset>(static_cast<uint8>(Sim.Preset));
		for (const auto& Pair : Sim.ActionBindings)
		{
			UEProfile.ActionBindings.Add(
				static_cast<EFociGameplayAction>(static_cast<uint8>(Pair.first)),
				static_cast<EFociPS5Button>(static_cast<uint8>(Pair.second))
			);
		}
		UEProfile.PassAssistance = static_cast<EFociAssistLevel>(static_cast<uint8>(Sim.PassAssistance));
		UEProfile.ShotAssistance = static_cast<EFociAssistLevel>(static_cast<uint8>(Sim.ShotAssistance));
		UEProfile.CrossAssistance = static_cast<EFociAssistLevel>(static_cast<uint8>(Sim.CrossAssistance));
		UEProfile.AutoSwitchMode = static_cast<EFociAutoSwitchMode>(static_cast<uint8>(Sim.AutoSwitch));
		UEProfile.bAnalogSprint = Sim.bAnalogSprint;
		UEProfile.bEnableVibration = Sim.bEnableVibration;
		return UEProfile;
	}
}

void UFociInputProfileSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	EnsureDefaultProfilesInitialized();
	LoadProfilesFromDisk();
}

void UFociInputProfileSubsystem::Deinitialize()
{
	SaveProfilesToDisk();
	Super::Deinitialize();
}

void UFociInputProfileSubsystem::EnsureDefaultProfilesInitialized()
{
	if (PlayerProfiles.Num() == UFociControlsSaveGame::MaxHumanPlayers)
	{
		return;
	}

	FociSim::InputProfileManager SimManager;
	PlayerProfiles.SetNum(UFociControlsSaveGame::MaxHumanPlayers);
	for (int32 i = 0; i < UFociControlsSaveGame::MaxHumanPlayers; ++i)
	{
		PlayerProfiles[i] = FromSimProfile(SimManager.GetPlayerProfile(i));
	}
}

bool UFociInputProfileSubsystem::LoadProfilesFromDisk()
{
	EnsureDefaultProfilesInitialized();

	const FString SlotName = TEXT("FociControlsSaveSlot");
	const int32 UserIdx = 0;
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, UserIdx))
	{
		return false;
	}

	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SlotName, UserIdx);
	UFociControlsSaveGame* FociSave = Cast<UFociControlsSaveGame>(Loaded);
	if (!FociSave)
	{
		return false;
	}

	for (int32 i = 0; i < FMath::Min(FociSave->PlayerProfiles.Num(), UFociControlsSaveGame::MaxHumanPlayers); ++i)
	{
		PlayerProfiles[i] = FociSave->PlayerProfiles[i];
		PlayerProfiles[i].PlayerIndex = i;
	}
	return true;
}

bool UFociInputProfileSubsystem::SaveProfilesToDisk()
{
	EnsureDefaultProfilesInitialized();

	USaveGame* NewSave = UGameplayStatics::CreateSaveGameObject(UFociControlsSaveGame::StaticClass());
	UFociControlsSaveGame* FociSave = Cast<UFociControlsSaveGame>(NewSave);
	if (!FociSave)
	{
		return false;
	}

	FociSave->PlayerProfiles = PlayerProfiles;
	return UGameplayStatics::SaveGameToSlot(FociSave, FociSave->SaveSlotName, FociSave->UserIndex);
}

FFociPlayerInputProfile UFociInputProfileSubsystem::GetPlayerProfile(int32 PlayerIndex) const
{
	if (PlayerProfiles.IsValidIndex(PlayerIndex))
	{
		return PlayerProfiles[PlayerIndex];
	}
	return FFociPlayerInputProfile();
}

void UFociInputProfileSubsystem::ApplyControlPreset(int32 PlayerIndex, EFociControlPreset Preset)
{
	EnsureDefaultProfilesInitialized();
	if (!PlayerProfiles.IsValidIndex(PlayerIndex))
	{
		return;
	}

	FociSim::InputProfileManager SimManager;
	SimManager.Profiles[static_cast<size_t>(PlayerIndex)] = ToSimProfile(PlayerProfiles[PlayerIndex]);
	SimManager.ApplyControlPreset(PlayerIndex, static_cast<FociSim::ControlPreset>(static_cast<uint8>(Preset)));
	PlayerProfiles[PlayerIndex] = FromSimProfile(SimManager.GetPlayerProfile(PlayerIndex));

	OnPlayerProfileChanged.Broadcast(PlayerIndex, PlayerProfiles[PlayerIndex]);
}

void UFociInputProfileSubsystem::RemapPlayerAction(int32 PlayerIndex, EFociGameplayAction Action, EFociPS5Button NewButton, bool bSwapConflictingInSameContext)
{
	EnsureDefaultProfilesInitialized();
	if (!PlayerProfiles.IsValidIndex(PlayerIndex))
	{
		return;
	}

	FociSim::InputProfileManager SimManager;
	SimManager.Profiles[static_cast<size_t>(PlayerIndex)] = ToSimProfile(PlayerProfiles[PlayerIndex]);
	SimManager.RemapPlayerAction(
		PlayerIndex,
		static_cast<FociSim::GameplayAction>(static_cast<uint8>(Action)),
		static_cast<FociSim::PS5Button>(static_cast<uint8>(NewButton)),
		bSwapConflictingInSameContext
	);
	PlayerProfiles[PlayerIndex] = FromSimProfile(SimManager.GetPlayerProfile(PlayerIndex));

	OnPlayerProfileChanged.Broadcast(PlayerIndex, PlayerProfiles[PlayerIndex]);
}

void UFociInputProfileSubsystem::SetPlayerTeamSide(int32 PlayerIndex, EFociTeamSide NewTeamSide)
{
	EnsureDefaultProfilesInitialized();
	if (!PlayerProfiles.IsValidIndex(PlayerIndex))
	{
		return;
	}

	PlayerProfiles[PlayerIndex].TeamSide = NewTeamSide;
	OnPlayerProfileChanged.Broadcast(PlayerIndex, PlayerProfiles[PlayerIndex]);
}

void UFociInputProfileSubsystem::SetPlayerAssistSettings(int32 PlayerIndex, EFociAssistLevel PassAssist, EFociAssistLevel ShotAssist, EFociAssistLevel CrossAssist, EFociAutoSwitchMode AutoSwitchMode)
{
	EnsureDefaultProfilesInitialized();
	if (!PlayerProfiles.IsValidIndex(PlayerIndex))
	{
		return;
	}

	PlayerProfiles[PlayerIndex].PassAssistance = PassAssist;
	PlayerProfiles[PlayerIndex].ShotAssistance = ShotAssist;
	PlayerProfiles[PlayerIndex].CrossAssistance = CrossAssist;
	PlayerProfiles[PlayerIndex].AutoSwitchMode = AutoSwitchMode;
	OnPlayerProfileChanged.Broadcast(PlayerIndex, PlayerProfiles[PlayerIndex]);
}

EFociPS5Button UFociInputProfileSubsystem::GetBoundButtonForAction(int32 PlayerIndex, EFociGameplayAction Action) const
{
	if (PlayerProfiles.IsValidIndex(PlayerIndex))
	{
		if (const EFociPS5Button* Found = PlayerProfiles[PlayerIndex].ActionBindings.Find(Action))
		{
			return *Found;
		}
	}
	return EFociPS5Button::Cross;
}

bool UFociInputProfileSubsystem::FindActionForButtonInContext(int32 PlayerIndex, EFociPS5Button Button, EFociControlContext Context, EFociGameplayAction& OutAction) const
{
	if (!PlayerProfiles.IsValidIndex(PlayerIndex))
	{
		return false;
	}

	for (const auto& Pair : PlayerProfiles[PlayerIndex].ActionBindings)
	{
		if (Pair.Value == Button && GetActionContext(Pair.Key) == Context)
		{
			OutAction = Pair.Key;
			return true;
		}
	}
	return false;
}

EFociControlContext UFociInputProfileSubsystem::GetActionContext(EFociGameplayAction Action)
{
	return static_cast<EFociControlContext>(static_cast<uint8>(
		FociSim::GetActionContext(static_cast<FociSim::GameplayAction>(static_cast<uint8>(Action)))
	));
}

bool UFociInputProfileSubsystem::AreActionsInSameContext(EFociGameplayAction ActionA, EFociGameplayAction ActionB)
{
	return FociSim::AreActionsInSameContext(
		static_cast<FociSim::GameplayAction>(static_cast<uint8>(ActionA)),
		static_cast<FociSim::GameplayAction>(static_cast<uint8>(ActionB))
	);
}

TMap<EFociGameplayAction, EFociPS5Button> UFociInputProfileSubsystem::BuildPresetBindings(EFociControlPreset Preset)
{
	const auto SimBindings = FociSim::BuildPresetBindings(static_cast<FociSim::ControlPreset>(static_cast<uint8>(Preset)));
	TMap<EFociGameplayAction, EFociPS5Button> OutBindings;
	for (const auto& Pair : SimBindings)
	{
		OutBindings.Add(
			static_cast<EFociGameplayAction>(static_cast<uint8>(Pair.first)),
			static_cast<EFociPS5Button>(static_cast<uint8>(Pair.second))
		);
	}
	return OutBindings;
}

FKey UFociInputProfileSubsystem::ConvertPS5ButtonToUnrealKey(EFociPS5Button Button)
{
	switch (Button)
	{
	case EFociPS5Button::Cross: return EKeys::Gamepad_FaceButton_Bottom;
	case EFociPS5Button::Circle: return EKeys::Gamepad_FaceButton_Right;
	case EFociPS5Button::Square: return EKeys::Gamepad_FaceButton_Left;
	case EFociPS5Button::Triangle: return EKeys::Gamepad_FaceButton_Top;
	case EFociPS5Button::L1: return EKeys::Gamepad_LeftShoulder;
	case EFociPS5Button::R1: return EKeys::Gamepad_RightShoulder;
	case EFociPS5Button::L2: return EKeys::Gamepad_LeftTrigger;
	case EFociPS5Button::R2: return EKeys::Gamepad_RightTrigger;
	case EFociPS5Button::L3: return EKeys::Gamepad_LeftThumbstick;
	case EFociPS5Button::R3: return EKeys::Gamepad_RightThumbstick;
	case EFociPS5Button::DPadUp: return EKeys::Gamepad_DPad_Up;
	case EFociPS5Button::DPadDown: return EKeys::Gamepad_DPad_Down;
	case EFociPS5Button::DPadLeft: return EKeys::Gamepad_DPad_Left;
	case EFociPS5Button::DPadRight: return EKeys::Gamepad_DPad_Right;
	case EFociPS5Button::Touchpad: return EKeys::Gamepad_Special_Left;
	case EFociPS5Button::Options: return EKeys::Gamepad_Special_Right;
	default: return EKeys::Gamepad_FaceButton_Bottom;
	}
}

EFociPS5Button UFociInputProfileSubsystem::ConvertUnrealKeyToPS5Button(const FKey& Key, bool& bOutIsValid)
{
	bOutIsValid = true;
	if (Key == EKeys::Gamepad_FaceButton_Bottom) return EFociPS5Button::Cross;
	if (Key == EKeys::Gamepad_FaceButton_Right) return EFociPS5Button::Circle;
	if (Key == EKeys::Gamepad_FaceButton_Left) return EFociPS5Button::Square;
	if (Key == EKeys::Gamepad_FaceButton_Top) return EFociPS5Button::Triangle;
	if (Key == EKeys::Gamepad_LeftShoulder) return EFociPS5Button::L1;
	if (Key == EKeys::Gamepad_RightShoulder) return EFociPS5Button::R1;
	if (Key == EKeys::Gamepad_LeftTrigger) return EFociPS5Button::L2;
	if (Key == EKeys::Gamepad_RightTrigger) return EFociPS5Button::R2;
	if (Key == EKeys::Gamepad_LeftThumbstick) return EFociPS5Button::L3;
	if (Key == EKeys::Gamepad_RightThumbstick) return EFociPS5Button::R3;
	if (Key == EKeys::Gamepad_DPad_Up) return EFociPS5Button::DPadUp;
	if (Key == EKeys::Gamepad_DPad_Down) return EFociPS5Button::DPadDown;
	if (Key == EKeys::Gamepad_DPad_Left) return EFociPS5Button::DPadLeft;
	if (Key == EKeys::Gamepad_DPad_Right) return EFociPS5Button::DPadRight;
	if (Key == EKeys::Gamepad_Special_Left) return EFociPS5Button::Touchpad;
	if (Key == EKeys::Gamepad_Special_Right) return EFociPS5Button::Options;

	bOutIsValid = false;
	return EFociPS5Button::Cross;
}

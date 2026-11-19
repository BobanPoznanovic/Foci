#pragma once

#include "Simulation/FociSimTypes.h"
#include <algorithm>
#include <array>
#include <map>
#include <string>

namespace FociSim
{

enum class PS5Button : uint8_t
{
	Cross = 0,
	Circle = 1,
	Square = 2,
	Triangle = 3,
	L1 = 4,
	R1 = 5,
	L2 = 6,
	R2 = 7,
	L3 = 8,
	R3 = 9,
	DPadUp = 10,
	DPadDown = 11,
	DPadLeft = 12,
	DPadRight = 13,
	Touchpad = 14,
	Options = 15
};

enum class ControlContext : uint8_t
{
	OnBall = 0,
	OffBall = 1,
	System = 2
};

enum class GameplayAction : uint8_t
{
	GroundPass = 0,
	ThroughBall = 1,
	LobPassCross = 2,
	Shoot = 3,
	Sprint = 4,
	FinesseModifier = 5,
	ChipModifier = 6,
	ShieldBall = 7,

	SwitchPlayer = 8,
	StandingTackle = 9,
	SlidingTackle = 10,
	TeammateContain = 11,
	Jockey = 12,
	GoalkeeperRush = 13,

	TacticalMentalityUp = 14,
	TacticalMentalityDown = 15,
	OffsideTrap = 16,
	PauseMatch = 17
};

enum class ControlPreset : uint8_t
{
	Classic = 0,
	Alternate = 1,
	TwoButton = 2,
	Custom = 3
};

enum class AutoSwitchMode : uint8_t
{
	AirAndLooseBalls = 0,
	AirBallsOnly = 1,
	ManualOnly = 2
};

struct PlayerInputProfile
{
	int32_t PlayerIndex = 0;
	std::string ProfileName = "Player 1";
	TeamSide PlayerTeamSide = TeamSide::Home;
	ControlPreset Preset = ControlPreset::Classic;
	std::map<GameplayAction, PS5Button> ActionBindings;
	AssistLevel PassAssistance = AssistLevel::Assisted;
	AssistLevel ShotAssistance = AssistLevel::Assisted;
	AssistLevel CrossAssistance = AssistLevel::Assisted;
	AutoSwitchMode AutoSwitch = AutoSwitchMode::AirAndLooseBalls;
	bool bAnalogSprint = true;
	bool bEnableVibration = true;
};

inline ControlContext GetActionContext(GameplayAction Action)
{
	const uint8_t Val = static_cast<uint8_t>(Action);
	if (Val <= 7)
	{
		return ControlContext::OnBall;
	}
	if (Val <= 13)
	{
		return ControlContext::OffBall;
	}
	return ControlContext::System;
}

inline bool AreActionsInSameContext(GameplayAction A, GameplayAction B)
{
	return GetActionContext(A) == GetActionContext(B);
}

inline std::map<GameplayAction, PS5Button> BuildPresetBindings(ControlPreset Preset)
{
	std::map<GameplayAction, PS5Button> Bindings;

	// Common OnBall
	Bindings[GameplayAction::GroundPass] = PS5Button::Cross;
	Bindings[GameplayAction::ThroughBall] = (Preset == ControlPreset::TwoButton) ? PS5Button::Cross : PS5Button::Triangle;
	Bindings[GameplayAction::LobPassCross] = (Preset == ControlPreset::Alternate) ? PS5Button::Circle : PS5Button::Square;
	Bindings[GameplayAction::Shoot] = (Preset == ControlPreset::Alternate) ? PS5Button::Square : PS5Button::Circle;
	Bindings[GameplayAction::Sprint] = PS5Button::R2;
	Bindings[GameplayAction::FinesseModifier] = PS5Button::R1;
	Bindings[GameplayAction::ChipModifier] = PS5Button::L1;
	Bindings[GameplayAction::ShieldBall] = PS5Button::L2;

	// OffBall
	Bindings[GameplayAction::SwitchPlayer] = (Preset == ControlPreset::TwoButton) ? PS5Button::Cross : PS5Button::L1;
	Bindings[GameplayAction::StandingTackle] = (Preset == ControlPreset::Alternate) ? PS5Button::Square : PS5Button::Circle;
	Bindings[GameplayAction::SlidingTackle] = (Preset == ControlPreset::Alternate) ? PS5Button::Circle : PS5Button::Square;
	Bindings[GameplayAction::TeammateContain] = PS5Button::R1;
	Bindings[GameplayAction::Jockey] = PS5Button::L2;
	Bindings[GameplayAction::GoalkeeperRush] = PS5Button::Triangle;

	// System
	Bindings[GameplayAction::TacticalMentalityUp] = PS5Button::DPadRight;
	Bindings[GameplayAction::TacticalMentalityDown] = PS5Button::DPadLeft;
	Bindings[GameplayAction::OffsideTrap] = PS5Button::DPadUp;
	Bindings[GameplayAction::PauseMatch] = PS5Button::Options;

	return Bindings;
}

class InputProfileManager
{
public:
	std::array<PlayerInputProfile, kMaxHumanPlayers> Profiles;

	InputProfileManager()
	{
		EnsureDefaultProfilesInitialized();
	}

	void EnsureDefaultProfilesInitialized()
	{
		for (int32_t i = 0; i < kMaxHumanPlayers; ++i)
		{
			PlayerInputProfile& Profile = Profiles[static_cast<size_t>(i)];
			Profile.PlayerIndex = i;
			Profile.ProfileName = "Player " + std::to_string(i + 1);
			Profile.PlayerTeamSide = (i < 2) ? TeamSide::Home : TeamSide::Away;
			Profile.Preset = ControlPreset::Classic;
			Profile.ActionBindings = BuildPresetBindings(ControlPreset::Classic);
			Profile.PassAssistance = AssistLevel::Assisted;
			Profile.ShotAssistance = AssistLevel::Assisted;
			Profile.CrossAssistance = AssistLevel::Assisted;
			Profile.AutoSwitch = AutoSwitchMode::AirAndLooseBalls;
			Profile.bAnalogSprint = true;
			Profile.bEnableVibration = true;
		}
	}

	void ApplyControlPreset(int32_t PlayerIndex, ControlPreset Preset)
	{
		if (PlayerIndex < 0 || PlayerIndex >= kMaxHumanPlayers)
		{
			return;
		}
		PlayerInputProfile& Profile = Profiles[static_cast<size_t>(PlayerIndex)];
		Profile.Preset = Preset;
		if (Preset != ControlPreset::Custom)
		{
			Profile.ActionBindings = BuildPresetBindings(Preset);
		}
	}

	void RemapPlayerAction(int32_t PlayerIndex, GameplayAction Action, PS5Button NewButton, bool bSwapConflictingInSameContext = true)
	{
		if (PlayerIndex < 0 || PlayerIndex >= kMaxHumanPlayers)
		{
			return;
		}
		PlayerInputProfile& Profile = Profiles[static_cast<size_t>(PlayerIndex)];
		if (Profile.ActionBindings.empty())
		{
			Profile.ActionBindings = BuildPresetBindings(ControlPreset::Classic);
		}

		auto It = Profile.ActionBindings.find(Action);
		if (It == Profile.ActionBindings.end())
		{
			const auto Defaults = BuildPresetBindings(ControlPreset::Classic);
			auto DefIt = Defaults.find(Action);
			PS5Button DefaultBtn = (DefIt != Defaults.end()) ? DefIt->second : PS5Button::Cross;
			Profile.ActionBindings[Action] = DefaultBtn;
			It = Profile.ActionBindings.find(Action);
		}

		const PS5Button OldButton = It->second;
		if (bSwapConflictingInSameContext && OldButton != NewButton)
		{
			for (auto& Pair : Profile.ActionBindings)
			{
				if (Pair.first != Action && Pair.second == NewButton && AreActionsInSameContext(Pair.first, Action))
				{
					Pair.second = OldButton;
				}
			}
		}

		Profile.ActionBindings[Action] = NewButton;
		Profile.Preset = ControlPreset::Custom;
	}

	void SetPlayerTeamSide(int32_t PlayerIndex, TeamSide NewTeamSide)
	{
		if (PlayerIndex < 0 || PlayerIndex >= kMaxHumanPlayers)
		{
			return;
		}
		Profiles[static_cast<size_t>(PlayerIndex)].PlayerTeamSide = NewTeamSide;
	}

	const PlayerInputProfile& GetPlayerProfile(int32_t PlayerIndex) const
	{
		const int32_t Clamped = std::max(0, std::min(kMaxHumanPlayers - 1, PlayerIndex));
		return Profiles[static_cast<size_t>(Clamped)];
	}

	PS5Button GetBoundButtonForAction(int32_t PlayerIndex, GameplayAction Action) const
	{
		const PlayerInputProfile& Profile = GetPlayerProfile(PlayerIndex);
		auto It = Profile.ActionBindings.find(Action);
		if (It != Profile.ActionBindings.end())
		{
			return It->second;
		}
		const auto Defaults = BuildPresetBindings(ControlPreset::Classic);
		auto DefIt = Defaults.find(Action);
		return (DefIt != Defaults.end()) ? DefIt->second : PS5Button::Cross;
	}
};

} // namespace FociSim

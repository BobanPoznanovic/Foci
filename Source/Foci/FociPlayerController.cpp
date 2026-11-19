#include "FociPlayerController.h"
#include "Input/FociInputProfileSubsystem.h"
#include "Gameplay/FociFootballerCharacter.h"
#include "Gameplay/FociBallActor.h"
#include "Simulation/FociSwitchingSim.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameInstance.h"
#include "InputCoreTypes.h"
#include "EngineUtils.h"

AFociPlayerController::AFociPlayerController()
{
	bAutoManageActiveCameraTarget = false;
}

void AFociPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

int32 AFociPlayerController::GetHumanPlayerIndex() const
{
	return GetLocalPlayer() ? FMath::Clamp(GetLocalPlayer()->GetControllerId(), 0, 3) : 0;
}

void AFociPlayerController::SetSharedBroadcastCamera(AActor* CameraActor)
{
	SharedBroadcastCamera = CameraActor;
	if (CameraActor)
	{
		SetViewTarget(CameraActor);
	}
}

void AFociPlayerController::SetControlledFootballer(AFociFootballerCharacter* NewFootballer)
{
	if (!NewFootballer || NewFootballer == GetPawn())
	{
		return;
	}

	AFociFootballerCharacter* OldFootballer = Cast<AFociFootballerCharacter>(GetPawn());

	if (NewFootballer->GetController() != nullptr && NewFootballer->GetController() != this)
	{
		NewFootballer->GetController()->UnPossess();
	}

	if (OldFootballer)
	{
		OldFootballer->ControllingHumanPlayerIndex = -1;
	}

	Possess(NewFootballer);

	if (OldFootballer && OldFootballer->GetController() == nullptr)
	{
		OldFootballer->SpawnDefaultController();
	}

	NewFootballer->ControllingHumanPlayerIndex = GetHumanPlayerIndex();

	if (SharedBroadcastCamera.IsValid())
	{
		SetViewTarget(SharedBroadcastCamera.Get());
	}
}

float AFociPlayerController::GetCurrentActionChargeRatio() const
{
	return FMath::Clamp(CurrentActionChargeTime / 0.75f, 0.0f, 1.0f);
}

int32 AFociPlayerController::SelectBestTeammateToSwitch3D(
	int32 RequestingHumanPlayerIndex,
	int32 CurrentControlledCandidateIndex,
	const FVector& CurrentFootballerLocation,
	const FVector& BallLocation,
	const FVector2D& RightStickFlickDirection,
	const TArray<FVector>& TeammateLocations,
	const TArray<int32>& TeammateControllingHumanIndices,
	const TArray<bool>& TeammateIsGoalkeeper)
{
	std::vector<FociSim::Vec3> Locations;
	Locations.reserve(TeammateLocations.Num());
	for (const FVector& Loc : TeammateLocations)
	{
		Locations.emplace_back(Loc.X, Loc.Y, Loc.Z);
	}

	std::vector<int32_t> Indices;
	Indices.reserve(TeammateControllingHumanIndices.Num());
	for (int32 Idx : TeammateControllingHumanIndices)
	{
		Indices.push_back(Idx);
	}

	std::vector<bool> GKFlags;
	GKFlags.reserve(TeammateIsGoalkeeper.Num());
	for (bool bGK : TeammateIsGoalkeeper)
	{
		GKFlags.push_back(bGK);
	}

	return FociSim::SelectBestTeammateToSwitch3D(
		RequestingHumanPlayerIndex,
		CurrentControlledCandidateIndex,
		FociSim::Vec3(CurrentFootballerLocation.X, CurrentFootballerLocation.Y, CurrentFootballerLocation.Z),
		FociSim::Vec3(BallLocation.X, BallLocation.Y, BallLocation.Z),
		FociSim::Vec2(RightStickFlickDirection.X, RightStickFlickDirection.Y),
		Locations,
		Indices,
		GKFlags
	);
}

bool AFociPlayerController::RequestTeammateSwitch(const FVector2D& RightStickFlick)
{
	AFociFootballerCharacter* Controlled = Cast<AFociFootballerCharacter>(GetPawn());
	if (!Controlled || !GetWorld())
	{
		return false;
	}

	TArray<AFociFootballerCharacter*> Candidates;
	TArray<FVector> Locations;
	TArray<int32> ControllingIndices;
	TArray<bool> IsGKFlags;
	int32 CurrentIdx = -1;

	for (TActorIterator<AFociFootballerCharacter> It(GetWorld()); It; ++It)
	{
		AFociFootballerCharacter* Candidate = *It;
		if (Candidate && Candidate->TeamSide == Controlled->TeamSide)
		{
			if (Candidate == Controlled)
			{
				CurrentIdx = Candidates.Num();
			}
			Candidates.Add(Candidate);
			Locations.Add(Candidate->GetActorLocation());
			ControllingIndices.Add(Candidate->ControllingHumanPlayerIndex);
			IsGKFlags.Add(Candidate->bIsGoalkeeper);
		}
	}

	const FVector BallLoc = CachedBall.IsValid() ? CachedBall->GetActorLocation() : Controlled->GetActorLocation();
	const int32 BestIdx = SelectBestTeammateToSwitch3D(
		GetHumanPlayerIndex(),
		CurrentIdx,
		Controlled->GetActorLocation(),
		BallLoc,
		RightStickFlick,
		Locations,
		ControllingIndices,
		IsGKFlags
	);

	if (Candidates.IsValidIndex(BestIdx))
	{
		SetControlledFootballer(Candidates[BestIdx]);
		return true;
	}
	return false;
}

void AFociPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	UFociInputProfileSubsystem* InputSub = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFociInputProfileSubsystem>() : nullptr;
	AFociFootballerCharacter* Controlled = Cast<AFociFootballerCharacter>(GetPawn());
	if (!InputSub || !Controlled)
	{
		return;
	}

	if (!CachedBall.IsValid() && GetWorld())
	{
		for (TActorIterator<AFociBallActor> It(GetWorld()); It; ++It)
		{
			CachedBall = *It;
			break;
		}
	}

	const int32 PlayerIdx = GetHumanPlayerIndex();
	const FFociPlayerInputProfile Profile = InputSub->GetPlayerProfile(PlayerIdx);

	auto KeyFor = [&](EFociGameplayAction Action) -> FKey
	{
		return UFociInputProfileSubsystem::ConvertPS5ButtonToUnrealKey(InputSub->GetBoundButtonForAction(PlayerIdx, Action));
	};

	// System context
	if (WasInputKeyJustPressed(KeyFor(EFociGameplayAction::PauseMatch)))
	{
		SetPause(!IsPaused());
	}

	// Analog sticks & locomotion
	const FVector2D LeftStick(
		GetInputAnalogKeyState(EKeys::Gamepad_LeftX),
		GetInputAnalogKeyState(EKeys::Gamepad_LeftY)
	);
	const FVector2D RightStick(
		GetInputAnalogKeyState(EKeys::Gamepad_RightX),
		GetInputAnalogKeyState(EKeys::Gamepad_RightY)
	);
	const float SprintAxis = Profile.bAnalogSprint
		? GetInputAnalogKeyState(EKeys::Gamepad_RightTriggerAxis)
		: (IsInputKeyDown(KeyFor(EFociGameplayAction::Sprint)) ? 1.0f : 0.0f);
	const bool bJockeyHeld = IsInputKeyDown(KeyFor(EFociGameplayAction::Jockey));
	Controlled->bIsFinesseModifierHeld = IsInputKeyDown(KeyFor(EFociGameplayAction::FinesseModifier));
	Controlled->bIsChipModifierHeld = IsInputKeyDown(KeyFor(EFociGameplayAction::ChipModifier));
	Controlled->SetMovementInput3D(LeftStick, SprintAxis, bJockeyHeld);

	// On-Ball vs Off-Ball context routing
	const bool bHasBall = CachedBall.IsValid() && (CachedBall->PossessingFootballer.Get() == Controlled);
	if (bHasBall)
	{
		if (!bIsChargingAction)
		{
			const EFociGameplayAction PriorityActions[] = {
				EFociGameplayAction::Shoot,
				EFociGameplayAction::LobPassCross,
				EFociGameplayAction::GroundPass,
				EFociGameplayAction::ThroughBall
			};
			for (EFociGameplayAction Act : PriorityActions)
			{
				if (WasInputKeyJustPressed(KeyFor(Act)))
				{
					bIsChargingAction = true;
					ChargingAction = Act;
					CurrentActionChargeTime = 0.0f;
					break;
				}
			}
		}
		else
		{
			CurrentActionChargeTime = FMath::Min(CurrentActionChargeTime + DeltaTime, 0.85f);
			const FKey ChargeKey = KeyFor(ChargingAction);
			if (WasInputKeyJustReleased(ChargeKey) || !IsInputKeyDown(ChargeKey))
			{
				const float PowerRatio = FMath::Clamp(CurrentActionChargeTime / 0.75f, 0.15f, 1.0f);
				switch (ChargingAction)
				{
				case EFociGameplayAction::Shoot:
					Controlled->ExecuteShootAction(CachedBall.Get(), LeftStick, PowerRatio, Profile.ShotAssistance);
					break;
				case EFociGameplayAction::GroundPass:
					Controlled->ExecutePassAction(CachedBall.Get(), LeftStick, PowerRatio, false, false, Profile.PassAssistance);
					break;
				case EFociGameplayAction::ThroughBall:
					Controlled->ExecutePassAction(CachedBall.Get(), LeftStick, PowerRatio, true, false, Profile.PassAssistance);
					break;
				case EFociGameplayAction::LobPassCross:
					Controlled->ExecutePassAction(CachedBall.Get(), LeftStick, PowerRatio, false, true, Profile.CrossAssistance);
					break;
				default:
					break;
				}
				bIsChargingAction = false;
				CurrentActionChargeTime = 0.0f;
			}
		}
	}
	else
	{
		bIsChargingAction = false;
		CurrentActionChargeTime = 0.0f;

		RightStickSwitchCooldown = FMath::Max(0.0f, RightStickSwitchCooldown - DeltaTime);
		if (WasInputKeyJustPressed(KeyFor(EFociGameplayAction::SwitchPlayer)))
		{
			RequestTeammateSwitch(FVector2D::ZeroVector);
		}
		else if (RightStickSwitchCooldown <= 0.0f && RightStick.SizeSquared() >= 0.2f)
		{
			if (RequestTeammateSwitch(RightStick))
			{
				RightStickSwitchCooldown = 0.25f;
			}
		}
		else if (WasInputKeyJustPressed(KeyFor(EFociGameplayAction::StandingTackle)))
		{
			Controlled->ExecuteTackleAction(CachedBall.Get(), false);
		}
		else if (WasInputKeyJustPressed(KeyFor(EFociGameplayAction::SlidingTackle)))
		{
			Controlled->ExecuteTackleAction(CachedBall.Get(), true);
		}
	}
}

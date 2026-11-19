#include "Core/FociMatchGameMode.h"
#include "Core/FociMatchGameState.h"
#include "Core/FociGraphicsScalability.h"
#include "Input/FociInputProfileSubsystem.h"
#include "Gameplay/FociBallActor.h"
#include "Gameplay/FociPitchVolume.h"
#include "Gameplay/FociFootballerCharacter.h"
#include "Gameplay/FociBroadcastCameraActor.h"
#include "AI/FociGoalkeeperCharacter.h"
#include "AI/FociAIController.h"
#include "FociPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "EngineUtils.h"

AFociMatchGameMode::AFociMatchGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	DefaultPawnClass = nullptr;
	PlayerControllerClass = AFociPlayerController::StaticClass();
	GameStateClass = AFociMatchGameState::StaticClass();
	FootballerClass = AFociFootballerCharacter::StaticClass();
	GoalkeeperClass = AFociGoalkeeperCharacter::StaticClass();
}

void AFociMatchGameMode::BeginPlay()
{
	Super::BeginPlay();

	UFociGraphicsScalability::ApplyGraphicsPreset(this, EFociGraphicsPreset::MacBook2018_RadeonBalanced);

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AFociPitchVolume> It(World); It; ++It)
	{
		PitchVolume = *It;
		break;
	}
	if (!PitchVolume)
	{
		PitchVolume = World->SpawnActor<AFociPitchVolume>(AFociPitchVolume::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
	}

	for (TActorIterator<AFociBallActor> It(World); It; ++It)
	{
		MatchBall = *It;
		break;
	}
	if (!MatchBall)
	{
		MatchBall = World->SpawnActor<AFociBallActor>(AFociBallActor::StaticClass(), FVector(0.0f, 0.0f, 11.0f), FRotator::ZeroRotator);
	}

	for (TActorIterator<AFociBroadcastCameraActor> It(World); It; ++It)
	{
		BroadcastCamera = *It;
		break;
	}
	if (!BroadcastCamera)
	{
		BroadcastCamera = World->SpawnActor<AFociBroadcastCameraActor>(AFociBroadcastCameraActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
	}
	if (BroadcastCamera && MatchBall)
	{
		BroadcastCamera->SetTrackedBall(MatchBall);
	}

	EnsureFourLocalHumanPlayers(World);
	SpawnSquadsAndAssignHumanPlayers();

	if (AFociMatchGameState* GS = GetGameState<AFociMatchGameState>())
	{
		GS->CurrentPhase = EFociMatchPhase::KickOff;
		PhaseStateTimer = 0.0f;
	}
}

void AFociMatchGameMode::EnsureFourLocalHumanPlayers(UWorld* World)
{
	if (!World)
	{
		return;
	}

	UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		return;
	}

	const int32 TargetCount = FMath::Clamp(NumLocalHumanPlayers, 1, 4);
	for (int32 Id = 0; Id < TargetCount; ++Id)
	{
		bool bExists = false;
		for (ULocalPlayer* LP : GI->GetLocalPlayers())
		{
			if (LP && LP->GetControllerId() == Id)
			{
				bExists = true;
				break;
			}
		}
		if (!bExists)
		{
			UGameplayStatics::CreatePlayer(World, Id, true);
		}
	}
}

void AFociMatchGameMode::SpawnSquadsAndAssignHumanPlayers()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	struct FAnchorSpec
	{
		FVector Pos;
		int32 Number;
	};

	static const TArray<FAnchorSpec> Anchors11v11 = {
		{FVector(-5000.0f, 0.0f, 96.0f), 1},
		{FVector(-250.0f, 0.0f, 96.0f), 9},
		{FVector(-2000.0f, 1400.0f, 96.0f), 10},
		{FVector(-800.0f, -1900.0f, 96.0f), 11},
		{FVector(-800.0f, 1900.0f, 96.0f), 7},
		{FVector(-2000.0f, -1400.0f, 96.0f), 8},
		{FVector(-2300.0f, 0.0f, 96.0f), 6},
		{FVector(-3400.0f, -2100.0f, 96.0f), 3},
		{FVector(-3600.0f, -750.0f, 96.0f), 4},
		{FVector(-3600.0f, 750.0f, 96.0f), 5},
		{FVector(-3400.0f, 2100.0f, 96.0f), 2}
	};

	static const TArray<FAnchorSpec> Anchors5v5 = {
		{FVector(-5000.0f, 0.0f, 96.0f), 1},
		{FVector(-250.0f, 0.0f, 96.0f), 9},
		{FVector(-1600.0f, 0.0f, 96.0f), 10},
		{FVector(-3200.0f, -1100.0f, 96.0f), 4},
		{FVector(-3200.0f, 1100.0f, 96.0f), 5}
	};

	const TArray<FAnchorSpec>& ActiveAnchors = bFiveASideMode ? Anchors5v5 : Anchors11v11;

	auto SpawnTeam = [&](EFociTeamSide Side, TArray<TObjectPtr<AFociFootballerCharacter>>& OutSquad)
	{
		OutSquad.Reset();
		const FRotator Facing = (Side == EFociTeamSide::Home) ? FRotator(0.0f, 0.0f, 0.0f) : FRotator(0.0f, 180.0f, 0.0f);

		for (int32 i = 0; i < ActiveAnchors.Num(); ++i)
		{
			const FVector HomePos = ActiveAnchors[i].Pos;
			const FVector SpawnPos = (Side == EFociTeamSide::Home)
				? HomePos
				: FVector(-HomePos.X, -HomePos.Y, HomePos.Z);

			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			UClass* SpawnClass = (i == 0)
				? (GoalkeeperClass ? GoalkeeperClass.Get() : AFociGoalkeeperCharacter::StaticClass())
				: (FootballerClass ? FootballerClass.Get() : AFociFootballerCharacter::StaticClass());

			AFociFootballerCharacter* Footballer = World->SpawnActor<AFociFootballerCharacter>(SpawnClass, SpawnPos, Facing, Params);
			if (Footballer)
			{
				Footballer->TeamSide = Side;
				Footballer->SquadNumber = ActiveAnchors[i].Number;
				Footballer->bIsGoalkeeper = (i == 0);
				Footballer->FormationHomeLocation = SpawnPos;
				Footballer->AIControllerClass = AFociAIController::StaticClass();
				Footballer->SpawnDefaultController();
				OutSquad.Add(Footballer);
			}
		}
	};

	SpawnTeam(EFociTeamSide::Home, HomeSquad);
	SpawnTeam(EFociTeamSide::Away, AwaySquad);

	UFociInputProfileSubsystem* InputSub = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFociInputProfileSubsystem>() : nullptr;
	int32 NextHomeIdx = 1;
	int32 NextAwayIdx = 1;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		AFociPlayerController* PC = Cast<AFociPlayerController>(It->Get());
		if (!PC)
		{
			continue;
		}

		const int32 PlayerIdx = PC->GetHumanPlayerIndex();
		EFociTeamSide DesiredSide = EFociTeamSide::Home;
		if (InputSub)
		{
			const FFociPlayerInputProfile Profile = InputSub->GetPlayerProfile(PlayerIdx);
			DesiredSide = (Profile.TeamSide == EFociTeamSide::Away) ? EFociTeamSide::Away : EFociTeamSide::Home;
		}

		AFociFootballerCharacter* Assigned = nullptr;
		if (DesiredSide == EFociTeamSide::Home && HomeSquad.IsValidIndex(NextHomeIdx))
		{
			Assigned = HomeSquad[NextHomeIdx++];
		}
		else if (DesiredSide == EFociTeamSide::Away && AwaySquad.IsValidIndex(NextAwayIdx))
		{
			Assigned = AwaySquad[NextAwayIdx++];
		}

		if (BroadcastCamera)
		{
			PC->SetSharedBroadcastCamera(BroadcastCamera);
		}
		if (Assigned)
		{
			PC->SetControlledFootballer(Assigned);
		}
	}
}

void AFociMatchGameMode::ResetSquadsForKickOff()
{
	auto ResetTeam = [](TArray<TObjectPtr<AFociFootballerCharacter>>& Squad, const FRotator& Facing)
	{
		for (AFociFootballerCharacter* Footballer : Squad)
		{
			if (!Footballer)
			{
				continue;
			}
			Footballer->SetActorLocationAndRotation(Footballer->FormationHomeLocation, Facing, false, nullptr, ETeleportType::TeleportPhysics);
			if (UCharacterMovementComponent* MoveComp = Footballer->GetCharacterMovement())
			{
				MoveComp->StopMovementImmediately();
			}
			Footballer->CurrentStamina = 1.0f;
		}
	};

	ResetTeam(HomeSquad, FRotator(0.0f, 0.0f, 0.0f));
	ResetTeam(AwaySquad, FRotator(0.0f, 180.0f, 0.0f));
}

bool AFociMatchGameMode::CheckAndApplyOffsideOnPass(AFociFootballerCharacter* Passer, AFociFootballerCharacter* Receiver)
{
	if (!Passer || !Receiver || !MatchBall || Passer->TeamSide != Receiver->TeamSide)
	{
		return false;
	}

	const TArray<TObjectPtr<AFociFootballerCharacter>>& Defenders = (Passer->TeamSide == EFociTeamSide::Home) ? AwaySquad : HomeSquad;
	TArray<float> DefenderXs;
	DefenderXs.Reserve(Defenders.Num());
	for (const AFociFootballerCharacter* Def : Defenders)
	{
		if (Def)
		{
			DefenderXs.Add(Def->GetActorLocation().X);
		}
	}

	if (AFociPitchVolume::IsPlayerInOffsidePosition3D(Receiver->GetActorLocation().X, MatchBall->GetActorLocation().X, Passer->TeamSide, DefenderXs))
	{
		if (AFociMatchGameState* GS = GetGameState<AFociMatchGameState>())
		{
			GS->bOffsideTriggered = true;
			GS->LastOffsideTeam = Passer->TeamSide;
			GS->OffsideSpot = Receiver->GetActorLocation();
			GS->CurrentPhase = EFociMatchPhase::FreeKick;
			PhaseStateTimer = 0.0f;
		}
		MatchBall->ResetBallToLocation(FVector(Receiver->GetActorLocation().X, Receiver->GetActorLocation().Y, MatchBall->BallRadiusCm));
		return true;
	}

	return false;
}

void AFociMatchGameMode::ResolveLooseBallCapture()
{
	if (!MatchBall || MatchBall->PossessingFootballer.IsValid() || MatchBall->GetActorLocation().Z > 120.0f)
	{
		return;
	}

	const FVector BallLoc = MatchBall->GetActorLocation();
	AFociFootballerCharacter* Best = nullptr;
	float BestDist = 85.0f;

	auto CheckSquad = [&](const TArray<TObjectPtr<AFociFootballerCharacter>>& Squad)
	{
		for (AFociFootballerCharacter* F : Squad)
		{
			if (!F || !MatchBall->CanBeCapturedBy(F))
			{
				continue;
			}
			const float Dist = FVector::Dist2D(F->GetActorLocation(), BallLoc);
			if (Dist <= BestDist)
			{
				BestDist = Dist;
				Best = F;
			}
		}
	};

	CheckSquad(HomeSquad);
	CheckSquad(AwaySquad);

	if (Best)
	{
		MatchBall->SetPossessingFootballer(Best, Best->TeamSide);
	}
}

void AFociMatchGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AFociMatchGameState* GS = GetGameState<AFociMatchGameState>();
	if (!GS || !MatchBall || !PitchVolume)
	{
		return;
	}

	switch (GS->CurrentPhase)
	{
	case EFociMatchPhase::KickOff:
	case EFociMatchPhase::FreeKick:
		PhaseStateTimer += DeltaTime;
		if (PhaseStateTimer >= KickOffDelaySeconds)
		{
			GS->bOffsideTriggered = false;
			GS->CurrentPhase = EFociMatchPhase::InPlay;
			PhaseStateTimer = 0.0f;
		}
		break;

	case EFociMatchPhase::InPlay:
	{
		GS->MatchClockSeconds += DeltaTime * GS->MatchTimeScale;
		ResolveLooseBallCapture();
		GS->PossessionTeam = MatchBall->PossessingFootballer.IsValid() ? MatchBall->LastTouchTeam : EFociTeamSide::Neutral;

		const EFociPitchEvent Event = AFociPitchVolume::EvaluateBallBoundary3D(
			MatchBall->GetActorLocation(),
			MatchBall->BallRadiusCm,
			MatchBall->LastTouchTeam
		);

		if (Event == EFociPitchEvent::GoalScoredHome)
		{
			GS->HomeScore++;
			GS->KickOffTeam = EFociTeamSide::Away;
			GS->CurrentPhase = EFociMatchPhase::GoalCelebration;
			PhaseStateTimer = 0.0f;
		}
		else if (Event == EFociPitchEvent::GoalScoredAway)
		{
			GS->AwayScore++;
			GS->KickOffTeam = EFociTeamSide::Home;
			GS->CurrentPhase = EFociMatchPhase::GoalCelebration;
			PhaseStateTimer = 0.0f;
		}
		else if (Event != EFociPitchEvent::InPlay)
		{
			MatchBall->ResetBallToLocation(PitchVolume->ComputeRestartLocation3D(Event, MatchBall->GetActorLocation()));
		}

		if (GS->CurrentHalf == 1 && GS->MatchClockSeconds >= 2700.0f)
		{
			GS->CurrentPhase = EFociMatchPhase::HalfTime;
			PhaseStateTimer = 0.0f;
		}
		else if (GS->CurrentHalf == 2 && GS->MatchClockSeconds >= 5400.0f)
		{
			GS->CurrentPhase = EFociMatchPhase::FullTime;
		}
		break;
	}

	case EFociMatchPhase::GoalCelebration:
		PhaseStateTimer += DeltaTime;
		if (PhaseStateTimer >= GoalCelebrationDuration)
		{
			ResetSquadsForKickOff();
			MatchBall->ResetBallToLocation(FVector(0.0f, 0.0f, MatchBall->BallRadiusCm));
			PhaseStateTimer = 0.0f;
			GS->CurrentPhase = EFociMatchPhase::KickOff;
		}
		break;

	case EFociMatchPhase::HalfTime:
		PhaseStateTimer += DeltaTime;
		if (PhaseStateTimer >= HalfTimeDuration)
		{
			GS->CurrentHalf = 2;
			GS->KickOffTeam = EFociTeamSide::Away;
			ResetSquadsForKickOff();
			MatchBall->ResetBallToLocation(FVector(0.0f, 0.0f, MatchBall->BallRadiusCm));
			PhaseStateTimer = 0.0f;
			GS->CurrentPhase = EFociMatchPhase::KickOff;
		}
		break;

	case EFociMatchPhase::FullTime:
	default:
		break;
	}
}

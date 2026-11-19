#include "Gameplay/FociBroadcastCameraActor.h"
#include "Gameplay/FociBallActor.h"
#include "Gameplay/FociFootballerCharacter.h"
#include "Simulation/FociCameraSim.h"
#include "Components/SceneComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EngineUtils.h"

AFociBroadcastCameraActor::AFociBroadcastCameraActor()
{
	PrimaryActorTick.bCanEverTick = true;

	FocusRoot = CreateDefaultSubobject<USceneComponent>(TEXT("FocusRoot"));
	RootComponent = FocusRoot;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(FocusRoot);
	CameraBoom->SetRelativeRotation(FRotator(-26.0f, -90.0f, 0.0f));
	CameraBoom->TargetArmLength = 3000.0f;
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = CameraLagSpeed;

	BroadcastCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("BroadcastCamera"));
	BroadcastCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	BroadcastCamera->FieldOfView = 42.0f;
}

void AFociBroadcastCameraActor::SetTrackedBall(AFociBallActor* InBall)
{
	TrackedBall = InBall;
}

void AFociBroadcastCameraActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!TrackedBall.IsValid() && GetWorld())
	{
		for (TActorIterator<AFociBallActor> It(GetWorld()); It; ++It)
		{
			TrackedBall = *It;
			break;
		}
	}

	const FVector BallLoc = TrackedBall.IsValid() ? TrackedBall->GetActorLocation() : GetActorLocation();

	TArray<FVector> HumanLocs;
	if (GetWorld())
	{
		for (TActorIterator<AFociFootballerCharacter> It(GetWorld()); It; ++It)
		{
			AFociFootballerCharacter* Footballer = *It;
			if (Footballer && Footballer->ControllingHumanPlayerIndex >= 0)
			{
				HumanLocs.Add(Footballer->GetActorLocation());
			}
		}
	}

	FVector TargetFocus = FVector::ZeroVector;
	float TargetArmLength = 3000.0f;
	ComputeBroadcastCameraFrame3D(
		BallLoc,
		HumanLocs,
		BallFocusWeight,
		MinArmLengthCm,
		MaxArmLengthCm,
		TargetFocus,
		TargetArmLength
	);

	TargetFocus.X = FMath::Clamp(TargetFocus.X, -4600.0f, 4600.0f);
	TargetFocus.Y = FMath::Clamp(TargetFocus.Y, -2200.0f, 2200.0f);

	SetActorLocation(FMath::VInterpTo(GetActorLocation(), TargetFocus, DeltaTime, CameraLagSpeed));
	if (CameraBoom)
	{
		CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArmLength, DeltaTime, CameraLagSpeed);
	}
}

void AFociBroadcastCameraActor::ComputeBroadcastCameraFrame3D(
	const FVector& BallLocation,
	const TArray<FVector>& HumanPlayerLocations,
	float BallWeight,
	float MinArmLength,
	float MaxArmLength,
	FVector& OutTargetFocusLocation,
	float& OutTargetArmLength)
{
	std::vector<FociSim::Vec3> SimLocs;
	SimLocs.reserve(HumanPlayerLocations.Num());
	for (const FVector& Loc : HumanPlayerLocations)
	{
		SimLocs.emplace_back(Loc.X, Loc.Y, Loc.Z);
	}

	FociSim::Vec3 OutFocus;
	FociSim::ComputeBroadcastCameraFrame3D(
		FociSim::Vec3(BallLocation.X, BallLocation.Y, BallLocation.Z),
		SimLocs,
		BallWeight,
		MinArmLength,
		MaxArmLength,
		OutFocus,
		OutTargetArmLength
	);

	OutTargetFocusLocation = FVector(OutFocus.X, OutFocus.Y, OutFocus.Z);
}

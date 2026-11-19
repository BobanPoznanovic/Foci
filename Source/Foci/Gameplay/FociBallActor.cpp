#include "Gameplay/FociBallActor.h"
#include "Simulation/FociBallPhysicsSim.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"

AFociBallActor::AFociBallActor()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(BallRadiusCm);
	CollisionSphere->SetSimulatePhysics(false);
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RootComponent = CollisionSphere;

	BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
	BallMesh->SetupAttachment(CollisionSphere);
	BallMesh->SetSimulatePhysics(false);
	BallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFociBallActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PossessionLockoutTimer = FMath::Max(0.0f, PossessionLockoutTimer - DeltaTime);

	if (ACharacter* OwnerChar = PossessingFootballer.Get())
	{
		FVector DribblePos = OwnerChar->GetActorLocation() + OwnerChar->GetActorForwardVector() * 55.0f;
		DribblePos.Z = GroundZ + BallRadiusCm;
		SetActorLocation(DribblePos);
		LinearVelocity = OwnerChar->GetVelocity();
	}
	else
	{
		FVector Pos = GetActorLocation();
		constexpr float MaxSubStep = 1.0f / 120.0f;
		float Remaining = FMath::Clamp(DeltaTime, 0.0f, 0.1f);
		while (Remaining > 0.0f)
		{
			const float Step = FMath::Min(MaxSubStep, Remaining);
			StepBallPhysics3D(
				Pos,
				LinearVelocity,
				AngularVelocity,
				Step,
				GroundZ,
				BallRadiusCm,
				TurfRestitution,
				TurfRollingFriction,
				AirDragCoefficient,
				MagnusCoefficient,
				SpinDecayRate
			);
			Remaining -= Step;
		}
		SetActorLocation(Pos);

		if (BallMesh && !LinearVelocity.IsNearlyZero(1.0f))
		{
			const float RollSpeed = LinearVelocity.Size2D();
			if (RollSpeed > 1.0f && BallRadiusCm > 0.0f)
			{
				const FVector RollAxis = FVector::CrossProduct(FVector::UpVector, LinearVelocity.GetSafeNormal2D());
				const float AngleDeg = FMath::RadiansToDegrees((RollSpeed * DeltaTime) / BallRadiusCm);
				BallMesh->AddWorldRotation(FQuat(RollAxis, FMath::DegreesToRadians(AngleDeg)));
			}
		}
	}
}

void AFociBallActor::KickBall(const FVector& NewLinearVelocity, const FVector& NewAngularVelocity, ACharacter* Kicker, EFociTeamSide KickerTeam)
{
	PossessingFootballer.Reset();
	LastTouchFootballer = Kicker;
	LastTouchTeam = KickerTeam;
	LinearVelocity = NewLinearVelocity;
	AngularVelocity = NewAngularVelocity;
	PossessionLockoutTimer = 0.18f;
}

void AFociBallActor::SetPossessingFootballer(ACharacter* NewOwner, EFociTeamSide OwnerTeam)
{
	PossessingFootballer = NewOwner;
	LastTouchFootballer = NewOwner;
	LastTouchTeam = OwnerTeam;
	LinearVelocity = FVector::ZeroVector;
	AngularVelocity = FVector::ZeroVector;
	PossessionLockoutTimer = 0.0f;
}

void AFociBallActor::ReleasePossession()
{
	PossessingFootballer.Reset();
}

void AFociBallActor::ResetBallToLocation(const FVector& NewWorldLocation)
{
	PossessingFootballer.Reset();
	LinearVelocity = FVector::ZeroVector;
	AngularVelocity = FVector::ZeroVector;
	PossessionLockoutTimer = 0.0f;
	SetActorLocation(NewWorldLocation);
}

bool AFociBallActor::CanBeCapturedBy(const ACharacter* Candidate) const
{
	return !PossessingFootballer.IsValid() && (PossessionLockoutTimer <= 0.0f || LastTouchFootballer.Get() != Candidate);
}

FVector AFociBallActor::PredictBallPositionAtTime(float FutureSeconds) const
{
	FociSim::BallPhysicsParams Params;
	Params.BallRadiusCm = BallRadiusCm;
	Params.BallMassKg = BallMassKg;
	Params.TurfRestitution = TurfRestitution;
	Params.TurfRollingFriction = TurfRollingFriction;
	Params.AirDragCoefficient = AirDragCoefficient;
	Params.MagnusCoefficient = MagnusCoefficient;
	Params.SpinDecayRate = SpinDecayRate;

	const FVector Loc = GetActorLocation();
	const FociSim::Vec3 Pred = FociSim::PredictBallPositionAtTime(
		FociSim::Vec3(Loc.X, Loc.Y, Loc.Z),
		FociSim::Vec3(LinearVelocity.X, LinearVelocity.Y, LinearVelocity.Z),
		FociSim::Vec3(AngularVelocity.X, AngularVelocity.Y, AngularVelocity.Z),
		FutureSeconds,
		GroundZ,
		Params
	);
	return FVector(Pred.X, Pred.Y, Pred.Z);
}

void AFociBallActor::StepBallPhysics3D(
	FVector& InOutPosition,
	FVector& InOutVelocity,
	FVector& InOutAngularVelocity,
	float DeltaTime,
	float InGroundZ,
	float RadiusCm,
	float Restitution,
	float RollingFriction,
	float DragCoeff,
	float MagnusCoeff,
	float SpinDecay)
{
	FociSim::Vec3 Pos(InOutPosition.X, InOutPosition.Y, InOutPosition.Z);
	FociSim::Vec3 Vel(InOutVelocity.X, InOutVelocity.Y, InOutVelocity.Z);
	FociSim::Vec3 AngVel(InOutAngularVelocity.X, InOutAngularVelocity.Y, InOutAngularVelocity.Z);

	FociSim::StepBallPhysics3D(
		Pos,
		Vel,
		AngVel,
		DeltaTime,
		InGroundZ,
		RadiusCm,
		Restitution,
		RollingFriction,
		DragCoeff,
		MagnusCoeff,
		SpinDecay
	);

	InOutPosition = FVector(Pos.X, Pos.Y, Pos.Z);
	InOutVelocity = FVector(Vel.X, Vel.Y, Vel.Z);
	InOutAngularVelocity = FVector(AngVel.X, AngVel.Y, AngVel.Z);
}

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

#include "Simulation/FociSimTypes.h"
#include "Simulation/FociScalabilitySim.h"
#include "Simulation/FociInputSim.h"
#include "Simulation/FociBallPhysicsSim.h"
#include "Simulation/FociPitchSim.h"
#include "Simulation/FociKickSolverSim.h"
#include "Simulation/FociSwitchingSim.h"
#include "Simulation/FociCameraSim.h"
#include "Simulation/FociAISim.h"

namespace
{

bool NearlyEqual(float A, float B, float Tol = 1.0e-3f)
{
	return std::fabs(A - B) <= Tol;
}

void Test_MacBook2018_GraphicsPresets()
{
	const auto Iris = FociSim::GetGraphicsConfigForPreset(FociSim::GraphicsPreset::MacBook2018_IrisPerformance);
	assert(Iris.TargetFPS == 60);
	assert(NearlyEqual(Iris.ScreenPercentage, 75.0f));
	assert(Iris.MSAACount == 2);
	assert(Iris.CSMMaxCascades == 1);
	assert(Iris.MaxCSMResolution == 1024);
	assert(Iris.bEnableCapsuleShadows == true);
	assert(Iris.SSRQuality == 1);
	assert(NearlyEqual(Iris.GrassCullDistanceMultiplier, 0.75f));

	const auto Radeon = FociSim::GetGraphicsConfigForPreset(FociSim::GraphicsPreset::MacBook2018_RadeonBalanced);
	assert(Radeon.TargetFPS == 60);
	assert(NearlyEqual(Radeon.ScreenPercentage, 85.0f));
	assert(Radeon.MSAACount == 2);
	assert(Radeon.CSMMaxCascades == 2);
	assert(Radeon.MaxCSMResolution == 1024);
	assert(Radeon.SSRQuality == 2);
	assert(NearlyEqual(Radeon.GrassCullDistanceMultiplier, 1.0f));

	const auto Replay = FociSim::GetGraphicsConfigForPreset(FociSim::GraphicsPreset::PhotorealBroadcastReplay);
	assert(Replay.TargetFPS == 30);
	assert(NearlyEqual(Replay.ScreenPercentage, 100.0f));
	assert(Replay.MSAACount == 4);
	assert(Replay.CSMMaxCascades == 3);
	assert(Replay.MaxCSMResolution == 2048);
	assert(Replay.SSRQuality == 3);
	assert(NearlyEqual(Replay.GrassCullDistanceMultiplier, 1.5f));
}

void Test_PS5DualSense_4PlayerRemappingAndConflictSwap()
{
	FociSim::InputProfileManager Mgr;
	for (int32_t i = 0; i < 4; ++i)
	{
		const auto& P = Mgr.GetPlayerProfile(i);
		assert(P.PlayerIndex == i);
		assert(P.Preset == FociSim::ControlPreset::Classic);
		assert(P.PlayerTeamSide == (i < 2 ? FociSim::TeamSide::Home : FociSim::TeamSide::Away));
	}

	Mgr.ApplyControlPreset(1, FociSim::ControlPreset::Alternate);
	assert(Mgr.GetBoundButtonForAction(1, FociSim::GameplayAction::Shoot) == FociSim::PS5Button::Square);
	assert(Mgr.GetBoundButtonForAction(1, FociSim::GameplayAction::LobPassCross) == FociSim::PS5Button::Circle);
	assert(Mgr.GetBoundButtonForAction(1, FociSim::GameplayAction::StandingTackle) == FociSim::PS5Button::Square);
	assert(Mgr.GetBoundButtonForAction(1, FociSim::GameplayAction::SlidingTackle) == FociSim::PS5Button::Circle);
	assert(Mgr.GetBoundButtonForAction(0, FociSim::GameplayAction::Shoot) == FociSim::PS5Button::Circle);

	Mgr.RemapPlayerAction(2, FociSim::GameplayAction::Shoot, FociSim::PS5Button::Triangle, true);
	assert(Mgr.GetBoundButtonForAction(2, FociSim::GameplayAction::Shoot) == FociSim::PS5Button::Triangle);
	assert(Mgr.GetBoundButtonForAction(2, FociSim::GameplayAction::ThroughBall) == FociSim::PS5Button::Circle);
	assert(Mgr.GetPlayerProfile(2).Preset == FociSim::ControlPreset::Custom);
	assert(Mgr.GetBoundButtonForAction(2, FociSim::GameplayAction::GoalkeeperRush) == FociSim::PS5Button::Triangle);
	assert(Mgr.GetPlayerProfile(0).Preset == FociSim::ControlPreset::Classic);
	assert(Mgr.GetPlayerProfile(1).Preset == FociSim::ControlPreset::Alternate);
	assert(Mgr.GetPlayerProfile(3).Preset == FociSim::ControlPreset::Classic);
}

void Test_3DBallPhysics_MagnusCurveAndTurfBounce()
{
	// 1. Magnus Finesse Curl
	{
		FociSim::Vec3 Pos(0.0f, 0.0f, 11.0f);
		FociSim::Vec3 Vel(2500.0f, 0.0f, 420.0f);
		FociSim::Vec3 Ang(0.0f, 0.0f, 35.0f);
		for (int i = 0; i < 120; ++i)
		{
			FociSim::StepBallPhysics3D(Pos, Vel, Ang, 1.0f / 120.0f);
		}
		assert(Pos.Y > 100.0f);
	}

	// 2. Aerial Lob Bounce & Rolling Stop
	{
		FociSim::Vec3 Pos(0.0f, 0.0f, 11.0f);
		FociSim::Vec3 Vel(1200.0f, 0.0f, 600.0f);
		FociSim::Vec3 Ang(0.0f, -18.0f, 0.0f);
		float PeakZ = 0.0f;
		bool bBounced = false;
		for (int i = 0; i < 720; ++i)
		{
			const float PrevVz = Vel.Z;
			FociSim::StepBallPhysics3D(Pos, Vel, Ang, 1.0f / 120.0f);
			PeakZ = std::max(PeakZ, Pos.Z);
			if (PrevVz < -100.0f && Vel.Z > 50.0f)
			{
				bBounced = true;
			}
		}
		assert(PeakZ > 150.0f);
		assert(bBounced);
		assert(std::fabs(Pos.Z - 11.0f) < 0.01f);
		assert(Vel.Size2D() < 1.0f);
	}

	// 3. Backspin Lift
	{
		const FociSim::Vec3 BackspinPos = FociSim::PredictBallPositionAtTime(
			FociSim::Vec3(0.0f, 0.0f, 11.0f),
			FociSim::Vec3(2000.0f, 0.0f, 500.0f),
			FociSim::Vec3(0.0f, -18.0f, 0.0f),
			0.5f
		);
		const FociSim::Vec3 FlatPos = FociSim::PredictBallPositionAtTime(
			FociSim::Vec3(0.0f, 0.0f, 11.0f),
			FociSim::Vec3(2000.0f, 0.0f, 500.0f),
			FociSim::Vec3(0.0f, 0.0f, 0.0f),
			0.5f
		);
		assert(BackspinPos.Z > FlatPos.Z + 5.0f);
	}
}

void Test_3DPitchBoundary_GoalLineTechnologyAndOffside()
{
	assert(FociSim::EvaluateBallBoundary3D(FociSim::Vec3(5255.0f, 0.0f, 100.0f), 11.0f, FociSim::TeamSide::Home) == FociSim::PitchEvent::InPlay);
	assert(FociSim::EvaluateBallBoundary3D(FociSim::Vec3(5270.0f, 300.0f, 210.0f), 11.0f, FociSim::TeamSide::Home) == FociSim::PitchEvent::GoalScoredHome);
	assert(FociSim::EvaluateBallBoundary3D(FociSim::Vec3(5270.0f, 300.0f, 270.0f), 11.0f, FociSim::TeamSide::Home) == FociSim::PitchEvent::GoalKickAway);
	assert(FociSim::EvaluateBallBoundary3D(FociSim::Vec3(5270.0f, 300.0f, 270.0f), 11.0f, FociSim::TeamSide::Away) == FociSim::PitchEvent::CornerKickHome);
	assert(FociSim::EvaluateBallBoundary3D(FociSim::Vec3(1000.0f, 3420.0f, 11.0f), 11.0f, FociSim::TeamSide::Home) == FociSim::PitchEvent::ThrowInAway);
	assert(FociSim::EvaluateBallBoundary3D(FociSim::Vec3(1000.0f, 3420.0f, 11.0f), 11.0f, FociSim::TeamSide::Away) == FociSim::PitchEvent::ThrowInHome);

	const std::vector<float> HomeDefs = {5100.0f, 2200.0f, 1800.0f};
	assert(FociSim::IsPlayerInOffsidePosition3D(2500.0f, 1500.0f, FociSim::TeamSide::Home, HomeDefs) == true);
	assert(FociSim::IsPlayerInOffsidePosition3D(1200.0f, 1500.0f, FociSim::TeamSide::Home, HomeDefs) == false);
	assert(FociSim::IsPlayerInOffsidePosition3D(-100.0f, 1500.0f, FociSim::TeamSide::Home, HomeDefs) == false);

	const std::vector<float> AwayDefs = {-5100.0f, -2200.0f, -1800.0f};
	assert(FociSim::IsPlayerInOffsidePosition3D(-2500.0f, -1500.0f, FociSim::TeamSide::Away, AwayDefs) == true);
	assert(FociSim::IsPlayerInOffsidePosition3D(100.0f, -1500.0f, FociSim::TeamSide::Away, AwayDefs) == false);
}

void Test_3DKickSolver_PassesAndShots()
{
	FociSim::Vec3 Vel, Spin;
	const FociSim::Vec3 Start(0.0f, 0.0f, 11.0f);
	const FociSim::Vec3 Aim(1.0f, 0.0f, 0.0f);
	const FociSim::Vec3 Ideal(2000.0f, 500.0f, 11.0f);

	// GroundPass
	FociSim::ComputeKickVelocityAndSpin3D(Start, Aim, 0.5f, false, false, false, false, false, FociSim::AssistLevel::Manual, Ideal, Vel, Spin);
	assert(NearlyEqual(Vel.Size(), 1650.0f, 5.0f));
	assert(NearlyEqual(Spin.Size(), 0.0f));

	// LobPassCross
	FociSim::ComputeKickVelocityAndSpin3D(Start, Aim, 0.5f, false, true, false, false, false, FociSim::AssistLevel::Manual, Ideal, Vel, Spin);
	assert(Vel.Z > 800.0f);

	// ThroughBall
	FociSim::ComputeKickVelocityAndSpin3D(Start, Aim, 0.5f, false, false, true, false, false, FociSim::AssistLevel::Manual, Ideal, Vel, Spin);
	assert(NearlyEqual(Vel.Size(), 1900.0f, 5.0f));

	// Standard Shot
	FociSim::ComputeKickVelocityAndSpin3D(Start, Aim, 0.5f, true, false, false, false, false, FociSim::AssistLevel::Manual, Ideal, Vel, Spin);
	assert(NearlyEqual(Vel.Size(), 2600.0f, 5.0f));

	// Finesse Shot
	FociSim::ComputeKickVelocityAndSpin3D(Start, Aim, 0.5f, true, false, false, true, false, FociSim::AssistLevel::Manual, Ideal, Vel, Spin);
	assert(NearlyEqual(std::fabs(Spin.Z), 38.0f));

	// Chip Shot
	FociSim::ComputeKickVelocityAndSpin3D(Start, Aim, 0.5f, true, false, false, false, true, FociSim::AssistLevel::Manual, Ideal, Vel, Spin);
	assert(Vel.Z > Vel.X * 0.85f);

	// Assist levels
	FociSim::Vec3 VelAssisted, VelSemi, VelManual;
	FociSim::ComputeKickVelocityAndSpin3D(Start, Aim, 0.5f, false, false, false, false, false, FociSim::AssistLevel::Assisted, Ideal, VelAssisted, Spin);
	FociSim::ComputeKickVelocityAndSpin3D(Start, Aim, 0.5f, false, false, false, false, false, FociSim::AssistLevel::Semi, Ideal, VelSemi, Spin);
	FociSim::ComputeKickVelocityAndSpin3D(Start, Aim, 0.5f, false, false, false, false, false, FociSim::AssistLevel::Manual, Ideal, VelManual, Spin);
	assert(VelAssisted.Y > VelSemi.Y && VelSemi.Y > VelManual.Y);
}

void Test_4PlayerSwitching_NeverHijacksHumanTeammates()
{
	const std::vector<FociSim::Vec3> Locations = {
		FociSim::Vec3(0.0f, 0.0f, 96.0f),
		FociSim::Vec3(500.0f, 0.0f, 96.0f),
		FociSim::Vec3(-5000.0f, 0.0f, 96.0f),
		FociSim::Vec3(900.0f, 100.0f, 96.0f),
		FociSim::Vec3(400.0f, -1200.0f, 96.0f),
		FociSim::Vec3(-800.0f, 0.0f, 96.0f)
	};
	const std::vector<int32_t> HumanIndices = {0, 1, -1, -1, -1, -1};
	const std::vector<bool> IsGK = {false, false, true, false, false, false};

	const FociSim::Vec3 CurrLoc = Locations[0];
	const FociSim::Vec3 BallLoc(550.0f, 50.0f, 11.0f);

	// L1 switch
	assert(FociSim::SelectBestTeammateToSwitch3D(0, 0, CurrLoc, BallLoc, FociSim::Vec2(0.0f, 0.0f), Locations, HumanIndices, IsGK) == 3);
	// Right-Stick RIGHT flick
	assert(FociSim::SelectBestTeammateToSwitch3D(0, 0, CurrLoc, BallLoc, FociSim::Vec2(1.0f, 0.0f), Locations, HumanIndices, IsGK) == 3);
	// Right-Stick UP flick -> Broadcast World (0, -1, 0)
	assert(FociSim::SelectBestTeammateToSwitch3D(0, 0, CurrLoc, BallLoc, FociSim::Vec2(0.0f, 1.0f), Locations, HumanIndices, IsGK) == 4);
	// Right-Stick DOWN flick -> Broadcast World (0, +1, 0) -> no candidate in cone
	assert(FociSim::SelectBestTeammateToSwitch3D(0, 0, CurrLoc, BallLoc, FociSim::Vec2(0.0f, -1.0f), Locations, HumanIndices, IsGK) == -1);
}

void Test_SharedBroadcastCamera_4PlayerFraming()
{
	FociSim::Vec3 Focus;
	float Arm = 0.0f;
	const FociSim::Vec3 Ball(1000.0f, 200.0f, 11.0f);

	// Empty human list
	FociSim::ComputeBroadcastCameraFrame3D(Ball, {}, 0.60f, 2400.0f, 4600.0f, Focus, Arm);
	assert(NearlyEqual(Focus.X, 1000.0f) && NearlyEqual(Focus.Y, 200.0f) && NearlyEqual(Focus.Z, 60.0f));
	assert(NearlyEqual(Arm, 2400.0f));

	// Tightly clustered
	const std::vector<FociSim::Vec3> Tight = {
		FociSim::Vec3(950.0f, 180.0f, 96.0f),
		FociSim::Vec3(1050.0f, 220.0f, 96.0f),
		FociSim::Vec3(980.0f, 210.0f, 96.0f),
		FociSim::Vec3(1020.0f, 190.0f, 96.0f)
	};
	float TightArm = 0.0f;
	FociSim::ComputeBroadcastCameraFrame3D(Ball, Tight, 0.60f, 2400.0f, 4600.0f, Focus, TightArm);
	assert(NearlyEqual(TightArm, 2400.0f));

	// Widely spread
	const std::vector<FociSim::Vec3> Wide = {
		FociSim::Vec3(-2000.0f, -1500.0f, 96.0f),
		FociSim::Vec3(2000.0f, 1500.0f, 96.0f),
		FociSim::Vec3(-1500.0f, 1500.0f, 96.0f),
		FociSim::Vec3(1500.0f, -1500.0f, 96.0f)
	};
	float WideArm = 0.0f;
	FociSim::ComputeBroadcastCameraFrame3D(Ball, Wide, 0.60f, 2400.0f, 4600.0f, Focus, WideArm);
	assert(WideArm > TightArm);
	assert(NearlyEqual(Focus.X, 600.0f, 1.0f));
	assert(NearlyEqual(Focus.Y, 120.0f, 1.0f));
}

void Test_TacticalAI_FormationAndGoalkeeper3DIntercept()
{
	const FociSim::Vec3 Anchor(-1000.0f, 0.0f, 96.0f);
	const FociSim::Vec3 Ball(400.0f, 1000.0f, 11.0f);
	const FociSim::Vec3 AttackPos = FociSim::ComputeDynamicFormationTarget3D(Anchor, Ball, FociSim::TeamSide::Home, true);
	const FociSim::Vec3 DefendPos = FociSim::ComputeDynamicFormationTarget3D(Anchor, Ball, FociSim::TeamSide::Home, false);
	assert(AttackPos.X > DefendPos.X);
	assert(AttackPos.Y > 0.0f);

	FociSim::Vec3 Intercept;
	float T = 0.0f;
	const bool bSaveable = FociSim::ComputeGoalkeeperIntercept3D(
		FociSim::Vec3(3250.0f, 100.0f, 15.0f),
		FociSim::Vec3(2000.0f, 150.0f, 500.0f),
		5250.0f,
		366.0f,
		244.0f,
		Intercept,
		T
	);
	assert(bSaveable);
	assert(NearlyEqual(T, 1.0f));
	assert(NearlyEqual(Intercept.X, 5250.0f) && NearlyEqual(Intercept.Y, 250.0f) && NearlyEqual(Intercept.Z, 25.0f));

	assert(!FociSim::ComputeGoalkeeperIntercept3D(FociSim::Vec3(3250.0f, 100.0f, 15.0f), FociSim::Vec3(-2000.0f, 0.0f, 100.0f), 5250.0f, 366.0f, 244.0f, Intercept, T));
	assert(!FociSim::ComputeGoalkeeperIntercept3D(FociSim::Vec3(3250.0f, 100.0f, 15.0f), FociSim::Vec3(2000.0f, 0.0f, 1200.0f), 5250.0f, 366.0f, 244.0f, Intercept, T));
	assert(!FociSim::ComputeGoalkeeperIntercept3D(FociSim::Vec3(3250.0f, 100.0f, 15.0f), FociSim::Vec3(2000.0f, 800.0f, 100.0f), 5250.0f, 366.0f, 244.0f, Intercept, T));
}

} // namespace

int main()
{
	Test_MacBook2018_GraphicsPresets();
	Test_PS5DualSense_4PlayerRemappingAndConflictSwap();
	Test_3DBallPhysics_MagnusCurveAndTurfBounce();
	Test_3DPitchBoundary_GoalLineTechnologyAndOffside();
	Test_3DKickSolver_PassesAndShots();
	Test_4PlayerSwitching_NeverHijacksHumanTeammates();
	Test_SharedBroadcastCamera_4PlayerFraming();
	Test_TacticalAI_FormationAndGoalkeeper3DIntercept();

	std::cout << "All 8 Foci C++17 simulation test suites passed!" << std::endl;
	return 0;
}

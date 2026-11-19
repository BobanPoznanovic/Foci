#!/usr/bin/env python3
import json
import os
import re
import subprocess
import sys

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))

REQUIRED_FILES = [
    "Foci.uproject",
    "Source/Foci/Foci.Build.cs",
    "Config/DefaultEngine.ini",
    "Config/Mac/MacEngine.ini",
    "Config/DefaultInput.ini",
    "Source/Foci/Simulation/FociSimTypes.h",
    "Source/Foci/Simulation/FociScalabilitySim.h",
    "Source/Foci/Core/FociGraphicsScalability.h",
    "Source/Foci/Core/FociGraphicsScalability.cpp",
    "Source/Foci/Input/FociInputTypes.h",
    "Source/Foci/Input/FociControlsSaveGame.h",
    "Source/Foci/Simulation/FociInputSim.h",
    "Source/Foci/Input/FociInputProfileSubsystem.h",
    "Source/Foci/Input/FociInputProfileSubsystem.cpp",
    "Source/Foci/Simulation/FociBallPhysicsSim.h",
    "Source/Foci/Gameplay/FociBallActor.h",
    "Source/Foci/Gameplay/FociBallActor.cpp",
    "Source/Foci/Simulation/FociPitchSim.h",
    "Source/Foci/Gameplay/FociPitchVolume.h",
    "Source/Foci/Gameplay/FociPitchVolume.cpp",
    "Source/Foci/Simulation/FociKickSolverSim.h",
    "Source/Foci/Gameplay/FociFootballerCharacter.h",
    "Source/Foci/Gameplay/FociFootballerCharacter.cpp",
    "Source/Foci/FociCharacter.h",
    "Source/Foci/Simulation/FociSwitchingSim.h",
    "Source/Foci/FociPlayerController.h",
    "Source/Foci/FociPlayerController.cpp",
    "Source/Foci/Simulation/FociCameraSim.h",
    "Source/Foci/Gameplay/FociBroadcastCameraActor.h",
    "Source/Foci/Gameplay/FociBroadcastCameraActor.cpp",
    "Source/Foci/Simulation/FociAISim.h",
    "Source/Foci/AI/FociGoalkeeperCharacter.h",
    "Source/Foci/AI/FociGoalkeeperCharacter.cpp",
    "Source/Foci/AI/FociAIController.h",
    "Source/Foci/AI/FociAIController.cpp",
    "Source/Foci/Core/FociMatchGameState.h",
    "Source/Foci/Core/FociMatchGameState.cpp",
    "Source/Foci/Core/FociMatchGameMode.h",
    "Source/Foci/Core/FociMatchGameMode.cpp",
]

for rel in REQUIRED_FILES:
    full = os.path.join(REPO_ROOT, rel)
    assert os.path.isfile(full), f"Missing required file: {rel}"

with open(os.path.join(REPO_ROOT, "Foci.uproject"), "r", encoding="utf-8") as f:
    uproject = json.load(f)
plugins = {p["Name"]: p.get("Enabled", False) for p in uproject.get("Plugins", [])}
assert plugins.get("RawInput") is True, "RawInput plugin must be enabled"
assert plugins.get("UnrealMCP") is True, "UnrealMCP plugin must be enabled"

default_engine = open(os.path.join(REPO_ROOT, "Config/DefaultEngine.ini"), "r", encoding="utf-8").read()
mac_engine = open(os.path.join(REPO_ROOT, "Config/Mac/MacEngine.ini"), "r", encoding="utf-8").read()
default_input = open(os.path.join(REPO_ROOT, "Config/DefaultInput.ini"), "r", encoding="utf-8").read()
assert "r.AntiAliasingMethod=3" in default_engine and "bUseSplitscreen=False" in default_engine
assert "t.MaxFPS=60" in mac_engine
assert "0x054C" in default_input and "0x0CE6" in default_input

source_root = os.path.join(REPO_ROOT, "Source/Foci")
local_prefixes = ("Core/", "Input/", "Gameplay/", "AI/", "Simulation/", "FociPlayerController.h")

for root, _, files in os.walk(source_root):
    for name in files:
        if not (name.endswith(".h") or name.endswith(".cpp")):
            continue
        full_path = os.path.join(root, name)
        rel_path = os.path.relpath(full_path, source_root)
        text = open(full_path, "r", encoding="utf-8").read()

        # Check brace balance
        assert text.count("{") == text.count("}"), f"Unbalanced braces in {rel_path}"

        # Check local includes resolve
        includes = re.findall(r'^\s*#include\s+"([^"]+)"', text, flags=re.MULTILINE)
        for inc in includes:
            if inc.startswith(local_prefixes):
                inc_path = os.path.join(source_root, inc)
                assert os.path.isfile(inc_path), f"Unresolved local include '{inc}' in {rel_path}"

        # Check UE .h rules
        if name.endswith(".h") and not rel_path.startswith("Simulation/") and name != "Foci.h":
            pragma_pos = text.find("#pragma once")
            first_inc_pos = text.find("#include")
            assert pragma_pos != -1 and (first_inc_pos == -1 or pragma_pos < first_inc_pos), f"#pragma once must precede #include in {rel_path}"
            assert '"CoreMinimal.h"' in text, f"Missing CoreMinimal.h in {rel_path}"
            base = name[:-2]
            expected_gen = f"{base}.generated.h"
            assert includes and includes[-1] == expected_gen, f"Last include in {rel_path} must be {expected_gen}, got {includes[-1] if includes else 'none'}"
            assert not re.search(r"UPROPERTY\([^\)]*BlueprintRead[^\)]*\)\s*uint32\b", text), f"Forbidden Blueprint uint32 UPROPERTY in {rel_path}"

# Compile-check all 9 Simulation/*.h headers
sim_dir = os.path.join(source_root, "Simulation")
sim_headers = sorted(os.path.join(sim_dir, f) for f in os.listdir(sim_dir) if f.endswith(".h"))
assert len(sim_headers) == 9, f"Expected 9 Simulation headers, found {len(sim_headers)}"

cmd = [
    "clang++",
    "-std=c++17",
    "-fsyntax-only",
    "-Wall",
    "-Wextra",
    "-Werror",
    "-I",
    source_root,
    "-x",
    "c++-header",
] + sim_headers

res = subprocess.run(cmd, check=False)
if res.returncode != 0:
    sys.exit(res.returncode)

print("UE project & header integrity audit passed!")

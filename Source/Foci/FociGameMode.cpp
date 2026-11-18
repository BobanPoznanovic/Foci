// Copyright Foci Project. All Rights Reserved.

#include "FociGameMode.h"
#include "FociCharacter.h"
#include "FociPlayerController.h"

AFociGameMode::AFociGameMode()
{
	DefaultPawnClass = AFociCharacter::StaticClass();
	PlayerControllerClass = AFociPlayerController::StaticClass();
}

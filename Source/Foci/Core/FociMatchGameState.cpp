#include "Core/FociMatchGameState.h"

int32 AFociMatchGameState::GetMatchMinutes() const
{
	return FMath::FloorToInt(MatchClockSeconds / 60.0f);
}

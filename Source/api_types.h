#ifndef _API_TYPES_H
#define _API_TYPES_H

#include "../types.h"
#include <chrono>

typedef std::chrono::high_resolution_clock ApiClock;

struct api_Timer {
	std::chrono::time_point<ApiClock> nextTick;
	sol::function callback;
};

struct api_PlayerStruct {
	int id;
	PlayerStruct &ply;

	const char *getName()
	{
		return ply._pName;
	}

	int getHP()
	{
		return ply._pHitPoints;
	}

	void setHP(double value)
	{
		SetPlayerHitPoints(id, value);
	}

	int getGold()
	{
		return ply._pGold;
	}

	void setGold(double value)
	{
		// TODO: Fix
		ply._pGold = value;
	}

	void kill()
	{
		StartPlayerKill(id, 0);
	}
};

#endif

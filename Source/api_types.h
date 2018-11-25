#ifndef _API_TYPES_H
#define _API_TYPES_H

#include "../types.h"

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
};

#endif

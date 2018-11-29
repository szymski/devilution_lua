#ifndef _API_TYPES_H
#define _API_TYPES_H

#include "../types.h"
#include "api.h"
#include <chrono>
#include <unordered_set>

typedef std::chrono::high_resolution_clock ApiClock;

struct api_Timer {
	std::chrono::time_point<ApiClock> nextTick;
	sol::function callback;
};

struct api_PlayerStruct {
	int id;
	PlayerStruct &ply;

	api_PlayerStruct(int id, PlayerStruct &ply)
	    : id(id)
	    , ply(ply)
	{
	}

	const char *getName()
	{
		return ply._pName;
	}

	int getClass()
	{
		return ply._pClass;
	}

	int getHP()
	{
		return ply._pHitPoints;
	}

	void setHP(double value)
	{
		SetPlayerHitPoints(id, value);
	}

	int getMaxHP()
	{
		return ply._pMaxHP;
	}

	int getMana()
	{
		return ply._pMana;
	}

	void setMana(double value)
	{
		ply._pMana = value;
		ply._pManaBase = value + ply._pMaxManaBase - ply._pMaxMana;

		if (id == myplr)
			drawmanaflag = true;
	}

	int getMaxMana()
	{
		return ply._pMaxMana;
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

	int getLevel()
	{
		return ply.plrlevel;
	}

	std::tuple<double, double> getPos()
	{
		return std::make_tuple(ply.WorldX, ply.WorldY);
	}

	void setPos(double x, double y)
	{
		SetPlayerOld(id);
		ply.WorldX = x;
		ply.WorldY = y;
		FixPlayerLocation(id, ply._pdir);
		FixPlrWalkTags(id);
		dPlayer[plr[id].WorldX][plr[id].WorldY] = id + 1;

	}

	void kill()
	{
		StartPlayerKill(id, 0);
	}

	std::string to_string()
	{
		std::ostringstream ss;
		ss << "Player[" << id << "][" << ply._pName << "]";
		return ss.str();
	}
};

struct api_MonsterStruct {
	int id;
	MonsterStruct &monster;
	bool valid = true;

	api_MonsterStruct(int id, MonsterStruct &monster)
	    : id(id)
	    , monster(monster)
	{
	}

	void assertValid()
	{
		if (!isValid())
			throw std::runtime_error("Monster is not valid.");
	}

	bool isValid()
	{
		return valid;
	}

	const char *getName()
	{
		assertValid();
		return monster.mName;
	}

	int getHP()
	{
		assertValid();
		return monster._mhitpoints;
	}

	void setHP(double value)
	{
		assertValid();
		monster._mhitpoints = value;
	}

	int getLevel()
	{
		assertValid();
		return monster.mLevel;
	}

	void setLevel(double value)
	{
		assertValid();
		monster.mLevel = value;
	}

	std::tuple<double, double> getPos()
	{
		assertValid();
		return std::make_tuple(monster._mx, monster._my);
	}

	void setPos(double x, double y)
	{
		assertValid();
		monster._mx = x;
		monster._my = y;
	}

	void kill(sol::optional<api_PlayerStruct> player)
	{
		assertValid();
		if (!isValid())
			printf("Should not happen\n");

		if (player)
			MonstStartKill(id, player->id, false);
		else
			MonstStartKill(id, -1, false);
	}

	std::string to_string()
	{
		if (!isValid())
			return "Invalid monster";

		std::ostringstream ss;
		ss << "Monster[" << id << "][" << monster.mName << "]";
		return ss.str();
	}
};

#endif

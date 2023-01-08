#pragma once
#include "Exam_HelperStructs.h"
#include "stdafx.h"

//Extension
struct SearchPoint
{
	Elite::Vector2 Position{};
	bool IsVisited{ false };
};

struct GridElement : public SearchPoint
{
	float Influence{};
	std::vector<GridElement*> pNeighbors{};
};

struct House : public HouseInfo
{
	std::vector<SearchPoint*> pSearchPoints{};
	float TimeSinceVisit{};
	bool IsVisited{ false };
};

struct Item
{
	ItemInfo itemInfo;
	EntityInfo entityInfo;
	bool IsVisited{ false };
	House* pHouse{ nullptr };
};

struct PurgeZone: public PurgeZoneInfo
{
	float EstimatedLifeTime{};
};

struct SteeringPlugin_Output_Extension : SteeringPlugin_Output
{
	bool IsValid{ true };
};

struct Enemy : public EnemyInfo
{
	Elite::Vector2 PositionWhenSeen{};
};

enum class InventoryItemType
{
	Pistol = static_cast<int>(eItemType::PISTOL),
	Shotgun = static_cast<int>(eItemType::SHOTGUN),
	Medkit = static_cast<int>(eItemType::MEDKIT),
	Food = static_cast<int>(eItemType::FOOD),
	Empty = static_cast<int>(eItemType::GARBAGE)
};
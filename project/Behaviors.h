/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "EliteMath/EMath.h"
#include "EliteAI/EliteDecisionMaking/EliteBehaviorTree/EBehaviorTree.h"
#include "SteeringBehaviors.h"
#include "EliteAI/EliteData/EBlackboard.h"

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------

namespace BT_Actions
{
	Elite::BehaviorState InitializeRotating(Elite::Blackboard* pBlackboard)
	{
		std::cout << "Init rotation\n";
		bool* pIsRotating{};

		if (pBlackboard->GetData("IsRotating", pIsRotating) == false || pIsRotating == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		*pIsRotating = true;

		float* pStartOrientation{};
		if (pBlackboard->GetData("StartOrientation", pStartOrientation) == false || pStartOrientation == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		AgentInfo* pAgentInfo;
		if (pBlackboard->GetData("AgentInfo", pAgentInfo) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		Elite::Vector2 target{};
		if (pBlackboard->GetData("Target", target) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		RotateClockWise* pRotateClockWise;
		if (pBlackboard->GetData("RotateClockWise", pRotateClockWise) == false || pRotateClockWise == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		pRotateClockWise->SetTarget(target);

		*pStartOrientation = pAgentInfo->Orientation;
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState SetRotationCompleted(Elite::Blackboard* pBlackboard)
	{
		bool* pIsRotating{};

		if (pBlackboard->GetData("IsRotating", pIsRotating) == false || pIsRotating == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		*pIsRotating = false;
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState SetClosestNotVisitedSearchPointAsTarget(Elite::Blackboard* pBlackboard)
	{
		AgentInfo* pAgentInfo;
		if (pBlackboard->GetData("AgentInfo", pAgentInfo) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		House* pHouse{};
		if (pBlackboard->GetData("TargetHouse", pHouse) == false || pHouse == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		SearchPoint* pClosestSearchPoint{};
		float closestDistSq{ INFINITY };

		for (auto& pSearchPoint : pHouse->pSearchPoints)
		{
			if (pSearchPoint->isVisited == true) continue;

			float distSq = pSearchPoint->position.DistanceSquared(pAgentInfo->Position);

			if (distSq < closestDistSq)
			{
				closestDistSq = distSq;
				pClosestSearchPoint = pSearchPoint;
			}
		}

		pBlackboard->ChangeData("Target", pClosestSearchPoint->position);

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState SetClosestNotVisitedHouseAsTarget(Elite::Blackboard* pBlackboard)
	{
		AgentInfo* pAgentInfo;
		if (pBlackboard->GetData("AgentInfo", pAgentInfo) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		std::vector<House*>* pHouse;

		if (pBlackboard->GetData("HouseVector", pHouse) == false || pHouse == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		House* pClosestSearchPoint{};
		float closestDistSq{ INFINITY };

		for (auto& pHouse : *pHouse)
		{
			if (pHouse->isVisited == true) continue;

			float distSq = pHouse->Center.DistanceSquared(pAgentInfo->Position);

			if (distSq < closestDistSq)
			{
				closestDistSq = distSq;
				pClosestSearchPoint = pHouse;
			}
		}

		IExamInterface* pInterface;
		if (pBlackboard->GetData("Interface", pInterface) == false || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		const Elite::Vector2 target{ pInterface->NavMesh_GetClosestPathPoint(pClosestSearchPoint->Center) };

		pBlackboard->ChangeData("Target", target);
		pBlackboard->ChangeData("TargetHouse", pClosestSearchPoint);

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState SetClosestItemAsTarget(Elite::Blackboard* pBlackboard)
	{
		std::vector<std::pair<EntityInfo, ItemInfo>>* pItemVector;

		if (pBlackboard->GetData("ItemVector", pItemVector) == false || pItemVector == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		AgentInfo* pAgentInfo;

		if (pBlackboard->GetData("AgentInfo", pAgentInfo) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		std::pair<EntityInfo, ItemInfo> closestItem;
		float closestDistSq{ INFINITY };

		for (auto& item : *pItemVector)
		{
			float distSq = item.second.Location.DistanceSquared(pAgentInfo->Position);

			if (distSq < closestDistSq)
			{
				closestDistSq = distSq;

				closestItem.first = item.first;
				closestItem.second = item.second;
			}
		}

		pBlackboard->ChangeData("Target", closestItem.second.Location);
		pBlackboard->ChangeData("TargetItem", closestItem);

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToRotateClockWise(Elite::Blackboard* pBlackboard)
	{
		ISteeringBehavior* pCurrentSteering;

		RotateClockWise* pRotateClockWise;
		if (pBlackboard->GetData("RotateClockWise", pRotateClockWise) == false || pRotateClockWise == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		pCurrentSteering = pRotateClockWise;

		if (pBlackboard->ChangeData("CurrentSteering", pCurrentSteering))
		{
			return Elite::BehaviorState::Success;
		}

		return Elite::BehaviorState::Failure;
	}

	Elite::BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
	{
		ISteeringBehavior* pCurrentSteering;
		
		Wander* pWander;
		if (pBlackboard->GetData("Wander", pWander) == false || pWander == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		pCurrentSteering = pWander;

		if (pBlackboard->ChangeData("CurrentSteering", pCurrentSteering))
		{
			return Elite::BehaviorState::Success;
		}

		return Elite::BehaviorState::Failure;
	}
		
	Elite::BehaviorState GrabItem(Elite::Blackboard* pBlackboard)
	{

		IExamInterface* pInterface;

		if (pBlackboard->GetData("Interface", pInterface) == false || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		std::pair<EntityInfo,ItemInfo> item;

		if (pBlackboard->GetData("TargetItem", item) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		std::cout << item.second.ItemHash << '\n';

		if (pInterface->Item_Grab(item.first, item.second))
		{

			pInterface->Inventory_AddItem(0, item.second);

			std::vector<std::pair<EntityInfo, ItemInfo>>* pItemVector;

			if (pBlackboard->GetData("ItemVector", pItemVector) == false || pItemVector == nullptr)
			{
				return Elite::BehaviorState::Failure;
			}

			auto remover = [&](std::pair<EntityInfo, ItemInfo> itemInVector)->bool 
			{
				return item.first.EntityHash == itemInVector.first.EntityHash; 
			};

			pItemVector->erase(std::remove_if(pItemVector->begin(), pItemVector->end(), remover),pItemVector->end());

			return Elite::BehaviorState::Success;
		}

		return Elite::BehaviorState::Failure;
	}
	
	Elite::BehaviorState DestroyItem(Elite::Blackboard* pBlackboard)
	{

		IExamInterface* pInterface;

		if (pBlackboard->GetData("Interface", pInterface) == false || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		std::pair<EntityInfo, ItemInfo> item;

		if (pBlackboard->GetData("TargetItem", item) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		std::cout << item.second.ItemHash << '\n';

		if (pInterface->Item_Destroy(item.first))
		{
			std::vector<std::pair<EntityInfo, ItemInfo>>* pItemVector;

			if (pBlackboard->GetData("ItemVector", pItemVector) == false || pItemVector == nullptr)
			{
				return Elite::BehaviorState::Failure;
			}

			auto remover = [&](std::pair<EntityInfo, ItemInfo> itemInVector)->bool
			{
				return item.first.EntityHash == itemInVector.first.EntityHash;
			};

			pItemVector->erase(std::remove_if(pItemVector->begin(), pItemVector->end(), remover), pItemVector->end());

			return Elite::BehaviorState::Success;
		}

		return Elite::BehaviorState::Failure;
	}

	Elite::BehaviorState ChangeToSeekTarget(Elite::Blackboard* pBlackboard)
	{
		ISteeringBehavior* pCurrentSteering;

		Seek* pSeek;
		if (pBlackboard->GetData("Seek", pSeek) == false || pSeek == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		Elite::Vector2 target;
		if (pBlackboard->GetData("Target", target) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		pSeek->SetTarget(target);
		pCurrentSteering = pSeek;

		if (pBlackboard->ChangeData("CurrentSteering", pCurrentSteering))
		{
			return Elite::BehaviorState::Success;
		}
		else
		{
			return Elite::BehaviorState::Failure;
		}
	}

	Elite::BehaviorState MarkHouseAsVisited(Elite::Blackboard* pBlackboard)
	{
		House* pHouse{};
		if (pBlackboard->GetData("TargetHouse", pHouse) == false || pHouse == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}
		
		pHouse->isVisited = true;
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState MarkSearchPointAsVisited(Elite::Blackboard* pBlackboard)
	{
		House* pHouse{};
		if (pBlackboard->GetData("TargetHouse", pHouse) == false || pHouse == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		Elite::Vector2 target{};
		if (pBlackboard->GetData("Target", target) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		auto finder = [&](SearchPoint* pSearchPoint)->bool
		{
			return pSearchPoint->position == target;
		};

		auto iterator{ std::find_if(pHouse->pSearchPoints.begin(), pHouse->pSearchPoints.end(), finder) };

		if(iterator == pHouse->pSearchPoints.end())
		{ 
			return Elite::BehaviorState::Failure;
		}
		
		(*iterator)->isVisited = true;

		return Elite::BehaviorState::Success;
	}
}

//-----------------------------------------------------------------
// Conditions
//-----------------------------------------------------------------

namespace BT_Conditions
{
	bool IsItemInGrabRange(Elite::Blackboard* pBlackboard)
	{
		Elite::Vector2 target;

		if (pBlackboard->GetData("Target", target) == false)
		{
			return false;
		}

		AgentInfo* pAgentInfo{};

		if (pBlackboard->GetData("AgentInfo", pAgentInfo) == false || pAgentInfo == nullptr)
		{
			return false;
		}

		if (target.DistanceSquared(pAgentInfo->Position) <= pAgentInfo->GrabRange * pAgentInfo->GrabRange)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool IsItemInVector(Elite::Blackboard* pBlackboard)
	{
		std::vector<std::pair<EntityInfo, ItemInfo>>* pItemVector;

		if (pBlackboard->GetData("ItemVector", pItemVector) == false || pItemVector == nullptr)
		{
			return false;
		}
	
		return !pItemVector->empty();
	}

	bool IsAgentInsideThisHouse(Elite::Blackboard* pBlackboard)
	{
		Elite::Vector2 target;

		if (pBlackboard->GetData("Target", target) == false)
		{
			return false;
		}

		House* pHouse{};

		if (pBlackboard->GetData("TargetHouse", pHouse) == false)
		{
			return false;
		}

		return target == pHouse->Center;
	}

	bool IsNotVisitedHouseInVector(Elite::Blackboard* pBlackboard)
	{
		std::vector<House*>* pHouseVector;

		if (pBlackboard->GetData("HouseVector", pHouseVector) == false || pHouseVector == nullptr)
		{
			return false;
		}

		auto finder = [&](House* pHouseInVector)->bool
		{
			return pHouseInVector->isVisited == false;
		};

		//There are not yet visited houses
		return std::find_if(pHouseVector->begin(), pHouseVector->end(), finder) != pHouseVector->end();
	}

	bool IsNotVisitedSearchPointInHouse(Elite::Blackboard* pBlackboard)
	{
		House* pHouse{};

		if (pBlackboard->GetData("TargetHouse", pHouse) == false || pHouse == nullptr)
		{
			return false;
		}

		auto finder = [&](SearchPoint* pSearchPointInHouse)->bool
		{
			return pSearchPointInHouse->isVisited == false;
		};

		//There are not yet visited search points
		return std::find_if(pHouse->pSearchPoints.begin(), pHouse->pSearchPoints.end(), finder) != pHouse->pSearchPoints.end();
	}

	bool IsItemUsefull(Elite::Blackboard* pBlackboard)
	{
		std::pair<EntityInfo,ItemInfo> item;

		if (pBlackboard->GetData("TargetItem", item) == false)
		{
			return false;
		}

		if (item.second.Type == eItemType::GARBAGE)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	bool IsAgentNotRotating(Elite::Blackboard* pBlackboard)
	{

		bool* pIsRotating{};
		if (pBlackboard->GetData("IsRotating", pIsRotating) == false || pIsRotating == nullptr)
		{
			return false;
		}
		
		return !(*pIsRotating);
	}

	bool IsRotationCompleted(Elite::Blackboard* pBlackboard)
	{
		float* pStartOrientation{};
		if (pBlackboard->GetData("StartOrientation", pStartOrientation) == false || pStartOrientation == nullptr)
		{
			return false;
		}

		AgentInfo* pAgentInfo{};

		if (pBlackboard->GetData("AgentInfo", pAgentInfo) == false || pAgentInfo == nullptr)
		{
			return false;
		}

		const Elite::Vector2 startVector{ Elite::OrientationToVector(*pStartOrientation) };
		const Elite::Vector2 currentVector{ Elite::OrientationToVector(pAgentInfo->Orientation) };

		//Cross < 0.f -> left, Dot > 0.5f half of the positive side
		return Elite::Cross(startVector,currentVector) < -0.2f && Elite::Dot(startVector, currentVector) > 0.5f;
	}

	bool HasArrivedAtLocation(Elite::Blackboard* pBlackboard)
	{
		Elite::Vector2 target{};
		if (pBlackboard->GetData("Target", target) == false)
		{
			return false;
		}

		AgentInfo* pAgentInfo{};

		if (pBlackboard->GetData("AgentInfo", pAgentInfo) == false || pAgentInfo == nullptr)
		{
			return false;
		}

		return target.DistanceSquared(pAgentInfo->Position) < 4.f;
	}
}


#endif
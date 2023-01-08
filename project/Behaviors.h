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
	//Cells

	Elite::BehaviorState SetBestCellAsTarget(Elite::Blackboard* pBlackboard)
	{
		GridElement** pCell;

		if (pBlackboard->GetData("CurrentCell", pCell) == false || pCell == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		AgentInfo* pAgentInfo;

		if (pBlackboard->GetData("AgentInfo", pAgentInfo) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		IExamInterface* pInterface;
		if (pBlackboard->GetData("Interface", pInterface) == false || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		Elite::Vector2 target{};

		if ((*pCell)->IsVisited)
		{
			float bestInfluence{};

			for (GridElement* pGridElement : (*pCell)->pNeighbors)
			{
				if (pGridElement->IsVisited) continue;

				if (bestInfluence <= pGridElement->Influence)
				{
					bestInfluence = pGridElement->Influence;
					target = pGridElement->Position;
				}
			}

			if (bestInfluence < 0.2f)
			{
				std::vector<GridElement*>* pGrid{};

				if (pBlackboard->GetData("GridVector", pGrid) == false || pCell == nullptr)
				{
					return Elite::BehaviorState::Failure;
				}

				for (GridElement* pGridElement : *pGrid)
				{
					if (pGridElement->IsVisited) continue;

					if (bestInfluence > 0.f)
					{
						//Go for long distances only when it is worth it
						if (pGridElement->Influence < 0.6f) continue;
					}
					
					if (bestInfluence <= pGridElement->Influence)
					{
						bestInfluence = pGridElement->Influence;
						target = pGridElement->Position;
					}
				}
			}
		}
		else
		{
			target = (*pCell)->Position;
		}
		
		pBlackboard->ChangeData("Target", pInterface->NavMesh_GetClosestPathPoint(target));

		return Elite::BehaviorState::Success;
	}

	//Movement
	Elite::BehaviorState SetRunning(Elite::Blackboard* pBlackboard)
	{
		bool* pShouldRun{};

		if (pBlackboard->GetData("ShouldRun", pShouldRun) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		*pShouldRun = true;

		return Elite::BehaviorState::Success;
	}
	//Rotation

	Elite::BehaviorState UpdateRotation(Elite::Blackboard* pBlackboard)
	{
		bool* pIsCompleted{};

		if (pBlackboard->GetData("IsRotationCompleted", pIsCompleted) == false)
		{
			return Elite::BehaviorState::Failure;
		}

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

		Elite::Vector2 startVector{ Elite::OrientationToVector(*pStartOrientation) };
		Elite::Vector2 currentVector{ Elite::OrientationToVector(pAgentInfo->Orientation) };

		*pIsCompleted = Elite::Cross(startVector, currentVector) > 0.2f && Elite::Dot(startVector, currentVector) > 0.5f;

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ResetRotation(Elite::Blackboard* pBlackboard)
	{
		bool* pIsRotating{};

		if (pBlackboard->GetData("IsRotating", pIsRotating) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		*pIsRotating = false;

		bool* pIsCompleted{};

		if (pBlackboard->GetData("IsRotationCompleted", pIsCompleted) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		*pIsCompleted = false;

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState InitializeRotating(Elite::Blackboard* pBlackboard)
	{
		bool* pIsRotating{};

		if (pBlackboard->GetData("IsRotating", pIsRotating) == false)
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

		*pStartOrientation = pAgentInfo->Orientation;
		return Elite::BehaviorState::Success;
	}

	//Enemy
	Elite::BehaviorState HandleAttackFromBehind(Elite::Blackboard* pBlackboard)
	{
		float* pDeltaTime{};
		if (pBlackboard->GetData("DeltaTime", pDeltaTime) == false || pDeltaTime == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		float* pAlertedTime{};
		if (pBlackboard->GetData("AlertedTime", pAlertedTime) == false || pAlertedTime == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		AgentInfo* pAgentInfo{};
		if (pBlackboard->GetData("AgentInfo", pAgentInfo) == false || pAgentInfo == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (pAgentInfo->Bitten)
		{
			*pAlertedTime += *pDeltaTime;
			std::cout << "Attack from behind\n";

			AgentInfo* pAgentInfo;

			if (pBlackboard->GetData("AgentInfo", pAgentInfo) == false)
			{
				return Elite::BehaviorState::Failure;
			}

			pBlackboard->ChangeData("Target", pAgentInfo->Position - Elite::OrientationToVector(pAgentInfo->Orientation) * 100.f);

			return Elite::BehaviorState::Success;
		}
		else if (*pAlertedTime > 0.f && *pAlertedTime < 4.f)
		{
			*pAlertedTime += *pDeltaTime;
			return Elite::BehaviorState::Success;
		}
		else
		{
			*pAlertedTime = 0.f;
			return Elite::BehaviorState::Failure;
		}
	}

	Elite::BehaviorState SetClosestEnemyAsTarget(Elite::Blackboard* pBlackboard)
	{
		float* pAlertedTime{};
		if (pBlackboard->GetData("AlertedTime", pAlertedTime) == false || pAlertedTime == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		*pAlertedTime = 0.f;

		std::vector<EnemyInfo>* pEnemyVector;

		if (pBlackboard->GetData("EnemyVector", pEnemyVector) == false || pEnemyVector == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		AgentInfo* pAgentInfo;

		if (pBlackboard->GetData("AgentInfo", pAgentInfo) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		IExamInterface* pInterface;

		if (pBlackboard->GetData("Interface", pInterface) == false || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		EnemyInfo closestEnemy;
		float closestDistSq{ INFINITY };

		for (auto& enemy : *pEnemyVector)
		{
			float distSq = enemy.Location.DistanceSquared(pAgentInfo->Position);

			if (distSq < closestDistSq)
			{
				closestDistSq = distSq;

				closestEnemy = enemy;
			}
		}

		pBlackboard->ChangeData("Target", closestEnemy.Location);

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState HandleShooting(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pInterface;

		if (pBlackboard->GetData("Interface", pInterface) == false || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		std::vector<std::pair<int,InventoryItemType>>* pInventory;

		if (pBlackboard->GetData("Inventory", pInventory) == false || pInventory == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		Elite::Vector2 target;
		if (pBlackboard->GetData("Target", target) == false )
		{
			return Elite::BehaviorState::Failure;
		}

		AgentInfo* pAgent{};
		if (pBlackboard->GetData("AgentInfo", pAgent) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		UINT slotIndex{ pInventory->size() };
		int leastAmmo{ 1000 };
		int currentAmmo{};

		const float distanceToTarget{ pAgent->Position.DistanceSquared(target) };

		ItemInfo inventoryItem{};
		for (const auto& inventoryPair : *pInventory)
		{
			if (inventoryPair.second == InventoryItemType::Pistol)
			{
				currentAmmo = pInterface->Inventory_GetItem(inventoryPair.first, inventoryItem);

				if (currentAmmo < leastAmmo)
				{
					slotIndex = inventoryPair.first;
					leastAmmo = currentAmmo;
				}
			}
			else if (inventoryPair.second == InventoryItemType::Shotgun)
			{
				currentAmmo = 1000;

				if (currentAmmo < leastAmmo)
				{
					slotIndex = inventoryPair.first;
					leastAmmo = currentAmmo;
				}

				const float squaredFOVRange{ pAgent->FOV_Range * pAgent->FOV_Range };
				if (distanceToTarget < 0.7f * squaredFOVRange && distanceToTarget > 0.2f)
				{
					slotIndex = inventoryPair.first;
					break;
				}
			}
		}

		if (slotIndex == pInventory->size())
		{
			return Elite::BehaviorState::Failure;
		}
		else
		{
			if (!pInterface->Inventory_UseItem(slotIndex))
			{
				//Can't shoot -> empty shotgun
				(*pInventory)[slotIndex].second = InventoryItemType::Empty;
				pInterface->Inventory_RemoveItem(slotIndex);
			}
			else if (pInterface->Weapon_GetAmmo(inventoryItem) == 0)
			{
				//No ammo -> empty pistol
				(*pInventory)[slotIndex].second = InventoryItemType::Empty;
				pInterface->Inventory_RemoveItem(slotIndex);
			}

			return Elite::BehaviorState::Success;
		}
	}
	//Items

	Elite::BehaviorState SetClosestItemAsTarget(Elite::Blackboard* pBlackboard)
	{
		std::vector<Item*>* pItemVector;

		if (pBlackboard->GetData("ItemVector", pItemVector) == false || pItemVector == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		AgentInfo* pAgentInfo;

		if (pBlackboard->GetData("AgentInfo", pAgentInfo) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		IExamInterface* pInterface;

		if (pBlackboard->GetData("Interface", pInterface) == false || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		Item* pClosestItem{};
		float closestDistSq{ INFINITY };

		for (Item* pItem : *pItemVector)
		{
			if (pItem->IsVisited) continue;

			float distSq = pItem->itemInfo.Location.DistanceSquared(pAgentInfo->Position);

			if (distSq < closestDistSq)
			{
				closestDistSq = distSq;

				pClosestItem = pItem;
			}
		}
		
		if (pClosestItem)
		{
			pBlackboard->ChangeData("Target", pInterface->NavMesh_GetClosestPathPoint(pClosestItem->itemInfo.Location));
			pBlackboard->ChangeData("TargetItem", pClosestItem);

			return Elite::BehaviorState::Success;
		}
		else
		{
			return Elite::BehaviorState::Failure;
		}
	}

	Elite::BehaviorState DestroyItem(Elite::Blackboard* pBlackboard)
	{

		IExamInterface* pInterface;

		if (pBlackboard->GetData("Interface", pInterface) == false || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		Item* pItem;

		if (pBlackboard->GetData("TargetItem", pItem) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		if (pInterface->Item_Destroy(pItem->entityInfo))
		{
			std::vector<Item*>* pItemVector;

			if (pBlackboard->GetData("ItemVector", pItemVector) == false || pItemVector == nullptr)
			{
				return Elite::BehaviorState::Failure;
			}

			auto remover = [&](Item* pItemInVector)->bool
			{
				return pItem == pItemInVector;
			};

			pItemVector->erase(std::remove_if(pItemVector->begin(), pItemVector->end(), remover), pItemVector->end());

			return Elite::BehaviorState::Success;
		}

		return Elite::BehaviorState::Failure;
	}

	Elite::BehaviorState HandleItemGrabbing(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pInterface;

		if (pBlackboard->GetData("Interface", pInterface) == false || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		std::vector<std::pair<int, InventoryItemType>>* pInventory{};

		if (pBlackboard->GetData("Inventory", pInventory) == false || pInventory == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		Item* pItem;

		if (pBlackboard->GetData("TargetItem", pItem) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pInterface->Item_Grab(pItem->entityInfo, pItem->itemInfo))
		{
			return Elite::BehaviorState::Failure;
		}

		//Check if item is very important
		InventoryItemType neededItemType{ static_cast<InventoryItemType>(pItem->itemInfo.Type) };
		InventoryItemType notNeededItemType{ InventoryItemType::Empty };

		int nrShotguns{}, nrPistols{}, nrMedkits{}, nrFood{};

		for (const auto& inventoryPair : *pInventory)
		{
			if (static_cast<int>(inventoryPair.second) == static_cast<int>(pItem->itemInfo.Type) || inventoryPair.second == InventoryItemType::Empty)
			{
				neededItemType = InventoryItemType::Empty;
				break;
			}

			switch (inventoryPair.second)
			{
			case InventoryItemType::Pistol:
				++nrPistols;
				break;
			case InventoryItemType::Shotgun:
				++nrShotguns;
				break;
			case InventoryItemType::Medkit:
				++nrMedkits;
				break;
			case InventoryItemType::Food:
				++nrFood;
				break;
			}
		}

		if (neededItemType != InventoryItemType::Empty)
		{
			const int nrWeapons{ nrPistols + nrShotguns };

			int highestNumber{ (std::max)(nrFood,nrMedkits) };
			highestNumber = (std::max)(highestNumber, nrWeapons);
			
			if (highestNumber == nrFood)
			{
				notNeededItemType = InventoryItemType::Food;
			}
			else if (highestNumber == nrMedkits)
			{
				notNeededItemType = InventoryItemType::Medkit;
			}
			else if (nrPistols >= nrShotguns)
			{
				notNeededItemType = InventoryItemType::Pistol;
			}
			else
			{
				notNeededItemType = InventoryItemType::Shotgun;
			}
		}

		//Check for the worst slot
		UINT slotIndex{ pInventory->size() };
		int largerstDifference{};
		int currentDifference{};

		int lowest{1000};
		int current{};

		ItemInfo inventoryItem{};
		for (const auto& inventoryPair : *pInventory)
		{
			//Place in inventory
			if (inventoryPair.second == InventoryItemType::Empty)
			{
				slotIndex = inventoryPair.first;
				break;
			}
			//Replace worst item of the same type
			else if (static_cast<int>(inventoryPair.second) == static_cast<int>(pItem->itemInfo.Type))
			{
				pInterface->Inventory_GetItem(inventoryPair.first, inventoryItem);

				switch (inventoryItem.Type)
				{
				case eItemType::PISTOL:
					current = pInterface->Weapon_GetAmmo(inventoryItem);
					currentDifference = pInterface->Weapon_GetAmmo(pItem->itemInfo) - current;
					if (current > 4) continue;
					break;
				case eItemType::SHOTGUN:
					currentDifference = 1;
					break;
				case eItemType::FOOD:
					currentDifference = pInterface->Food_GetEnergy(pItem->itemInfo) - pInterface->Food_GetEnergy(inventoryItem);
					break;
				case eItemType::MEDKIT:
					currentDifference = pInterface->Medkit_GetHealth(pItem->itemInfo) - pInterface->Medkit_GetHealth(inventoryItem);
					break;
				}

				if (currentDifference > largerstDifference)
				{
					largerstDifference = currentDifference;
					slotIndex = inventoryPair.first;
				}
			}
			//Item is very important, replace worst item of another type
			else if (static_cast<int>(neededItemType) == static_cast<int>(pItem->itemInfo.Type) && notNeededItemType == inventoryPair.second)
			{
				std::cout << "item is very important!\n";
				pInterface->Inventory_GetItem(inventoryPair.first, inventoryItem);

				switch (inventoryItem.Type)
				{
				case eItemType::PISTOL:
					current = pInterface->Weapon_GetAmmo(inventoryItem);
				case eItemType::SHOTGUN:
					current = 5;
					break;
				case eItemType::FOOD:
					current = pInterface->Food_GetEnergy(inventoryItem);
					break;
				case eItemType::MEDKIT:
					current = pInterface->Medkit_GetHealth(inventoryItem);
					break;
				}

				if (current <= lowest)
				{
					lowest = current;
					slotIndex = inventoryPair.first;
				}
			}
		}

		//Add item
		if (slotIndex != pInventory->size())
		{
			//Remove old item
			if ((*pInventory)[slotIndex].second != InventoryItemType::Empty)
			{
				pInterface->Inventory_RemoveItem(slotIndex);
			}

			//Add new item to inventory
			(*pInventory)[slotIndex].second = static_cast<InventoryItemType>(pItem->itemInfo.Type);

			pInterface->Inventory_AddItem(slotIndex, pItem->itemInfo);

			//Remove from item list
			std::vector<Item*>* pItemVector;

			if (pBlackboard->GetData("ItemVector", pItemVector) == false || pItemVector == nullptr)
			{
				return Elite::BehaviorState::Failure;
			}

			auto remover = [&](Item* pItemInVector)->bool
			{
				return pItem == pItemInVector;
			};

			pItemVector->erase(std::remove_if(pItemVector->begin(), pItemVector->end(), remover), pItemVector->end());
		}
		//Leave on ground
		else
		{
			std::cout << "Leave on ground!\n";
			pItem->IsVisited = true;
		}

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState HandleFoodAndMedkitUsage(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pInterface;

		if (pBlackboard->GetData("Interface", pInterface) == false || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		std::vector<std::pair<int, InventoryItemType>>* pInventory{};

		if (pBlackboard->GetData("Inventory", pInventory) == false || pInventory == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		AgentInfo* pAgentInfo{};
		if (pBlackboard->GetData("AgentInfo", pAgentInfo) == false || pAgentInfo == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		ItemInfo inventoryItem{};
		
		//Medkit
		if (pAgentInfo->Health < 9)
		{
			const float neededHealth{ 10 - pAgentInfo->Health };

			for (auto& inventoryPair : *pInventory)
			{
				//Continue if not medkit
				if (inventoryPair.second != InventoryItemType::Medkit) continue;

				//Continue if to good
				pInterface->Inventory_GetItem(inventoryPair.first, inventoryItem);
				if (pInterface->Medkit_GetHealth(inventoryItem) > neededHealth) continue;

				//Use item
				pInterface->Inventory_UseItem(inventoryPair.first);

				//Remove if empty
				pInterface->Inventory_GetItem(inventoryPair.first, inventoryItem);
				if (pInterface->Medkit_GetHealth(inventoryItem) == 0)
				{
					inventoryPair.second = InventoryItemType::Empty;
					pInterface->Inventory_RemoveItem(inventoryPair.first);
				}
				
				return Elite::BehaviorState::Success;
			}
		}

		//Food
		if (pAgentInfo->Energy < 9)
		{
			const float neededEnergy{ 10 - pAgentInfo->Energy };

			for (auto& inventoryPair : *pInventory)
			{
				//Continue if not food
				if (inventoryPair.second != InventoryItemType::Food) continue;

				//Continue if to good
				pInterface->Inventory_GetItem(inventoryPair.first, inventoryItem);
				if (pInterface->Food_GetEnergy(inventoryItem) > neededEnergy) continue;

				//Use item
				pInterface->Inventory_UseItem(inventoryPair.first);

				//Remove if empty
				pInterface->Inventory_GetItem(inventoryPair.first, inventoryItem);
				if (pInterface->Food_GetEnergy(inventoryItem) == 0)
				{
					inventoryPair.second = InventoryItemType::Empty;
					pInterface->Inventory_RemoveItem(inventoryPair.first);
				}

				return Elite::BehaviorState::Success;
			}
		}

		return Elite::BehaviorState::Failure;
	}
	
	//Change Steering

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

	Elite::BehaviorState ChangeToFleeTarget(Elite::Blackboard* pBlackboard)
	{
		ISteeringBehavior* pCurrentSteering;

		Flee* pFlee;
		if (pBlackboard->GetData("Flee", pFlee) == false || pFlee == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		Elite::Vector2 target;
		if (pBlackboard->GetData("Target", target) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		pFlee->SetTarget(target);

		pCurrentSteering = pFlee;

		if (pBlackboard->ChangeData("CurrentSteering", pCurrentSteering))
		{
			return Elite::BehaviorState::Success;
		}

		return Elite::BehaviorState::Failure;
	}

	Elite::BehaviorState ChangeToFaceTarget(Elite::Blackboard* pBlackboard)
	{
		ISteeringBehavior* pCurrentSteering;

		Face* pFace;
		if (pBlackboard->GetData("Face", pFace) == false || pFace == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		Elite::Vector2 target;
		if (pBlackboard->GetData("Target", target) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		pFace->SetTarget(target);

		pCurrentSteering = pFace;

		if (pBlackboard->ChangeData("CurrentSteering", pCurrentSteering))
		{
			return Elite::BehaviorState::Success;
		}
		else
		{
			return Elite::BehaviorState::Failure;
		}
	}

	Elite::BehaviorState ChangeToFaceAndSeekTarget(Elite::Blackboard* pBlackboard)
	{
		ISteeringBehavior* pCurrentSteering;

		Seek* pSeek;
		if (pBlackboard->GetData("Seek", pSeek) == false || pSeek == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		Face* pFace;
		if (pBlackboard->GetData("Face", pFace) == false || pFace == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		PrioritySteering* pFaceAndSeek;
		if (pBlackboard->GetData("FaceAndSeek", pFaceAndSeek) == false || pFaceAndSeek == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		Elite::Vector2 target;
		if (pBlackboard->GetData("Target", target) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		pFace->SetTarget(target);
		pSeek->SetTarget(target);

		pCurrentSteering = pFaceAndSeek;

		if (pBlackboard->ChangeData("CurrentSteering", pCurrentSteering))
		{
			return Elite::BehaviorState::Success;
		}
		else
		{
			return Elite::BehaviorState::Failure;
		}
	}

	Elite::BehaviorState ChangeToFleeAndFaceTarget(Elite::Blackboard* pBlackboard)
	{
		ISteeringBehavior* pCurrentSteering;

		Flee* pFlee;
		if (pBlackboard->GetData("Flee", pFlee) == false || pFlee == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		Face* pFace;
		if (pBlackboard->GetData("Face", pFace) == false || pFace == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		AddedSteering* pFleeAndFace;
		if (pBlackboard->GetData("FleeAndFace", pFleeAndFace) == false || pFleeAndFace == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		Elite::Vector2 target;
		if (pBlackboard->GetData("Target", target) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		pFace->SetTarget(target);
		pFlee->SetTarget(target);

		pCurrentSteering = pFleeAndFace;

		if (pBlackboard->ChangeData("CurrentSteering", pCurrentSteering))
		{
			return Elite::BehaviorState::Success;
		}
		else
		{
			return Elite::BehaviorState::Failure;
		}
	}

	Elite::BehaviorState ChangeToWanderAndSeekTarget(Elite::Blackboard* pBlackboard)
	{
		ISteeringBehavior* pCurrentSteering;

		Seek* pSeek;
		if (pBlackboard->GetData("Seek", pSeek) == false || pSeek == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		Wander* pWander;
		if (pBlackboard->GetData("Wander", pWander) == false || pWander == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		AddedSteering* pWanderAndSeek;
		if (pBlackboard->GetData("WanderAndSeek", pWanderAndSeek) == false || pWanderAndSeek == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		Elite::Vector2 target;
		if (pBlackboard->GetData("Target", target) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		pWander->SetTarget(target);
		pSeek->SetTarget(target);

		pCurrentSteering = pWanderAndSeek;

		if (pBlackboard->ChangeData("CurrentSteering", pCurrentSteering))
		{
			return Elite::BehaviorState::Success;
		}
		else
		{
			return Elite::BehaviorState::Failure;
		}
	}

	Elite::BehaviorState ChangeToSeekAndFaceTarget(Elite::Blackboard* pBlackboard)
	{
		ISteeringBehavior* pCurrentSteering;

		Seek* pSeek;
		if (pBlackboard->GetData("Seek", pSeek) == false || pSeek == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		Face* pFace;
		if (pBlackboard->GetData("Face", pFace) == false || pFace == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		AddedSteering* pSeekAndFace;
		if (pBlackboard->GetData("SeekAndFace", pSeekAndFace) == false || pSeekAndFace == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		Elite::Vector2 target;
		if (pBlackboard->GetData("Target", target) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		AgentInfo* pAgentInfo{};
		if (pBlackboard->GetData("AgentInfo", pAgentInfo) == false || pAgentInfo == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		pFace->SetTarget(target);
		pSeek->SetTarget(target);

		pCurrentSteering = pSeekAndFace;

		if (pBlackboard->ChangeData("CurrentSteering", pCurrentSteering))
		{
			return Elite::BehaviorState::Success;
		}
		else
		{
			return Elite::BehaviorState::Failure;
		}
	}

	Elite::BehaviorState ChangeToSeekTargetAndFaceBack(Elite::Blackboard* pBlackboard)
	{
		ISteeringBehavior* pCurrentSteering;

		Seek* pSeek;
		if (pBlackboard->GetData("Seek", pSeek) == false || pSeek == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		Face* pFace;
		if (pBlackboard->GetData("Face", pFace) == false || pFace == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		AddedSteering* pSeekAndFace;
		if (pBlackboard->GetData("SeekAndFace", pSeekAndFace) == false || pSeekAndFace == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		Elite::Vector2 target;
		if (pBlackboard->GetData("Target", target) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		AgentInfo* pAgentInfo{};
		if (pBlackboard->GetData("AgentInfo", pAgentInfo) == false || pAgentInfo == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		const Elite::Vector2 faceTarget{ pAgentInfo->Position * 2.f - target };

		pFace->SetTarget(faceTarget);
		pSeek->SetTarget(target);

		pCurrentSteering = pSeekAndFace;

		if (pBlackboard->ChangeData("CurrentSteering", pCurrentSteering))
		{
			return Elite::BehaviorState::Success;
		}
		else
		{
			return Elite::BehaviorState::Failure;
		}
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
	//Purge Zones
	Elite::BehaviorState SetClosestPointOutsidePurgeZoneAsTarget(Elite::Blackboard* pBlackboard)
	{
		AgentInfo* pAgentInfo;
		if (pBlackboard->GetData("AgentInfo", pAgentInfo) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		std::vector<PurgeZone*>* pPurgeZones;

		if (pBlackboard->GetData("PurgeVector", pPurgeZones) == false || pPurgeZones == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		PurgeZone* pClosestPurgeZone{};
		float closestDistSq{ INFINITY };

		for (auto* pPurgeZone : *pPurgeZones)
		{
			float distSq = pPurgeZone->Center.DistanceSquared(pAgentInfo->Position);

			if (distSq < closestDistSq)
			{
				closestDistSq = distSq;
				pClosestPurgeZone = pPurgeZone;
			}
		}

		Elite::Vector2 direction{ pAgentInfo->Position - pClosestPurgeZone->Center };
		direction.Normalize();

		IExamInterface* pInterface;
		if (pBlackboard->GetData("Interface", pInterface) == false || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		pBlackboard->ChangeData("Target", pInterface->NavMesh_GetClosestPathPoint(pClosestPurgeZone->Center + direction * pClosestPurgeZone->Radius));

		return Elite::BehaviorState::Success;
	}
	//House
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
			if (pSearchPoint->IsVisited == true) continue;

			float distSq = pSearchPoint->Position.DistanceSquared(pAgentInfo->Position);

			if (distSq < closestDistSq)
			{
				closestDistSq = distSq;
				pClosestSearchPoint = pSearchPoint;
			}
		}

		IExamInterface* pInterface;

		if (pBlackboard->GetData("Interface", pInterface) == false || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		pBlackboard->ChangeData("Target", pInterface->NavMesh_GetClosestPathPoint(pClosestSearchPoint->Position));

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState SetClosestNotVisitedHouseAsTarget(Elite::Blackboard* pBlackboard)
	{
		AgentInfo* pAgentInfo;
		if (pBlackboard->GetData("AgentInfo", pAgentInfo) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		std::vector<House*>* pHouses;

		if (pBlackboard->GetData("HouseVector", pHouses) == false || pHouses == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		House* pClosestSearchPoint{};
		float closestDistSq{ INFINITY };

		for (auto& pHouse : *pHouses)
		{
			if (pHouse->IsVisited == true) continue;

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

		pBlackboard->ChangeData("Target", pInterface->NavMesh_GetClosestPathPoint(pClosestSearchPoint->Center));
		pBlackboard->ChangeData("TargetHouse", pClosestSearchPoint);

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState MarkHouseAsVisited(Elite::Blackboard* pBlackboard)
	{
		House* pHouse{};
		if (pBlackboard->GetData("TargetHouse", pHouse) == false || pHouse == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}
		
		pHouse->IsVisited = true;
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
			return pSearchPoint->Position == target;
		};

		auto iterator{ std::find_if(pHouse->pSearchPoints.begin(), pHouse->pSearchPoints.end(), finder) };

		if(iterator == pHouse->pSearchPoints.end())
		{ 
			return Elite::BehaviorState::Failure;
		}
		
		(*iterator)->IsVisited = true;

		return Elite::BehaviorState::Success;
	}
}

//-----------------------------------------------------------------
// Conditions
//-----------------------------------------------------------------

namespace BT_Conditions
{
	//Items
	
	bool IsItemNotGarbage(Elite::Blackboard* pBlackboard)
	{
		Item* pItem{};

		if (pBlackboard->GetData("TargetItem", pItem) == false)
		{
			return false;
		}

		if (pItem->itemInfo.Type == eItemType::GARBAGE)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	bool IsItemNotInGrabRange(Elite::Blackboard* pBlackboard)
	{
		Item* pItem{};

		if (pBlackboard->GetData("TargetItem", pItem) == false)
		{
			return false;
		}

		AgentInfo* pAgentInfo{};

		if (pBlackboard->GetData("AgentInfo", pAgentInfo) == false || pAgentInfo == nullptr)
		{
			return false;
		}

		IExamInterface* pInterface{};

		if (pBlackboard->GetData("Interface", pInterface) == false || pInterface == nullptr)
		{
			return false;
		}

		bool isInFov{ false };

		EntityInfo ei = {};
		for (int i = 0;; ++i)
		{
			if (pInterface->Fov_GetEntityByIndex(i, ei))
			{
				if (ei.EntityHash == pItem->entityInfo.EntityHash)
				{
					isInFov = true;
					break;
				}

				continue;
			}

			break;
		}

		if (pItem->itemInfo.Location.DistanceSquared(pAgentInfo->Position) <= pAgentInfo->GrabRange * pAgentInfo->GrabRange && isInFov)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	bool IsNotVisitedItemInVector(Elite::Blackboard* pBlackboard)
	{
		std::vector<Item*>* pItemVector;

		if (pBlackboard->GetData("ItemVector", pItemVector) == false || pItemVector == nullptr)
		{
			return false;
		}

		House* pTargetHouse{};

		if (pBlackboard->GetData("TargetHouse", pTargetHouse) == false || pTargetHouse == nullptr)
		{
			return false;
		}

		auto findNotVisited = [&](Item* pItem)->bool {return pItem->IsVisited == false && pItem->pHouse == pTargetHouse; };

		return pItemVector->end() != std::find_if(pItemVector->begin(), pItemVector->end(), findNotVisited);
	}

	//Enemy

	bool IsEnemyInVector(Elite::Blackboard* pBlackboard)
	{
		std::vector<EnemyInfo>* pEnemyVector;

		if (pBlackboard->GetData("EnemyVector", pEnemyVector) == false || pEnemyVector == nullptr)
		{
			return false;
		}

		return !pEnemyVector->empty();
	}

	//Purge Zones
	bool IsInPurgeZone(Elite::Blackboard* pBlackboard)
	{
		std::vector<PurgeZone*>* pPurgeZones{};
		if (pBlackboard->GetData("PurgeVector", pPurgeZones) == false || pPurgeZones == nullptr)
		{
			return false;
		}

		AgentInfo* pAgentInfo{};
		if (pBlackboard->GetData("AgentInfo", pAgentInfo) == false || pAgentInfo == nullptr)
		{
			return false;
		}

		for (PurgeZone* pPurgeZone : *pPurgeZones)
		{
			const float squaredDistance{ pAgentInfo->Position.DistanceSquared(pPurgeZone->Center) };
			if (squaredDistance - 50.f < pPurgeZone->Radius * pPurgeZone->Radius)
			{
				std::cout << "Is in purge zone!!\n";
				return true;
			}
		}
		
		return false;
	}

	//House
	bool IsAgentNotInsideTargetHouse(Elite::Blackboard* pBlackboard)
	{
		House* pHouse{};
		if (pBlackboard->GetData("TargetHouse", pHouse) == false || pHouse == nullptr)
		{
			return false;
		}

		AgentInfo* pAgentInfo{};
		if (pBlackboard->GetData("AgentInfo", pAgentInfo) == false || pAgentInfo == nullptr)
		{
			return false;
		}

		Elite::Vector2 halfSize{ pHouse->Size / 2.f };
		Elite::Vector2 distance{ pAgentInfo->Position - pHouse->Center };

		return !(abs(distance.x) < halfSize.x && abs(distance.y) < halfSize.y);
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
			return pHouseInVector->IsVisited == false;
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
			return pSearchPointInHouse->IsVisited == false;
		};

		//There are not yet visited search points
		return std::find_if(pHouse->pSearchPoints.begin(), pHouse->pSearchPoints.end(), finder) != pHouse->pSearchPoints.end();
	}

	//Arriving

	bool HasNotArrivedAtLocation(Elite::Blackboard* pBlackboard)
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

		return !(target.DistanceSquared(pAgentInfo->Position) < 25.f);
	}

	bool IsAimingFinished(Elite::Blackboard* pBlackboard)
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

		const float currentOrientation{ pAgentInfo->Orientation };
		const float desiredOrientation{ Elite::VectorToOrientation(target - pAgentInfo->Position) };

		return abs(currentOrientation - desiredOrientation) < 0.07f;
	}

	//Rotation

	bool ShouldLookBack(Elite::Blackboard* pBlackboard)
	{
		float* pDeltaTime{};
		if (pBlackboard->GetData("DeltaTime", pDeltaTime) == false || pDeltaTime == nullptr)
		{
			return false;
		}

		float* pWalkingTime{};
		if (pBlackboard->GetData("WalkingTime", pWalkingTime) == false || pWalkingTime == nullptr)
		{
			return false;
		}

		*pWalkingTime += *pDeltaTime;

		if (*pWalkingTime >= 7.f)
		{
			*pWalkingTime = 0.f;
			return false;
		}
		else if(*pWalkingTime >= 5.f)
		{
			return true;
		}
		else
		{
			return false;
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

	bool IsRotationNotCompleted(Elite::Blackboard* pBlackboard)
	{
		bool* pIsRotationCompleted{};

		if (pBlackboard->GetData("IsRotationCompleted", pIsRotationCompleted) == false)
		{
			return false;
		}

		return !(*pIsRotationCompleted);
	}

	bool HasNoWeapon(Elite::Blackboard* pBlackboard)
	{

		std::vector<std::pair<int, InventoryItemType>>* pInventory;

		if (pBlackboard->GetData("Inventory", pInventory) == false || pInventory == nullptr)
		{
			return false;
		}

		bool hasWeapon{ false };
		for (const auto& inventoryPair : *pInventory)
		{
			if (inventoryPair.second == InventoryItemType::Pistol || inventoryPair.second == InventoryItemType::Shotgun)
			{
				hasWeapon = true;
				break;
			}
		}

		return !hasWeapon;
	}
}


#endif
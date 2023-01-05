#include "stdafx.h"
#include "Plugin.h"
#include "IExamInterface.h"
#include "SteeringBehaviors.h"
#include "CombinedSteeringBehaviors.h"
#include "Behaviors.h"



using namespace std;

//Called only once (first)
void Plugin::DllInit()
{
}

//Called only once, during initialization
void Plugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	//Retrieving the interface
	//This interface gives you access to certain actions the AI_Framework can perform for you
	m_pInterface = static_cast<IExamInterface*>(pInterface);

	//Bit information about the plugin
	//Please fill this in!!
	info.BotName = "MinionExam";
	info.Student_FirstName = "Jan";
	info.Student_LastName = "Supierz";
	info.Student_Class = "2DAE15N";

	//World Grid
	WorldInfo world{ m_pInterface->World_GetInfo() };

	const Elite::Vector2 startPos{ world.Center - world.Dimensions / 2.f };

	constexpr int nrCells{ 9 };
	m_CellSize = world.Dimensions.x / nrCells;

	for (int x{}; x < nrCells; ++x)
	{
		for (int y{}; y < nrCells; ++y)
		{
			GridElement* pGridElement{ new GridElement };

			pGridElement->Position = startPos;

			pGridElement->Position.x += x * m_CellSize;
			pGridElement->Position.y += y * m_CellSize;

			m_pGrid.push_back(pGridElement);
			m_pCurrentGridElement = pGridElement;
		}
	}

	const float offset{ 10.f };
	for (size_t index{}; index < m_pGrid.size(); ++index)
	{
		for (size_t idx{}; idx < m_pGrid.size(); ++idx)
		{
			if (index == idx) continue;

			if (m_pGrid[index]->Position.Distance(m_pGrid[idx]->Position) < m_CellSize + offset)
			{
				m_pGrid[index]->pNeighbors.push_back(m_pGrid[idx]);
			}
		}
	}


	//Inventory
	m_NextFreeSlot = 0;

	//Called when the plugin is loaded
	m_pSeekBehaviour = new Seek();
	m_pFleeBehaviour = new Flee();
	m_pWanderBehaviour = new Wander();
	m_pFaceBehaviour = new Face();
	m_pRotateClockWiseBehaviour = new RotateClockWise();
	m_pFleeAndFaceBehaviour = new AddedSteering(std::vector<ISteeringBehavior*>{ m_pFleeBehaviour, m_pFaceBehaviour});
	m_pSeekAndFaceBehaviour = new AddedSteering(std::vector<ISteeringBehavior*>{ m_pSeekBehaviour, m_pFaceBehaviour});
	m_pFaceAndSeekBehaviour = new PrioritySteering(std::vector<ISteeringBehavior*>{ m_pFaceBehaviour, m_pSeekBehaviour});
	m_pBlackboard = CreateBlackboard();

	//Create BehaviorTree
	m_pBehaviourTree = new Elite::BehaviorTree(m_pBlackboard,
		new Elite::BehaviorSelector(
		{
			//Enemies
			new Elite::BehaviorSelector(
			{
				new Elite::BehaviorSequence(
				{
					new Elite::BehaviorConditional(BT_Conditions::IsEnemyInVector),
					new Elite::BehaviorAction(BT_Actions::SetClosestEnemyAsTarget),
					new Elite::BehaviorSelector(
					{
							//Aiming finished
							new Elite::BehaviorSequence(
							{
								new Elite::BehaviorConditional(BT_Conditions::IsAimingFinished),
								new Elite::BehaviorAction(BT_Actions::HandleShooting)
							}),
						//Run away and aim
						new Elite::BehaviorAction(BT_Actions::ChangeToFleeAndFaceTarget)
					}),
				}),
				new Elite::BehaviorSequence(
				{
					new Elite::BehaviorConditional(BT_Conditions::WasBitten),
					new Elite::BehaviorAction(BT_Actions::SetPositionBehindAsTarget),
					new Elite::BehaviorAction(BT_Actions::ChangeToFleeAndFaceTarget)
				})
			}),
			//Houses
			new Elite::BehaviorSequence(
			{
				new Elite::BehaviorConditional(BT_Conditions::IsNotVisitedHouseInVector),
				new Elite::BehaviorAction(BT_Actions::SetClosestNotVisitedHouseAsTarget),
				new Elite::BehaviorSelector(
				{
					//Explore the building
					new Elite::BehaviorSequence(
					{
						new Elite::BehaviorConditional(BT_Conditions::IsAgentInsideThisHouse),
						new Elite::BehaviorSelector(
						{
							//Items
							new Elite::BehaviorSequence(
							{
								new Elite::BehaviorConditional(BT_Conditions::IsNotVisitedItemInVector),
								new Elite::BehaviorAction(BT_Actions::SetClosestItemAsTarget),
								new Elite::BehaviorSelector(
								{
										//Item in grab range
										new Elite::BehaviorSequence(
										{
											new Elite::BehaviorConditional(BT_Conditions::IsItemInGrabRange),
											new Elite::BehaviorSelector(
											{
												//Grab usefull item
												new Elite::BehaviorSequence(
												{
													new Elite::BehaviorConditional(BT_Conditions::IsItemNotGarbage),
													new Elite::BehaviorAction(BT_Actions::HandleItemGrabbing)
												}),
												//Destroy not usefull item
												new Elite::BehaviorAction(BT_Actions::DestroyItem)
											})
										}),
									//Run to nearest item
									new Elite::BehaviorAction(BT_Actions::ChangeToFaceAndSeekTarget)
								}),
							}),
							//Building needs exploration
							new Elite::BehaviorSequence(
							{
								new Elite::BehaviorConditional(BT_Conditions::IsNotVisitedSearchPointInHouse),
								new Elite::BehaviorAction(BT_Actions::SetClosestNotVisitedSearchPointAsTarget),
								new Elite::BehaviorSelector(
								{
									//Has arrived
									new Elite::BehaviorSequence(
									{
										new Elite::BehaviorConditional(BT_Conditions::HasArrivedAtLocation),
										new Elite::BehaviorSelector(
										{
											//Start Rotating
											new Elite::BehaviorSequence(
											{
												new Elite::BehaviorConditional(BT_Conditions::IsAgentNotRotating),
												new Elite::BehaviorAction(BT_Actions::InitializeRotating),
												new Elite::BehaviorAction(BT_Actions::ChangeToRotateClockWise)
											}),
											//Stop Rotating
											new Elite::BehaviorSequence(
											{
												new Elite::BehaviorSelector(
												{
													new Elite::BehaviorConditional(BT_Conditions::IsRotationCompleted),
													new Elite::BehaviorConditional(BT_Conditions::IsEnoughTimeInHouse)
												}),
												new Elite::BehaviorAction(BT_Actions::SetRotationCompleted),
												new Elite::BehaviorAction(BT_Actions::MarkSearchPointAsVisited)
											}),
											//Keep rotating
											new Elite::BehaviorAction(BT_Actions::ChangeToRotateClockWise)
										})
									}),
									//Run to nearest search point
									new Elite::BehaviorAction(BT_Actions::ChangeToSeekTarget)
								}),
							}),
							//Mark house as visited 
							new Elite::BehaviorAction(BT_Actions::MarkHouseAsVisited)
						})
					}),
					//Run to nearest house
					new Elite::BehaviorAction(BT_Actions::ChangeToSeekTarget)
				})
			}),
			//Fallback to wander
			new Elite::BehaviorSequence(
			{
				new Elite::BehaviorSelector(
				{
					new Elite::BehaviorSequence(
					{
						new Elite::BehaviorConditional(BT_Conditions::IsInsideAHouse),
						new Elite::BehaviorAction(BT_Actions::SetTargetHouseDoorAsTarget),
						new Elite::BehaviorAction(BT_Actions::ChangeToSeekAndFaceTarget)
					}),
					new Elite::BehaviorSequence(
					{
						new Elite::BehaviorAction(BT_Actions::SetBestCellAsTarget),
						new Elite::BehaviorAction(BT_Actions::ChangeToSeekAndFaceTarget)
					})
				})
			})
		}
	));
}

//Called only once
void Plugin::DllShutdown()
{
	//Called wheb the plugin gets unloaded
	delete m_pBehaviourTree;

	delete m_pFleeBehaviour;
	delete m_pSeekBehaviour;
	delete m_pWanderBehaviour;
	delete m_pFaceBehaviour;
	delete m_pRotateClockWiseBehaviour;
	delete m_pSeekAndFaceBehaviour;
	delete m_pFaceAndSeekBehaviour;

	for (House* pHouse : m_pHouses)
	{
		for (SearchPoint* pSearchPoint :pHouse->pSearchPoints)
		{
			delete pSearchPoint;
			pSearchPoint = nullptr;
		}

		delete pHouse;
		pHouse = nullptr;
	}

	for (GridElement* pGridElement : m_pGrid)
	{
		delete pGridElement;
		pGridElement = nullptr;
	}
}

//Called only once, during initialization
void Plugin::InitGameDebugParams(GameDebugParams& params)
{
	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = false; //GodMode > You can't die, can be useful to inspect certain behaviors (Default = false)
	params.LevelFile = "GameLevel.gppl";
	params.AutoGrabClosestItem = true; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
	params.StartingDifficultyStage = 1;
	params.InfiniteStamina = false;
	params.SpawnDebugPistol = true;
	params.SpawnDebugShotgun = true;
	params.SpawnPurgeZonesOnMiddleClick = true;
	params.PrintDebugMessages = true;
	params.ShowDebugItemNames = true;
	params.Seed = 36;
}

//Only Active in DEBUG Mode
//(=Use only for Debug Purposes)
void Plugin::Update(float dt)
{
}

//Update
//This function calculates the new SteeringPlugin_Output, called once per frame
SteeringPlugin_Output Plugin::UpdateSteering(float dt)
{
	m_AgentInfo = m_pInterface->Agent_GetInfo();

	const float halfSide{ m_CellSize / 2.f };
	const float quarterSize{ m_CellSize / 4.f };

	for (GridElement* pGridElement : m_pGrid)
	{
		const float distanceX{ std::abs(m_AgentInfo.Position.x - pGridElement->Position.x) };
		const float distanceY{ std::abs(m_AgentInfo.Position.y - pGridElement->Position.y) };

		if (distanceX <= quarterSize && distanceY <= quarterSize)
		{
			pGridElement->IsVisited = true;

			if (distanceX <= halfSide && distanceY <= halfSide)
			{
				m_pCurrentGridElement = pGridElement;
				break;
			}
		}
	}

	bool shouldRecalculateInfluence{ false };


	auto vHousesInFOV = GetHousesInFOV();//uses m_pInterface->Fov_GetHouseByIndex(...)
	auto vEntitiesInFOV = GetEntitiesInFOV(); //uses m_pInterface->Fov_GetEntityByIndex(...)

	for (auto& e : vHousesInFOV)
	{
		auto compareHouse = [&](House* house) -> bool { return house->Center == e.Center; };

		if (std::find_if(m_pHouses.begin(), m_pHouses.end(), compareHouse) == m_pHouses.end()) 
		{
			shouldRecalculateInfluence = true;

			std::cout << "New house\n";
			House* pHouse{ new House };
			pHouse->Center = e.Center;
			pHouse->DoorLocation = e.Center;
			pHouse->Size = e.Size;
			pHouse->IsVisited = false;

			const float searchRange{ m_AgentInfo.FOV_Range * 2.f };

			Elite::Vector2 nrSearchPoints{ pHouse->Size / searchRange };

			int nrSearchPointsX{ static_cast<int>(nrSearchPoints.x) };
			int nrSearchPointsY{ static_cast<int>(nrSearchPoints.y) };

			nrSearchPointsX = (std::max)(nrSearchPointsX, 1);
			nrSearchPointsY = (std::max)(nrSearchPointsY, 1);
			
			Elite::Vector2 startPosition{ pHouse->Center - Elite::Vector2{(nrSearchPointsX / 2) * m_AgentInfo.FOV_Range,(nrSearchPointsY / 2) * m_AgentInfo.FOV_Range} };

			for (int x{}; x < nrSearchPointsX; ++x)
			{
				for (int y{}; y < nrSearchPointsY; ++y)
				{
					SearchPoint* pSearchPoint{ new SearchPoint };

					pSearchPoint->Position.x = startPosition.x + x * m_AgentInfo.FOV_Range;
					pSearchPoint->Position.y = startPosition.y + y * m_AgentInfo.FOV_Range;

					pHouse->pSearchPoints.push_back(pSearchPoint);

					std::cout << "New search point\n";
				}
			}

			m_pHouses.push_back(pHouse);
		}
	}

	m_Enemies.clear();

	for (auto& e : vEntitiesInFOV)
	{
		if (e.Type == eEntityType::ITEM)
		{
			ItemInfo itemInfo;
			m_pInterface->Item_GetInfo(e, itemInfo);

			auto compareItem = [&](Item* pItemInVec) -> bool { return pItemInVec->itemInfo.ItemHash == itemInfo.ItemHash; };

			if (std::find_if(m_pItems.begin(), m_pItems.end(), compareItem) == m_pItems.end())
			{
				std::cout << "New item\n";

				Item* pItem{ new Item };
				pItem->entityInfo = e;
				pItem->itemInfo = itemInfo;

				m_pItems.push_back(pItem);
			}
		}
		if (e.Type == eEntityType::ENEMY)
		{
			EnemyInfo enemyInfo;
			m_pInterface->Enemy_GetInfo(e, enemyInfo);

			m_Enemies.push_back(enemyInfo);
		}
	}

	m_pBehaviourTree->Update(dt);

	//Mark cell as visited
	for (GridElement* pGridElement : m_pGrid)
	{
		if (pGridElement->IsVisited) continue;

		float squaredDistanceToAgent{ pGridElement->Position.DistanceSquared(m_AgentInfo.Position) };

		if (squaredDistanceToAgent < 10.f)
		{
			pGridElement->IsVisited = true;
			continue;
		}
	}

	if (shouldRecalculateInfluence)
	{
		//Recalculate influence
		for (GridElement* pGridElement : m_pGrid)
		{
			pGridElement->Influence = 0.f;

			for (House* pHouse : m_pHouses)
			{
				const float nrCellsAway{ (pGridElement->Position.Distance(pHouse->Center)) / m_CellSize };
				pGridElement->Influence += 1.f / nrCellsAway;
			}
		}
	}

	for (auto& e : vHousesInFOV)
	{
		if (m_AgentInfo.IsInHouse)
		{
			auto compareHouse = [&](House* house) -> bool { return house->Center == e.Center; };

			auto iterator{ std::find_if(m_pHouses.begin(), m_pHouses.end(), compareHouse) };

			if (iterator != m_pHouses.end())
			{
				(*iterator)->TimeInside += dt;
				break;
			}
		}
	}
	
	SteeringPlugin_Output steering{};
	ISteeringBehavior* pCurrentSteering{};
	if (m_pBlackboard->GetData("CurrentSteering", pCurrentSteering) && pCurrentSteering)
	{
		steering = pCurrentSteering->CalculateSteering(dt, m_AgentInfo);
		m_pInterface->Draw_Direction(m_AgentInfo.Position, steering.LinearVelocity, 10.f, Elite::Vector3{ 1.f,1.f,1.f });
		//steering.RunMode = m_CanRun; //If RunMode is True > MaxLinSpd is increased for a limited time (till your stamina runs out)
	}
	
	return steering;
}

//This function should only be used for rendering debug elements
void Plugin::Render(float dt) const
{
	//This Render function should only contain calls to Interface->Draw_... functions
	for (House* pHouse : m_pHouses)
	{
		for (SearchPoint* pSearchPoint : pHouse->pSearchPoints)
		{
			if (pSearchPoint->IsVisited)
			{
				//White circle - visited
				m_pInterface->Draw_Circle(pSearchPoint->Position, 1.f, Elite::Vector3{ 1.f,1.f,1.f });
			}
			else
			{
				//Red circle - not visited 
				m_pInterface->Draw_Circle(pSearchPoint->Position, 2.f, Elite::Vector3{ 1.f,0.f,0.f });
			}
		}
		m_pInterface->Draw_Direction(pHouse->Center, pHouse->DoorLocation - pHouse->Center, 10.f, Elite::Vector3{ 0.f,1.f,0.f });
	}

	const float halfCell{ m_CellSize / 2.f };
	Elite::Vector3 color{};

	for (GridElement* pGridElement : m_pGrid)
	{
		if (pGridElement->IsVisited) continue;

		color.x = pGridElement->Influence;

		m_pInterface->Draw_Segment(pGridElement->Position + Elite::Vector2{ -halfCell, -halfCell }, pGridElement->Position + Elite::Vector2{ -halfCell,halfCell }, color);
		m_pInterface->Draw_Segment(pGridElement->Position + Elite::Vector2{ -halfCell, halfCell }, pGridElement->Position + Elite::Vector2{ halfCell,halfCell }, color);
		m_pInterface->Draw_Segment(pGridElement->Position + Elite::Vector2{ halfCell, halfCell }, pGridElement->Position + Elite::Vector2{ halfCell,-halfCell }, color);
		m_pInterface->Draw_Segment(pGridElement->Position + Elite::Vector2{ -halfCell, -halfCell }, pGridElement->Position + Elite::Vector2{ halfCell,-halfCell }, color);
	}
}

vector<HouseInfo> Plugin::GetHousesInFOV() const
{
	vector<HouseInfo> vHousesInFOV = {};

	HouseInfo hi = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetHouseByIndex(i, hi))
		{
			vHousesInFOV.push_back(hi);
			continue;
		}

		break;
	}

	return vHousesInFOV;
}

vector<EntityInfo> Plugin::GetEntitiesInFOV() const
{
	vector<EntityInfo> vEntitiesInFOV = {};

	EntityInfo ei = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetEntityByIndex(i, ei))
		{
			vEntitiesInFOV.push_back(ei);
			continue;
		}

		break;
	}

	return vEntitiesInFOV;
}

//////////////////////////////////////
//   Added
////////////////////////////////
Elite::Blackboard* Plugin::CreateBlackboard()
{
	Elite::Blackboard* pBlackboard = new Elite::Blackboard();
	pBlackboard->AddData("Interface", m_pInterface);

	//Vectors
	pBlackboard->AddData("ItemVector", &m_pItems);
	pBlackboard->AddData("EnemyVector", &m_Enemies);
	pBlackboard->AddData("HouseVector", &m_pHouses);
	pBlackboard->AddData("GridVector", &m_pGrid);
	pBlackboard->AddData("CellSize", m_CellSize);

	//Steerings
	pBlackboard->AddData("CurrentSteering", static_cast<ISteeringBehavior*>(nullptr));
	pBlackboard->AddData("Wander", m_pWanderBehaviour);
	pBlackboard->AddData("RotateClockWise", m_pRotateClockWiseBehaviour);
	pBlackboard->AddData("Seek", m_pSeekBehaviour);
	pBlackboard->AddData("Face", m_pFaceBehaviour);
	pBlackboard->AddData("Flee", m_pFleeBehaviour);
	pBlackboard->AddData("SeekAndFace", m_pSeekAndFaceBehaviour);
	pBlackboard->AddData("FaceAndSeek", m_pFaceAndSeekBehaviour);
	pBlackboard->AddData("FleeAndFace", m_pFleeAndFaceBehaviour);

	//Agent
	pBlackboard->AddData("AgentInfo", &m_AgentInfo);
	pBlackboard->AddData("IsRotating", &m_IsRotating);
	pBlackboard->AddData("StartOrientation", &m_StartOrientation);
	pBlackboard->AddData("NextFreeSlot", &m_NextFreeSlot);

	pBlackboard->AddData("CurrentCell", &m_pCurrentGridElement);

	//Targets
	Elite::Vector2 target{};
	pBlackboard->AddData("Target", target);
	pBlackboard->AddData("TargetItem", static_cast<Item*>(nullptr));
	pBlackboard->AddData("TargetHouse", static_cast<House*>(nullptr));

	return pBlackboard;
}
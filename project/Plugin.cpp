#include "stdafx.h"
#include "Plugin.h"
#include "IExamInterface.h"
#include "SteeringBehaviors.h"
#include "CombinedSteeringBehaviors.h"
#include "Behaviors.h"



using namespace std;

//Called only once
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

	//Called when the plugin is loaded
	m_pSeekBehaviour = new Seek();
	m_pWanderBehaviour = new Wander();
	m_pFaceBehaviour = new Face();
	m_pRotateClockWiseBehaviour = new RotateClockWise();
	m_pSeekAndFaceBehaviour = new AddedSteering(std::vector<ISteeringBehavior*>{ m_pSeekBehaviour, m_pFaceBehaviour});

	m_pBlackboard = CreateBlackboard();

	//Create BehaviorTree
	m_pBehaviourTree = new Elite::BehaviorTree(m_pBlackboard,
		new Elite::BehaviorSelector(
		{
			//Items
			new Elite::BehaviorSequence(
			{
				new Elite::BehaviorConditional(BT_Conditions::IsItemInVector),
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
								new Elite::BehaviorConditional(BT_Conditions::IsItemUsefull),
								new Elite::BehaviorAction(BT_Actions::GrabItem)
							}),
							//Destroy not usefull item
							new Elite::BehaviorAction(BT_Actions::DestroyItem)
						})
					}),
					//Run to nearest item
					new Elite::BehaviorAction(BT_Actions::ChangeToSeekTarget)
				}),
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
												new Elite::BehaviorConditional(BT_Conditions::IsRotationCompleted),
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
			new Elite::BehaviorAction(BT_Actions::ChangeToWander)
		}
	));
}

//Called only once
void Plugin::DllShutdown()
{
	//Called wheb the plugin gets unloaded
	delete m_pBehaviourTree;

	delete m_pSeekBehaviour;
	delete m_pWanderBehaviour;
	delete m_pFaceBehaviour;
	delete m_pRotateClockWiseBehaviour;
	delete m_pSeekAndFaceBehaviour;

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

	auto vHousesInFOV = GetHousesInFOV();//uses m_pInterface->Fov_GetHouseByIndex(...)
	auto vEntitiesInFOV = GetEntitiesInFOV(); //uses m_pInterface->Fov_GetEntityByIndex(...)

	for (auto& e : vHousesInFOV)
	{
		auto compareHouse = [&](House* house) -> bool { return house->Center == e.Center; };

		if (std::find_if(m_pHouses.begin(), m_pHouses.end(), compareHouse) == m_pHouses.end()) 
		{
			std::cout << "New house\n";
			House* pHouse{ new House };
			pHouse->Center = e.Center;
			pHouse->Size = e.Size;
			pHouse->isVisited = false;

			const float searchRange{ m_AgentInfo.FOV_Range * 2.f };

			Elite::Vector2 nrSearchPoints{ pHouse->Size / searchRange };

			int nrSearchPointsX{ static_cast<int>(nrSearchPoints.x) };
			int nrSearchPointsY{ static_cast<int>(nrSearchPoints.y) };

			Elite::Vector2 startPosition{ pHouse->Center - Elite::Vector2{nrSearchPointsX * m_AgentInfo.FOV_Range, nrSearchPointsY * m_AgentInfo.FOV_Range} };

			nrSearchPointsX = (std::max)(nrSearchPointsX, 1);
			nrSearchPointsY = (std::max)(nrSearchPointsY, 1);

			for (int indexX{}; indexX < nrSearchPointsX; ++indexX)
			{
				for (int indexY{}; indexY < nrSearchPointsY; ++indexY)
				{
					SearchPoint* pSearchPoint{ new SearchPoint };

					pSearchPoint->position.x = startPosition.x + indexX * m_AgentInfo.FOV_Range;
					pSearchPoint->position.y = startPosition.y + indexY * m_AgentInfo.FOV_Range;

					pHouse->pSearchPoints.push_back(pSearchPoint);

					std::cout << "New search point\n";
				}
			}

			m_pHouses.push_back(pHouse);
		}
	}

	for (auto& e : vEntitiesInFOV)
	{
		if (e.Type == eEntityType::ITEM)
		{
			ItemInfo itemInfo;
			m_pInterface->Item_GetInfo(e, itemInfo);

			auto compareHouse = [&](std::pair<EntityInfo,ItemInfo> x) -> bool { return x.second.ItemHash == itemInfo.ItemHash; };

			if (std::find_if(m_Items.begin(), m_Items.end(), compareHouse) == m_Items.end())
			{
				std::cout << "New item\n";
				m_Items.push_back(std::make_pair(e, itemInfo));
			}
		}
	}

	m_pBehaviourTree->Update(dt);

	//for (auto& e : vHousesInFOV)
	//{
	//	const Elite::Vector2 targetLocation = m_pInterface->NavMesh_GetClosestPathPoint(e.Center);
	//
	//	if (targetLocation == e.Center && !m_IsInsideHouse)
	//	{
	//		m_DoorLocation = agentInfo.Position;
	//		m_IsInsideHouse = true;
	//	}
	//
	//	m_pSeekBehaviour->SetTarget(targetLocation);
	//
	//
	//	if (m_IsInsideHouse)
	//	{
	//		m_pFaceBehaviour->SetTarget(m_DoorLocation);
	//	}
	//	else
	//	{
	//		m_pFaceBehaviour->SetTarget(e.Center);
	//	}
	//
	//	m_pCurrentSteering = m_pSeekAndFaceBehaviour;
	//}
	//
	//for (auto& e : vEntitiesInFOV)
	//{
	//	if (e.Type == eEntityType::PURGEZONE)
	//	{
	//		PurgeZoneInfo zoneInfo{};
	//		m_pInterface->PurgeZone_GetInfo(e, zoneInfo);
	//		//std::cout << "Purge Zone in FOV:" << e.Location.x << ", "<< e.Location.y << "---Radius: "<< zoneInfo.Radius << std::endl;
	//	}
	//
	//	if (e.Type == eEntityType::ITEM)
	//	{
	//		ItemInfo itemInfo{};
	//		m_pInterface->Item_GetInfo(e, itemInfo);
	//
	//		m_pSeekBehaviour->SetTarget(e.Location);
	//		m_pFaceBehaviour->SetTarget(e.Location);
	//		m_pCurrentSteering = m_pSeekAndFaceBehaviour;
	//
	//		if (agentInfo.Position.DistanceSquared(e.Location) <= agentInfo.GrabRange * agentInfo.GrabRange)
	//		{
	//			if (itemInfo.Type == eItemType::GARBAGE)
	//			{
	//				m_pInterface->Item_Destroy(e);
	//			}
	//			else
	//			{
	//				m_pInterface->Item_Grab(e, itemInfo);
	//
	//				if (m_InventorySlot < 5)
	//				{
	//					m_pInterface->Inventory_AddItem(m_InventorySlot++, itemInfo);
	//				}
	//			}
	//		}
	//	}
	//
	//	if (e.Type == eEntityType::ENEMY)
	//	{
	//		EnemyInfo enemyInfo{};
	//		m_pInterface->Enemy_GetInfo(e, enemyInfo);
	//
	//		m_pFaceBehaviour->SetTarget(e.Location);
	//		m_pCurrentSteering = m_pFaceBehaviour;
	//
	//		if (m_pFaceBehaviour->CalculateSteering(dt, agentInfo).AngularVelocity == 0.f)
	//		{
	//			ItemInfo itemInfo{};
	//
	//			m_pInterface->Inventory_GetItem(1, itemInfo);
	//			m_pInterface->Inventory_UseItem(1);
	//		}
	//	}
	//}
	//
	//
	//if (vEntitiesInFOV.size() == 0 && !m_IsEnemyNearBy && vHousesInFOV.size() == 0)
	//{
	//	m_pCurrentSteering = m_pWanderBehaviour;
	//}
	//
	////Was bitten from behind
	//if (vEntitiesInFOV.size() == 0 && agentInfo.WasBitten)
	//{
	//	m_pSeekBehaviour->SetTarget(agentInfo.Position + Elite::OrientationToVector(agentInfo.Orientation) * 100.f);
	//	m_pFaceBehaviour->SetTarget(agentInfo.Position - Elite::OrientationToVector(agentInfo.Orientation) * 100.f);
	//
	//	m_pCurrentSteering = m_pSeekAndFaceBehaviour;
	//
	//	m_IsEnemyNearBy = true;
	//}
	
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
			if (pSearchPoint->isVisited)
			{
				//White circle - visited
				m_pInterface->Draw_Circle(pSearchPoint->position, 1.f, Elite::Vector3{ 1.f,1.f,1.f });
			}
			else
			{
				//Red circle - not visited 
				m_pInterface->Draw_Circle(pSearchPoint->position, 4.f, Elite::Vector3{ 1.f,0.f,0.f });
			}
		}
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
	pBlackboard->AddData("CurrentSteering", static_cast<ISteeringBehavior*>(nullptr));
	pBlackboard->AddData("ItemVector", &m_Items);
	pBlackboard->AddData("HouseVector", &m_pHouses);
	pBlackboard->AddData("Wander", m_pWanderBehaviour);
	pBlackboard->AddData("RotateClockWise", m_pRotateClockWiseBehaviour);
	pBlackboard->AddData("Seek", m_pSeekBehaviour);
	pBlackboard->AddData("SeekAndFace", m_pSeekAndFaceBehaviour);
	pBlackboard->AddData("AgentInfo", &m_AgentInfo);
	pBlackboard->AddData("IsRotating", &m_IsRotating);
	pBlackboard->AddData("StartOrientation", &m_StartOrientation);

	Elite::Vector2 target{};
	pBlackboard->AddData("Target", target);

	std::pair<EntityInfo,ItemInfo> item{};
	pBlackboard->AddData("TargetItem", item);
	pBlackboard->AddData("TargetHouse", static_cast<House*>(nullptr));

	return pBlackboard;
}
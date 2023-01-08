#pragma once
#include "IExamPlugin.h"
#include "Extensions.h"
#include "EliteAI/EliteDecisionMaking/EDecisionMaking.h"

class IBaseInterface;
class IExamInterface;

class Seek;
class Wander;
class Face;
class Flee;
class RotateClockWise;
class AddedSteering;
class PrioritySteering;
class ISteeringBehavior;

class Plugin :public IExamPlugin
{
public:
	Plugin() {};
	virtual ~Plugin() {};

	void Initialize(IBaseInterface* pInterface, PluginInfo& info) override;
	void DllInit() override;
	void DllShutdown() override;

	void InitGameDebugParams(GameDebugParams& params) override;
	void Update(float dt) override;

	SteeringPlugin_Output UpdateSteering(float dt) override;
	void Render(float dt) const override;

	
private:
	//Interface, used to request data from/perform actions with the AI Framework
	IExamInterface* m_pInterface = nullptr;
	std::vector<HouseInfo> GetHousesInFOV() const;
	std::vector<EntityInfo> GetEntitiesInFOV() const;

	//Added private functions
	Elite::Blackboard* CreateBlackboard();

	//Added variables
	Elite::BehaviorTree* m_pBehaviourTree{};
	Elite::Blackboard* m_pBlackboard{};

	Seek* m_pSeekBehaviour{};
	Wander* m_pWanderBehaviour{};
	Face* m_pFaceBehaviour{};
	Flee* m_pFleeBehaviour{};
	RotateClockWise* m_pRotateClockWiseBehaviour{};
	AddedSteering* m_pWanderAndSeekBehaviour{};
	AddedSteering* m_pSeekAndFaceBehaviour{};
	AddedSteering* m_pFleeAndFaceBehaviour{};
	PrioritySteering* m_pFaceAndSeekBehaviour{};

	//AgentInfo
	AgentInfo m_AgentInfo{};
	bool m_ShouldRun{ false };

	//Vectors
	std::vector <Item*> m_pItems{};
	std::vector <PurgeZone*> m_pPurgeZones{};
	std::vector<House*> m_pHouses{};
	std::vector<GridElement*> m_pGrid{};
	std::vector<EnemyInfo> m_Enemies{};
	std::vector<std::pair<int, InventoryItemType>> m_Inventory{};

	//Rotation
	bool m_IsRotating{false};
	bool m_IsRotationCompleted{ false };
	float m_StartOrientation{};

	//Grid
	GridElement* m_pCurrentGridElement{};
	float m_CellSize{};

	//Timers
	float m_DeltaTime{};
	float m_WalkingTime{};
	float m_AlertedTime{};
};

//ENTRY
//This is the first function that is called by the host program
//The plugin returned by this function is also the plugin used by the host program
extern "C"
{
	__declspec (dllexport) IPluginBase* Register()
	{
		return new Plugin();
	}
}
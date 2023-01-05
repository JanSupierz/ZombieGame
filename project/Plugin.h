#pragma once
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"

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

	UINT m_InventorySlot = 0;

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
	AddedSteering* m_pSeekAndFaceBehaviour{};
	AddedSteering* m_pFleeAndFaceBehaviour{};
	PrioritySteering* m_pFaceAndSeekBehaviour{};

	//AgentInfo
	AgentInfo m_AgentInfo{};
	bool m_IsEnemyNearBy = false;

	float m_TimeSinceBitten{};

	//Pair entityInfo + itemInfo
	std::vector <Item*> m_pItems{};
	//Pair houseInfo + isVisited
	std::vector<House*> m_pHouses{};
	std::vector<GridElement*> m_pGrid{};
	std::vector<EnemyInfo> m_Enemies{};
	float m_CellSize{};

	bool m_IsInsideHouse{};

	bool m_IsRotating{false};
	float m_StartOrientation{};

	UINT m_NextFreeSlot{};

	GridElement* m_pCurrentGridElement{};
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
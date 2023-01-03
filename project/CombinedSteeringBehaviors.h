#pragma once
#include "SteeringBehaviors.h"


//****************
//BLENDED STEERING
class BlendedSteering final: public ISteeringBehavior
{
public:
	struct WeightedBehavior
	{
		ISteeringBehavior* pBehavior = nullptr;
		float weight = 0.f;

		WeightedBehavior(ISteeringBehavior* pBehavior, float weight) :
			pBehavior(pBehavior),
			weight(weight)
		{};
	};

	friend class TheFlock;

	BlendedSteering(std::vector<WeightedBehavior> weightedBehaviors);

	void AddBehaviour(WeightedBehavior weightedBehavior) { m_WeightedBehaviors.push_back(weightedBehavior); }
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo& agentInfo) override;

	// returns a reference to the weighted behaviors, can be used to adjust weighting. Is not intended to alter the behaviors themselves.
	std::vector<WeightedBehavior>& GetWeightedBehaviorsRef() { return m_WeightedBehaviors; }

private:
	std::vector<WeightedBehavior> m_WeightedBehaviors = {};

	using ISteeringBehavior::SetTarget; // made private because targets need to be set on the individual behaviors, not the combined behavior
};

//*****************
//PRIORITY STEERING
class PrioritySteering final: public ISteeringBehavior
{
public:
	PrioritySteering(std::vector<ISteeringBehavior*> priorityBehaviors)
		:m_PriorityBehaviors(priorityBehaviors) 
	{}

	void AddBehaviour(ISteeringBehavior* pBehavior) { m_PriorityBehaviors.push_back(pBehavior); }
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo& agentInfo) override;

private:
	std::vector<ISteeringBehavior*> m_PriorityBehaviors = {};

	using ISteeringBehavior::SetTarget; // made private because targets need to be set on the individual behaviors, not the combined behavior
};

//*****************
//ADDED STEERING
class AddedSteering final : public ISteeringBehavior
{
public:
	AddedSteering(std::vector<ISteeringBehavior*> addedBehaviors)
		:m_AddedBehaviors(addedBehaviors)
	{}

	void AddBehaviour(ISteeringBehavior* pBehavior) { m_AddedBehaviors.push_back(pBehavior); }
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo& agentInfo) override;

private:
	std::vector<ISteeringBehavior*> m_AddedBehaviors = {};

	using ISteeringBehavior::SetTarget; // made private because targets need to be set on the individual behaviors, not the combined behavior
};

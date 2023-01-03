#include "stdafx.h"
#include "CombinedSteeringBehaviors.h"
#include <algorithm>
#include "Exam_HelperStructs.h"

BlendedSteering::BlendedSteering(std::vector<WeightedBehavior> weightedBehaviors)
	:m_WeightedBehaviors(weightedBehaviors)
{
};

//****************
//BLENDED STEERING
SteeringPlugin_Output BlendedSteering::CalculateSteering(float deltaT, AgentInfo& agentInfo)
{
	SteeringPlugin_Output blendedSteering = {};
	auto totalWeight = 0.f;

	for (auto weightedBehavior : m_WeightedBehaviors)
	{
		auto steering = weightedBehavior.pBehavior->CalculateSteering(deltaT, agentInfo);
		blendedSteering.LinearVelocity += weightedBehavior.weight * steering.LinearVelocity;
		blendedSteering.AngularVelocity += weightedBehavior.weight * steering.AngularVelocity;

		totalWeight += weightedBehavior.weight;
	}

	if (totalWeight > 0.f)
	{
		auto scale = 1.f / totalWeight;

		blendedSteering.LinearVelocity = scale * blendedSteering.LinearVelocity;
		blendedSteering.AngularVelocity = scale * blendedSteering.AngularVelocity;
	}

	return blendedSteering;
}

//*****************
//PRIORITY STEERING
SteeringPlugin_Output PrioritySteering::CalculateSteering(float deltaT, AgentInfo& agentInfo)
{
	SteeringPlugin_Output steering = {};

	for (auto pBehavior : m_PriorityBehaviors)
	{
		steering = pBehavior->CalculateSteering(deltaT, agentInfo);
		//if(pBehavior.IsValid)
		break;
	}

	//If non of the behavior return a valid output, last behavior is returned
	return steering;
}

//*****************
//ADDED STEERING
SteeringPlugin_Output AddedSteering::CalculateSteering(float deltaT, AgentInfo& agentInfo)
{
	SteeringPlugin_Output steering = {};

	for (auto pBehavior : m_AddedBehaviors)
	{
		SteeringPlugin_Output partialSteering = pBehavior->CalculateSteering(deltaT, agentInfo);;

		steering.LinearVelocity += partialSteering.LinearVelocity;
		steering.AngularVelocity += partialSteering.AngularVelocity;

	}

	return steering;
}
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
SteeringPlugin_Output_Extension BlendedSteering::CalculateSteering(float deltaT, AgentInfo& agentInfo)
{
	SteeringPlugin_Output_Extension blendedSteering = {};
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
SteeringPlugin_Output_Extension PrioritySteering::CalculateSteering(float deltaT, AgentInfo& agentInfo)
{
	SteeringPlugin_Output_Extension steering = {};

	for (auto pBehavior : m_PriorityBehaviors)
	{
		steering = pBehavior->CalculateSteering(deltaT, agentInfo);
		if(steering.IsValid) break;
	}

	//If non of the behavior return a valid output, last behavior is returned
	return steering;
}

//*****************
//ADDED STEERING
SteeringPlugin_Output_Extension AddedSteering::CalculateSteering(float deltaT, AgentInfo& agentInfo)
{
	SteeringPlugin_Output_Extension steering = {};

	for (auto pBehavior : m_AddedBehaviors)
	{
		SteeringPlugin_Output_Extension partialSteering = pBehavior->CalculateSteering(deltaT, agentInfo);;

		steering.LinearVelocity += partialSteering.LinearVelocity;
		steering.AngularVelocity += partialSteering.AngularVelocity;
		steering.IsValid = steering.IsValid && partialSteering.IsValid;
		steering.AutoOrient = steering.AutoOrient && partialSteering.AutoOrient;
		steering.RunMode = steering.RunMode && partialSteering.RunMode;
	}

	return steering;
}
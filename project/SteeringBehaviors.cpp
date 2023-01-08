//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "IExamPlugin.h"
#include "EliteMath\EMatrix2x3.h"

using namespace Elite;

//SEEK
//****

SteeringPlugin_Output_Extension Seek::CalculateSteering(float deltaT, AgentInfo& agentInfo)
{
	SteeringPlugin_Output_Extension steering = {};

	steering.LinearVelocity = m_Target.Position - agentInfo.Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed;

	return steering;
}

//FLEE
//****
SteeringPlugin_Output_Extension Flee::CalculateSteering(float deltaT, AgentInfo& agentInfo)
{
	SteeringPlugin_Output_Extension steering = {};

	steering.LinearVelocity = agentInfo.Position - m_Target.Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed;

	return steering;
}

//ARRIVE
//****
SteeringPlugin_Output_Extension Arrive::CalculateSteering(float deltaT, AgentInfo& agentInfo)
{
	const float stopDistance{ 2.f };
	const float slowRadius{ 15.f };
	SteeringPlugin_Output_Extension steering = {};

	steering.LinearVelocity = m_Target.Position - agentInfo.Position;

	const float distance = steering.LinearVelocity.Magnitude();

	if (distance > stopDistance)
	{
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= agentInfo.MaxLinearSpeed;

		if (distance < slowRadius)
		{
			steering.LinearVelocity *= (distance / slowRadius);
		}
	}
	else
	{
		steering.LinearVelocity = { 0.f,0.f };
	}

	return steering;
}

//FACE
//****
SteeringPlugin_Output_Extension Face::CalculateSteering(float deltaT, AgentInfo& agentInfo)
{
	SteeringPlugin_Output_Extension steering = {};

	const Vector2 desiredDirection{ m_Target.Position - agentInfo.Position };
	const Vector2 currentDirection{ OrientationToVector(agentInfo.Orientation) };

	const float angle{ AngleBetween(currentDirection, desiredDirection) };
	const float slowAngle{ 0.2f };
	const float stopAngle{ 0.07f };

	if (abs(angle) < slowAngle)
	{
		if (abs(angle) <= stopAngle)
		{
			steering.IsValid = false;
			steering.AngularVelocity = 0.f;
		}
		else
		{
			steering.AngularVelocity = (angle < 0 ? -1 : 1) * agentInfo.MaxAngularSpeed * abs(angle)/slowAngle;
		}
	}
	else
	{
		steering.AngularVelocity = (angle < 0 ? -1 : 1) * agentInfo.MaxAngularSpeed;
	}

	steering.AutoOrient = false;

	return steering;
}

//WANDER
//****
SteeringPlugin_Output_Extension Wander::CalculateSteering(float deltaT, AgentInfo& agentInfo)
{
	const Elite::Vector2 moveDirection{ agentInfo.LinearVelocity.GetNormalized()};
	const Elite::Vector2 lookDirection{ OrientationToVector(agentInfo.Orientation) };
	
	if (Dot(moveDirection, lookDirection) < 0.8f)
	{
		if (Cross(moveDirection, lookDirection) > 0.f)
		{
			m_CurrentDirection = -1;
		}
		else
		{
			m_CurrentDirection = 1;
		}
	}

	m_Target.Position = { agentInfo.Position + OrientationToVector(agentInfo.Orientation + m_CurrentDirection) * m_OffsetDistance };

	SteeringPlugin_Output_Extension steering = {};
	steering = Face::CalculateSteering(deltaT, agentInfo);

	return steering;
}

//PURSUIT
//****
SteeringPlugin_Output_Extension Pursuit::CalculateSteering(float deltaT, AgentInfo& agentInfo)
{
	SteeringPlugin_Output_Extension steering = {};

	Vector2 direction{ m_Target.Position - agentInfo.Position };

	float neededTime{}, step{ 0.1f };

	// afstand/snelheid = tijd nodig om bij directionPoint te geraken
	while (direction.Magnitude() / agentInfo.MaxLinearSpeed > neededTime && neededTime < 10)
	{
		direction += m_Target.LinearVelocity * step;
		neededTime += step;
	}

	steering.LinearVelocity = direction;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed;

	return steering;
}

//EVADE
//****
SteeringPlugin_Output_Extension Evade::CalculateSteering(float deltaT, AgentInfo& agentInfo)
{
	SteeringPlugin_Output_Extension steering = {};

	Vector2 direction{ m_Target.Position - agentInfo.Position };

	float neededTime{}, step{ 0.1f };

	if (direction.Magnitude() > m_EvadeRadius)
	{
		return steering;
	}

	// afstand/snelheid = tijd nodig om bij directionPoint te geraken
	while (direction.Magnitude() / agentInfo.MaxLinearSpeed > neededTime && neededTime < 10)
	{
		direction += m_Target.LinearVelocity * step;
		neededTime += step;
	}

	steering.LinearVelocity = direction;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed;
	steering.LinearVelocity *= -1;

	return steering;
}

//Rotate clock wise
SteeringPlugin_Output_Extension RotateClockWise::CalculateSteering(float deltaT, AgentInfo& agentInfo)
{
	SteeringPlugin_Output_Extension steering{};

	steering.AutoOrient = false;
	steering.AngularVelocity = -agentInfo.MaxAngularSpeed;

	return steering;
}

//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "IExamPlugin.h"
#include "EliteMath\EMatrix2x3.h"

using namespace Elite;

//SEEK
//****

SteeringPlugin_Output Seek::CalculateSteering(float deltaT, AgentInfo& agentInfo)
{
	SteeringPlugin_Output steering = {};

	steering.LinearVelocity = m_Target.Position - agentInfo.Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed;

	return steering;
}

//FLEE
//****
SteeringPlugin_Output Flee::CalculateSteering(float deltaT, AgentInfo& agentInfo)
{
	SteeringPlugin_Output steering = {};

	steering.LinearVelocity = agentInfo.Position - m_Target.Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed;

	return steering;
}

//ARRIVE
//****
SteeringPlugin_Output Arrive::CalculateSteering(float deltaT, AgentInfo& agentInfo)
{
	const float stopDistance{ 2.f };
	const float slowRadius{ 15.f };
	SteeringPlugin_Output steering = {};

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
SteeringPlugin_Output Face::CalculateSteering(float deltaT, AgentInfo& agentInfo)
{
	SteeringPlugin_Output steering = {};

	const Vector2 desiredDirection{ m_Target.Position - agentInfo.Position };
	const Vector2 currentDirection{ OrientationToVector(agentInfo.Orientation) };

	const float angle{ AngleBetween(currentDirection, desiredDirection) };
	const float stopAngle{ 0.1f };

	if (abs(angle) < stopAngle)
	{
		steering.AngularVelocity = 0.f;
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
SteeringPlugin_Output Wander::CalculateSteering(float deltaT, AgentInfo& agentInfo)
{
	m_WanderAngle += randomFloat(-m_MaxAngleChange, m_MaxAngleChange);

	const Vector2 currentDirection{ Elite::OrientationToVector(agentInfo.Orientation) };
	m_Target.Position = { agentInfo.Position + (currentDirection * m_OffsetDistance) + (Vector2{ cosf(m_WanderAngle),sinf(m_WanderAngle) } * m_Radius) };

	SteeringPlugin_Output steering = {};

	steering.LinearVelocity = Seek::CalculateSteering(deltaT, agentInfo).LinearVelocity;
	steering.AutoOrient = true;

	return steering;
}

//PURSUIT
//****
SteeringPlugin_Output Pursuit::CalculateSteering(float deltaT, AgentInfo& agentInfo)
{
	SteeringPlugin_Output steering = {};

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
SteeringPlugin_Output Evade::CalculateSteering(float deltaT, AgentInfo& agentInfo)
{
	SteeringPlugin_Output steering = {};

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

SteeringPlugin_Output RotateClockWise::CalculateSteering(float deltaT, AgentInfo& agentInfo)
{
	SteeringPlugin_Output steering{ Arrive::CalculateSteering(deltaT,agentInfo) };

	steering.AutoOrient = false;
	steering.AngularVelocity = agentInfo.MaxAngularSpeed;

	return steering;
}

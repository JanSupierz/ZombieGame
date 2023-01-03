/*=============================================================================*/
// Copyright 2021-2022 Elite Engine
// Authors: Matthieu Delaere, Thomas Goussaert
/*=============================================================================*/
// SteeringBehaviors.h: SteeringBehaviors interface and different implementations
/*=============================================================================*/
#ifndef ELITE_STEERINGBEHAVIORS
#define ELITE_STEERINGBEHAVIORS

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "Exam_HelperStructs.h"
#include "SteeringHelpers.h"
struct AgentInfo;

#pragma region **ISTEERINGBEHAVIOR** (BASE)
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	virtual SteeringPlugin_Output_Extension CalculateSteering(float deltaT, AgentInfo& agentInfo) = 0;

	//Seek Functions
	void SetTarget(const TargetData& target) { m_Target = target; }

	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehavior, T>::value>::type* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	TargetData m_Target;
};
#pragma endregion

///////////////////////////////////////
//SEEK
//****
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() = default;

	//Seek Behaviour
	SteeringPlugin_Output_Extension CalculateSteering(float deltaT, AgentInfo& agentInfo) override;
};

///////////////////////////////////////
//FLEE
//****
class Flee : public ISteeringBehavior
{
public:
	Flee() = default;
	virtual ~Flee() = default;

	//Flee Behaviour
	SteeringPlugin_Output_Extension CalculateSteering(float deltaT, AgentInfo& agentInfo) override;
};

///////////////////////////////////////
//ARRIVE
//****
class Arrive : public ISteeringBehavior
{
public:
	Arrive() = default;
	virtual ~Arrive() = default;

	//Arrive Behaviour
	SteeringPlugin_Output_Extension CalculateSteering(float deltaT, AgentInfo& agentInfo) override;
};

///////////////////////////////////////
//FACE
//****
class Face : public ISteeringBehavior
{
public:
	Face() = default;
	virtual ~Face() = default;

	//Face Behaviour
	SteeringPlugin_Output_Extension CalculateSteering(float deltaT, AgentInfo& agentInfo) override;
};

///////////////////////////////////////
//WANDER
//****
class Wander : public Seek
{
public:
	Wander() = default;
	virtual ~Wander() = default;

	//Wander Behaviour
	SteeringPlugin_Output_Extension CalculateSteering(float deltaT, AgentInfo& agentInfo) override;

	void SetWanderOffset(float offset) { m_OffsetDistance = offset; }
	void SetWanderRadius(float radius) { m_Radius = radius; }
	void SetMaxAngleChange(float rad) { m_MaxAngleChange = rad; }

protected:
	float m_OffsetDistance = 6.f; //Offset (AgentDirection)
	float m_Radius = 6.f; //WanderRadius
	float m_MaxAngleChange = Elite::ToRadians(45.f); //Max WanderAngle change per frame
	float m_WanderAngle = 0.f; //Internal
};

///////////////////////////////////////
//PURSUIT
//****
class Pursuit : public ISteeringBehavior
{
public:
	Pursuit() = default;
	virtual ~Pursuit() = default;

	//Pursuit Behaviour
	SteeringPlugin_Output_Extension CalculateSteering(float deltaT, AgentInfo& agentInfo) override;
};

//EVADE
//****
class Evade : public ISteeringBehavior
{
public:
	Evade() = default;
	virtual ~Evade() = default;

	//Evade Behaviour
	SteeringPlugin_Output_Extension CalculateSteering(float deltaT, AgentInfo& agentInfo) override;
	void SetEvadeRadius(float evadeRadius) { m_EvadeRadius = evadeRadius; }

private:
	float m_EvadeRadius{ 10.f };
};

///////////////////////////////////////
//ROTATE CLOCK WISE
//****
class RotateClockWise : public Arrive
{
public:
	RotateClockWise() = default;
	virtual ~RotateClockWise() = default;

	//Rotate Clock Wise Behaviour
	SteeringPlugin_Output_Extension CalculateSteering(float deltaT, AgentInfo& agentInfo) override;
};

#endif



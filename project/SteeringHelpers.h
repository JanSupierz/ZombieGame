#pragma once
#include "Exam_HelperStructs.h"

//SteeringParams (alias TargetData)
struct SteeringParams //Also used as Target for SteeringBehaviors
{
	Elite::Vector2 Position;
	float Orientation;

	Elite::Vector2 LinearVelocity;
	float AngularVelocity;

	SteeringParams(Elite::Vector2 Position = Elite::ZeroVector2, float orientation = 0.f, 
		Elite::Vector2 linearVel = Elite::ZeroVector2, float angularVel = 0.f) :
		Position(Position),
		Orientation(orientation),
		LinearVelocity(linearVel),
		AngularVelocity(angularVel) {}

#pragma region Functions
	void Clear()
	{
		Position = Elite::ZeroVector2;
		LinearVelocity = Elite::ZeroVector2;

		Orientation = 0.f;
		AngularVelocity = 0.f;
	}

	Elite::Vector2 GetDirection() const  //Zero Orientation > {0,-1}
	{
		return Elite::Vector2(cos(Orientation), sin(Orientation));
	}

	float GetOrientationFromVelocity() const
	{
		if (LinearVelocity.Magnitude() == 0)
			return 0.f;

		return atan2f(LinearVelocity.x, -LinearVelocity.y);
	}
#pragma endregion

#pragma region Operator Overloads
	SteeringParams(const SteeringParams & other)
	{
		Position = other.Position;
		Orientation = other.Orientation;
		LinearVelocity = other.LinearVelocity;
		AngularVelocity = other.AngularVelocity;
	}

	SteeringParams& operator=(const SteeringParams& other)
	{
		Position = other.Position;
		Orientation = other.Orientation;
		LinearVelocity = other.LinearVelocity;
		AngularVelocity = other.AngularVelocity;

		return *this;
	}

	bool operator==(const SteeringParams& other) const
	{
		return Position == other.Position && Orientation == other.Orientation && LinearVelocity == other.LinearVelocity && AngularVelocity == other.AngularVelocity;
	}

	bool operator!=(const SteeringParams& other) const
	{
		return Position != other.Position || Orientation != other.Orientation || LinearVelocity != other.LinearVelocity || AngularVelocity != other.AngularVelocity;
	}
#pragma endregion

};
using TargetData = SteeringParams; //Alias for SteeringBehavior usage (Bit clearer in its context ;) )
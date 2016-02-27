#pragma once

#ifndef VECTOR_H
#define VECTOR_H
struct Vector
{
	float x, y, z;

	inline Vector()
	{
		Zero();
	}
	inline Vector(float flin)
	{
		x = flin;
		y = flin;
		z = flin;
	}
	
	inline Vector(float a, float b, float c)
	{
		x = a;
		y = b;
		z = c;
	}
	
	inline void Zero()
	{
		x = 0;
		y = 0;
		z = 0;
	}
	
	inline void Init(float a, float b, float c)
	{
		x = a;
		y = b;
		z = c;
	}
	float& operator[](int a)
	{
		if (a == 0)
		{
			return x;
		}
			
		if (a == 1)
		{
			return y;
		}
			
		if (a == 2)
		{
			return z;
		}
		
		return x;

	}
	
	float operator[](int a) const
	{
		if (a == 0)
		{
			return x;
		}

		if (a == 1)
		{
			return y;
		}

		if (a == 2)
		{
			return z;
		}

		return x;

	}
};

typedef Vector QAngle;
#endif
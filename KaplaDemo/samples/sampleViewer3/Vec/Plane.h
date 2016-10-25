#ifndef PLANE_H
#define PLANE_H

#include "Vec3.H"

//	Singe / VecReal Precision Vec 3
//  Matthias Mueller
//  derived from Plane

namespace M
{

class Plane
{
public:
	Plane() {}
	Plane(VecReal nx, VecReal ny, VecReal nz, VecReal distance)
		: n(nx, ny, nz)
		, d(distance)
	{}

	Plane(const Vec3& normal, VecReal distance) 
		: n(normal)
		, d(distance)
	{}

	Plane(const Vec3& point, const Vec3& normal)
		: n(normal)		
		, d(-point.dot(n))		// p satisfies normal.dot(p) + d = 0
	{
	}

	Plane(const Vec3& p0, const Vec3& p1, const Vec3& p2)
	{
		n = (p1 - p0).cross(p2 - p0).getNormalized();
		d = -p0.dot(n);
	}

	VecReal distance(const Vec3& p) const
	{
		return p.dot(n) + d;
	}

	bool contains(const Vec3& p) const
	{
		return vecAbs(distance(p)) < (1.0e-7f);
	}

	Vec3 project(const Vec3 & p) const
	{
		return p - n * distance(p);
	}

	Vec3 pointInPlane() const
	{
		return -n*d;
	}

	void normalize()
	{
		VecReal denom = 1.0f / n.magnitude();
		n *= denom;
		d *= denom;
	}

	Vec3	n;			//!< The normal to the plane
	VecReal	d;			//!< The distance from the origin
};

}

#endif


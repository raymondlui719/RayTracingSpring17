#include <cmath>
#include <assert.h>

#include "Box.h"

bool Box::intersectLocal( const ray& r, isect& i ) const
{
	// lbound = -0.5, ubound = 0.5
	i.obj = this;

	vec3f dir = r.getDirection();
	vec3f origin = r.getPosition();

	double tfar = std::numeric_limits<double>::infinity();
	double tnear = -tfar;
	vec3f Nnear, Nfar;

	for (int i = 0; i < 3; i++)
	{
		if (abs(dir[i]) < RAY_EPSILON)
		{
			// then the ray is parallel to the plane
			if (origin[i] < -0.5 || origin[i] > 0.5)
			{
				return false;
			}
		}
		else
		{
			// compute the intersection distance of the planes
			vec3f N1, N2;
			double t1 = (-0.5 - origin[i]) / dir[i];
			double t2 = (0.5 - origin[i]) / dir[i];
			N1[i] = -1;
			N2[i] = 1;
			// t1 is the intersection with near plane
			if (t1 > t2)
			{
				double t = t1;
				t1 = t2;
				t2 = t;
				double n = N1[i];
				N1[i] = N2[i];
				N2[i] = n;
			}
			// want largest Tnear
			if (t1 > tnear)
			{
				tnear = t1;
				Nnear = N1;
			}
			// want smallest Tfar
			if (t2 < tfar)
			{
				tfar = t2;
				Nfar = N2;
			}
			// box is missed or is behind the ray
			if (tnear > tfar || tfar < RAY_EPSILON)
			{
				return false;
			}
		}		
	}
	// Box survived all above tests, return true with intersection point Tnear
	i.setT(tnear);
	i.setN(Nnear);
	return true;
}

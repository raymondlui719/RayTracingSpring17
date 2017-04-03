#include <cmath>

#include "light.h"

double DirectionalLight::distanceAttenuation( const vec3f& P ) const
{
	// distance to light is infinite, so f(di) goes to 0.  Return 1.
	return 1.0;
}


vec3f DirectionalLight::shadowAttenuation( const vec3f& P ) const
{
	vec3f dir = getDirection(P);
	isect i;
	vec3f resultColor = getColor(P);

	vec3f iP = P;
	ray R = ray(iP, dir);
	bool isSecondIntersect = false;	// second intersection of the same scene object
	while (scene->intersect(R, i))	// handle transparent object intersection case
	{
		if (i.getMaterial().kt.iszero()) {
			// if there is a non-transparent object along a path to the light source
			return vec3f(0.0, 0.0, 0.0);
		}
		// set new ray
		iP = R.at(i.t);
		R = ray(iP, dir);

		if (!isSecondIntersect) {
			// attenuate the result to the transmissive coef
			resultColor = prod(resultColor, i.getMaterial().kt);
			isSecondIntersect = true;
		}
		else {
			isSecondIntersect = false;
		}
	}
	return resultColor;
}

vec3f DirectionalLight::getColor( const vec3f& P ) const
{
	// Color doesn't depend on P 
	return color;
}

vec3f DirectionalLight::getDirection( const vec3f& P ) const
{
	return -orientation;
}

double PointLight::distanceAttenuation( const vec3f& P ) const
{
	double dSquared = (P - position).length_squared();
	double d = (P - position).length();
	// apply user-supplied constants to calculate f(d)
	double coeff = m_const_atten_coeff + m_linear_atten_coeff * d + m_quadratic_atten_coeff * dSquared;
	if (coeff != 0.0 && coeff > 1.0) {
		return 1.0 / coeff;
	}
	return 1.0;
}

vec3f PointLight::getColor( const vec3f& P ) const
{
	// Color doesn't depend on P 
	return color;
}

vec3f PointLight::getDirection( const vec3f& P ) const
{
	return (position - P).normalize();
}

void PointLight::setDistanceAttenuation(const double constant, const double linear, const double quadratic)
{
	m_const_atten_coeff = constant;
	m_linear_atten_coeff = linear;
	m_quadratic_atten_coeff = quadratic;
}


vec3f PointLight::shadowAttenuation(const vec3f& P) const
{
	vec3f dir = getDirection(P);
	isect i;
	vec3f resultColor = getColor(P);
	double light_t = (position - P).length();
	
	vec3f iP = P;
	ray R = ray(iP, dir);
	bool isSecondIntersect = false;	// second intersection of the same scene object
	while (scene->intersect(R, i))	// handle transparent object intersection case
	{
		light_t -= i.t;
		if (light_t < RAY_EPSILON) {
			// if the intersected object locates behind the point light
			return resultColor;
		}
		if (i.getMaterial().kt.iszero()) {
			// if there is a non-transparent object along a path to the light source
			return vec3f(0.0, 0.0, 0.0);
		}
		// set new ray
		iP = R.at(i.t);
		R = ray(iP, dir);
		
		if (!isSecondIntersect) {
			// attenuate the result to the transmissive coef
			resultColor = prod(resultColor, i.getMaterial().kt);
			isSecondIntersect = true;
		}
		else {
			isSecondIntersect = false;
		}
	}
	return resultColor;
}

double AmbientLight::distanceAttenuation(const vec3f& P) const
{
	return 1.0;
}

vec3f AmbientLight::getColor(const vec3f& P) const
{
	return color;
}

vec3f AmbientLight::getDirection(const vec3f& P) const
{
	return vec3f(1, 1, 1);
}


vec3f AmbientLight::shadowAttenuation(const vec3f& P) const
{
	return vec3f(1, 1, 1);
}

#include "ray.h"
#include "material.h"
#include "light.h"

// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
vec3f Material::shade( Scene *scene, const ray& r, const isect& i ) const
{
	vec3f result = ke;	// iter 0
	vec3f ambient = prod(ka, scene->getAmbient()); // iter 1
	
	vec3f trans = vec3f(1, 1, 1) - kt;
	
	result += prod(trans, ambient);

	// iter 2 & 3
	vec3f normal = i.N;
	vec3f P = r.at(i.t);
	for (Scene::cliter j = scene->beginLights(); j != scene->endLights(); ++j) {
		vec3f atten = (*j)->distanceAttenuation(P) * (*j)->shadowAttenuation(P + i.N * RAY_EPSILON);
		vec3f Lj = (*j)->getDirection(P);
		vec3f diffuse = prod(kd * maximum(normal.dot(Lj), 0.0), trans);
		vec3f R = ((2.0 * (normal.dot(Lj)) * normal) - Lj).normalize();
		vec3f specular = ks * (pow(maximum(R * (-r.getDirection()), 0.0), shininess * 128.0));
		result += prod(atten, diffuse + specular);
	}
	result = result.clamp();
	return result;
}

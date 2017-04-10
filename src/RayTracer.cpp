// The main ray tracer.

#include <Fl/fl_ask.h>

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"
#include "fileio/read.h"
#include "fileio/parse.h"
#include "ui/TraceUI.h"

extern TraceUI* traceUI;

// Trace a top-level ray through normalized window coordinates (x,y)
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.
vec3f RayTracer::trace( Scene *scene, double x, double y )
{
    ray r( vec3f(0,0,0), vec3f(0,0,0) );
    scene->getCamera()->rayThrough( x,y,r );
	mediaStack = std::stack<const Material*>();
	Material air;
	mediaStack.push(&air);
	return traceRay( scene, r, vec3f(1.0,1.0,1.0), 0 ).clamp();
}

// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
vec3f RayTracer::traceRay( Scene *scene, const ray& r, 
	const vec3f& thresh, int depth )
{
	isect i;

	if( scene->intersect( r, i ) ) {

		const Material& m = i.getMaterial();
		vec3f shade = m.shade(scene, r, i);
		const vec3f intensity(shade[0] * thresh[0], shade[1] * thresh[1], shade[2] * thresh[2]);

		//handle reflection
		vec3f reflection;
		if (!m.kr.iszero() && depth < traceUI->getDepth())
		{
			vec3f rDir = ((2.0 * (i.N.dot(-r.getDirection())) * i.N) - (-r.getDirection())).normalize();
			vec3f rPoint = r.at(i.t) + i.N * RAY_EPSILON;
			ray reflectedRay = ray(rPoint, rDir);
			const vec3f next_thresh(thresh[0] * m.kr[0], thresh[1] * m.kr[1], thresh[2] * m.kr[2]);
			reflection = prod(m.kr, traceRay(scene, reflectedRay, next_thresh, depth + 1));
		}

		//handle refraction
		vec3f transmission;
		if (!m.kt.iszero() && depth < traceUI->getDepth())
		{
			const Material* currMat = mediaStack.top();
			double ni, nt;
			vec3f point, normal;
			if (currMat == &m) {
				mediaStack.pop();
				const Material *outside = mediaStack.top();
				mediaStack.push(currMat);
				ni = m.index;
				nt = outside->index;
				normal = -i.N;
			}
			else {
				ni = currMat->index;
				nt = m.index;
				normal = i.N;
			}

			const double nr = ni / nt;
			const double ndotr = normal.dot(-r.getDirection());
			point = r.at(i.t) - normal * RAY_EPSILON;
			double cos_i = max(min(normal * ((-r.getDirection()).normalize()), 1.0), -1.0); //SYSNOTE: min(x, 1.0) to prevent cos_i becomes bigger than 1
			double sin_i = sqrt(1 - cos_i * cos_i);
			double sin_t = sin_i * nr;

			if (sin_t <= 1.0) {
				mediaStack.push(&m);
				double cos_t = sqrt(1 - sin_t*sin_t);
				vec3f tDir = (nr * cos_i - cos_t) * normal - nr * (-r.getDirection());
				ray transmittedRay = ray(r.at(i.t), tDir);
				const vec3f next_thresh(thresh[0] * m.kt[0], thresh[1] * m.kt[1], thresh[2] * m.kt[2]);
				transmission = prod(m.kt, traceRay(scene, transmittedRay, next_thresh, depth + 1));
			}
		}

		return intensity + reflection + transmission;
	}
	else
	{
		return vec3f( 0.0, 0.0, 0.0 );
	}
}

/*
double RayTracer::getFresnel(isect& i, const ray& r)
{
	if (!traceUI->IsEnableFresnel())
	{
		return 1.0;
	}
	vec3f normal;
	if (i.obj->hasInterior())
	{
		double indexA, indexB;
		if (i.N*r.getDirection() > RAY_EPSILON)
		{
			if (mediaTracker.empty())
			{
				indexA = 1.0;
			}
			else
			{
				indexA = mediaTracker.rbegin()->second.index;
			}
			mediaTracker.erase(i.obj->getOrder());
			if (mediaTracker.empty())
			{
				indexB = 1.0;
			}
			else
			{
				indexB = mediaTracker.rbegin()->second.index;
			}
			normal = -i.N;
			mediaTracker.insert(make_pair(i.obj->getOrder(), i.getMaterial()));
		}
		// For ray get in the object
		else
		{
			if (mediaTracker.empty())
			{
				indexA = 1.0;
			}
			else
			{
				indexA = mediaTracker.rbegin()->second.index;
			}
			normal = i.N;
			mediaTracker.insert(make_pair(i.obj->getOrder(), i.getMaterial()));
			indexB = mediaTracker.rbegin()->second.index;
			mediaTracker.erase(i.obj->getOrder());
		}

		double r0 = (indexA - indexB) / (indexA + indexB);
		r0 = r0 * r0;

		const double cos_i = max(min(i.N.dot(-r.getDirection().normalize()), 1.0), -1.0);
		double sin_i = sqrt(1 - cos_i*cos_i);
		double sin_t = sin_i * (indexA / indexB);

		if (indexA <= indexB)
		{
			return r0 + (1 - r0)*pow(1 - cos_i, 5);
		}
		else
		{
			if (sin_t > 1.0)
			{
				return 1.0;
			}
			else
			{
				double cos_t = sqrt(1 - sin_t*sin_t);
				return r0 + (1 - r0) * pow(1 - cos_t, 5);
			}
		}
	}
	else
	{
		return 1.0;
	}
}
*/

RayTracer::RayTracer()
{
	buffer = NULL;
	buffer_width = buffer_height = 256;
	scene = NULL;

	m_bSceneLoaded = false;
}


RayTracer::~RayTracer()
{
	delete [] buffer;
	delete scene;
}

void RayTracer::getBuffer( unsigned char *&buf, int &w, int &h )
{
	buf = buffer;
	w = buffer_width;
	h = buffer_height;
}

double RayTracer::aspectRatio()
{
	return scene ? scene->getCamera()->getAspectRatio() : 1;
}

bool RayTracer::sceneLoaded()
{
	return m_bSceneLoaded;
}

bool RayTracer::loadScene( char* fn )
{
	try
	{
		scene = readScene( fn );
	}
	catch( ParseError pe )
	{
		fl_alert( "ParseError: %s\n", pe );
		return false;
	}

	if( !scene )
		return false;
	
	buffer_width = 256;
	buffer_height = (int)(buffer_width / scene->getCamera()->getAspectRatio() + 0.5);

	bufferSize = buffer_width * buffer_height * 3;
	buffer = new unsigned char[ bufferSize ];
	
	// separate objects into bounded and unbounded
	scene->initScene();
	
	// Add any specialized scene loading code here
	
	m_bSceneLoaded = true;

	return true;
}

void RayTracer::traceSetup( int w, int h )
{
	if( buffer_width != w || buffer_height != h )
	{
		buffer_width = w;
		buffer_height = h;

		bufferSize = buffer_width * buffer_height * 3;
		delete [] buffer;
		buffer = new unsigned char[ bufferSize ];
	}
	memset( buffer, 0, w*h*3 );
}

void RayTracer::traceLines( int start, int stop )
{
	vec3f col;
	if( !scene )
		return;

	if( stop > buffer_height )
		stop = buffer_height;

	for( int j = start; j < stop; ++j )
		for( int i = 0; i < buffer_width; ++i )
			tracePixel(i,j);
}

void RayTracer::tracePixel( int i, int j )
{
	vec3f col;

	if( !scene )
		return;

	double x = double(i)/double(buffer_width);
	double y = double(j)/double(buffer_height);

	col = trace( scene,x,y );

	unsigned char *pixel = buffer + ( i + j * buffer_width ) * 3;

	pixel[0] = (int)( 255.0 * col[0]);
	pixel[1] = (int)( 255.0 * col[1]);
	pixel[2] = (int)( 255.0 * col[2]);
}
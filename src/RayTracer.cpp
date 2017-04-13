// The main ray tracer.

#include <Fl/fl_ask.h>

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"
#include "fileio/read.h"
#include "fileio/parse.h"
#include "ui/TraceUI.h"
#include "fileio/bitmap.h"

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
	const vec3f& thresh, int depth, double intensity )
{
	isect i;

	if( scene->intersect( r, i ) ) {

		const Material& m = i.getMaterial();
		vec3f shade = m.shade(scene, r, i);
		
		const vec3f result(shade[0] * thresh[0], shade[1] * thresh[1], shade[2] * thresh[2]);

		vec3f reflection;
		vec3f transmission;
		if (depth < traceUI->getDepth()	&& (traceUI->m_intThreshSlider->value() == 0 || intensity > traceUI->m_intThreshSlider->value()))
		{
			//handle reflection
			if (!m.kr.iszero())
			{
				vec3f rDir = ((2.0 * (i.N.dot(-r.getDirection())) * i.N) - (-r.getDirection())).normalize();
				vec3f rPoint = r.at(i.t) + i.N * RAY_EPSILON;
				ray reflectedRay = ray(rPoint, rDir);
				const vec3f next_thresh(thresh[0] * m.kr[0], thresh[1] * m.kr[1], thresh[2] * m.kr[2]);
				reflection = prod(m.kr, traceRay(scene, reflectedRay, next_thresh, depth + 1, m.kr.length() * intensity));
			}

			//handle refraction
			if (!m.kt.iszero())
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
					transmission = prod(m.kt, traceRay(scene, transmittedRay, next_thresh, depth + 1, m.kt.length() * intensity));
				}
			}
		}

		return result + reflection + transmission;
	}
	else
		if (useBackground)
		{
			vec3f x = scene->getCamera()->getU();
			vec3f y = scene->getCamera()->getV();
			vec3f z = scene->getCamera()->getLook();
			double dis_x = r.getDirection() * x;
			double dis_y = r.getDirection() * y;
			double dis_z = r.getDirection() * z;
			return getBackgroundImage(dis_x / dis_z + 0.5, dis_y / dis_z + 0.5);
		}
		else
		{
			return vec3f(0.0, 0.0, 0.0);
		}
}

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

void RayTracer::loadBackground(char* fn)
{
	unsigned char* data = NULL;
	data = readBMP(fn, background_width, background_height);
	if (data) {
		if (backgroundImage == NULL) delete[] backgroundImage;
		useBackground = true;
		backgroundImage = data;
	}
}

void RayTracer::clearBackground() {
	if (backgroundImage == NULL) delete[] backgroundImage;
	backgroundImage = NULL;
	useBackground = false;
	background_height = background_width = 0;
}

vec3f RayTracer::getBackgroundImage(double x, double y) {
	if (!useBackground) return vec3f(0, 0, 0);
	int xGrid = int(x*background_width);
	int yGrid = int(y*background_height);
	if (xGrid < 0 || xGrid >= background_width || yGrid < 0 || yGrid >= background_height)
	{
		return vec3f(0, 0, 0);
	}
	double val1 = backgroundImage[(yGrid*background_width + xGrid) * 3] / 255.0;
	double val2 = backgroundImage[(yGrid*background_width + xGrid) * 3 + 1] / 255.0;
	double val3 = backgroundImage[(yGrid*background_width + xGrid) * 3 + 2] / 255.0;
	return vec3f(val1, val2, val3);
}
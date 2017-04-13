#ifndef __RAYTRACER_H__
#define __RAYTRACER_H__

// The main ray tracer.

#include "scene/scene.h"
#include "scene/ray.h"
#include <map>
#include <stack>

class RayTracer
{
public:
    RayTracer();
    ~RayTracer();

    vec3f trace( Scene *scene, double x, double y );
	vec3f traceRay( Scene *scene, const ray& r, const vec3f& thresh, int depth, double intensity = 1.0 );


	void getBuffer( unsigned char *&buf, int &w, int &h );
	double aspectRatio();
	void traceSetup( int w, int h );
	void traceLines( int start = 0, int stop = 10000000 );
	void tracePixel( int i, int j );
	void loadBackground(char* fn);
	void clearBackground();
	vec3f getBackgroundImage(double x, double y);

	bool loadScene( char* fn );

	bool sceneLoaded();

private:
	unsigned char *buffer;
	int buffer_width, buffer_height;
	int bufferSize;
	bool useBackground;
	unsigned char *backgroundImage;
	int background_height, background_width;
	std::stack<const Material*> mediaStack;
	Scene *scene;

	bool m_bSceneLoaded;
};

#endif // __RAYTRACER_H__

#ifndef __READ_H__
#define __READ_H__

#include <string>
#include <iostream>

#include "../scene/scene.h"
#include "../ui/TraceUI.h"

Scene *readScene( const string& filename );
Scene *readScene( istream& is );

#endif // __READ_H__

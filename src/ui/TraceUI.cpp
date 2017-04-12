//
// TraceUI.h
//
// Handles FLTK integration and other user interface tasks
//
#include <stdio.h>
#include <time.h>
#include <string.h>

#include <FL/fl_ask.h>

#include "TraceUI.h"
#include "../RayTracer.h"

static bool done;

//------------------------------------- Help Functions --------------------------------------------
TraceUI* TraceUI::whoami(Fl_Menu_* o)	// from menu item back to UI itself
{
	return ( (TraceUI*)(o->parent()->user_data()) );
}

//--------------------------------- Callback Functions --------------------------------------------
void TraceUI::cb_load_scene(Fl_Menu_* o, void* v) 
{
	TraceUI* pUI=whoami(o);
	
	char* newfile = fl_file_chooser("Open Scene?", "*.ray", NULL );

	if (newfile != NULL) {
		char buf[256];

		if (pUI->raytracer->loadScene(newfile)) {
			sprintf(buf, "Ray <%s>", newfile);
			done=true;	// terminate the previous rendering
		} else{
			sprintf(buf, "Ray <Not Loaded>");
		}

		pUI->m_mainWindow->label(buf);
	}
}

void TraceUI::cb_save_image(Fl_Menu_* o, void* v) 
{
	TraceUI* pUI=whoami(o);
	
	char* savefile = fl_file_chooser("Save Image?", "*.bmp", "save.bmp" );
	if (savefile != NULL) {
		pUI->m_traceGlWindow->saveImage(savefile);
	}
}

void TraceUI::cb_exit(Fl_Menu_* o, void* v)
{
	TraceUI* pUI=whoami(o);

	// terminate the rendering
	done=true;

	pUI->m_traceGlWindow->hide();
	pUI->m_mainWindow->hide();
}

void TraceUI::cb_exit2(Fl_Widget* o, void* v) 
{
	TraceUI* pUI=(TraceUI *)(o->user_data());
	
	// terminate the rendering
	done=true;

	pUI->m_traceGlWindow->hide();
	pUI->m_mainWindow->hide();
}

void TraceUI::cb_about(Fl_Menu_* o, void* v) 
{
	fl_message("RayTracer Project, FLTK version for CS 341 Spring 2002. Latest modifications by Jeff Maurer, jmaurer@cs.washington.edu");
}

void TraceUI::cb_sizeSlides(Fl_Widget* o, void* v)
{
	TraceUI* pUI=(TraceUI*)(o->user_data());
	
	pUI->m_nSize=int( ((Fl_Slider *)o)->value() ) ;
	int	height = (int)(pUI->m_nSize / pUI->raytracer->aspectRatio() + 0.5);
	pUI->m_traceGlWindow->resizeWindow( pUI->m_nSize, height );
}

void TraceUI::cb_depthSlides(Fl_Widget* o, void* v)
{
	((TraceUI*)(o->user_data()))->m_nDepth = int(((Fl_Slider *)o)->value());
}

void TraceUI::cb_conAtnSlides(Fl_Widget* o, void* v)
{
	((TraceUI*)(o->user_data()))->m_nConAtn = double(((Fl_Slider *)o)->value());
}

void TraceUI::cb_linAtnSlides(Fl_Widget* o, void* v)
{
	((TraceUI*)(o->user_data()))->m_nLinAtn = double(((Fl_Slider *)o)->value());
}

void TraceUI::cb_quadAtnSlides(Fl_Widget* o, void* v)
{
	((TraceUI*)(o->user_data()))->m_nQuadAtn = double(((Fl_Slider *)o)->value());
}

void TraceUI::cb_ambLightSlides(Fl_Widget* o, void* v)
{
	((TraceUI*)(o->user_data()))->m_nAmbLight = double(((Fl_Slider *)o)->value());
}

void TraceUI::cb_intThershSlides(Fl_Widget* o, void* v)
{
	((TraceUI*)(o->user_data()))->m_nIntThresh = double(((Fl_Slider *)o)->value());
}

void TraceUI::cb_render(Fl_Widget* o, void* v)
{
	char buffer[256];

	TraceUI* pUI=((TraceUI*)(o->user_data()));
	
	if (pUI->raytracer->sceneLoaded()) {
		int width=pUI->getSize();
		int	height = (int)(width / pUI->raytracer->aspectRatio() + 0.5);
		pUI->m_traceGlWindow->resizeWindow( width, height );

		pUI->m_traceGlWindow->show();

		pUI->raytracer->traceSetup(width, height);
		
		// Save the window label
		const char *old_label = pUI->m_traceGlWindow->label();

		// start to render here	
		done=false;
		clock_t prev, now;
		prev=clock();
		
		pUI->m_traceGlWindow->refresh();
		Fl::check();
		Fl::flush();

		for (int y=0; y<height; y++) {
			for (int x=0; x<width; x++) {
				if (done) break;
				
				// current time
				now = clock();

				// check event every 1/2 second
				if (((double)(now-prev)/CLOCKS_PER_SEC)>0.5) {
					prev=now;

					if (Fl::ready()) {
						// refresh
						pUI->m_traceGlWindow->refresh();
						// check event
						Fl::check();

						if (Fl::damage()) {
							Fl::flush();
						}
					}
				}

				pUI->raytracer->tracePixel( x, y );
		
			}
			if (done) break;

			// flush when finish a row
			if (Fl::ready()) {
				// refresh
				pUI->m_traceGlWindow->refresh();

				if (Fl::damage()) {
					Fl::flush();
				}
			}
			// update the window label
			sprintf(buffer, "(%d%%) %s", (int)((double)y / (double)height * 100.0), old_label);
			pUI->m_traceGlWindow->label(buffer);
			
		}
		done=true;
		pUI->m_traceGlWindow->refresh();

		// Restore the window label
		pUI->m_traceGlWindow->label(old_label);		
	}
}

void TraceUI::cb_stop(Fl_Widget* o, void* v)
{
	done=true;
}

void TraceUI::show()
{
	m_mainWindow->show();
}

void TraceUI::setRayTracer(RayTracer *tracer)
{
	raytracer = tracer;
	m_traceGlWindow->setRayTracer(tracer);
}

int TraceUI::getSize()
{
	return m_nSize;
}

int TraceUI::getDepth()
{
	return m_nDepth;
}

// menu definition
Fl_Menu_Item TraceUI::menuitems[] = {
	{ "&File",		0, 0, 0, FL_SUBMENU },
		{ "&Load Scene...",	FL_ALT + 'l', (Fl_Callback *)TraceUI::cb_load_scene },
		{ "&Save Image...",	FL_ALT + 's', (Fl_Callback *)TraceUI::cb_save_image },
		{ "&Exit",			FL_ALT + 'e', (Fl_Callback *)TraceUI::cb_exit },
		{ 0 },

	{ "&Help",		0, 0, 0, FL_SUBMENU },
		{ "&About",	FL_ALT + 'a', (Fl_Callback *)TraceUI::cb_about },
		{ 0 },

	{ 0 }
};

TraceUI::TraceUI() {
	// init.
	m_nDepth = 5;
	m_nSize = 300;
	m_nConAtn = 0.0;
	m_nLinAtn = 0.0;
	m_nQuadAtn = 0.0;
	m_nAmbLight = 1.0;
	m_nIntThresh = 0.15;
	m_mainWindow = new Fl_Window(100, 40, 400, 250, "Ray <Not Loaded>");
		m_mainWindow->user_data((void*)(this));	// record self to be used by static callback functions
		// install menu bar
		m_menubar = new Fl_Menu_Bar(0, 0, 400, 25);
		m_menubar->menu(menuitems);

		// install slider depth		1
		m_depthSlider = new Fl_Value_Slider(10, 30, 180, 20, "Depth");
		m_depthSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_depthSlider->type(FL_HOR_NICE_SLIDER);
        m_depthSlider->labelfont(FL_COURIER);
        m_depthSlider->labelsize(12);
		m_depthSlider->minimum(0);
		m_depthSlider->maximum(10);
		m_depthSlider->step(1);
		m_depthSlider->value(m_nDepth);
		m_depthSlider->align(FL_ALIGN_RIGHT);
		m_depthSlider->callback(cb_depthSlides);

		// install slider size		2
		m_sizeSlider = new Fl_Value_Slider(10, 55, 180, 20, "Size");
		m_sizeSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_sizeSlider->type(FL_HOR_NICE_SLIDER);
        m_sizeSlider->labelfont(FL_COURIER);
        m_sizeSlider->labelsize(12);
		m_sizeSlider->minimum(64);
		m_sizeSlider->maximum(512);
		m_sizeSlider->step(1);
		m_sizeSlider->value(m_nSize);
		m_sizeSlider->align(FL_ALIGN_RIGHT);
		m_sizeSlider->callback(cb_sizeSlides);

		// install slider att	3
		m_conAttenSlider = new Fl_Value_Slider(10, 80, 180, 20, "Attenuation, Constant");
		m_conAttenSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_conAttenSlider->type(FL_HOR_NICE_SLIDER);
		m_conAttenSlider->labelfont(FL_COURIER);
		m_conAttenSlider->labelsize(12);
		m_conAttenSlider->minimum(0.0);
		m_conAttenSlider->maximum(1.0);
		m_conAttenSlider->step(0.01);
		m_conAttenSlider->value(m_nConAtn);
		m_conAttenSlider->align(FL_ALIGN_RIGHT);
		m_conAttenSlider->callback(cb_conAtnSlides);

		// install slider att	4
		m_linAttenSlider = new Fl_Value_Slider(10, 105, 180, 20, "Attenuation, Linear");
		m_linAttenSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_linAttenSlider->type(FL_HOR_NICE_SLIDER);
		m_linAttenSlider->labelfont(FL_COURIER);
		m_linAttenSlider->labelsize(12);
		m_linAttenSlider->minimum(0.0);
		m_linAttenSlider->maximum(1.0);
		m_linAttenSlider->step(0.01);
		m_linAttenSlider->value(m_nLinAtn);
		m_linAttenSlider->align(FL_ALIGN_RIGHT);
		m_linAttenSlider->callback(cb_linAtnSlides);

		// install slider att	5
		m_quadAttenSlider = new Fl_Value_Slider(10, 130, 180, 20, "Attenuation, Quadratic");
		m_quadAttenSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_quadAttenSlider->type(FL_HOR_NICE_SLIDER);
		m_quadAttenSlider->labelfont(FL_COURIER);
		m_quadAttenSlider->labelsize(12);
		m_quadAttenSlider->minimum(0.0);
		m_quadAttenSlider->maximum(1.0);
		m_quadAttenSlider->step(0.01);
		m_quadAttenSlider->value(m_nQuadAtn);
		m_quadAttenSlider->align(FL_ALIGN_RIGHT);
		m_quadAttenSlider->callback(cb_quadAtnSlides);

		// install slider size	6
		m_ambLightSlider = new Fl_Value_Slider(10, 155, 180, 20, "Ambient Light");
		m_ambLightSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_ambLightSlider->type(FL_HOR_NICE_SLIDER);
		m_ambLightSlider->labelfont(FL_COURIER);
		m_ambLightSlider->labelsize(12);
		m_ambLightSlider->minimum(0);
		m_ambLightSlider->maximum(1);
		m_ambLightSlider->step(0.01);
		m_ambLightSlider->value(m_nAmbLight);
		m_ambLightSlider->align(FL_ALIGN_RIGHT);
		m_ambLightSlider->callback(cb_ambLightSlides);

		// install slider scale	7
		m_intThreshSlider = new Fl_Value_Slider(10, 180, 180, 20, "Intensity Threshold");
		m_intThreshSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_intThreshSlider->type(FL_HOR_NICE_SLIDER);
		m_intThreshSlider->labelfont(FL_COURIER);
		m_intThreshSlider->labelsize(12);
		m_intThreshSlider->minimum(0);
		m_intThreshSlider->maximum(1);
		m_intThreshSlider->step(0.01);
		m_intThreshSlider->value(m_nIntThresh);
		m_intThreshSlider->align(FL_ALIGN_RIGHT);
		m_intThreshSlider->callback(cb_intThershSlides);

		m_renderButton = new Fl_Button(240, 27, 70, 25, "&Render");
		m_renderButton->user_data((void*)(this));
		m_renderButton->callback(cb_render);

		m_stopButton = new Fl_Button(240, 55, 70, 25, "&Stop");
		m_stopButton->user_data((void*)(this));
		m_stopButton->callback(cb_stop);

		m_mainWindow->callback(cb_exit2);
		m_mainWindow->when(FL_HIDE);
    m_mainWindow->end();

	// image view
	m_traceGlWindow = new TraceGLWindow(100, 150, m_nSize, m_nSize, "Rendered Image");
	m_traceGlWindow->end();
	m_traceGlWindow->resizable(m_traceGlWindow);
}
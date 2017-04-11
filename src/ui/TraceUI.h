//
// rayUI.h
//
// The header file for the UI part
//

#ifndef __rayUI_h__
#define __rayUI_h__

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Button.H>

#include <FL/fl_file_chooser.H>		// FLTK file chooser

#include "TraceGLWindow.h"

class TraceUI {
public:
	TraceUI();

	// The FLTK widgets
	Fl_Window*			m_mainWindow;
	Fl_Menu_Bar*		m_menubar;

	Fl_Slider*			m_sizeSlider;
	Fl_Slider*			m_depthSlider;

	Fl_Button*			m_renderButton;
	Fl_Button*			m_stopButton;

	TraceGLWindow*		m_traceGlWindow;

	// member functions
	void show();

	void		setRayTracer(RayTracer *tracer);

	int			getSize();
	int			getDepth();
	double getConstantAttenuation() const
	{
		return m_nConAtn;
	}
	double getLinearAttenuation() const
	{
		return m_nLinAtn;
	}
	double getQuadraticAttenuation() const
	{
		return m_nQuadAtn;
	}


private:
	RayTracer*	raytracer;

	int			m_nSize;
	int			m_nDepth;
	double		m_nConAtn;
	double		m_nLinAtn;
	double		m_nQuadAtn;
	double		m_nAmbLight;
	int			m_nInt;
	double		m_nDist;

// static class members
	static Fl_Menu_Item menuitems[];

	static TraceUI* whoami(Fl_Menu_* o);

	static void cb_load_scene(Fl_Menu_* o, void* v);
	static void cb_save_image(Fl_Menu_* o, void* v);
	static void cb_exit(Fl_Menu_* o, void* v);
	static void cb_about(Fl_Menu_* o, void* v);

	static void cb_exit2(Fl_Widget* o, void* v);

	static void cb_sizeSlides(Fl_Widget* o, void* v);
	static void cb_depthSlides(Fl_Widget* o, void* v);
	static void cb_conAtnSlides(Fl_Widget* o, void* v);
	static void cb_linAtnSlides(Fl_Widget* o, void* v);
	static void cb_quadAtnSlides(Fl_Widget* o, void* v);
	static void cb_ambLightSlides(Fl_Widget* o, void* v);
	static void cb_intSlides(Fl_Widget* o, void* v);
	static void cb_distSlides(Fl_Widget* o, void* v);

	static void cb_render(Fl_Widget* o, void* v);
	static void cb_stop(Fl_Widget* o, void* v);
};

#endif

/*
* Copyright (C) 2010 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#define GLES2

#include <malloc.h>
#include <jni.h>
#include <errno.h>

#include <string.h>
#include <unistd.h>
#include <sys/resource.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#ifdef GLES2

#define ENTRY_VOID(func) static decltype(func)  *_##func = (decltype(func)*)eglGetProcAddress(#func);
#define ENTRY(func) static decltype(func)  *_##func = (decltype(func)*)eglGetProcAddress(#func); return _##func;

GL_APICALL void GL_APIENTRY glGenQueriesEXT(GLsizei n, GLuint* ids){ ENTRY_VOID(glGenQueriesEXT);}
GL_APICALL void GL_APIENTRY glDeleteQueriesEXT(GLsizei n, const GLuint* ids) { ENTRY_VOID(glDeleteQueriesEXT); }
GL_APICALL GLboolean GL_APIENTRY glIsQueryEXT(GLuint id);
GL_APICALL void GL_APIENTRY glBeginQueryEXT(GLenum target, GLuint id) { ENTRY_VOID(glBeginQueryEXT); }
GL_APICALL void GL_APIENTRY glEndQueryEXT(GLenum target) { ENTRY_VOID(glEndQueryEXT); }
GL_APICALL void GL_APIENTRY glQueryCounterEXT(GLuint id, GLenum target) { ENTRY_VOID(glQueryCounterEXT); }
GL_APICALL void GL_APIENTRY glGetQueryivEXT(GLenum target, GLenum pname, GLint* params) { ENTRY_VOID(glGetQueryivEXT); }
GL_APICALL void GL_APIENTRY glGetQueryObjectivEXT(GLuint id, GLenum pname, GLint* params) { ENTRY_VOID(glGetQueryObjectivEXT); }
GL_APICALL void GL_APIENTRY glGetQueryObjectuivEXT(GLuint id, GLenum pname, GLuint* params) { ENTRY_VOID(glGetQueryObjectuivEXT); }
GL_APICALL void GL_APIENTRY glGetQueryObjecti64vEXT(GLuint id, GLenum pname, GLint64* params) { ENTRY_VOID(glGetQueryObjecti64vEXT); }
GL_APICALL void GL_APIENTRY glGetQueryObjectui64vEXT(GLuint id, GLenum pname, GLuint64* params) { ENTRY_VOID(glGetQueryObjectui64vEXT); }
GL_APICALL void GL_APIENTRY glGetInteger64vAPPLE(GLenum pname, GLint64* params) { ENTRY_VOID(glGetInteger64vAPPLE); }
#define GL_TIMESTAMP GL_TIMESTAMP_EXT
#define GL_TIME_ELAPSED GL_TIME_ELAPSED_EXT
#define glGetInteger64v glGetInteger64vAPPLE
#define glGetQueryObjectui64v glGetQueryObjectui64vEXT
#define glBeginQuery glBeginQueryEXT
#define glQueryCounter glQueryCounterEXT
#define glEndQuery glEndQueryEXT
#define glGenQueries glGenQueriesEXT
#define glGetQueryiv glGetQueryivEXT
#define glDeleteQueries glDeleteQueriesEXT
#define GL_QUERY_COUNTER_BITS GL_QUERY_COUNTER_BITS_EXT
#define GL_QUERY_RESULT GL_QUERY_RESULT_EXT
#endif

#include <android/sensor.h>

#include <android/log.h>
#include "android_native_app_glue.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "AndroidProject1.NativeActivity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "AndroidProject1.NativeActivity", __VA_ARGS__))

#define MICROPROFILE_IMPL
#define MICROPROFILEUI_IMPL
#define MICROPROFILEDRAW_IMPL

#define MICROPROFILE_GPU_TIMERS_GL 1

#define MICROPROFILE_PRINTF LOGW

#include "../../../Cinder/blocks/Cinder-VNM/ui/microprofile/microprofile.h"
//#include "../../../Cinder/blocks/Cinder-VNM/ui/microprofile/microprofileui.h"
//#include "../../../Cinder/blocks/Cinder-VNM/ui/microprofile/microprofiledraw.h"


/**
* Our saved state data.
*/
struct saved_state {
	float angle;
	int32_t x;
	int32_t y;
};

/**
* Shared state for our app.
*/
struct engine {
	struct android_app* app;

	ASensorManager* sensorManager;
	const ASensor* accelerometerSensor;
	ASensorEventQueue* sensorEventQueue;

	int animating;
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
	int32_t width;
	int32_t height;
	struct saved_state state;
};

/**
* Initialize an EGL context for the current display.
*/
static int engine_init_display(struct engine* engine) {
	// initialize OpenGL ES and EGL

	/*
	* Here specify the attributes of the desired configuration.
	* Below, we select an EGLConfig with at least 8 bits per color
	* component compatible with on-screen windows
	*/
	const EGLint attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_NONE
	};
	EGLint w, h, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;

	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	eglInitialize(display, 0, 0);

	/* Here, the application chooses the configuration it desires. In this
	* sample, we have a very simplified selection process, where we pick
	* the first EGLConfig that matches our criteria */
	eglChooseConfig(display, attribs, &config, 1, &numConfigs);

	/* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
	* guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
	* As soon as we picked a EGLConfig, we can safely reconfigure the
	* ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

	surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
	context = eglCreateContext(display, config, NULL, NULL);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		LOGW("Unable to eglMakeCurrent");
		return -1;
	}

	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	engine->display = display;
	engine->context = context;
	engine->surface = surface;
	engine->width = w;
	engine->height = h;
	engine->state.angle = 0;

	// Initialize GL state.
	glEnable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	MicroProfileOnThreadCreate("Main");
	MicroProfileSetForceEnable(true);
	MicroProfileSetEnableAllGroups(true);
	MicroProfileSetForceMetaCounters(true);
	MicroProfileGpuInitGL();
	MicroProfileWebServerStart();

	return 0;
}

/**
* Just the current frame in the display.
*/
static void engine_draw_frame(struct engine* engine) {
	if (engine->display == NULL) {
		// No display.
		return;
	}

	MICROPROFILE_SCOPEI("Main", "frame", -1);
	MICROPROFILE_SCOPEGPUI("frame", -1);

	// Just fill the screen with a color.
	{
		glClearColor(((float)engine->state.x) / engine->width, engine->state.angle,
			((float)engine->state.y) / engine->height, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		{
			MICROPROFILE_SCOPEGPUI("draw", -1);
			glDrawArrays(0, 0, 0);
		}
	}

	//MicroProfileDraw(128, 64);

	{
		MICROPROFILE_SCOPEI("Main", "eglSwapBuffers", 0);
		eglSwapBuffers(engine->display, engine->surface);
	}

	MicroProfileFlip();
}

/**
* Tear down the EGL context currently associated with the display.
*/
static void engine_term_display(struct engine* engine) {
	if (engine->display != EGL_NO_DISPLAY) {
		eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (engine->context != EGL_NO_CONTEXT) {
			eglDestroyContext(engine->display, engine->context);
		}
		if (engine->surface != EGL_NO_SURFACE) {
			eglDestroySurface(engine->display, engine->surface);
		}
		eglTerminate(engine->display);
	}
	engine->animating = 0;
	engine->display = EGL_NO_DISPLAY;
	engine->context = EGL_NO_CONTEXT;
	engine->surface = EGL_NO_SURFACE;
}

/**
* Process the next input event.
*/
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
	struct engine* engine = (struct engine*)app->userData;
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
		engine->state.x = AMotionEvent_getX(event, 0);
		engine->state.y = AMotionEvent_getY(event, 0);
		return 1;
	}
	return 0;
}

/**
* Process the next main command.
*/
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
	struct engine* engine = (struct engine*)app->userData;
	switch (cmd) {
	case APP_CMD_SAVE_STATE:
		// The system has asked us to save our current state.  Do so.
		engine->app->savedState = malloc(sizeof(struct saved_state));
		*((struct saved_state*)engine->app->savedState) = engine->state;
		engine->app->savedStateSize = sizeof(struct saved_state);
		break;
	case APP_CMD_INIT_WINDOW:
		// The window is being shown, get it ready.
		if (engine->app->window != NULL) {
			engine_init_display(engine);
			engine_draw_frame(engine);
		}
		break;
	case APP_CMD_TERM_WINDOW:
		// The window is being hidden or closed, clean it up.
		engine_term_display(engine);
		break;
	case APP_CMD_GAINED_FOCUS:
		// When our app gains focus, we start monitoring the accelerometer.
		if (engine->accelerometerSensor != NULL) {
			ASensorEventQueue_enableSensor(engine->sensorEventQueue,
				engine->accelerometerSensor);
			// We'd like to get 60 events per second (in microseconds).
			ASensorEventQueue_setEventRate(engine->sensorEventQueue,
				engine->accelerometerSensor, (1000L / 60) * 1000);
		}
		break;
	case APP_CMD_LOST_FOCUS:
		// When our app loses focus, we stop monitoring the accelerometer.
		// This is to avoid consuming battery while not being used.
		if (engine->accelerometerSensor != NULL) {
			ASensorEventQueue_disableSensor(engine->sensorEventQueue,
				engine->accelerometerSensor);
		}
		// Also stop animating.
		engine->animating = 0;
		engine_draw_frame(engine);
		break;
	}
}

/**
* This is the main entry point of a native application that is using
* android_native_app_glue.  It runs in its own thread, with its own
* event loop for receiving input events and doing other things.
*/
void android_main(struct android_app* state) {
	struct engine engine;

	memset(&engine, 0, sizeof(engine));
	state->userData = &engine;
	state->onAppCmd = engine_handle_cmd;
	state->onInputEvent = engine_handle_input;
	engine.app = state;

	// Prepare to monitor accelerometer
	engine.sensorManager = ASensorManager_getInstance();
	engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager,
		ASENSOR_TYPE_ACCELEROMETER);
	engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager,
		state->looper, LOOPER_ID_USER, NULL, NULL);

	if (state->savedState != NULL) {
		// We are starting with a previous saved state; restore from it.
		engine.state = *(struct saved_state*)state->savedState;
	}

	engine.animating = 1;

	// loop waiting for stuff to do.

	while (1) {
		// Read all pending events.
		int ident;
		int events;
		struct android_poll_source* source;

		// If not animating, we will block forever waiting for events.
		// If animating, we loop until all events are read, then continue
		// to draw the next frame of animation.
		while ((ident = ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,
			(void**)&source)) >= 0) {

			// Process this event.
			if (source != NULL) {
				source->process(state, source);
			}

			// If a sensor has data, process it now.
			if (ident == LOOPER_ID_USER) {
				if (engine.accelerometerSensor != NULL) {
					ASensorEvent event;
					while (ASensorEventQueue_getEvents(engine.sensorEventQueue,
						&event, 1) > 0) {
						//LOGI("accelerometer: x=%f y=%f z=%f", event.acceleration.x, event.acceleration.y, event.acceleration.z);
					}
				}
			}

			// Check if we are exiting.
			if (state->destroyRequested != 0) {
				engine_term_display(&engine);
				return;
			}
		}

		if (engine.animating) {
			// Done with events; draw next animation frame.
			engine.state.angle += .01f;
			if (engine.state.angle > 1) {
				engine.state.angle = 0;
			}

			// Drawing is throttled to the screen update rate, so there
			// is no need to do timing here.
			engine_draw_frame(&engine);
		}
	}

	MicroProfileOnThreadExit();
}

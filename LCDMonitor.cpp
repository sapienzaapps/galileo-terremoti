
#include "LCDMonitor.h"
#include "Log.h"
#include "Utils.h"
#include <math.h>

LCDMonitor *LCDMonitor::singleton = NULL;
volatile float LCDMonitor::curval = 0;
float LCDMonitor::olddwval = 0;

LCDMonitor::LCDMonitor() {
	int rc = pthread_create(&uiThread, NULL, uiWorker, NULL);
	if(rc) {
		Log::e("Error during SDL UI thread creation");
	}
	Log::i("LCD Monitor thread created");
}

LCDMonitor::~LCDMonitor() {
	pthread_cancel(uiThread);
	SDL_Quit();
	Log::i("LCD Monitor quit");
}

void *LCDMonitor::uiWorker(void *mem) {
	SDL_Window* window = NULL;
	if (SDL_Init( SDL_INIT_VIDEO ) < 0) {
		Log::e("Error during SDL init: %s", SDL_GetError());
		pthread_exit(NULL);
	}
	SDL_ShowCursor(SDL_DISABLE);

	SDL_Renderer *renderer;

	SDL_CreateWindowAndRenderer(LCDMONITOR_WIDTH, LCDMONITOR_HEIGHT, 0, &window, &renderer);
	if (window == NULL) {
		Log::e("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		SDL_Quit();
		pthread_exit(NULL);
	}

	SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

	// OSX seems to need these lines
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	bool quit = false;
	SDL_Event evt;
	int xpos = 0;
	while(!quit) {
		while(SDL_PollEvent(&evt) != 0) {
			if(evt.type == SDL_QUIT || (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_q)) {
				quit = true;
			}
		}

		float drawingval = sinf(xpos/1.5)*curval;

		int zoom = 10;

		// Clear screen line
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderDrawLine(renderer, xpos, 0, xpos, LCDMONITOR_HEIGHT);

		// Drawing current point
		SDL_SetRenderDrawColor(renderer, 0x00, 0xb8, 0x14, 0xFF);
		SDL_RenderDrawPoint(renderer, xpos, (LCDMONITOR_HEIGHT/2)-(int)((-drawingval)*zoom));

		// Drawing semi-continuos line
		if (fabs(olddwval - drawingval) > 1) {
			SDL_SetRenderDrawColor(renderer, 0x00, 0xb8, 0x14, 0xFF);
			SDL_RenderDrawLine(renderer, xpos, (LCDMONITOR_HEIGHT/2)-(int)((-drawingval)*zoom), xpos, (LCDMONITOR_HEIGHT/2)-(int)((-olddwval)*zoom));
		}
		olddwval = drawingval;

		int nextpos = xpos+1 == LCDMONITOR_WIDTH ? 0 : xpos+1;
		SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
		SDL_RenderDrawLine(renderer, nextpos, 0, nextpos, LCDMONITOR_HEIGHT);

		xpos++;
		if(xpos == LCDMONITOR_WIDTH) {
			xpos = 0;
		}
		SDL_RenderPresent(renderer);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();

	pthread_exit(NULL);
}

LCDMonitor* LCDMonitor::getInstance() {
	if(singleton == NULL) {
		singleton = new LCDMonitor();
	}
	return singleton;
}

void LCDMonitor::sendNewValue(float val) {
	curval = val;
}

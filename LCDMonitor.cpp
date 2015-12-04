
#include "LCDMonitor.h"
#include "Log.h"

LCDMonitor *LCDMonitor::singleton = NULL;
volatile float LCDMonitor::curval = 0;

LCDMonitor::LCDMonitor() {
	int rc = pthread_create(&uiThread, NULL, uiWorker, NULL);
	if(rc) {
		Log::e("Error during SDL UI thread creation");
	}
}

LCDMonitor::~LCDMonitor() {
	pthread_cancel(uiThread);
	SDL_Quit();
}

void *LCDMonitor::uiWorker(void *mem) {
	SDL_Window* window = NULL;
	if (SDL_Init( SDL_INIT_VIDEO ) < 0) {
		Log::e("Error during SDL init: %s", SDL_GetError());
		pthread_exit(NULL);
	}

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

		int zoom = 5;
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderDrawLine(renderer, xpos, 0, xpos, LCDMONITOR_HEIGHT);
		SDL_SetRenderDrawColor(renderer, 0x00, 0xb8, 0x14, 0xFF);
		SDL_RenderDrawPoint(renderer, xpos, (LCDMONITOR_HEIGHT/2)-(int)((-curval)*zoom));
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

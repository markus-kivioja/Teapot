#ifndef _CONSTANT_PARAMETERS_H_
#define _CONSTANT_PARAMETERS_H_

#define REVERSE_DEPTH

namespace
{
	const float RUN_SPEED_FACTOR = 1.0f;

	const float RAIN_SPEED = 0.27f;
	const float RAIN_RATE = 1.0f;
	const float RAIN_DROP_WIDTH = 0.01f;
	const float RAIN_DROP_HEIGHT = 1.0f;

	const float SPLASH_PARTICLE_RADIUS = 0.014f;
	const float SPLASH_PARTICLE_SPEED = 0.028f;
	const int SPLASH_PARTICLE_LIFESPAN = 400;
	const float SPLASH_PARTICLE_GRAVITY = 0.0028f;

	const float DRIBBLE_SLIDE_SPEED = 0.2f;
	const int DRIBBLE_LIFESPAN = 1000;

	const int WAVE_LIFESPAN = 1000;

	const float GBUF_LOOKUP_DISTANCE_FACTOR = 0.02f;
	const float TARGET_FRAME_TIME = 16.0f;

	const int BUFS_WIDTH = 1024;
	const int BUFS_HEIGHT = 1024;

	const float LIGHT_PARTICLE_RADIUS = 0.004f;

	const int POINT_LIGHT_COUNT = 100; // Must be greater than 0
	const float POINT_LIGHT_RADIUS = 1.0f;

	const int SPOT_LIGHT_COUNT = 4; // DON'T CHANGE

	const bool FULLSCREEN = true;
	const bool VSYNC = true;
}

#endif
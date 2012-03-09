/*
 * SDL OpenGL Tutorial.
 * (c) Michael Vance, 2000
 * briareos@lokigames.com
 *
 * Distributed under terms of the LGPL.
 */

#include <SDL.h>
#include <GLES/gl.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static GLboolean should_rotate = GL_TRUE;

static float width, height, angle;
static float cube_color[4];
static float pos_x, pos_y;
static float cube_pos_x, cube_pos_y, cube_pos_z;
GLfloat light_ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat light_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
GLfloat light_pos[] = { 0.0f, 25.0f, 0.0f, 1.0f };
GLfloat light_direction[] = { 0.0f, 0.0f, -30.0f, 1.0f };

static float cube_vertices[] = {
		// FRONT
		-2.0f, -2.0f, 2.0f, 2.0f, -2.0f, 2.0f, -2.0f,
		2.0f,
		2.0f,
		2.0f,
		2.0f,
		2.0f,
		// BACK
		-2.0f, -2.0f, -2.0f, -2.0f, 2.0f, -2.0f, 2.0f, -2.0f,
		-2.0f,
		2.0f,
		2.0f,
		-2.0f,
		// LEFT
		-2.0f, -2.0f, 2.0f, -2.0f, 2.0f, 2.0f, -2.0f, -2.0f, -2.0f,
		-2.0f,
		2.0f,
		-2.0f,
		// RIGHT
		2.0f, -2.0f, -2.0f, 2.0f, 2.0f, -2.0f, 2.0f, -2.0f, 2.0f, 2.0f,
		2.0f,
		2.0f,
		// TOP
		-2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, -2.0f, 2.0f, -2.0f, 2.0f, 2.0f,
		-2.0f,
		// BOTTOM
		-2.0f, -2.0f, 2.0f, -2.0f, -2.0f, -2.0f, 2.0f, -2.0f, 2.0f, 2.0f, -2.0f,
		-2.0f, };

float cube_normals[] = {
		// FRONT
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f,
		1.0f,
		// BACK
		0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		0.0f,
		-1.0f,
		// LEFT
		-1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		0.0f,
		// RIGHT
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		// TOP
		0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		// BOTTOM
		0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
		0.0f };

static void quit_tutorial( int code )
{
    /*
     * Quit SDL so we can release the fullscreen
     * mode and restore the previous video settings,
     * etc.
     */
    SDL_Quit( );

    /* Exit program. */
    exit( code );
}

static void handle_key_down( SDL_keysym* keysym )
{

    /*
     * We're only interested if 'Esc' has
     * been presssed.
     */
    switch( keysym->sym ) {
    case SDLK_ESCAPE:
        quit_tutorial( 0 );
        break;
    case SDLK_SPACE:
        should_rotate = !should_rotate;
        break;
    default:
        break;
    }

}

static void process_events( void )
{
    /* Our SDL event placeholder. */
    SDL_Event event;

    /* Grab all the events off the queue. */
    while( SDL_PollEvent( &event ) ) {

        switch( event.type ) {
        case SDL_KEYDOWN:
            /* Handle key presses. */
            handle_key_down( &event.key.keysym );
            break;
        case SDL_QUIT:
            /* Handle quit requests (like Ctrl-c). */
            quit_tutorial( 0 );
            break;
        }

    }

}

void enable_2d() {
	glViewport(0, 0, (int) width, (int) height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrthof(0.0f, width / height, 0.0f, 1.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glScalef(1.0f / height, 1.0f / height, 1.0f);
}

void enable_3d() {
	glViewport(0, 0, (int) width, (int) height);

	GLfloat aspect_ratio = width / height;

	GLfloat fovy = 45;
	GLfloat zNear = 1.0f;
	GLfloat zFar = 1000.0f;

	GLfloat top = tan(fovy * 0.0087266462599716478846184538424431f) * zNear;
	GLfloat bottom = -top;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glFrustumf(aspect_ratio * bottom, aspect_ratio * top, bottom, top, zNear,
			zFar);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void update() {
	angle = fmod((angle + 1.0f), 360.0 );
}

static void draw_screen( void )
{
	//Typical render pass
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Then render the cube
	enable_3d();
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);

	glTranslatef(cube_pos_x, cube_pos_y, cube_pos_z);

	glRotatef(30.0f, 1.0f, 0.0f, 0.0f);
	glRotatef(15.0f, 0.0f, 0.0f, 1.0f);
	glRotatef(angle, 0.0f, 1.0f, 0.0f);

	glColor4f(cube_color[0], cube_color[1], cube_color[2], cube_color[3]);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, cube_vertices);
	glNormalPointer(GL_FLOAT, 0, cube_normals);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 16, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 20, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_DEPTH_TEST);

    SDL_GL_SwapBuffers( );
}

void perspectiveGL( GLfloat fovY, GLfloat aspect, GLfloat zNear, GLfloat zFar )
{
	const GLfloat pi = 3.1415926535897932384626433832795;
	GLfloat fW, fH;

	fH = tan( (fovY / 2) / 180 * pi ) * zNear;
	fH = tan( fovY / 360 * pi ) * zNear;
	fW = fH * aspect;
	glFrustumf( -fW, fW, -fH, fH, zNear, zFar );
}

static void setup_opengl( int width, int height )
{
	cube_pos_x = 2.9f;
	cube_pos_y = 0.3f;
	cube_pos_z = -20.0f;

	cube_color[0] = 1.0f;
	cube_color[1] = 0.0f;
	cube_color[2] = 0.0f;
	cube_color[3] = 1.0f;

	//Common gl setup
	glShadeModel(GL_SMOOTH);
	glClearColor(0.775f, 0.775f, 0.775f, 1.0f);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light_direction);

	glEnable(GL_CULL_FACE);
}

int main( int argc, char* argv[] )
{
    /* Information about the current video settings. */
    const SDL_VideoInfo* info = NULL;
    /* Dimensions of our window. */
    /* Color depth in bits of our window. */
    int bpp = 0;
    /* Flags we will pass into SDL_SetVideoMode. */
    int flags = 0;

    /* First, initialize SDL's video subsystem. */
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        /* Failed, exit. */
        fprintf( stderr, "Video initialization failed: %s\n",
             SDL_GetError( ) );
        quit_tutorial( 1 );
    }

    /* Let's get some video information. */
    info = SDL_GetVideoInfo( );

    if( !info ) {
        /* This should probably never happen. */
        fprintf( stderr, "Video query failed: %s\n",
             SDL_GetError( ) );
        quit_tutorial( 1 );
    }

    /*
     * Set our width/height to 640/480 (you would
     * of course let the user decide this in a normal
     * app). We get the bpp we will request from
     * the display. On X11, VidMode can't change
     * resolution, so this is probably being overly
     * safe. Under Win32, ChangeDisplaySettings
     * can change the bpp.
     */
    width = 1024;
    height = 600;
    bpp = info->vfmt->BitsPerPixel;

    /*
     * Now, we want to setup our requested
     * window attributes for our OpenGL window.
     * We want *at least* 5 bits of red, green
     * and blue. We also want at least a 16-bit
     * depth buffer.
     *
     * The last thing we do is request a double
     * buffered window. '1' turns on double
     * buffering, '0' turns it off.
     *
     * Note that we do not use SDL_DOUBLEBUF in
     * the flags to SDL_SetVideoMode. That does
     * not affect the GL attribute state, only
     * the standard 2D blitting setup.
     */
//    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
//    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
//    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
//    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
//    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    /*
     * We want to request that SDL provide us
     * with an OpenGL window, in a fullscreen
     * video mode.
     */
    flags = SDL_OPENGL | SDL_FULLSCREEN;

    /*
     * Set the video mode
     */
    if( SDL_SetVideoMode( width, height, bpp, flags ) == 0 ) {
        /*
         * This could happen for a variety of reasons,
         * including DISPLAY not being set, the specified
         * resolution not being available, etc.
         */
        fprintf( stderr, "Video mode set failed: %s\n",
             SDL_GetError( ) );
        quit_tutorial( 1 );
    }

    /*
     * At this point, we should have a properly setup
     * double-buffered window for use with OpenGL.
     */
    setup_opengl( width, height );

    /*
     * Now we want to begin our normal app process--
     * an event loop with a lot of redrawing.
     */
    while( 1 ) {
        /* Process incoming events. */
        process_events( );
        /* Draw the screen. */
        draw_screen( );
        update();
    }

    /* Never reached. */
    return 0;
}

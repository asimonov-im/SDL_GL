/*
 * SDL OpenGL Tutorial.
 * (c) Michael Vance, 2000
 * briareos@lokigames.com
 *
 * Distributed under terms of the LGPL.
 */

#include <SDL.h>
#include <SDL_syswm.h>
#include <GLES/gl.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <bps/bps.h>
#include <bps/paymentservice.h>
#include <bps/navigator.h>
#include <errno.h>
#include <pthread.h>

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

static int _paymentProcessingStarted;
static pthread_t _paymentThread;

static void quit_tutorial( int code )
{
	if (_paymentProcessingStarted)
    	 pthread_join(_paymentThread, NULL);

	/*
     * Quit SDL so we can release the fullscreen
     * mode and restore the previous video settings,
     * etc.
     */
    SDL_Quit();

    /* Exit program. */
    exit(code);
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

/**
 * Handle the failure case for either event. Print the error information.
 */
void failureCommon(bps_event_t *event)
{
    if (event == NULL) {
        fprintf(stderr, "Invalid event.\n");
        return;
    }

    unsigned request_id = paymentservice_event_get_request_id(event);
    int error_id = paymentservice_event_get_error_id(event);
    const char* error_text = paymentservice_event_get_error_text(event);

    fprintf(stderr, "Payment System error. Request ID: %d  Error ID: %d  Text: %s\n",
            request_id, error_id, error_text ? error_text : "N/A");
}

/**
 * Upon successful completion of a purchase, print a string containing
 * information about the digital good that was purchased.
 */
void onPurchaseSuccess(bps_event_t *event)
{
    if (event == NULL) {
        fprintf(stderr, "Invalid event.\n");
        return;
    }

    unsigned request_id = paymentservice_event_get_request_id(event);
    const char* date = paymentservice_event_get_date(event, 0);
    const char* digital_good = paymentservice_event_get_digital_good_id(event, 0);
    const char* digital_sku = paymentservice_event_get_digital_good_sku(event, 0);
    const char* license_key = paymentservice_event_get_license_key(event, 0);
    const char* metadata = paymentservice_event_get_metadata(event, 0);
    const char* purchase_id = paymentservice_event_get_purchase_id(event, 0);

    fprintf(stderr, "Purchase success. Request Id: %d\n Date: %s\n DigitalGoodID: %s\n SKU: %s\n License: %s\n Metadata: %s\n PurchaseId: %s\n\n",
        request_id,
        date ? date : "N/A",
        digital_good ? digital_good : "N/A",
        digital_sku ? digital_sku : "N/A",
        license_key ? license_key : "N/A",
        metadata ? metadata : "N/A",
        purchase_id ? purchase_id : "N/A");
}

/**
 * On successful completion of a get existing purchases request,
 * print the existing purchases.
 */
void onGetExistingPurchasesSuccess(bps_event_t *event)
{
    if (event == NULL) {
        fprintf(stderr, "Invalid event.\n");
        return;
    }

    unsigned request_id = paymentservice_event_get_request_id(event);
    int purchases = paymentservice_event_get_number_purchases(event);

    fprintf(stderr, "Get existing purchases success. Request ID: %d\n", request_id);
    fprintf(stderr, "Number of existing purchases: %d\n", purchases);
    fprintf(stderr, "Existing purchases:\n");

    int i = 0;
    for (i = 0; i<purchases; i++) {
        const char* date = paymentservice_event_get_date(event, i);
        const char* digital_good = paymentservice_event_get_digital_good_id(event, i);
        const char* digital_sku = paymentservice_event_get_digital_good_sku(event, i);
        const char* license_key = paymentservice_event_get_license_key(event, i);
        const char* metadata = paymentservice_event_get_metadata(event, i);
        const char* purchase_id = paymentservice_event_get_purchase_id(event, i);

        fprintf(stderr, "  Date: %s  PurchaseID: %s  DigitalGoodID: %s  SKU: %s  License: %s  Metadata: %s\n",
            date ? date : "N/A",
            purchase_id ? purchase_id : "N/A",
            digital_good ? digital_good : "N/A",
            digital_sku ? digital_sku : "N/A",
            license_key ? license_key : "N/A",
            metadata ? metadata : "N/A");
    }
}

void payment_main(void *p)
{
    char *group_id = (char *)p;

	fprintf(stderr, "Starting payment processing thread with window group: %s\n", group_id);

    /*
     * Before we can listen for events from the BlackBerry Tablet OS platform
     * services, we need to initialize the BPS infrastructure
     */
    bps_initialize();

    /*
     * Once the BPS infrastructure has been initialized we can register for
     * events from the various BlackBerry Tablet OS platform services. The
     * Navigator service manages and delivers application life cycle and
     * visibility events.
     * For this sample, we request Navigator events so that we can track when
     * the system is terminating the application (NAVIGATOR_EXIT event), and as a
     * convenient way to trigger a purchase request (NAVIGATOR_SWIPE_DOWN).
     * We request PaymentService events so we can be notified when the payment service
     * responds to our requests/queries.
     */
    navigator_request_events(0);
    paymentservice_request_events(0);

    /*
     * Set the Payment Service connection mode to local. This allows us to
     * test the API without the need to contact the AppWorld nor payment servers.
     */
    paymentservice_set_connection_mode(true);

    /*
     * Create a set of purchase parameters, which describe the digital good
     * to be purchased and the application the goods are associated with.
     */
    const char* digital_good_id = "Digital-Good-1-ID";
    const char* digital_good_name = "Sample Digital Good 1";
    const char* digital_good_sku = "SAMPLE_DIGITAL_GOOD_SKU_1";
    const char* metadata = "Sample purchase metadata";
    const char* purchase_app_icon = "http://www.rim.com/products/appworld_3col.jpg";
    const char* purchase_app_name = "Payment Service Sample App";

    /*
     * Define a request ID to hold the returned value from the purchase request.
     */
    unsigned request_id = 0;

    /*
     * Process Payment Service and Navigator events until we receive a NAVIGATOR_EXIT event.
     */
    while (1) {
    	/*
         * Using a negative timeout (-1) in the call to bps_get_event(...)
         * ensures that we don't busy wait by blocking until an event is
         * available.
         */
        bps_event_t *event = NULL;
        bps_get_event(&event, -1);

        if (event) {
            /*
             * If it is a Payment Service event, determine the response code
             * and handle the event accordingly.
             */
            if (bps_event_get_domain(event) == paymentservice_get_domain()) {
                if (SUCCESS_RESPONSE == paymentservice_event_get_response_code(event)) {
                    if (PURCHASE_RESPONSE == bps_event_get_code(event)) {
                        onPurchaseSuccess(event);
                        unsigned request_id = 0;
                        if (paymentservice_get_existing_purchases_request(false, group_id, &request_id) != BPS_SUCCESS) {
                            fprintf(stderr, "Error: get existing purchases failed.\n");
                        }
                    } else
                        onGetExistingPurchasesSuccess(event);
                } else {
                    failureCommon(event);
                }
            }

            /*
             * If it is a NAVIGATOR_SWIPE_DOWN event, initiate the purchase of
             * the sample digital good.
             */
            if (bps_event_get_domain(event) == navigator_get_domain()) {
            	if (NAVIGATOR_EXIT == bps_event_get_code(event)) {
            		// Exit event processing loop
            		break;
            	} else if (NAVIGATOR_SWIPE_DOWN == bps_event_get_code(event)) {
                    if (paymentservice_purchase_request(digital_good_id, digital_good_sku, digital_good_name,
                            metadata, purchase_app_name, purchase_app_icon, group_id, &request_id) != BPS_SUCCESS) {
                        fprintf(stderr, "Error: purchase request failed.\n");
                    }
                }
            }
        }
    }

    /*
     * Clean up the BPS infrastructure and exit
     */
    bps_shutdown();

    fprintf(stderr, "Terminating payment processing thread\n");
    pthread_exit(0);
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
        fprintf( stderr, "Video initialization failed: %s\n", SDL_GetError());
        quit_tutorial( 1 );
    }

    /* Let's get some video information. */
    info = SDL_GetVideoInfo( );

    if( !info ) {
        /* This should probably never happen. */
        fprintf( stderr, "Video query failed: %s\n", SDL_GetError());
        quit_tutorial(1);
    }

    fprintf(stderr, "Current: %d x %d\n", info->current_w, info->current_h);

    width = 1024;
    height = 600;
    bpp = info->vfmt->BitsPerPixel;

    /*
     * We want to request that SDL provide us
     * with an OpenGL window, in a fullscreen
     * video mode.
     */
    flags = SDL_OPENGL | SDL_FULLSCREEN;

    /*
     * Set the video mode
     */
    if( SDL_SetVideoMode( 0, 0, bpp, flags ) == 0 ) {
        /*
         * This could happen for a variety of reasons,
         * including DISPLAY not being set, the specified
         * resolution not being available, etc.
         */
        fprintf( stderr, "Video mode set failed: %s\n", SDL_GetError());
        quit_tutorial(1);
    }

    /*
     * At this point, we should have a properly setup
     * double-buffered window for use with OpenGL.
     */
    setup_opengl( width, height );

    // Get the window group
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWMInfo(&wmInfo);
	char group[64];
	screen_get_window_property_cv(wmInfo.window, SCREEN_PROPERTY_GROUP, sizeof(group), group);

    // Create another thread for payment processing before entering SDL runtime loop
	_paymentProcessingStarted = 1;
    if (pthread_create(&_paymentThread, NULL, (void *)&payment_main, (void *)group)) {
		fprintf(stderr, "Failed to create payment processing thread: %s\n", strerror(errno));
		_paymentProcessingStarted = 0;
		quit_tutorial(1);
	}

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

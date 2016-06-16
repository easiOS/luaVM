#ifndef H_AMETHYST
#define H_AMETHYST

#include <video.h>

#define AM_MAX_WINDOWS 32

typedef struct am_win am_win;

typedef enum {
	KEYPRESS, KEYRELEASE
} am_event_type;

typedef struct {
	am_event_type type;
	union {
		struct {
			
		} key;
	} data;
} am_event;

struct am_win {
	int x, y; // coordinates of window
	int w, h; // dimensions of window
	int z; // level of window
	char title[64]; // title of window
    unsigned flags; //flags
    //flags:
    ///31-1 0
    ///rrrr F
    ///F - focused
    int (*load)(am_win*); // loads the program itself; args: (PTR to window); returns: status code, (0 = ok)
    int (*unload)(am_win*); // called before the window is deconstructed; args: (PTR to window); returns: status code, (0 = ok)
    void (*update)(am_win*, unsigned); // called every frame; args: (PTR to window, milliseconds elapsed since last call); returns: nothing
    void (*draw)(am_win*, int, int); // called every frame; args: (PTR to window, X coordinate of top-left point of window,
    								 // Y coordinate of top-left point of window); returns: nothing
    void (*event)(am_win*, am_event*); // called every event; args: (PTR to window, PTR to event); returns: nothing
    void* windata; // PTR to memory where the window is able to store it's data
    rgb_t bg; // background color of window
};

// PTR to window, ...

am_win* amethyst_create_window();
void amethyst_destroy_window(am_win* w);
int amethyst_main(int width, int height); // to be called by the kernel_main
int amethyst_init(am_win* wmw, int x, int y, int width, int height); // initializes the window manager
void amethyst_set_active(am_win* w);

#endif /* H_AMETHYST */
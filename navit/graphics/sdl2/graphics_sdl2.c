/**
 * Navit, a modular navigation system.
 * Copyright (C) 2005-2008 Navit Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include <glib.h>
#include "config.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <glib.h>
#include <pthread.h>
#include <poll.h>
#include <signal.h>
#include "config.h"
#include "debug.h"
#include "point.h"
#include "graphics.h"
#include "color.h"
#include "plugin.h"
#include "window.h"
#include "navit.h"
#include "keys.h"
#include "item.h"
#include "attr.h"
#include "item.h"
#include "attr.h"
#include "point.h"
#include "graphics.h"
#include "color.h"
#include "navit.h"
#include "event.h"
#include "debug.h"
#include "callback.h"
#include "font/freetype/font_freetype.h"
#include "graphics_sdl2.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>

SDL_Event event;


static struct callback_list* callbacks;

static struct graphics_priv {
	SDL_Window *gWindow;
	SDL_Renderer *gRenderer;
	SDL_Texture *gOverlay;
    int width;
    int height;
    int fullscreen;
    char *title;
    Uint32 bg_color;
    struct navit *nav;
    struct callback_list *cbl;
    struct font_freetype_methods freetype_methods;
    Uint32 video_flags;
} graphics_priv;

static struct graphics_font_priv {
	int size;
} graphics_font_priv;

static struct graphics_gc_priv {
	struct graphics_priv *gr;
    int dashed[4];
    SDL_Color fg;
	SDL_Color bg;
	int linewidth;
} graphics_gc_priv;

static struct graphics_image_priv {
	int w,h;
	SDL_Surface *img;
	SDL_Texture *tex;
} graphics_image_priv;

static void graphics_destroy(struct graphics_priv *gr) {
}

static void font_destroy(struct graphics_font_priv *font) {

}

static struct graphics_font_methods font_methods = {
    font_destroy
};

//static struct graphics_font_priv *font_new(struct graphics_priv *gr, struct graphics_font_methods *meth, char *font, int size, int flags) {
//    *meth=font_methods;
//    struct graphics_font_priv *this=g_new0(struct graphics_font_priv, 1);
//    this->size=size;
//    //this->font=TTF_OpenFont(font, 16);
//    //if(!font) {
//    //	dbg(lvl_debug,"TTF_OpenFont: %s\n", TTF_GetError());
//    //}
//    return &this;
//}

static void gc_destroy(struct graphics_gc_priv *gc) {
	g_free(gc);
}

static void gc_set_linewidth(struct graphics_gc_priv *gc, int w) {
	if(w>0){
		gc->linewidth=w;
	}
}

static void gc_set_dashes(struct graphics_gc_priv *gc, int w, int offset, unsigned char *dash_list, int n) {
}

static void gc_set_foreground(struct graphics_gc_priv *gc, struct color *c) {
    gc->fg.r = c->r/256;
    gc->fg.g = c->g/256;
    gc->fg.b = c->b/256;
    gc->fg.a = c->a/256;
}

static void gc_set_background(struct graphics_gc_priv *gc, struct color *c) {
    gc->bg.r = c->r/256;
    gc->bg.g = c->g/256;
    gc->bg.b = c->b/256;
    gc->bg.a = c->a/256;
}

static struct graphics_gc_methods gc_methods = {
    gc_destroy,
    gc_set_linewidth,
    gc_set_dashes,
    gc_set_foreground,
    gc_set_background
};

static struct graphics_gc_priv *gc_new(struct graphics_priv *gr, struct graphics_gc_methods *meth) {
    //*meth=gc_methods;
    //return &graphics_gc_priv;
    struct graphics_gc_priv *gc=g_new0(struct graphics_gc_priv, 1);
    *meth=gc_methods;
    gc->gr=gr;
    gc->linewidth=1; /* upper layer should override anyway? */
    return gc;
}

void image_destroy(struct graphics_image_priv *img){
	g_free(img);
}

static struct graphics_image_methods image_methods = {
	image_destroy
};

static struct graphics_image_priv *image_new(struct graphics_priv *gr, struct graphics_image_methods *meth, char *path, int *w, int *h, struct point *hot, int rotation) {
	struct graphics_image_priv *gi;

	/* FIXME: meth is not used yet.. so gi leaks. at least xpm is small */

	gi = g_new0(struct graphics_image_priv, 1);
	*meth=image_methods;
	if(!gi){
		return NULL;
	}
	gi->img = IMG_Load(path);

    if(gi->img) {
        *w=gi->img->w;
        *h=gi->img->h;
        gi->w=gi->img->w;
        gi->h=gi->img->h;
        hot->x=*w/2;
        hot->y=*h/2;
        gi->tex = SDL_CreateTextureFromSurface(gr->gRenderer, gi->img);
        SDL_FreeSurface(gi->img);
    } else {
        /* TODO: debug "colour parse errors" on xpm */
        dbg(lvl_error,"image_new on '%s' failed: %s", path, IMG_GetError());
        g_free(gi);
        gi = NULL;
    }


    return gi;
}

static void draw_lines(struct graphics_priv *gr, struct graphics_gc_priv *gc, struct point *p, int count) {
	//TODO: gc->linewidth not yet supported
	//TODO: gc->dashes    not yet supported
	struct point tmp;
	for (int i = 0; i < count-1; i++) {
		aalineRGBA(gr->gRenderer,p[i].x,p[i].y,p[i+1].x,p[i+1].y,gc->fg.r,gc->fg.g,gc->fg.b,gc->fg.a);
	}

}

static void draw_polygon(struct graphics_priv *gr, struct graphics_gc_priv *gc, struct point *p, int count) {
	Sint16 vx[count];
	Sint16 vy[count];
	for (int i = 0; i < count; i++) {
		vx[i]=(Sint16)p[i].x;
		vy[i]=(Sint16)p[i].y;
	}
	filledPolygonRGBA(gr->gRenderer,vx,vy,count,gc->fg.r,gc->fg.g,gc->fg.b,gc->fg.a);
	aapolygonRGBA(gr->gRenderer,vx,vy,count,gc->fg.r,gc->fg.g,gc->fg.b,gc->fg.a);

}

static void draw_rectangle(struct graphics_priv *gr, struct graphics_gc_priv *gc, struct point *p, int w, int h) {
    SDL_Rect r;
    r.x = p->x;
    r.y = p->y;
    r.w = w;
    r.h = h;
    SDL_SetRenderDrawColor( gr->gRenderer, gc->fg.r, gc->fg.g, gc->fg.b, gc->fg.a);
    SDL_RenderFillRect( gr->gRenderer, &r );
}

static void draw_circle(struct graphics_priv *gr, struct graphics_gc_priv *gc, struct point *p, int r) {
	filledCircleRGBA(gr->gRenderer,p->x,p->y,r,gc->fg.r,gc->fg.g,gc->fg.b,gc->fg.a);
}


static void draw_text(struct graphics_priv *gr, struct graphics_gc_priv *fg, struct graphics_gc_priv *bg, struct graphics_font_priv *font, char *text, struct point *p, int dx, int dy) {
    struct font_freetype_text *t;
    int color = 1;

    if (!font) {
        dbg(lvl_error, "no font, returning");
        return;
    }
    t = gr->freetype_methods.text_new(text, (struct font_freetype_font *) font, dx, dy);

    struct point p_eff;
    p_eff.x = p->x;
    p_eff.y = p->y;

    //display_text_draw(t, gr, fg, bg, color, &p_eff);
    gr->freetype_methods.text_destroy(t);
}

static void draw_image(struct graphics_priv *gr, struct graphics_gc_priv *fg, struct point *p, struct graphics_image_priv *img) {
    SDL_Rect r;

    r.x = p->x;
    r.y = p->y;
    r.w = img->w;
    r.h = img->h;

    SDL_RenderCopy(gr->gRenderer,img->tex, NULL,&r);
}

static void draw_drag(struct graphics_priv *gr, struct point *p) {
}

static void background_gc(struct graphics_priv *gr, struct graphics_gc_priv *gc) {
}

static void draw_mode(struct graphics_priv *gr, enum draw_mode_num mode) {
	switch (mode) {
		case draw_mode_begin:
	        SDL_RenderClear(gr->gRenderer);

			break;
		case draw_mode_end:
			SDL_RenderPresent(gr->gRenderer);
			break;
		default:
			break;
	}
}

static void overlay_draw_mode(struct graphics_priv *gr, enum draw_mode_num mode) {
	switch (mode) {
		case draw_mode_begin:
	        SDL_RenderClear(gr->gRenderer);
	        SDL_SetRenderTarget(gr->gRenderer, gr->gOverlay);
			break;
		case draw_mode_end:
			SDL_SetRenderTarget(gr->gRenderer, NULL);
			SDL_RenderCopy(gr->gRenderer, gr->gOverlay, NULL, NULL);
			SDL_RenderPresent(gr->gRenderer);

			break;
		default:
			break;
	}
}

static struct graphics_priv * overlay_new(struct graphics_priv *gr, struct graphics_methods *meth, struct point *p, int w, int h, int wraparound);

static void resize_callback(struct graphics_priv *gr, int w, int h) {
    dbg(lvl_debug,"resize_callback w:%i h:%i",w,h)
	gr->width=w;
	gr->height=h;
	callback_list_call_attr_2(gr->cbl, attr_resize, GINT_TO_POINTER(gr->width), GINT_TO_POINTER(gr->height));
}

static int graphics_sdl2_fullscreen(struct window *win, int on) {
	struct graphics_priv *gr=(struct graphics_priv *)win->priv;
    /* Update video flags */
    if(on) {
    	SDL_SetWindowFullscreen(gr->gWindow,SDL_WINDOW_FULLSCREEN_DESKTOP);
    } else {
    	SDL_SetWindowFullscreen(gr->gWindow,0);
    }
    return 1;
}

static void graphics_sdl2_disable_suspend(struct window *w) {
	SDL_DisableScreenSaver();
}

static void *get_data(struct graphics_priv *this, char const *type) {
    if (strcmp(type, "window") == 0) {
        struct window *win;
        win = g_new0(struct window, 1);
        win->priv = this;
        win->fullscreen = graphics_sdl2_fullscreen;
        win->disable_suspend = graphics_sdl2_disable_suspend;
        resize_callback(this,this->width,this->height);
        return win;
    }
    return NULL;
}

static void image_free(struct graphics_priv *gr, struct graphics_image_priv *priv) {
}


static void overlay_disable(struct graphics_priv *gr, int disable) {
	//int x=disable;
}

static void overlay_resize(struct graphics_priv *gr, struct point *p, int w, int h, int wraparound) {
}

static struct graphics_methods graphics_methods = {
    graphics_destroy,
    draw_mode,
    draw_lines, // todo
    draw_polygon,// done
    draw_rectangle,  // done
    draw_circle,// done
	draw_text,// todo
    draw_image,// done
    NULL, // draw_image_warp
	draw_drag,
	NULL, //font_new will be set at runtime
	gc_new,// done
	background_gc,
	overlay_new,
	image_new,
	get_data,
	image_free,
	NULL, //get_text_bbox will be set at runtime
	overlay_disable,
	NULL, //overlay_resize,
	NULL, //sett_attr,
    NULL, /* show_native_keyboard */
    NULL, /* hide_native_keyboard */
};

static struct graphics_priv *overlay_new(struct graphics_priv *gr, struct graphics_methods *meth, struct point *p, int w, int h, int wraparound) {
    *meth=graphics_methods;
    meth->draw_mode=overlay_draw_mode;
    meth->font_new=(struct graphics_font_priv *
                          (*)(struct graphics_priv *, struct graphics_font_methods *, char *, int,
                              int)) gr->freetype_methods.font_new;
    meth->get_text_bbox=(void*) gr->freetype_methods.get_text_bbox;
    struct graphics_priv *this=g_new0(struct graphics_priv, 1);
    this->gRenderer = gr->gRenderer;
    this->gOverlay = SDL_CreateTexture(gr->gRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);

    return &graphics_priv;
}

static gboolean graphics_sdl2_idle(void *data) {
	struct graphics_priv *gr = (struct graphics_priv *)data;
	int button= 4;

	int w;
	int h;
	struct point p;
	char keybuf[8];

	SDL_GL_GetDrawableSize(gr->gWindow, &w, &h);

	if((w != gr->width) || (h != gr->height)){
		resize_callback(gr,w,h);
	}
	//resize_callback(gr,gr->width,gr->height);

	//SDL_GetMouseState( &x, &y);
	//int num = SDL_NumJoysticks();
	while (SDL_PollEvent(&event)) {
		/* handle your event here */
//		if(event.key.repeat){
//			continue; // Bei festgehaltener Taste die zusätzlich generierten Tasteneingaben ignorieren
//		}
		switch( event.type ){
			case SDL_QUIT:
				callback_list_call_attr_0(gr->cbl, attr_window_closed);
				break;
			case SDL_KEYDOWN:
				keybuf[0]=event.key.keysym.sym;
				callback_list_call_attr_1(gr->cbl, attr_keypress, (void *)keybuf);
				break;

			case SDL_MOUSEWHEEL:
				if(event.wheel.y > 0){
					button = 4;
				} else {
					button = 5;
				}
				SDL_GetMouseState(&p.x,&p.y);

				callback_list_call_attr_3(gr->cbl, attr_button, GINT_TO_POINTER(1), GINT_TO_POINTER(button),
						(void *)&p);
				        callback_list_call_attr_3(gr->cbl, attr_button, GINT_TO_POINTER(0), GINT_TO_POINTER(button),
				        		(void *)&p);
				break;

			case SDL_MOUSEMOTION:
				p.x = event.motion.x;
				p.y = event.motion.y;
				callback_list_call_attr_1(gr->cbl, attr_motion, (void *)&p);
				break;

			case SDL_MOUSEBUTTONDOWN: {
				p.x = event.button.x;
				p.y = event.button.y;
				callback_list_call_attr_3(gr->cbl, attr_button, GINT_TO_POINTER(1), GINT_TO_POINTER((int)event.button.button), (void *)&p);
				break;
			}

			case SDL_MOUSEBUTTONUP: {
				p.x = event.button.x;
				p.y = event.button.y;
				callback_list_call_attr_3(gr->cbl, attr_button, GINT_TO_POINTER(0), GINT_TO_POINTER((int)event.button.button), (void *)&p);
				break;
			}

			default:
				break;
		}
	}
    return TRUE;
}

static struct graphics_priv *graphics_sdl2_new(struct navit *nav, struct graphics_methods *meth, struct attr **attrs, struct callback_list *cbl) {
	struct graphics_priv *this=g_new0(struct graphics_priv, 1);
    struct attr *attr;
    struct font_priv *(*font_freetype_new) (void *meth);

    *meth=graphics_methods;

    /* initialize fonts */
    font_freetype_new = plugin_get_category_font("freetype");

    if (!font_freetype_new) {
        g_free(this);
        return NULL;
    }

    font_freetype_new(&this->freetype_methods);


    meth->font_new = (struct graphics_font_priv *
                      (*)(struct graphics_priv *, struct graphics_font_methods *, char *, int,
                          int)) this->freetype_methods.font_new;
    meth->get_text_bbox = (void*) this->freetype_methods.get_text_bbox;

    this->nav = nav;
    this->cbl = cbl;

    this->width = 100;
    if ((attr = attr_search(attrs, NULL, attr_w)))
        this->width = attr->u.num;
    this->height = 100;
    if ((attr = attr_search(attrs, NULL, attr_h)))
        this->height = attr->u.num;
    if ((attr=attr_search(attrs, NULL, attr_window_title)))
        this->title=g_strdup(attr->u.str);
    else
        this->title=g_strdup("Navit");
    this->fullscreen=0;
    if ((attr = attr_search(attrs, NULL, attr_fullscreen))){
    	if(attr->u.num >= 1){
    		this->fullscreen=1;
    	}
    }


    if ((attr=attr_search(attrs, NULL, attr_background_color))) {
    	this->bg_color = (attr->u.color->a / 0x101) << 24
					   | (attr->u.color->r / 0x101) << 16
					   | (attr->u.color->g / 0x101) << 8
					   | (attr->u.color->b / 0x101);
		dbg(lvl_debug, "attr_background_color %04x %04x %04x %04x (%08x)",
			attr->u.color->r, attr->u.color->g, attr->u.color->b, attr->u.color->a, this->bg_color);
	} else {
		this->bg_color = 0xa0000000;
	}


	if (!event_request_system("glib", "graphics_sdl2")){
		return NULL;
    }
    callbacks = cbl;
    resize_callback(this,this->width,this->height);

    this->video_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
    if (this->fullscreen){
    	this->video_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    SDL_Init(SDL_INIT_EVERYTHING);

    int initted=IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG);
    if(!initted) {
        printf("IMG_Init: Failed to init!\n");
        printf("IMG_Init: %s\n", IMG_GetError());
        return NULL;
    }

	this->gWindow= SDL_CreateWindow(this->title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, this->width, this->height, this->video_flags );
	this->gRenderer = SDL_CreateRenderer(this->gWindow, -1, SDL_RENDERER_ACCELERATED);

	g_timeout_add(G_PRIORITY_DEFAULT+10, graphics_sdl2_idle, this);

	resize_callback(this,this->width,this->width);

    return this;
}






void plugin_init(void) {
    plugin_register_category_graphics("sdl2", graphics_sdl2_new);
    //plugin_register_category_event("sdl2", event_sdl2_new);
}

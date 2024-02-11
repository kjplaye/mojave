#include <SDL2/SDL.h>
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define FPS 60
#define FRAME_DELAY (1000 / FPS)

#define IMAGE_ERASE_FILE "mojave_erase.bmp"
#define IMAGE_PALETTE_FILE "mojave_palette.bmp"
#define IMAGE_ROTATION_FILE "mojave_rotation.bmp"
#define IMAGE_SLIDER_SPEED_FILE "mojave_speed.bmp"
#define IMAGE_SLIDER_ZOOM_FILE "mojave_zoom.bmp"
#define IMAGE_SLIDER_POINT_FILE "mojave_point.bmp"
#define IMAGE_SLIDER_INTENSITY_FILE "mojave_intensity.bmp"
#define IMAGE_CUT_FILE "mojave_cut.bmp"

#define TTF_FILE_FILES {"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", "/usr/share/fonts/dejavu/DejaVuSans.ttf"}
#define FONT_NUM_LOCATIONS 2

#define POINT_SCREEN 0
#define CONTROL_SCREEN 1
#define BRUSH_SCREEN 2
#define SCREENS 3
#define SCREEN_WIDTHS {800,660,800}
#define SCREEN_HEIGHTS {800,1000,200}
#define SCREEN_XPOSES {0,850,0}
#define SCREEN_YPOSES {0,0,900}
#define MAX_STRING 100

#define CONTROL_BG_COLOR 0x404040
#define CONTROL_FG_COLOR 0x3080ff
#define CONTROL_RADIUS 32
#define CONTROL_Y_STEP 80
#define CONTROL_LINE_STEP 0.01
#define CONTROL_ARROW_COLOR 0xffffff
#define CONTROL_ARROW_THICKNESS 1
#define CONTROL_BOX_RADIUS 32
#define CONTROL_NUM_BOX 4
#define CONTROL_BOX_FG 0xff0000
#define CONTROL_BOX_BG 0x0080ff
#define CONTROL_BOX_SELECT 0x000050
#define CONTROL_BOX_SHIFT 1.2
#define CONTROL_NUMBER_X 60
#define CONTROL_NUMBER_XLOC 20
#define CONTROL_NUMBER_YLOC 17
#define POINT_ZOOM 300
#define POINT_ZOOM_MULT 1.05
#define ROTATION_SPEED_MULT 1.05
#define ZOOM_GROVE_Y 100
#define SPEED_GROVE_X 440
#define ZOOM_GROVE_X 480
#define POINTSIZE_GROVE_X 520
#define INTENSITY_GROVE_X 560
#define ZOOM_GROVE_WIDTH 5
#define ZOOM_GROVE_HEIGHT 400
#define ZOOM_SLIDER_WIDTH 30
#define ZOOM_SLIDER_HEIGHT 10
#define ZOOM_GROVE_COLOR 0x000000
#define ZOOM_SLIDER_COLOR 0xffffff
#define ZOOM_LOG_RATIO 18.0
#define RANDOM_SEED (lrand48())
#define CONTROL_SCROLL_DELTA 20
#define CONTROL_SCROLL_DELTA_PAGE 200
#define POINTSIZE_LOG_RATIO 80.0
#define POINTSIZE_DIV 4.0

#define BRUSH_BG 0x404040
#define BIT_RADIUS 8
#define BIT_X_SKIP 25
#define BIT_COLOR 0xffffff
#define BIT_SELECT_COLOR 0x8080ff
#define PALETTE_BOX_SIZE 30
#define PALETTE_BOX_SKIP 60
#define SELECTED_PALETTE_BOX_BG 0x000000
#define BRUSH_MODE_MARGIN 30
#define BRUSH_MODE_BOX_SIZE 50
#define ROTATION_MODE_MARGIN_X 52
#define ROTATION_MODE_MARGIN_Y 10
#define ROTATION_MODE_BOX_SIZE 50
#define PALETTE_BOX_MARGIN 10
#define BIT_USED_COLOR 0xffff00
#define BIT_UNUSED_COLOR 0x909090
#define BRUSH_COLOR_MODES 8
#define BRUSH_COLOR_MODE_DIRECT 0
#define BRUSH_SIZE_RATIO 1.05
#define DEFAULT_BRUSH_XSIZE 32
#define DEFAULT_BRUSH_YSIZE 32
#define OFFSCREEN -100
#define PALETTE_ICON_SEPARATOR_X 490
#define ERASER_BRUSH_COLOR 0x606060
#define PIE_CHART_SIZE 50
#define PIE_CHART_X 500
#define PIE_CHART_Y 90

#define R_POWERS 13
#define RX_THETA_MAX 0.01
#define RY_THETA_MAX 0.01
#define KEYBOARD_ROTATION_DX 0
#define KEYBOARD_ROTATION_DY 10
#define UNDO_SIZE 1024
#define MAX_TEXT_NUMBER 256
#define GRID_COLOR 0x808080
#define DEFAULT_POINT_SIZE 3
#define MIN_POINT_SIZE 1
#define MAX_POINT_SIZE 16
#define POINT_SIZE_STEP 0.75
#define MAX_DECIMATION_MODE 4

#define SQR(x) ((x)*(x))
#define LCG(x) ((134775813 * (x) + 2531011) & 0xffffff)
#define COLOR_HASH(x,t) LCG(LCG(x) ^ LCG(LCG((t) + 12345))  )
#define RANDOM(n) ((int)(drand48() * (n)))

#define ERROR(x) {fprintf(stderr,(x));fprintf(stderr,"\n");exit(1);}
#define point(i, x, y) pnt[i][(int)(x) + (int)(y)*SCREEN_WIDTH[i]]
#define AA(i, j) ((i)*dim + (j))
#define RR(k, i, j) ((k) * dim * dim + (i) * dim + (j))
#define MIN(x,y) ((x)<(y)) ? (x) : (y)
#define MAX(x,y) ((x)<(y)) ? (y) : (x)

// SDL window stuff
SDL_Window *screen[SCREENS];
SDL_Surface *screen_surface[SCREENS];
SDL_Renderer * renderer[SCREENS];
SDL_Texture * texture[SCREENS];
unsigned * pnt[SCREENS];
int SCREEN_WIDTH[SCREENS] = SCREEN_WIDTHS;
int SCREEN_HEIGHT[SCREENS] = SCREEN_HEIGHTS;
int SCREEN_XPOS[SCREENS] = SCREEN_XPOSES;
int SCREEN_YPOS[SCREENS] = SCREEN_YPOSES;
SDL_Surface * text[MAX_TEXT_NUMBER];
SDL_Surface * text_x;
SDL_Surface * text_y;
SDL_Surface * text_r;
SDL_Surface * text_xy;
SDL_Surface * text_decimation[3];
TTF_Font* font;
char ttf_file[FONT_NUM_LOCATIONS][MAX_STRING] = TTF_FILE_FILES;

// Global transform data
// A shape (dim, dim) is the projection onto the first 2 dimensions.
// R is such that R[k] = R^(2^k)
double *A;
double rotation_direction_exists = 0;
double *Rx;
double *Rx_inv;
double *Ry;
double *Ry_inv;
int dim;

// Global image stuff
SDL_Surface *image_erase;
SDL_Surface *image_palette;
SDL_Surface *image_rotation;
SDL_Surface *image_slider_speed;
SDL_Surface *image_slider_zoom;
SDL_Surface *image_slider_point;
SDL_Surface *image_slider_intensity;
SDL_Surface *image_cut;

// box is shape (dim, num-boxes)
int (*box)[CONTROL_NUM_BOX];

// point rendering
double gamma_correct = 1.0;
unsigned char gamma_map[256];
double point_size = DEFAULT_POINT_SIZE;
SDL_Surface * point_surface = NULL;
SDL_Texture * point_texture = NULL;

// Pallete = (x >> mask_location) & 7
int mask_location = 0;

// Brush color
int selected_color = 0;

// Pallete mode modifier.
int use_color_hash = 0;

// Decimation mode
int decimation_mode = 0;
uint32_t decimation_color[MAX_DECIMATION_MODE] = {0x000080, 0x008000, 0x808000,
                                                  0x800000};
int decimation[MAX_DECIMATION_MODE] = {1,10,100,1000};

// brush info
int brush_color_mode = 0;
int brush_xsize = DEFAULT_BRUSH_XSIZE;
int brush_ysize = DEFAULT_BRUSH_YSIZE;
int brush_x = OFFSCREEN;
int brush_y = OFFSCREEN;
unsigned brush_color[8] = {
			   0xffffff,
			   0x0000ff,
			   0x00ff00,
			   0x00ffff,
			   0xff0000,
			   0xff00ff,
			   0xffff00,
			   0x808080};

// Rotation info
unsigned rotation_seed = 0;

// Control scroll
int control_scroll = 0;

// Sliders
double zoom_ratio = 1.0;
double rotation_speed = 1.0;

// Mode flags
int rotation_mode = 0;
int rotation_mode_color[2] = {0xff0000, 0x00ff00}; 
int erase_mode_on = 0;

// Undo info
int undo_length = 1;
int max_undo_length = 1;

// Mouse info
int last_mouse_x = -1;
int last_mouse_y = -1;

// SDL refresh
void refresh(int i)
{
  SDL_UpdateTexture(texture[i], NULL, pnt[i], SCREEN_WIDTH[i] * sizeof(unsigned));
  SDL_RenderClear(renderer[i]);
  SDL_RenderCopy(renderer[i], texture[i], NULL, NULL);
  SDL_RenderPresent(renderer[i]);
}

// SDL screen init
void screen_init(int i, int init, char * name, int xpos, int ypos)
{
  if (init)
    {
      screen[i] = SDL_CreateWindow(name, xpos, ypos,
				   SCREEN_WIDTH[i], SCREEN_HEIGHT[i],
				   SDL_WINDOW_RESIZABLE);
      //      screen_surface[i] = SDL_GetWindowSurface(screen[i]);
      renderer[i] = SDL_CreateRenderer(screen[i], -1, SDL_RENDERER_ACCELERATED);

      SDL_RendererInfo info;
      SDL_GetRendererInfo(renderer[i], &info);
    }
  SDL_RenderClear(renderer[i]);
  SDL_GL_SwapWindow(screen[i]);
  texture[i] = SDL_CreateTexture(renderer[i],SDL_PIXELFORMAT_ARGB8888,
			      SDL_TEXTUREACCESS_STREAMING,
			      SCREEN_WIDTH[i], SCREEN_HEIGHT[i]);
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,
  	      "linear");  // make the scaled rendering look smoother.
  SDL_RenderSetLogicalSize(renderer[i], SCREEN_WIDTH[i], SCREEN_HEIGHT[i]);

  if (pnt[i]) free(pnt[i]);
  if ((pnt[i] = malloc(SCREEN_WIDTH[i] * SCREEN_HEIGHT[i] * sizeof(unsigned))) == 0) ERROR("OUT OF MEMORY");
}

void blt(SDL_Surface * surface, int screen_index, int x, int y,
	 int width, int height, unsigned icon_color)
{
  for(int dy = 0; dy < height; dy++)
    for(int dx = 0; dx < width; dx++)
      {
	unsigned rx = surface->w * dx / width;
       	unsigned ry = surface->h * dy / height;
	unsigned c = * (uint32_t *) ((uint8_t *) surface->pixels
				     + ry * surface->pitch
				     + rx * surface->format->BytesPerPixel);
	if ((c & 0xffffff) == 0xffffff) continue;
	point(screen_index, x + dx, y + dy) = icon_color;
      }
}

// Get RGB color value from logical color label
unsigned get_color(unsigned color_value)
{
  if (brush_color_mode == BRUSH_COLOR_MODE_DIRECT)
    return brush_color[(color_value >> mask_location) & 7];
  else
    return COLOR_HASH(color_value, brush_color_mode);
}

// The global transformation to point window coordinates (define by A)
void transform(double * data_point, double * out_x, double * out_y)
{
  double x = 0.0;
  double y = 0.0;
  for(int j=0; j < dim; j++)
    {
      x += A[AA(j,0)] * data_point[j];
      y += A[AA(j,1)] * data_point[j];
    }
  double x0 = SCREEN_WIDTH[POINT_SCREEN]/2.0;
  double y0 = SCREEN_HEIGHT[POINT_SCREEN]/2.0;
  double xs = x * POINT_ZOOM * zoom_ratio + x0;
  double ys = y * POINT_ZOOM * zoom_ratio + y0;

  // Use clipped points
  if (xs < 0) xs = 0.0;
  if (ys < 0) ys = 0.0;
  if (xs >= SCREEN_WIDTH[POINT_SCREEN]) xs = SCREEN_WIDTH[POINT_SCREEN] - 1;
  if (ys >= SCREEN_HEIGHT[POINT_SCREEN]) ys = SCREEN_HEIGHT[POINT_SCREEN] - 1;

  *out_x = xs;
  *out_y = ys;
}

void xy_transform(double * data_point, double * out_x, double * out_y,
		  int i, int j, int * xy_dim, int xy_cnt)
{
  double x0 = (j + 0.5) * SCREEN_WIDTH[POINT_SCREEN] / xy_cnt;
  double y0 = (i + 0.5) * SCREEN_HEIGHT[POINT_SCREEN] / xy_cnt;
  double dx = data_point[xy_dim[i]];
  double dy = data_point[xy_dim[j]];
  dx *= POINT_ZOOM * zoom_ratio;
  dy *= POINT_ZOOM * zoom_ratio;
  dx /= xy_cnt;
  dy /= xy_cnt;
  if (dx < -0.5 * SCREEN_WIDTH[POINT_SCREEN] / xy_cnt)
    dx = -0.5 * SCREEN_WIDTH[POINT_SCREEN] / xy_cnt;
  if (dy < -0.5 * SCREEN_WIDTH[POINT_SCREEN] / xy_cnt)
    dy = -0.5 * SCREEN_WIDTH[POINT_SCREEN] / xy_cnt;
  if (dx > 0.5 * SCREEN_WIDTH[POINT_SCREEN] / xy_cnt)
    dx = 0.5 * SCREEN_WIDTH[POINT_SCREEN] / xy_cnt;
  if (dy > 0.5 * SCREEN_WIDTH[POINT_SCREEN] / xy_cnt)
    dy = 0.5 * SCREEN_WIDTH[POINT_SCREEN] / xy_cnt;
  
  *out_x = y0 + dx;
  *out_y = x0 + dy;
}


// Draws one of the sliders onto the control window.
void draw_slider(double log_ratio, double ratio, double grove_width,
		 double grove_height, double grove_x, double grove_y,
		 double slider_width, double slider_height,
		 unsigned grove_color, unsigned slider_color)
{
  int slider_y = (log_ratio * log2(ratio)) + grove_height / 2.0;
  if (slider_y < 0) slider_y = 0;
  if (slider_y >= grove_height) slider_y = grove_height - 1;
  for(int y = grove_y; y < grove_y + grove_height; y++)
    for(int x = grove_x; x < grove_x + grove_width; x++)
      point(CONTROL_SCREEN, x + CONTROL_NUMBER_X, y) = grove_color;
  for(int dy = -slider_height/2; dy <= slider_height/2; dy++)
    for(int dx = -slider_width/2; dx <= slider_width/2; dx++)
      point(CONTROL_SCREEN, CONTROL_NUMBER_X + grove_x + grove_width/2 + dx,
	    grove_y + dy + slider_y) = slider_color;
}

void blt_text(int screen_number, SDL_Surface * the_text, int x, int y,
	      unsigned color)
{
  for(int dx=0;dx<the_text->w;dx++)
    for(int dy=0;dy<the_text->h;dy++)
      {
	uint8_t c = * (uint8_t *) ((uint8_t *) the_text->pixels
				   + dy * the_text->pitch
				   + dx * the_text->format->BytesPerPixel);
	if (c)
	  {
	    int xx = x + dx;
	    int yy = y + dy;
	    if (xx>=0 && yy>=0 && xx < SCREEN_WIDTH[screen_number]
		&& yy < SCREEN_HEIGHT[screen_number])
	      point(screen_number, xx, yy) = color;
	  }
      }
}

// Draws the control window
void draw_controls()
{
  // Background fill
  for(int y=0; y<SCREEN_HEIGHT[1]; y++)
    for(int x=0; x<SCREEN_WIDTH[1]; x++)
      point(CONTROL_SCREEN,x,y) = CONTROL_BG_COLOR;

  // Main controls
  for(int i = 0; i < dim; i++)
    {
      // Text
      int number = i % MAX_TEXT_NUMBER;
      blt_text(CONTROL_SCREEN, text[number], CONTROL_NUMBER_XLOC,
	       CONTROL_NUMBER_YLOC + i*CONTROL_Y_STEP - control_scroll, 0xa0a0a0);
      
      // Circle displays
      for(int y=0;y<2*CONTROL_RADIUS;y++)
	{
	  int yy = y + i*CONTROL_Y_STEP - control_scroll;
	  if (yy < 0 || yy >= SCREEN_HEIGHT[CONTROL_SCREEN]) continue;
	  int y0 = y - CONTROL_RADIUS;
	  for(int x=0;x<2*CONTROL_RADIUS;x++)
	    {
	      int xx = x + CONTROL_NUMBER_X;   	 
	      int x0 = x - CONTROL_RADIUS;
	      if (SQR(x0) + SQR(y0) <= SQR(CONTROL_RADIUS))
		{
		  if (xx < 0 || xx >= SCREEN_WIDTH[CONTROL_SCREEN]) continue;
		  point(CONTROL_SCREEN, xx, yy) = CONTROL_FG_COLOR;
		}
	    }
	}
      double dx = CONTROL_RADIUS * A[AA(i,0)];
      double dy = CONTROL_RADIUS * A[AA(i,1)];
      for(double r=0.0; r<1.0; r+=CONTROL_LINE_STEP)
	{
	  for(int dx2=-CONTROL_ARROW_THICKNESS;dx2<=CONTROL_ARROW_THICKNESS;dx2++)
	    {
	      int xx =CONTROL_NUMBER_X + dx2 + (int)(CONTROL_RADIUS+dx*r);
	      if (xx <= 0 || xx >= SCREEN_WIDTH[CONTROL_SCREEN]) continue;
	      for(int dy2=-CONTROL_ARROW_THICKNESS;dy2<=CONTROL_ARROW_THICKNESS;
		  dy2++)
		{  		 
		  int yy = dy2 + (int)(i*CONTROL_Y_STEP+CONTROL_RADIUS+dy*r)
		    - control_scroll;
		  if (yy <= 0 || yy >= SCREEN_HEIGHT[CONTROL_SCREEN]) continue;
		  point(CONTROL_SCREEN, xx, yy) = CONTROL_ARROW_COLOR;
		}
	    }
	}
      
      // Boxes
      SDL_Surface * box_text[4] = {text_x, text_y, text_r, text_xy};
      for(int bi = 0; bi < CONTROL_NUM_BOX; bi++)
	{
	  for(int y=-CONTROL_BOX_RADIUS;y<=CONTROL_BOX_RADIUS;y++)
	    {
	      int yy = y+i*CONTROL_Y_STEP + CONTROL_RADIUS - control_scroll;
	      if (yy < 0 || yy >= SCREEN_HEIGHT[CONTROL_SCREEN]) continue;
	      for(int x=-CONTROL_BOX_RADIUS;x<=CONTROL_BOX_RADIUS;x++)
		{
		  int xx = CONTROL_NUMBER_X +
		    x+CONTROL_Y_STEP*(bi+CONTROL_BOX_SHIFT) + CONTROL_RADIUS;
		  if (xx < 0 || xx >= SCREEN_WIDTH[CONTROL_SCREEN]) continue;
		  int color;
		  if (y==-CONTROL_BOX_RADIUS || x==-CONTROL_BOX_RADIUS || \
		      y==CONTROL_BOX_RADIUS || x==CONTROL_BOX_RADIUS)
		    color = CONTROL_BOX_FG;
		  else
		    color = box[i][bi] ? CONTROL_BOX_SELECT : CONTROL_BOX_BG;
		  point(CONTROL_SCREEN, xx, yy) = color;
		}
	    }
	  int x = 27+CONTROL_NUMBER_X + CONTROL_Y_STEP*(bi+CONTROL_BOX_SHIFT);
	  int y = i*CONTROL_Y_STEP + CONTROL_RADIUS - control_scroll - 13;
	  if (bi == 3) x-=12;
	  blt_text(CONTROL_SCREEN, box_text[bi], x, y, 0x900000);
	}
    }

  // Rotation mode indicator
  blt(image_rotation, CONTROL_SCREEN,
      SCREEN_WIDTH[CONTROL_SCREEN] - ROTATION_MODE_MARGIN_X
      - ROTATION_MODE_BOX_SIZE - 60,
      ROTATION_MODE_MARGIN_Y,
      ROTATION_MODE_BOX_SIZE, ROTATION_MODE_BOX_SIZE,
      rotation_mode_color[rotation_mode]);

  // Decimation indicator
  blt(image_cut, CONTROL_SCREEN,
      SCREEN_WIDTH[CONTROL_SCREEN] - ROTATION_MODE_MARGIN_X
      - ROTATION_MODE_BOX_SIZE + 20,
      ROTATION_MODE_MARGIN_Y - 3,
      ROTATION_MODE_BOX_SIZE, ROTATION_MODE_BOX_SIZE,
      decimation_color[decimation_mode]);

  // Decimation text
  if (decimation_mode)
    blt_text(CONTROL_SCREEN, text_decimation[decimation_mode - 1],
	     25 - 10 * decimation_mode + SCREEN_WIDTH[CONTROL_SCREEN] -
	     ROTATION_MODE_MARGIN_X - ROTATION_MODE_BOX_SIZE + 20,
	     ROTATION_MODE_MARGIN_Y + 10, 0xffffff);
  
  // Draw sliders
  draw_slider(ZOOM_LOG_RATIO, zoom_ratio, ZOOM_GROVE_WIDTH, ZOOM_GROVE_HEIGHT,
	      ZOOM_GROVE_X, ZOOM_GROVE_Y, ZOOM_SLIDER_WIDTH, ZOOM_SLIDER_HEIGHT,
	      ZOOM_GROVE_COLOR, ZOOM_SLIDER_COLOR);
  draw_slider(ZOOM_LOG_RATIO, rotation_speed, ZOOM_GROVE_WIDTH, ZOOM_GROVE_HEIGHT,
	      SPEED_GROVE_X, ZOOM_GROVE_Y, ZOOM_SLIDER_WIDTH, ZOOM_SLIDER_HEIGHT,
	      ZOOM_GROVE_COLOR, ZOOM_SLIDER_COLOR);
  draw_slider(POINTSIZE_LOG_RATIO, point_size/POINTSIZE_DIV, ZOOM_GROVE_WIDTH,
	      ZOOM_GROVE_HEIGHT, POINTSIZE_GROVE_X, ZOOM_GROVE_Y, ZOOM_SLIDER_WIDTH,
	      ZOOM_SLIDER_HEIGHT, ZOOM_GROVE_COLOR, ZOOM_SLIDER_COLOR);
  draw_slider(ZOOM_LOG_RATIO, gamma_correct, ZOOM_GROVE_WIDTH,
	      ZOOM_GROVE_HEIGHT, INTENSITY_GROVE_X, ZOOM_GROVE_Y, ZOOM_SLIDER_WIDTH,
	      ZOOM_SLIDER_HEIGHT, ZOOM_GROVE_COLOR, ZOOM_SLIDER_COLOR);

  // Slider indicators
  blt(image_slider_speed, CONTROL_SCREEN,
      SCREEN_WIDTH[CONTROL_SCREEN] - ROTATION_MODE_MARGIN_X - 81 - 40,
      ROTATION_MODE_MARGIN_Y + 55,
      ROTATION_MODE_BOX_SIZE/1.7, ROTATION_MODE_BOX_SIZE/1.7,
      0xa0a0ff);
  blt(image_slider_zoom, CONTROL_SCREEN,
      SCREEN_WIDTH[CONTROL_SCREEN] - ROTATION_MODE_MARGIN_X - 81,
      ROTATION_MODE_MARGIN_Y + 55,
      ROTATION_MODE_BOX_SIZE/1.7, ROTATION_MODE_BOX_SIZE/1.7,
      0xa0a0ff);
  blt(image_slider_point, CONTROL_SCREEN,
      SCREEN_WIDTH[CONTROL_SCREEN] - ROTATION_MODE_MARGIN_X - 81 + 40,
      ROTATION_MODE_MARGIN_Y + 55,
      ROTATION_MODE_BOX_SIZE/1.7, ROTATION_MODE_BOX_SIZE/1.7,
      0xa0a0ff);
  blt(image_slider_intensity, CONTROL_SCREEN,
      SCREEN_WIDTH[CONTROL_SCREEN] - ROTATION_MODE_MARGIN_X - 81 + 80,
      ROTATION_MODE_MARGIN_Y + 55,
      ROTATION_MODE_BOX_SIZE/1.7, ROTATION_MODE_BOX_SIZE/1.7,
      0xa0a0ff);
}

int cmp_int64(const void * p1, const void * p2)
{
  uint64_t x1 = *((uint64_t *) p1);
  uint64_t x2 = *((uint64_t *) p2);
  if (x1 < x2) return -1;
  if (x2 < x1) return 1;
  return 0;
}

// Draws the brush window
void draw_palette(int num_data, double (*data)[dim], int32_t * color,
		  int32_t * hide)
{
  unsigned mask = 0;
  for(int i = 0; i < num_data; i++) mask |= color[i];
  
  for(int y=0;y<SCREEN_HEIGHT[BRUSH_SCREEN];y++)
    for(int x=0;x<SCREEN_WIDTH[BRUSH_SCREEN];x++)
      point(BRUSH_SCREEN,x,y) = BRUSH_BG;

  for(int bx=0;bx<30;bx++)
    {
      int bit_color = ((mask >> bx) & 1) ? BIT_USED_COLOR : BIT_UNUSED_COLOR;
      for(int x=-BIT_RADIUS;x<=BIT_RADIUS;x++)
	for(int y=-BIT_RADIUS;y<=BIT_RADIUS;y++)
	  {
	    if (SQR(x) + SQR(y) <= SQR(BIT_RADIUS))
	      {
		point(BRUSH_SCREEN, x + (bx + 1) * BIT_X_SKIP,
		      30 + y + BIT_RADIUS) = bit_color;
	      }
	  }

      if (bx % 3 != 1) continue;

      bit_color = ((mask >> bx) & 7) ? BIT_USED_COLOR : BIT_UNUSED_COLOR;

      for(int x=-BIT_RADIUS;x<=BIT_RADIUS;x++)
	for(int y=-BIT_RADIUS;y<=BIT_RADIUS;y++)
	  {
	    if (SQR(x) + SQR(y) <= SQR(BIT_RADIUS))
	      {
		point(BRUSH_SCREEN, x + (bx + 1) * BIT_X_SKIP,
		      30 + y + BIT_RADIUS + BIT_X_SKIP + 10) = bit_color;
	      }
	  }
    }

  if (brush_color_mode == BRUSH_COLOR_MODE_DIRECT)
    {
      for(int x = 0; x < 3 * BIT_X_SKIP; x++)
	for(int y = 0; y < 5; y++)
	  {
	    point(BRUSH_SCREEN, BIT_X_SKIP*mask_location + x+10,
		  y + BIT_X_SKIP / 2) = BIT_SELECT_COLOR;
	    point(BRUSH_SCREEN, BIT_X_SKIP*mask_location + x+10,
		  y + 30 + BIT_X_SKIP) = BIT_SELECT_COLOR;
	  }
    }
  
  if (brush_color_mode == BRUSH_COLOR_MODE_DIRECT)
    {
      for(int c=0; c < 8; c++)
	{
	  if (c == selected_color)
	    {
 	      for(int x=-PALETTE_BOX_MARGIN;
		  x<PALETTE_BOX_SIZE+PALETTE_BOX_MARGIN;x++)
		for(int y=-PALETTE_BOX_MARGIN;
		    y<PALETTE_BOX_SIZE+PALETTE_BOX_MARGIN;y++)
		  point(BRUSH_SCREEN, BIT_X_SKIP/2 + x + c*PALETTE_BOX_SKIP,
			y+130) = SELECTED_PALETTE_BOX_BG;      
	    }
	  
	  for(int x=0;x<PALETTE_BOX_SIZE;x++)
	    for(int y=0;y<PALETTE_BOX_SIZE;y++)
	      point(BRUSH_SCREEN, BIT_X_SKIP/2 + x + c*PALETTE_BOX_SKIP,
		    y+130) = brush_color[c];      
	}
    }
  else
    {
      int c0 = selected_color - 4;
      if (c0 < 0) c0 = 0;
      for(int c=0; c < 8; c++)
	{
	  if (c0 + c == selected_color)
	    {
	      for(int x=-PALETTE_BOX_MARGIN;
		  x<PALETTE_BOX_SIZE+PALETTE_BOX_MARGIN;x++)
		for(int y=-PALETTE_BOX_MARGIN;
		    y<PALETTE_BOX_SIZE+PALETTE_BOX_MARGIN;y++)
		  point(BRUSH_SCREEN, BIT_X_SKIP/2 + x + c*PALETTE_BOX_SKIP,
			y+130) = SELECTED_PALETTE_BOX_BG;
	    }
	  
	  for(int x=0;x<PALETTE_BOX_SIZE;x++)
	    for(int y=0;y<PALETTE_BOX_SIZE;y++)
	      point(BRUSH_SCREEN, BIT_X_SKIP/2 + x + c*PALETTE_BOX_SKIP, y+130)
		= COLOR_HASH(c0 + c, brush_color_mode);
	}
    }
  
  // Draw brush mode icon
  blt(image_erase, BRUSH_SCREEN,
      SCREEN_WIDTH[BRUSH_SCREEN] - BRUSH_MODE_MARGIN - BRUSH_MODE_BOX_SIZE,
      SCREEN_HEIGHT[BRUSH_SCREEN] - BRUSH_MODE_MARGIN - BRUSH_MODE_BOX_SIZE,
      BRUSH_MODE_BOX_SIZE, BRUSH_MODE_BOX_SIZE,
      erase_mode_on ? 0x00ff00 : 0xff0000);

  // Draw color mode icon
  blt(image_palette, BRUSH_SCREEN,
      SCREEN_WIDTH[BRUSH_SCREEN] - 2 * BRUSH_MODE_MARGIN - 2 * BRUSH_MODE_BOX_SIZE,
      SCREEN_HEIGHT[BRUSH_SCREEN] - BRUSH_MODE_MARGIN - BRUSH_MODE_BOX_SIZE,
      BRUSH_MODE_BOX_SIZE, BRUSH_MODE_BOX_SIZE,
      brush_color_mode ? COLOR_HASH(0,brush_color_mode) : 0xff0000);

  // Text - numbering of boxes
  if (brush_color_mode == BRUSH_COLOR_MODE_DIRECT)
    {
      for(int i = 0; i < 8;i++)
	{
	  int number = i % MAX_TEXT_NUMBER;	  
	  blt_text(BRUSH_SCREEN, text[number],
		   9 + BIT_X_SKIP/2 + i*PALETTE_BOX_SKIP , 40 + 130, 0xa0a0a0);
	}
    }
  else 
    {
      int c0 = selected_color - 4;
      if (c0 < 0) c0 = 0;

      for(int i = 0; i < 8;i++)
	{

	  int number = (i + c0) % MAX_TEXT_NUMBER;	  
	  blt_text(BRUSH_SCREEN, text[number],
		   9 + BIT_X_SKIP/2 + i*PALETTE_BOX_SKIP , 40 + 130, 0xa0a0a0);
	}
    }


  // Text - selected color
  char the_text[MAX_STRING];
  SDL_Color white = {255, 255, 255};
  SDL_Surface * text_surface;
  sprintf(the_text, "%x", selected_color);
  text_surface = TTF_RenderText_Solid(font, the_text, white);
  if (text_surface == NULL) ERROR("TTF_RenderText_Solid failure");
  blt_text(BRUSH_SCREEN, text_surface,
	   SCREEN_WIDTH[BRUSH_SCREEN] - 2 * BRUSH_MODE_MARGIN -
	   2 *BRUSH_MODE_BOX_SIZE, 40 + 130, 0xa0a0a0);
  SDL_FreeSurface(text_surface);

  // Pie-chart
  uint64_t sorted_color[num_data];
  for(int i=0;i<num_data;i++) sorted_color[i] =
				(((uint64_t) get_color(color[i]) ) << 32) + hide[i];
  qsort(sorted_color, num_data, sizeof(uint64_t), &cmp_int64);
  for(int y=0;y<2*PIE_CHART_SIZE;y++)
    for(int x=0;x<2*PIE_CHART_SIZE;x++)
      {
	int dx = x - PIE_CHART_SIZE;
	int dy = y - PIE_CHART_SIZE;
	int c = 0;
        if (SQR(dx) + SQR(dy) >= SQR(PIE_CHART_SIZE)) continue;
	uint64_t sc = sorted_color[(int)(num_data * (atan2(dy,dx) + M_PI)
					 / (2 * M_PI + 0.0001))];
	if (SQR(dx) + SQR(dy) >= SQR(3.0*PIE_CHART_SIZE/4.0)
	    || !(sc & 0xffffffff)) c = sc >> 32;
	point(BRUSH_SCREEN, x + PIE_CHART_X, y + PIE_CHART_Y) = c;	  
      }
}

void xy_tally(int *xy_dim, int *xy_cnt)
{
  for(int i=0;i<dim;i++)
    {
      if (box[i][3]) xy_dim[(*xy_cnt)++] = i;
    }
}

void draw_point(int x, int y, unsigned c)
{
  SDL_Rect dst_rect = {x - point_size, y - point_size,
		       point_surface->w, point_surface->h};
  int r = gamma_map[(c>>16) & 0xff];
  int g = gamma_map[(c>>8) & 0xff];
  int b = gamma_map[(c>>0) & 0xff];
  SDL_SetTextureColorMod(point_texture, r, g, b);
  SDL_RenderCopy(renderer[POINT_SCREEN], point_texture, NULL, &dst_rect);
}

// Draws the main view - points window
void draw_points(int num_data, double (*data)[dim], int32_t * color, int32_t * hide)
{
  int xy_dim[dim];
  int xy_cnt = 0;
  xy_tally(xy_dim, &xy_cnt);
  
  // Draw points
  if (xy_cnt)
    {
      // xy multiplot mode
      for(int i=0;i<xy_cnt;i++)
	for(int j=0;j<xy_cnt;j++)
	  {
	    // Draw points
	    for(int k=0;k<num_data;k+=decimation[decimation_mode])
	      {
		if (hide[k]) continue;
		double x,y;
		xy_transform(data[k], &x, &y, i, j, xy_dim, xy_cnt);		
		draw_point(x,y,get_color(color[k]));
	      }
	    // Draw grid
	    // TODO - fix this, it currently doesn't display.
	    SDL_SetRenderDrawColor(renderer[POINT_SCREEN],
				   (GRID_COLOR >> 16) & 0xff,
				   (GRID_COLOR >> 8) & 0xff,
				   (GRID_COLOR >> 0) & 0xff,
				   255);
	    // Horizontal lines.
            SDL_Rect hline = {0,j * SCREEN_HEIGHT[POINT_SCREEN] / xy_cnt,
			     SCREEN_WIDTH[POINT_SCREEN],1};	   
	    SDL_RenderFillRect(renderer[POINT_SCREEN], &hline);
	    // Vertical lines.
            SDL_Rect vline = {i * SCREEN_HEIGHT[POINT_SCREEN] / xy_cnt,0,
			      1,SCREEN_HEIGHT[POINT_SCREEN]};	   
	    SDL_RenderFillRect(renderer[POINT_SCREEN], &vline);
	    
	    //	    for(int x=0;x<SCREEN_WIDTH[POINT_SCREEN];x++)
	    //	      point(POINT_SCREEN, x, j * SCREEN_HEIGHT[POINT_SCREEN] / xy_cnt)
	    //		= GRID_COLOR;
	    //	    for(int y=0;y<SCREEN_HEIGHT[POINT_SCREEN];y++)
	    //	      point(POINT_SCREEN, i * SCREEN_WIDTH[POINT_SCREEN] / xy_cnt, y)
	    //	      = GRID_COLOR;

	    
	    SDL_SetRenderDrawColor(renderer[POINT_SCREEN], 0,0,0,255); 

	  }
    }
  else
    {
      // Standard plot
      for(int i=0; i < num_data; i+=decimation[decimation_mode])
	{
   	  if (hide[i]) continue;
	  double x,y;
	  transform(data[i], &x, &y);
	  draw_point(x,y,get_color(color[i]));
	}
    }

  if (brush_xsize == 0 || brush_ysize == 0) return;
  
  // Draw brush rectangle
  unsigned brush_cursor_color = (brush_color_mode == BRUSH_COLOR_MODE_DIRECT) ?
    brush_color[selected_color] : COLOR_HASH(selected_color, brush_color_mode);
  if (erase_mode_on) brush_cursor_color = ERASER_BRUSH_COLOR;
  if (brush_x >= 0 && brush_y >= 0)
    {
      SDL_Rect rect = {brush_x, brush_y, brush_xsize, brush_ysize};
      SDL_SetRenderDrawColor(renderer[POINT_SCREEN],
			     (brush_cursor_color >> 16) & 0xff,
			     (brush_cursor_color >> 8) & 0xff,
			     (brush_cursor_color >> 0) & 0xff,
			     255);
      if (erase_mode_on)
	SDL_RenderFillRect(renderer[POINT_SCREEN], &rect);
      else
	SDL_RenderDrawRect(renderer[POINT_SCREEN], &rect);
      SDL_SetRenderDrawColor(renderer[POINT_SCREEN], 0,0,0,255);         
  }

  SDL_RenderPresent(renderer[POINT_SCREEN]);
}

// Sets out to the identity
void SO_clear(double * out)
{
  for(int i=0;i<dim;i++)
    for(int j=0;j<dim;j++)
      out[AA(i,j)] = i==j;
}

// Multiplies rotations matrices
void SO_mult(double * out, double * A1, double * A2)
{
  double temp[SQR(dim)];
  for(int i=0;i<dim;i++)
    for(int j=0;j<dim;j++)
      {
	temp[AA(i,j)] = 0.0;
	for(int k=0;k<dim;k++) temp[AA(i, j)] += A1[AA(i,k)] * A2[AA(k,j)];
    }
  for(int i=0;i<dim;i++)
    for(int j=0;j<dim;j++)
      out[AA(i,j)] = temp[AA(i,j)];
}

// Inverts a rotation matrix (easy since it is just atranspose.)
void SO_inverse(double *out, double *in)
{
  for(int i=0;i<dim;i++)
    for(int j=0;j<dim;j++)
      out[AA(i,j)] = in[AA(j,i)];
}

// Create a rotation matrix whiche rotates in the dim1/dim2 plane
void SO_pair(double * out, int dim1, int dim2, double theta)
{
  for(int i=0;i<dim;i++)
    for(int j=0;j<dim;j++)
      out[AA(i,j)] = (i==j);
  out[AA(dim1,dim1)] = cos(theta);
  out[AA(dim1,dim2)] = -sin(theta);
  out[AA(dim2,dim1)] = sin(theta);
  out[AA(dim2,dim2)] = cos(theta);
}

// Create powers of a rotation matrix to expedite special powers.
void SO_powers(double * R)
{
  for(int i=1;i<R_POWERS;i++)
    SO_mult(&R[i*dim*dim], &R[0], &R[(i-1)*dim*dim]);
}

// Special power (using SO_powers)
void SO_power(double *out, double *R, int power)
{
  SO_clear(out);
  for(int i=0;i<R_POWERS;i++)
    {
      if ((power >> i) & 1) SO_mult(out,out,&R[i*dim*dim]);
    }
}

// Compute the dz-th power of Rz
void rotate_dim(double * Rz, double * Rz_inv, int dz)
{
  double Z[SQR(dim)];
  if (dz > 0)
    {
      SO_power(Z, Rz, dz);
      SO_mult(A, Z, A);
    }
  else if (dz < 0)
    {
      SO_power(Z, Rz_inv, -dz);
      SO_mult(A, Z, A);
    }
}

// Rotate using dx and dy (powers)
void SO_rotate(int dx, int dy)
{
  rotate_dim(Rx, Rx_inv, dx);
  rotate_dim(Ry, Ry_inv, dy);
}

// Pick new rotation directions for dx and dy
void new_rotation_direction(unsigned seed)
{
  rotation_seed = seed;
  srand48(seed);
  
  // Reset rotation directions
  int cnt = 0;
  for(int i=0;i<dim;i++) cnt += box[i][2];
  if (cnt < 2) return;     

  // Set up rotations Rx a pair
  int ind_1 = RANDOM(cnt);
  int ind_2 = RANDOM(cnt-1);
  int dim1, dim2;
  if (ind_2 >= ind_1) ind_2++;
  cnt = 0;
  for(int i=0;i<dim;i++)
    {
      if (cnt == ind_1) dim1 = i;
      if (cnt == ind_2) dim2 = i;
      cnt += box[i][2];
    }
  double theta = (1.0-2.0*drand48()) * RX_THETA_MAX * rotation_speed;
  SO_pair(&Rx[0], dim1, dim2, theta);
  SO_powers(Rx);
  SO_inverse(&Rx_inv[0], &Rx[0]); 
  SO_powers(Rx_inv);
  
  // Set up rotations Ry a bunch of pairs
  SO_clear(&Ry[0]);
  for(int k=0;k<SQR(dim);k++)
    {
      int ind_1 = RANDOM(cnt);
      int ind_2 = RANDOM(cnt-1);
      int dim1, dim2;
      if (ind_2 >= ind_1) ind_2++;
      cnt = 0;
      for(int i=0;i<dim;i++)
	{
	  if (cnt == ind_1) dim1 = i;
	  if (cnt == ind_2) dim2 = i;
	  cnt += box[i][2];
	}
      double theta = (1.0-2.0*drand48()) * RY_THETA_MAX * rotation_speed;
      double P[SQR(dim)];
      SO_pair(P, dim1, dim2, theta);
      SO_mult(&Ry[0], P, &Ry[0]);
    }      
  SO_powers(Ry);
  SO_inverse(&Ry_inv[0], &Ry[0]); 
  SO_powers(Ry_inv);
  
  // Set rotation direction flag
  rotation_direction_exists = 1;
}

void change_rotation_mode()
{
  erase_mode_on = 0;
  rotation_mode = !rotation_mode;
  new_rotation_direction(RANDOM_SEED);
  if (rotation_mode)
    {
      int cnt = 0;
      for(int i=0;i<dim;i++) cnt+=box[i][2];
      if (cnt < 2)
	{
	  for(int i=0;i<dim;i++)
	    {
	      box[i][0] = 0;
	      box[i][1] = 0;
	      box[i][2] = 1;
	      box[i][3] = 0;
	    }
	}
    }
}

void create_point_texture()
{
  int surface_dim = 2 * point_size + 1;
  if (point_surface) SDL_FreeSurface(point_surface);
  if (point_texture) SDL_DestroyTexture(point_texture);
  point_surface = SDL_CreateRGBSurface(0,surface_dim,surface_dim,32,0,0,0,0);
  for(int dy=-point_size;dy<=point_size;dy++)
    for(int dx=-point_size;dx<=point_size;dx++)
      {
	unsigned color = (SQR(dx) + SQR(dy) <= SQR(point_size) - 0.1) ?
	  0xffffffff : 0x00000000;
	int x = dx + point_size;
	int y = dy + point_size;
	Uint32 * const target_pixel =
	  (Uint32 *) ((Uint8 *) point_surface->pixels
		      + y * point_surface->pitch
		      + x * point_surface->format->BytesPerPixel);
	*target_pixel = color;
      }
  point_texture = SDL_CreateTextureFromSurface(renderer[POINT_SCREEN],
					       point_surface);
  SDL_SetTextureBlendMode(point_texture, SDL_BLENDMODE_ADD);
}

void set_gamma()
{
  for(int i=0;i<256;i++) gamma_map[i] = 256 * pow(i/256.0, gamma_correct);
}

// Move the slides given an (x,y) choice
void move_sliders(int x, int y, int is_clicking)
{
  static int active_toggle;
  int middle_grove_x0 = (SPEED_GROVE_X + ZOOM_GROVE_X) / 2  - 40;
  int middle_grove_x1 = (SPEED_GROVE_X + ZOOM_GROVE_X) / 2;
  int middle_grove_x2 = (ZOOM_GROVE_X + POINTSIZE_GROVE_X) / 2;
  int middle_grove_x3 = (POINTSIZE_GROVE_X + INTENSITY_GROVE_X) / 2;
  double ratio = pow(2, (y - ZOOM_GROVE_Y - ZOOM_GROVE_HEIGHT / 2.0)
		     / ZOOM_LOG_RATIO);
  double ratio2 = POINTSIZE_DIV * pow(2, (y - ZOOM_GROVE_Y
					  - ZOOM_GROVE_HEIGHT / 2.0)
				      / POINTSIZE_LOG_RATIO);
  if (x >= middle_grove_x0 && x < middle_grove_x1 &&
      (is_clicking || active_toggle == 1))
    {
      if (is_clicking) active_toggle = 1;
      rotation_speed = ratio;
      new_rotation_direction(rotation_seed);
    }
  else if (x >= middle_grove_x1 && x < middle_grove_x2 &&
	   (is_clicking || active_toggle == 2))
    {
      if (is_clicking) active_toggle = 2;
      zoom_ratio = ratio;
    }
  else if (x >= middle_grove_x2 && x < middle_grove_x3 &&
	   (is_clicking || active_toggle == 3))
    {
      if (is_clicking) active_toggle = 3;
      point_size = ratio2;
      create_point_texture();
    }
  else if (x >= middle_grove_x3 && (is_clicking || active_toggle == 4))
    {
      if (is_clicking) active_toggle = 4;
      gamma_correct = ratio;
      set_gamma();
    }
}

void create_text()
{
  char the_text[MAX_STRING];
  SDL_Color white = {255, 255, 255};
  
  for(int i = 0; i < MAX_TEXT_NUMBER; i++)
    {
      sprintf(the_text, "%x", i);
      text[i] = TTF_RenderText_Solid(font, the_text, white);
      if (text[i] == NULL) ERROR("TTF_RenderText_Solid failure");	
    }

  sprintf(the_text, "x");
  text_x = TTF_RenderText_Solid(font, the_text, white);
  if (text_x == NULL) ERROR("TTF_RenderText_Solid failure");

  sprintf(the_text, "y");
  text_y = TTF_RenderText_Solid(font, the_text, white);
  if (text_y == NULL) ERROR("TTF_RenderText_Solid failure");

  sprintf(the_text, "r");
  text_r = TTF_RenderText_Solid(font, the_text, white);
  if (text_r == NULL) ERROR("TTF_RenderText_Solid failure");
  
  sprintf(the_text, "x/y");
  text_xy = TTF_RenderText_Solid(font, the_text, white);
  if (text_xy == NULL) ERROR("TTF_RenderText_Solid failure");

  for(int i = 0; i<3 ; i++)
    {
      sprintf(the_text, "%d", (int) pow(10, i+1));
      text_decimation[i] = TTF_RenderText_Solid(font, the_text, white);
      if (text_decimation[i] == NULL) ERROR("TTF_RenderText_Solid failure");
    }
}

void service_box_0_1(int i, int bi)
{
  rotation_mode = 0;
  int flag = 0;
  for(int j=0;j<dim;j++)
    {
      flag |= box[j][2];
      box[j][3] = box[j][2] = 0;
    }
  if (flag) SO_clear(A);
  if (!box[i][bi])
    {
      if (!box[i][!bi])
	{
	  box[i][bi] = 1;
	  A[AA(i,bi)] = 1.0;
	  for(int j=0;j<dim;j++)
	    {
	      if (i!=j)
		{
		  A[AA(j,bi)] = 0;
		  box[j][bi] = 0;
		}
	    }
	}
      else
	{
	  int j;
	  for(j=0;j<dim;j++)
	    {
	      if (box[j][bi] == 1) break;
	    }
	  box[i][bi] = 1;
	  A[AA(i,bi)] = 1.0;
	  box[i][!bi] = 0;
	  A[AA(i,!bi)] = 0.0;
	  box[j][bi] = 0;
	  A[AA(j,bi)] = 0,0;
	  box[j][!bi] = 1;
	  A[AA(j,!bi)] = 1.0;
	}
    }				
}

void service_box_2(int i, int bi)
{
  int cnt = 0;
  for(int j=0;j<dim;j++) cnt+=box[j][2];
  rotation_direction_exists = 0;
  if ((cnt>1) | !box[i][bi])
    box[i][bi] = !box[i][bi];
  for(int j=0;j<dim;j++)
    {
      A[AA(j,0)] = A[AA(j,1)] = 0.0;
      box[j][0] = box[j][1] = box[j][3] = 0;
    }
  double sum2 = 0.0;
  for(int j=0;j<dim;j++)
    {
      if (box[j][bi])
	{
	  A[AA(j,0)] = 1 - 2*drand48();
	  sum2 += SQR(A[AA(j,0)]);
	}
    }
  sum2 = sqrt(sum2);
  for(int j=0;j<dim;j++) A[AA(j,0)] /= sum2;
  double dot = 0.0;
  for(int j=0;j<dim;j++)
    {
      if (box[j][bi])
	{
	  A[AA(j,1)] = 1 - 2*drand48();
	  dot += A[AA(j,0)] * A[AA(j,1)];
	}
    }
  for(int j=0;j<dim;j++)
    A[AA(j,1)] -= dot * A[AA(j,0)];
  sum2 = 0.0;
  for(int j=0;j<dim;j++) sum2 += SQR(A[AA(j,1)]);
  sum2 = sqrt(sum2);
  for(int j=0;j<dim;j++) A[AA(j,1)] /= sum2;
}

void service_box_3(int i, int bi)
{
  box[i][bi] = !box[i][bi];
  if (box[i][bi])
    {
      for(int j=0;j<dim;j++)
	for (int k=0;k<3;k++)
	  box[j][k] = 0;
    }
}

void clear_all()
{
  brush_x = OFFSCREEN;
  brush_y = OFFSCREEN;
  brush_xsize = DEFAULT_BRUSH_XSIZE;
  brush_ysize = DEFAULT_BRUSH_YSIZE;
  rotation_direction_exists = 0;
  erase_mode_on = 0;
  brush_color_mode = 0;
}

void undo_save(int num_data, int32_t undo[UNDO_SIZE][num_data],
	       int32_t undo_hide[UNDO_SIZE][num_data],
	       int32_t * color, int32_t * hide)
{
  if (undo_length < UNDO_SIZE)
    {
      int save_undo = 0;
      if (undo_length == 0) save_undo = 1;
      else
	{
	  for(int i = 0; i < num_data; i++)
	    {
	      if (color[i] != undo[undo_length - 1][i] ||
		  hide[i] != undo_hide[undo_length - 1][i])
		{
		  save_undo = 1;
		  break;
		}
	    }
	}
      if (save_undo)
	{
	  for(int i=0;i<num_data;i++)
	    {
	      undo[undo_length][i] = color[i];
	      undo_hide[undo_length][i] = hide[i];
	    }
	  undo_length++;
	  max_undo_length = undo_length;
	}
    }
}

void color_picker(int * mouse_x, int * mouse_y, double (*data)[dim],
		  int * color, int num_data)
//void color_picker(double (*data)[dim], int * color, int num_data)
{
  double min_dist_sqr = INFINITY;
  unsigned best_color = 0x000000;
  for(int i = 0; i < num_data; i++)
    {
      double trans_x, trans_y;
      transform(data[i], &trans_x, &trans_y);
      double this_dist_sqr = SQR(trans_x - *mouse_x) +
	SQR(trans_y - *mouse_y);
      if (this_dist_sqr < min_dist_sqr)
	{
	  min_dist_sqr = this_dist_sqr;
	  best_color = color[i];
	}
    }
  if (brush_color_mode == BRUSH_COLOR_MODE_DIRECT)
    selected_color = (best_color >> mask_location) & 7;
  else
    selected_color = best_color;
  printf("Brush color = %x\n", selected_color);
}

void service_left_button_on_point(int mouse_x, int mouse_y, double (*data)[dim],
				 int * color, int * hide, int num_data)
{
  if (SDL_GetModState() & KMOD_CTRL)
    {
      if (brush_x < 0 || brush_y < 0)
	{
	  brush_x = mouse_x;
	  brush_y = mouse_y;
	  brush_xsize = DEFAULT_BRUSH_XSIZE;
	  brush_ysize = DEFAULT_BRUSH_YSIZE;
	}
      else
	{
	  brush_xsize = mouse_x - brush_x;
	  brush_ysize = mouse_y - brush_y;
	}
    }
  else
    {
      int xy_dim[dim];
      int xy_cnt = 0;
      xy_tally(xy_dim, &xy_cnt);
      brush_x = mouse_x - brush_xsize;
      brush_y = mouse_y - brush_ysize;
      for(int k=0;k<num_data;k++)
	{
	  if (hide[k]) continue;
	  double out_points[dim*dim][2];
	  int num_out_points = 0;
	  if (xy_cnt)
	    {
	      for(int i=0;i<xy_cnt;i++)
		for(int j=0;j<xy_cnt;j++)
		  {
		    xy_transform(data[k], &out_points[num_out_points][0],
				 &out_points[num_out_points][1], i, j, xy_dim,
				 xy_cnt);			
		    num_out_points++;		       
		  }
	    }
	  else
	    {
	      transform(data[k], &out_points[0][0], &out_points[0][1]);
	      num_out_points++;
	    }
	  
	  for(int l=0;l<num_out_points;l++)
	    {
	      double x = out_points[l][0];
	      double y = out_points[l][1];
	      int x1 = (brush_xsize >= 0)
		? brush_x : brush_x + brush_xsize;
	      int x2 = (brush_xsize < 0)
		? brush_x : brush_x + brush_xsize;
	      int y1 = (brush_ysize >= 0)
		? brush_y : brush_y + brush_ysize;
	      int y2 = (brush_ysize < 0)
		? brush_y : brush_y + brush_ysize;
	      if (x >= x1 && x < x2 && y >= y1 && y < y2)
		{
		  if (erase_mode_on)
		    hide[k] = 1;
		  else if (brush_color_mode == BRUSH_COLOR_MODE_DIRECT)
		    color[k] = (color[k] &
				((7 << mask_location) ^ 0xffffffff))
		      ^ (selected_color << mask_location);
		  else
		    color[k] = selected_color;
		}
	    }
	}
    }
}

void service_mouse_motion_on_point(int mouse_x, int mouse_y, int mouse_state,
				   double (*data)[dim], int * color, int * hide,
				   int num_data)
{
  if (mouse_state & SDL_BUTTON_LMASK)
    service_left_button_on_point(mouse_x, mouse_y, data, color, hide, num_data);
  if (mouse_state & SDL_BUTTON_RMASK)
    {
      brush_x = OFFSCREEN;
      brush_y = OFFSCREEN;
      
      // Advance rotation
      if (!rotation_direction_exists)
	new_rotation_direction(RANDOM_SEED);
      if (last_mouse_x >= 0 && last_mouse_y >= 0)
	{
	  int dx = mouse_x - last_mouse_x;
	  int dy = mouse_y - last_mouse_y;
	  SO_rotate(dx,dy);
	}		      
      last_mouse_x = mouse_x;
      last_mouse_y = mouse_y;
    }
}

void service_left_button_on_brush(int button_x, int button_y, double (*data)[dim],
				  int * color, int num_data)
{
  if (button_y < 100)
    {
      int bx = 3 * (button_x / (3 * BIT_X_SKIP));
      if (bx < 0) bx = 0;
      if (bx > 27) bx = 27;
      mask_location = bx;			  
    }
  else if (button_x < PALETTE_ICON_SEPARATOR_X)
    {
      int c = (button_x - BIT_X_SKIP/2)
	/ PALETTE_BOX_SKIP;
      if (c < 0) c=0;
      if (c > 7) c=7;			  
      if (brush_color_mode == BRUSH_COLOR_MODE_DIRECT)
	selected_color = c;
      else
	{
	  int c0 = selected_color - 4;
	  if (c0 < 0) c0 = 0;
	  selected_color = c0 + c;
	}
    }
  else if (button_x >= PIE_CHART_X && button_y >= PIE_CHART_Y &&
	   button_x < PIE_CHART_X + 2*PIE_CHART_SIZE &&
	   button_y < PIE_CHART_Y + 2*PIE_CHART_SIZE)
    {
      int dx = button_x - (PIE_CHART_X + PIE_CHART_SIZE);
      int dy = button_y - (PIE_CHART_Y + PIE_CHART_SIZE);
      if (SQR(dx) + SQR(dy) < SQR(PIE_CHART_SIZE))
	{
	  uint64_t sorted_color[num_data];
	  for(int i=0;i<num_data;i++) sorted_color[i] =
					(((uint64_t) get_color(color[i]) ) << 32)
					+ color[i];
	  qsort(sorted_color, num_data, sizeof(uint64_t), &cmp_int64);
	  uint32_t sc = sorted_color[(int)(num_data * (atan2(dy,dx) + M_PI)
					   / (2 * M_PI + 0.0001))] & 0xffffffff;
	  selected_color = sc;
	}
    }
  else if (button_x >= SCREEN_WIDTH[BRUSH_SCREEN]
	   - BRUSH_MODE_MARGIN - BRUSH_MODE_BOX_SIZE &&
	   button_x < SCREEN_WIDTH[BRUSH_SCREEN]
	   - BRUSH_MODE_MARGIN &&
	   button_y >= SCREEN_HEIGHT[BRUSH_SCREEN]
	   - BRUSH_MODE_MARGIN - BRUSH_MODE_BOX_SIZE &&
	   button_y < SCREEN_HEIGHT[BRUSH_SCREEN]
	   - BRUSH_MODE_MARGIN)
    {
      rotation_mode = 0;
      erase_mode_on = !erase_mode_on;
    }
  else if (button_x >= SCREEN_WIDTH[BRUSH_SCREEN]
	   - 2* BRUSH_MODE_MARGIN - 2 * BRUSH_MODE_BOX_SIZE &&
	   button_x < SCREEN_WIDTH[BRUSH_SCREEN]
	   - 2 * BRUSH_MODE_MARGIN &&
	   button_y >= SCREEN_HEIGHT[BRUSH_SCREEN]
	   - BRUSH_MODE_MARGIN - BRUSH_MODE_BOX_SIZE &&
	   button_y < SCREEN_HEIGHT[BRUSH_SCREEN]
	   - BRUSH_MODE_MARGIN)
    {
      if (++brush_color_mode >= BRUSH_COLOR_MODES)
	brush_color_mode=0;
    }
}

void service_left_button_on_control(int button_x, int button_y)
{
  int shift_x = button_x - CONTROL_NUMBER_X;
  if (shift_x < SPEED_GROVE_X - ZOOM_SLIDER_WIDTH/2.0)
    {			
      int bi = round(((double) shift_x - CONTROL_RADIUS)
		     / CONTROL_Y_STEP - CONTROL_BOX_SHIFT);
      double xx = shift_x -
	CONTROL_Y_STEP*(bi+CONTROL_BOX_SHIFT) - CONTROL_RADIUS;
      int i = round(((double) button_y + control_scroll
		      - CONTROL_RADIUS) / CONTROL_Y_STEP);
      double yy = button_y + control_scroll - CONTROL_Y_STEP*i - CONTROL_RADIUS;
      if (bi>=0 && bi < CONTROL_NUM_BOX && i>=0 && i < dim &&
	  xx >= -CONTROL_BOX_RADIUS &&
	  xx <= CONTROL_BOX_RADIUS &&
	  yy >= -CONTROL_BOX_RADIUS && yy <= CONTROL_BOX_RADIUS)
	{
	  if (bi >= 0 && bi < 2) service_box_0_1(i ,bi);
	  if (bi==2) service_box_2(i, bi);
	  if (bi==3) service_box_3(i, bi);
	  new_rotation_direction(RANDOM_SEED);
	}
      brush_x = OFFSCREEN;
      brush_y = OFFSCREEN;
    }
  else if (button_y >= ZOOM_GROVE_Y &&
	   button_y < ZOOM_GROVE_Y + ZOOM_GROVE_HEIGHT)
    move_sliders(shift_x, button_y, 1);
  else if (button_y >= ROTATION_MODE_MARGIN_Y &&
	   button_y < ROTATION_MODE_MARGIN_Y +
	   ROTATION_MODE_BOX_SIZE &&
	   button_x >= SCREEN_WIDTH[CONTROL_SCREEN]
	   - ROTATION_MODE_MARGIN_X - ROTATION_MODE_BOX_SIZE - 60 &&
	   button_x <= SCREEN_WIDTH[CONTROL_SCREEN]
	   - ROTATION_MODE_MARGIN_X - 80) change_rotation_mode();
  else if
    (button_y >= ROTATION_MODE_MARGIN_Y &&
     button_y < ROTATION_MODE_MARGIN_Y +
	   ROTATION_MODE_BOX_SIZE &&
	   button_x >= SCREEN_WIDTH[CONTROL_SCREEN]
	   - ROTATION_MODE_MARGIN_X - ROTATION_MODE_BOX_SIZE + 20 &&
	   button_x <= SCREEN_WIDTH[CONTROL_SCREEN]
	   - ROTATION_MODE_MARGIN_X)
    {
      if (++decimation_mode == MAX_DECIMATION_MODE) decimation_mode = 0;
    }
}


// data must be normalized to be in [-1,1]
// color will be modified in place
void mojave(double * data_flat, int32_t * color, int num_data, int dim_in,
	    char * name, char * mojave_path)
{
  set_gamma();
  
  int32_t * hide;
  if ((hide = malloc(sizeof(int32_t) * num_data)) == 0)
    {
      fprintf(stderr, "Out of memory\n");
      exit(1);
    }
  for(int i=0;i<num_data;i++) hide[i] = 0;

  if (TTF_Init()) {fprintf(stderr, "TTF_Init error!");exit(1);}
  int font_found = 0;
  for(int i=0;i<FONT_NUM_LOCATIONS;i++)
    {
      font = TTF_OpenFont(ttf_file[i], 24);
      if (font != NULL)
	{
	  font_found = 1;
	  break;
	}
    }
  if (!font_found) ERROR("TTF_OpenFont error");
  create_text();

  for(int i = 0; i < 100; i++) lrand48();
  SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
  
  dim = dim_in;  
  if (dim <= 1) return;
  double (*data)[dim] = (double (*)[dim]) data_flat;

  int32_t * undo_flat;
  if ((undo_flat = malloc(sizeof(int32_t) * num_data * UNDO_SIZE)) == 0)
    {
      fprintf(stderr, "Out of memory\n");
      exit(1);
    }
  int32_t * undo_hide_flat;
  if ((undo_hide_flat = malloc(sizeof(int32_t) * num_data * UNDO_SIZE)) == 0)
    {
      fprintf(stderr, "Out of memory\n");
      exit(1);
    }
  int32_t (*undo)[num_data] = (int32_t (*)[num_data]) undo_flat;
  int32_t (*undo_hide)[num_data] = (int32_t (*)[num_data]) undo_hide_flat;
  for(int i=0;i<num_data;i++) undo[0][i] = color[i];
  for(int i=0;i<num_data;i++) undo_hide[0][i] = 0;//hide[i];
  
  // Set up initial transform (A) \in SO(dim)
  if ((A = malloc(SQR(dim) * sizeof(double))) == NULL)
    {fprintf(stderr, "Out of memory\n");exit(1);}  
  for(int y=0;y<dim;y++)
    for(int i=0;i<dim;i++)
      A[AA(y,i)] = (i==y);

  // Set up initial rotations (R,R_inv)
  if ((Rx = malloc(R_POWERS * SQR(dim) * sizeof(double))) == NULL)
    {fprintf(stderr, "Out of memory\n");exit(1);}  
  if ((Rx_inv = malloc(R_POWERS * SQR(dim) * sizeof(double))) == NULL)
    {fprintf(stderr, "Out of memory\n");exit(1);}
  if ((Ry = malloc(R_POWERS * SQR(dim) * sizeof(double))) == NULL)
    {fprintf(stderr, "Out of memory\n");exit(1);}  
  if ((Ry_inv = malloc(R_POWERS * SQR(dim) * sizeof(double))) == NULL)
    {fprintf(stderr, "Out of memory\n");exit(1);}
  for(int k=0;k<R_POWERS;k++)
    for(int y=0;y<dim;y++)
      for(int i=0;i<dim;i++)
	{
	  Rx_inv[RR(k,y,i)] = Rx[RR(k,y,i)] = (i==y);
	  Ry_inv[RR(k,y,i)] = Ry[RR(k,y,i)] = (i==y);
	}
  
  // Set up initial controls (boxes)
  if ((box = malloc(dim * CONTROL_NUM_BOX * sizeof(int))) == NULL)
    {fprintf(stderr, "Out of memory\n");exit(1);}
  for(int y=0; y<dim; y++)
    for(int i=0; i<CONTROL_NUM_BOX; i++)
      box[y][i] = 0;
  box[0][0] = 1;
  box[1][1] = 1;

  SDL_Init(SDL_INIT_VIDEO);
  atexit(SDL_Quit);

  char brush_window_name[MAX_STRING];
  char control_window_name[MAX_STRING];
  sprintf(brush_window_name, "%s - Brush", name);
  sprintf(control_window_name, "%s - Control", name);  
  screen_init(POINT_SCREEN, 1, name, SCREEN_XPOS[POINT_SCREEN],
	      SCREEN_YPOS[POINT_SCREEN]);
  SCREEN_HEIGHT[CONTROL_SCREEN] = MIN(MAX(CONTROL_Y_STEP * dim,
					  ZOOM_GROVE_Y + ZOOM_GROVE_HEIGHT + 20),
				      SCREEN_HEIGHT[CONTROL_SCREEN]);
  screen_init(CONTROL_SCREEN, 1, control_window_name, SCREEN_XPOS[CONTROL_SCREEN],
	      SCREEN_YPOS[CONTROL_SCREEN]);    
  screen_init(BRUSH_SCREEN, 1, brush_window_name, SCREEN_XPOS[BRUSH_SCREEN],
	      SCREEN_YPOS[BRUSH_SCREEN]);
  SDL_SetRenderDrawColor(renderer[POINT_SCREEN],0,0,0,255);

  char image_file[MAX_STRING];
  sprintf(image_file, "%s/%s", mojave_path, IMAGE_ERASE_FILE);
  if ((image_erase = SDL_LoadBMP(image_file))==NULL)
    ERROR("SDL_LoadBMP error (erase)");
  sprintf(image_file, "%s/%s", mojave_path, IMAGE_PALETTE_FILE);
  if ((image_palette = SDL_LoadBMP(image_file))==NULL)
    ERROR("SDL_LoadBMP (palette)");
  sprintf(image_file, "%s/%s", mojave_path, IMAGE_ROTATION_FILE);
  if ((image_rotation = SDL_LoadBMP(image_file))==NULL)
    ERROR("SDL_LoadBMP (rotation)");
  sprintf(image_file, "%s/%s", mojave_path, IMAGE_SLIDER_SPEED_FILE);
  if ((image_slider_speed = SDL_LoadBMP(image_file))==NULL)
    ERROR("SDL_LoadBMP (slider_speed)");
  sprintf(image_file, "%s/%s", mojave_path, IMAGE_SLIDER_ZOOM_FILE);
  if ((image_slider_zoom = SDL_LoadBMP(image_file))==NULL)
    ERROR("SDL_LoadBMP (slider_zoom)");
  sprintf(image_file, "%s/%s", mojave_path, IMAGE_SLIDER_POINT_FILE);
  if ((image_slider_point = SDL_LoadBMP(image_file))==NULL)
    ERROR("SDL_LoadBMP (slider_point)");
  sprintf(image_file, "%s/%s", mojave_path,
	  IMAGE_SLIDER_INTENSITY_FILE);
  if ((image_slider_intensity = SDL_LoadBMP(image_file))==NULL)
    ERROR("SDL_LoadBMP (slider_intensity)");
  sprintf(image_file, "%s/%s", mojave_path, IMAGE_CUT_FILE);
  if ((image_cut = SDL_LoadBMP(image_file))==NULL)
    ERROR("SDL_LoadBMP (cut)");

  create_point_texture();
  
  SDL_Event event, dummy;
  int flag = 1;
  int refresh_flag = 1;
  int mouse_x, mouse_y;
  int was_delay = 0;
  unsigned frame_time = 0;
  int mouse_motion_occured = 0;
  while(flag)
    {
      unsigned frame_start = SDL_GetTicks();

      // Non-event driven rotation
      if (rotation_mode & !mouse_motion_occured)
	{
	  SO_rotate(KEYBOARD_ROTATION_DX, KEYBOARD_ROTATION_DY);
	  refresh_flag = 1;
	}
      
      // Refresh logic
      if (refresh_flag)
	{
	  // Point Screen	  
          SDL_RenderClear(renderer[POINT_SCREEN]);
          draw_points(num_data, data, color, hide);

	  // Control screen
	  draw_controls();
	  refresh(CONTROL_SCREEN);

	  // Brush screen
	  draw_palette(num_data, data, color, hide);
	  refresh(BRUSH_SCREEN);
	}
      refresh_flag = 0;
      mouse_motion_occured = 0;
      // Event loop, suppress mouse motions.
      if (SDL_PollEvent(&event))
	{
	  switch(event.type)
	    {
            case SDL_WINDOWEVENT:	      
	      if (event.window.event == SDL_WINDOWEVENT_CLOSE)
		{
		  flag = 0;
		}
	      break;
	    case SDL_KEYDOWN:
	      switch(event.key.keysym.sym)
		{
		case SDLK_q:
		  flag = 0;
		  break;
		case SDLK_d:
		  if (++decimation_mode == MAX_DECIMATION_MODE) decimation_mode = 0;
		  refresh_flag = 1;
		  break;
		case SDLK_x:
		  for(int i = 0; i < dim; i++)
		    {
		      box[i][0] = 0;
		      box[i][1] = 0;
		      box[i][2] = 0;
		      box[i][3] = 1;
		      rotation_mode = 0;
		    }
		  refresh_flag = 1;
		  break;
		case SDLK_h:
		  for(int i=0;i<num_data;i++)
		    {
		      if (brush_color_mode == BRUSH_COLOR_MODE_DIRECT)
			{
			  if (((color[i] >> mask_location) & 7) == selected_color)
			    hide[i] = 1;
			}
		      else {
			{
			  if (color[i] == selected_color) hide[i] = 1;
			}
		      }
		    }
		  refresh_flag = 1;
		  break;
		case SDLK_c:
		  if (++brush_color_mode >= BRUSH_COLOR_MODES)  brush_color_mode=0;
		  refresh_flag = 1;
		  break;
		case SDLK_SPACE:
		  for(int i=0;i<num_data;i++) hide[i] = 0;
		  new_rotation_direction(RANDOM_SEED);
		  refresh_flag = 1;
		  break;
		case SDLK_s:
		  for(int i=0;i<dim;i++) box[i][0] = box[i][1] = box[i][2] = 0;
		  for(int i=0;i<num_data;i++) hide[i] = 0;
		  box[0][0] = 1;
		  box[1][1] = 1;
		  clear_all();
		  SO_clear(A);
		  rotation_mode = 0;
		  zoom_ratio = 1.0;
		  rotation_speed = 1.0;
		  point_size = DEFAULT_POINT_SIZE;
		  gamma_correct = 1.0;
		  refresh_flag = 1;
		  break;
		case SDLK_r:
		  change_rotation_mode();
		  new_rotation_direction(RANDOM_SEED);
		  refresh_flag = 1;
		  break;
		case SDLK_e:
		  rotation_mode = 0;
		  erase_mode_on = !erase_mode_on;
		  refresh_flag = 1;
		  break;
		case SDLK_n:
		  if (brush_color_mode != BRUSH_COLOR_MODE_DIRECT)
		    {
		      int max_color = 0;
		      for(int i=0;i<num_data;i++)
			{
			  if (color[i] > max_color) max_color = color[i];
			}
		      selected_color = max_color + 1;
		      refresh_flag = 1;
		    }
		  break;
		case SDLK_o:
		  // @@@@@@
		  //SDL_GetMouseState(&mouse_x, &mouse_y);
		  if (event.window.windowID ==
		      SDL_GetWindowID(screen[POINT_SCREEN]))
		    {		      
		      color_picker(&mouse_x, &mouse_y, data, color, num_data);
		      //color_picker(data, color, num_data);
		      refresh_flag = 1;
		    }
		  break;
		case SDLK_i:
		  printf("Estimated frame rate is %6.02f fps\n",
			 1000.0 / frame_time);
		  printf("Current color = %08x\n", selected_color);
		  printf("Data size = (%d, %d)\n", num_data, dim);
		  break;
		case SDLK_DOWN:
		  control_scroll += CONTROL_SCROLL_DELTA;
		  int max_y = dim * CONTROL_Y_STEP - SCREEN_HEIGHT[CONTROL_SCREEN];
		  if (control_scroll > max_y) control_scroll = max_y;
		  refresh_flag = 1;
		  break;
		case SDLK_UP:
		  control_scroll -= CONTROL_SCROLL_DELTA;
		  if (control_scroll < 0) control_scroll = 0;
		  refresh_flag = 1;
		  break;		 
		case SDLK_PAGEDOWN:
		  control_scroll += CONTROL_SCROLL_DELTA_PAGE;
		  int max_y2 = dim * CONTROL_Y_STEP - SCREEN_HEIGHT[CONTROL_SCREEN];
		  if (control_scroll > max_y2) control_scroll = max_y2;
		  refresh_flag = 1;
		  break;
		case SDLK_PAGEUP:
		  control_scroll -= CONTROL_SCROLL_DELTA_PAGE;
		  if (control_scroll < 0) control_scroll = 0;
		  refresh_flag = 1;
		  break;   
 		case SDLK_LEFT:
		  selected_color--;
		  if (brush_color_mode == BRUSH_COLOR_MODE_DIRECT)
		    {
		      if (selected_color < 0) selected_color = 7;
		    }
		  else
		    {
		      if (selected_color < 0) selected_color = 0xfffff;
		    }
		  refresh_flag = 1;
		  break;		  
 		case SDLK_RIGHT:
		  selected_color++;
		  if (brush_color_mode == BRUSH_COLOR_MODE_DIRECT)
		    {
		      if (selected_color >= 8) selected_color = 0;
		    }
		  else
		    {
		      if (selected_color >= 0x100000) selected_color = 0;
		    }
		  refresh_flag = 1;
		  break;
		case SDLK_z:
		  if (undo_length > 1)
		    {
		      undo_length--;
		      for(int i=0;i<num_data;i++)
			{
			  color[i] = undo[undo_length-1][i];
			  hide[i] = undo_hide[undo_length-1][i];
			}
		      refresh_flag = 1;
		    }
		  break;
		case SDLK_y:
		  if (undo_length < UNDO_SIZE && undo_length < max_undo_length)
		    {
		      for(int i=0;i<num_data;i++)
			{
			  color[i] = undo[undo_length][i];
			  hide[i] = undo_hide[undo_length][i];
			}
		      undo_length++;
		      refresh_flag = 1;
		    }
		  break;
		case SDLK_MINUS:
		  zoom_ratio /= POINT_ZOOM_MULT;
		  refresh_flag = 1;
		  break;
		case SDLK_EQUALS:
		  zoom_ratio *= POINT_ZOOM_MULT;
		  refresh_flag = 1;
		  break;
		case SDLK_LEFTBRACKET:
		  rotation_speed /= ROTATION_SPEED_MULT;
		  new_rotation_direction(rotation_seed);
		  refresh_flag = 1;
		  break;
		case SDLK_RIGHTBRACKET:
		  rotation_speed *= ROTATION_SPEED_MULT;
		  new_rotation_direction(rotation_seed);
		  refresh_flag = 1;
		  break;
		case SDLK_COMMA:
		  point_size -= POINT_SIZE_STEP;
		  if (point_size < MIN_POINT_SIZE) point_size = MIN_POINT_SIZE;
		  create_point_texture();
		  refresh_flag = 1;
		  break;
		case SDLK_PERIOD:
		  point_size += POINT_SIZE_STEP;
		  if (point_size > MAX_POINT_SIZE) point_size = MAX_POINT_SIZE;
		  create_point_texture();
		  refresh_flag = 1;
		  break;
		case SDLK_SEMICOLON:
		  gamma_correct *= 1.05;
		  set_gamma();
		  refresh_flag = 1;
		  break;
		case SDLK_QUOTE:
		  gamma_correct /= 1.05;
		  set_gamma();
		  refresh_flag = 1;
		  break;
		}
	    case SDL_MOUSEMOTION:
	      if (frame_time != 0)
		{
		  mouse_motion_occured = 1;
		  continue;
		}
	      SDL_GetMouseState(&mouse_x, &mouse_y);
	      if (event.window.windowID ==
		  SDL_GetWindowID(screen[POINT_SCREEN]))
		{
		  service_mouse_motion_on_point(event.button.x, event.button.y,
						event.motion.state, data, color,
						hide, num_data);
		  refresh_flag = 1;
		}
	      else if (event.window.windowID ==
		       SDL_GetWindowID(screen[CONTROL_SCREEN]) &&
		       event.button.y >= ZOOM_GROVE_Y &&
		       event.button.y < ZOOM_GROVE_Y + ZOOM_GROVE_HEIGHT &&
		       event.button.button == SDL_BUTTON_LEFT)
		{
		  move_sliders(mouse_x - CONTROL_NUMBER_X, mouse_y, 0);
		  refresh_flag = 1;
		}
	      break;	      
	    case SDL_MOUSEBUTTONDOWN:
	      switch(event.button.button)
		{
		case SDL_BUTTON_LEFT:
		  if (event.window.windowID ==
		      SDL_GetWindowID(screen[CONTROL_SCREEN]))
		    {
		      service_left_button_on_control(event.button.x,
						     event.button.y);
		      refresh_flag = 1;
		    }
		  else if (event.window.windowID ==
			   SDL_GetWindowID(screen[BRUSH_SCREEN]))
		    {
		      service_left_button_on_brush(event.button.x, event.button.y,
						   data, color, num_data);
		      refresh_flag = 1;
		    }
		  else if (event.window.windowID ==
			   SDL_GetWindowID(screen[POINT_SCREEN]))
		    {
		      service_left_button_on_point(mouse_x, mouse_y,
						  data, color, hide, num_data);
		      refresh_flag = 1;
		    }
		  break;
		}
	      break;
	    case SDL_MOUSEBUTTONUP:
	      undo_save(num_data, undo, undo_hide, color, hide);
	      break;
	    case SDL_MOUSEWHEEL:
	      if  (event.window.windowID ==
		   SDL_GetWindowID(screen[POINT_SCREEN]))
		{
		  if (event.wheel.y > 0)
		    zoom_ratio /= POINT_ZOOM_MULT;		  
		  else if (event.wheel.y < 0)
		    zoom_ratio *= POINT_ZOOM_MULT;		  
		}
	      else if (event.window.windowID ==
		       SDL_GetWindowID(screen[CONTROL_SCREEN]))
		{
		  if (event.wheel.y > 0)
		    {
		      control_scroll += CONTROL_SCROLL_DELTA;
		      int max_y2 = dim * CONTROL_Y_STEP -
			SCREEN_HEIGHT[CONTROL_SCREEN];
		      if (control_scroll > max_y2) control_scroll = max_y2;
		    }
		  else if (event.wheel.y < 0)
		    {
                      control_scroll -= CONTROL_SCROLL_DELTA;
                      if (control_scroll < 0) control_scroll = 0;
		    }
		}
	      else if (event.window.windowID ==
		       SDL_GetWindowID(screen[BRUSH_SCREEN]))
		{
		  if (event.wheel.y > 0)
		    {
		      selected_color++;
		      if (selected_color >= 8 &&
			  brush_color_mode == BRUSH_COLOR_MODE_DIRECT)
			selected_color = 0;
		    }
		  else if (event.wheel.y < 0)
		    {
		      selected_color--;
		      if (selected_color < 0)
			selected_color =
			  (brush_color_mode == BRUSH_COLOR_MODE_DIRECT) ? 7 : 0;
		    }
		}
	      refresh_flag = 1;
	      break;
	    }
	}
      frame_time = SDL_GetTicks() - frame_start;
      if (FRAME_DELAY > frame_time) SDL_Delay(FRAME_DELAY - frame_time);
    }

  SDL_Quit();
}

#include <stdio.h>
#include <stdlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "SDL.h"

#define SCREENWIDTH 800
#define SCREENHEIGHT 600



//TODO:
//normalalzing acutally seems to make it less fun?
//fix the issue of collisions to be more robust
//have a size on the bricks
//have a gameover state
//have a score system



#define BRICKGAP 2
#define BRICKCOLS 10
#define BRICKROWS 14 //temp halved

#define BRICKWIDTH 80
#define BRICKHEIGHT 20 //temp doubled

typedef unsigned char byte;

//this could be packed into an int, it's worth noting

typedef struct {
    int exists;
    int width;
    int height;
    int x;
    int y;
} Brick;

typedef struct {
    byte r;
    byte g;
    byte b;
    byte a;
} Color;

typedef struct {
    float x;
    float y;
} Vec2;

typedef struct {
    Vec2 position;
    int width;
    int height;
} Paddle;

typedef struct {
    Vec2 position;
    Vec2 heading;
    int radius;
} Ball;

typedef enum {
    GAME_START,
    GAME_PLAYING,
    GAME_OVER
} GAMESTATE;

Vec2 vec_normalize(Vec2 in);

void draw_rect(SDL_Renderer *renderer, int x, int y, int w, int h, Color color);

Color init_color(byte red, byte green, byte blue, byte alpha);

Vec2 init_vec2(float x, float y);

Paddle init_paddle(float x, float y, int width, int height);

Ball init_ball(float x, float y, float x_heading, float y_heading);


void move_ball(Brick *bricks, Ball *ball, float ball_speed, Paddle paddle_p1, float dt, int *score_p1, int *bricks_destroye);

void reset_ball(Ball *ball);

void move_paddle(Paddle *paddle, Ball ball, float dt);

float distance(float x, float y);

float float_min(float x, float y);

float float_max(float x, float y);

void draw_scores(SDL_Renderer *renderer, int score_p1);

void draw_bricks(SDL_Renderer *renderer, Brick *bricks);

Brick init_brick(int x, int y);

void reset_bricks(Brick *bricks)
{
    for (int i = 0; i < BRICKROWS; i++) {
	for (int j = 0; j < BRICKCOLS; j++) {
	    Brick brick = init_brick(j*(BRICKWIDTH) , i*(BRICKHEIGHT));
	    bricks[i*BRICKCOLS + j] = brick;
	}
    }
}

int main(int argc, char **argv)
{

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
	printf("didn't init SDL\n");
	return 1;
    }

    SDL_Window *window;
    SDL_Renderer *renderer;
    if (SDL_CreateWindowAndRenderer(SCREENWIDTH, SCREENHEIGHT, 0, &window, &renderer) != 0) {
	printf("didn't create window/renderer \n");
	return 1;
    }
    
    
    int game_running = 1;

    /////////////Game specific values/////////////
    Color white = {0xff, 0xff, 0xff, 0x00};
    Paddle paddle_p1 = init_paddle(SCREENWIDTH/2, SCREENHEIGHT - 20.0f, 100.0f, 10.0f);

    Ball ball = init_ball(SCREENWIDTH/2, SCREENHEIGHT/2, -1.0f, 0.5f);
    int mouse_x;
    int mouse_y;
    float ball_speed = 300.0f;
    float dt;
    unsigned int current_time, time_elapsed, last_time = 0;
    int score_p1 = 0;
    int score_p2 = 0;
    GAMESTATE gamestate = GAME_START;

    Brick *bricks = (Brick*)malloc(sizeof(Brick)*BRICKROWS*BRICKCOLS);
    for (int i = 0; i < BRICKROWS; i++) {
	for (int j = 0; j < BRICKCOLS; j++) {
	    Brick brick = init_brick(j*(BRICKWIDTH) , i*(BRICKHEIGHT));
	    bricks[i*BRICKCOLS + j] = brick;
	}
    }

    int bricks_destroyed = 0;
    int total_bricks = BRICKROWS * BRICKCOLS;
    
    while (game_running) {
	current_time = SDL_GetTicks();
	time_elapsed = current_time - last_time;
	dt = ((float)time_elapsed)/1000.0f;
	last_time = current_time;
	
	SDL_Event event;
	SDL_PollEvent(&event);
	SDL_GetMouseState(&mouse_x, &mouse_y);
	//get the mouse position
	if (event.type == SDL_QUIT) {
	    game_running = 0;
	}
	if (gamestate == GAME_START) {
	    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0x00, 0x00);
	    SDL_RenderClear(renderer);
	    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0x00);
	    draw_rect(renderer, paddle_p1.position.x, paddle_p1.position.y, paddle_p1.width, paddle_p1.height, white);

	
	    draw_rect(renderer, ball.position.x, ball.position.y, 5.0f, 5.0f, white);
	    draw_scores(renderer, score_p1, score_p2);
	    if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
		    gamestate = GAME_PLAYING;
		}
	    }
	} else if (gamestate == GAME_PLAYING) {
	    paddle_p1.position.x = mouse_x;
	
	    move_ball(bricks, &ball, ball_speed, paddle_p1, dt, &score_p1, &bricks_destroyed);
	    SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0x00);
	    SDL_RenderClear(renderer);
	    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0x00);
	    draw_rect(renderer, paddle_p1.position.x, paddle_p1.position.y, paddle_p1.width, paddle_p1.height, white);

	    draw_bricks(renderer, bricks);
	
	    draw_rect(renderer, ball.position.x, ball.position.y, 5.0f, 5.0f, white);
	    draw_scores(renderer, score_p1, score_p2);
	    if (score_p1 == 5 || score_p2 == 5) {
		gamestate = GAME_OVER;
	    }
	    if (bricks_destroyed == total_bricks) {
		bricks_destroyed = 0;
		reset_bricks(bricks);
		reset_ball(&ball);
		gamestate = GAME_START;
	    }
	} else if (gamestate == GAME_OVER) {
	    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0x00, 0x00);
	    SDL_RenderClear(renderer);
	    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0x00);
	    draw_rect(renderer, paddle_p1.position.x, paddle_p1.position.y, paddle_p1.width, paddle_p1.height, white);

	
	    draw_rect(renderer, ball.position.x, ball.position.y, 5.0f, 5.0f, white);
	    draw_scores(renderer, score_p1);
	    if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
		    score_p1 = 0;
		    score_p2 = 0;
		    gamestate = GAME_PLAYING;
		}
	    }
	}



	SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}

void draw_rect(SDL_Renderer *renderer, int x, int y, int w, int h, Color color)
{
    SDL_Rect temp_rect;
    Color old_color;
    SDL_GetRenderDrawColor(renderer, &old_color.r, &old_color.g, &old_color.b, &old_color.a);
    temp_rect.x = x;
    temp_rect.y = y;
    temp_rect.w = w;
    temp_rect.h = h;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &temp_rect);
    SDL_SetRenderDrawColor(renderer, old_color.r, old_color.g, old_color.b, old_color.a);
    
}

Color init_color(byte red, byte green, byte blue, byte alpha)
{
    Color result;
    result.r = red;
    result.g = green;
    result.b = blue;
    result.a = alpha;

    return result;
}

Vec2 init_vec2(float x, float y)
{
    Vec2 result;
    result.x = x;
    result.y = y;
    return result;
}

Paddle init_paddle(float x, float y, int width, int height)
{
    Vec2 position = init_vec2(x, y);
    Paddle result;
    result.position = position;
    result.width = width;
    result.height = height;
    return result;
}

Ball init_ball(float x, float y, float x_heading, float y_heading)
{

    int ball_radius = 5;
    Vec2 position = init_vec2(x,y);
    Vec2 heading = init_vec2(x_heading, y_heading);
    Ball result;
    result.position = position;
    result.heading = heading;
    result.radius = ball_radius;
    return result;
}

void move_ball(Brick *bricks, Ball *ball, float ball_speed, Paddle paddle_p1, float dt, int *score_p1, int *bricks_destroyed)
{
    Vec2 previous_position = {ball->position.x, ball->position.y};

    ball->position.x += ball_speed*ball->heading.x*dt;
    ball->position.y += ball_speed*ball->heading.y*dt;

    //reflect along y axis
    //we should really make sure we *bounce back straight away* rather than waiting to update next frame, because it might not move us enough
    //if less time passes
    if (ball->position.x <=0 || ball->position.x >= (SCREENWIDTH - ball->radius)) {
	ball->heading.x *= -1.0f;
	ball->position.x += ball_speed*ball->heading.x*dt;
    }
    //reflect along x axis and handle case of
    //paddle

    if (ball->position.y <= 0) {
	ball->heading.y *= -1.0f;
    }

    if (ball->position.y >= SCREENHEIGHT) {
	reset_ball(ball);
	*score_p1 = *score_p1 + 1;
    } else if (ball->position.y >= paddle_p1.position.y) {
	if (ball->position.x >= paddle_p1.position.x && ball->position.x <= (paddle_p1.position.x + paddle_p1.width)) {
	    //check these calculations
	    float p1_center = paddle_p1.position.x + paddle_p1.width/2;
	    float p1_ball_offset = (ball->position.x - p1_center)/(paddle_p1.width/2); 
	    ball->heading.y *= -1.0f;
	    ball->heading.x = p1_ball_offset;
	    //move it back!
	    ball->position.y -= dt*ball_speed;
	} 
    }

    //is there a way to quickly identify which brick to check?
    //based on ball x/ ball y?

    //yeah....

    //it's the ball X position divided by the brickwidth
    //and the ball Y position...um...
    //divided by the brickheight?

    int prev_index_x = (int)((int)(previous_position.x)/(int)BRICKWIDTH);//col;
    int prev_index_y = (int)((int)(previous_position.y)/(int)BRICKHEIGHT);//row;
    
    int brick_index_x = (int)((int)(ball->position.x)/(int)BRICKWIDTH);//col
    int brick_index_y = (int)((int)(ball->position.y)/(int)BRICKHEIGHT);//row
    
    if (brick_index_y < BRICKROWS && brick_index_x < BRICKCOLS) {
	Brick brick = bricks[brick_index_y*BRICKCOLS + brick_index_x];
	if (brick.exists) {
	    //use our algorithm here
	    
	    int both_failed = 1;
	    Brick neighbour_brick_col = bricks[brick_index_y*BRICKCOLS + prev_index_x];
	    Brick neighbour_brick_row = bricks[prev_index_y*BRICKCOLS + brick_index_x];

	    if (prev_index_x != brick_index_x) {
		//came in horizontally
		if (neighbour_brick_row.exists) {
		    ball->heading.x *= -1.0f;
		    both_failed = 0;
		}
	    }
	    if (prev_index_y != brick_index_y) {
		//cam in vertically
		if (neighbour_brick_col.exists) {
		    
		    ball->heading.y *= -1.0f;
		    both_failed = 0;
		}
	    }
	    if (both_failed) {
		ball->heading.x *= -1.0f;
		ball->heading.y *= -1.0f;
	    }
	    //ball->heading.x *= -1.0f;
	    brick.exists = 0;
	    (*bricks_destroyed) += 1;
	    bricks[brick_index_y*BRICKCOLS + brick_index_x] = brick;
	}
    }
    
#if 0
    for (int i = 0; i < BRICKROWS; i++) {
	for (int j = 0; j < BRICKCOLS; j++) {
	    Brick brick = bricks[i*BRICKCOLS + j];
	    if (brick.exists) {
		if (ball->position.x >= brick.x && (ball->position.x <= brick.x + brick.width)
		    && ball->position.y >= brick.y && (ball->position.y <= brick.y + brick.height)) {
		    ball->heading.y *= -1.0f;
		    //ball->heading.x *= -1.0f;
		    brick.exists = 0;
		    bricks[i*BRICKCOLS + j] = brick;
		}
	    }
	}
    }
#endif
    ball->heading = vec_normalize(ball->heading);
    //

}

void reset_ball(Ball *ball)
{
    ball->position.x = SCREENWIDTH/2;
    ball->position.y = SCREENHEIGHT/2;
    ball->heading.x *= -1.0f;
}


float float_min(float x, float y)
{
    if (x <= y) {
	return x;
    }
    return y;
}

float float_max(float x, float y)
{
    if (x >= y) {
	return x;
    }
    return y;
}

float distance(float x, float y)
{
    float result;
    result = sqrt((x-y)*(x-y));
    return result;
}

void draw_scores(SDL_Renderer *renderer, int score_p1, int score_p2)
{
    int score_width = 5;
    int score_height = 5;
    Color score_color = {255,255,255,1};
    for (int i = 0; i < score_p1; i++) {
	draw_rect(renderer, (float)(SCREENWIDTH/4 + i*score_width*2), (float)30, score_width, score_height, score_color );
    }

}

static Vec2 vec_normalize(Vec2 in)
{
    Vec2 result;
    float mag = sqrt(in.x*in.x + in.y*in.y);
    if (mag != 0.0f) {
	result.x = in.x/mag;
	result.y = in.y/mag;
    } else {
	result.x = 0.0f;
	result.y = 0.0f;
    }

    return result;
}

Brick init_brick(int x, int y)
{
    Brick result;
    result.x = x;
    result.y = y;
    result.width = BRICKWIDTH;
    result.height = BRICKHEIGHT;
    result.exists = 1;
    return result;
}

void draw_bricks(SDL_Renderer *renderer, Brick *bricks)
{
    Color blue = {0x00, 0x00, 0xff, 0x00};
    
    for (int i = 0; i < BRICKROWS; i++) {
	for (int j = 0; j < BRICKCOLS; j++) {
	    Brick brick = bricks[i*BRICKCOLS + j];
	    if (brick.exists) {
		draw_rect(renderer, brick.x , brick.y, brick.width-BRICKGAP, brick.height-BRICKGAP, blue);
	    }
	}
    }
}

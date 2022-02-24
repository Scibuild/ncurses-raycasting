#include<curses.h>
#include<math.h>
#define PI 3.1415926535

#define HORIZONTAL 0
#define VERTICAL   1

#define GRID_SIZE 1
#define STEP_SIZE 1
// pi/2 = 90
#define FOV 1.570796
#define DOF 50
struct Player {
  double x, y, dx, dy, a;
};

#define LEVEL_WIDTH 16 
#define LEVEL_HEIGHT 16 
int level[] = {
  1, 1, 2, 2, 2, 1, 1, 1, 2, 2, 2, 1, 1, 1, 1, 1, 
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
  1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 
  1, 0, 0, 2, 0, 3, 0, 0, 0, 2, 0, 3, 0, 3, 0, 1, 
  1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 
  1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 
  1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 
  1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 
  1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 
  1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 
  1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 
  1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 2, 
  1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 
  1, 3, 3, 1, 1, 1, 1, 3, 3, 1, 1, 1, 1, 2, 2, 1, 
};


void draw_line(int sx, int sy, int len, int mode, const chtype ch);
void draw_level(int sx, int sy);
void draw_player(int sx, int sy, struct Player* p);
void render_2d(struct Player* player);
bool is_point_in_world(double x, double y);
void render_3d(int sx, int sy, int width, int height, struct Player* player);
int get_level_block(int x, int y);

int main() {
  initscr();
  start_color();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);
  init_pair(1, 20, COLOR_BLACK); // lighter dark blue
  init_pair(2, 19, COLOR_BLACK); // darker dark blue
  init_pair(3, 40, COLOR_BLACK); // lighter dark green
  init_pair(4, 34, COLOR_BLACK); // darker dark green
  init_pair(5, 160, COLOR_BLACK); // lighter dark red
  init_pair(6, 124, COLOR_BLACK); // darker dark red
  init_pair(255, COLOR_WHITE, COLOR_BLACK); // Player is normal

  struct Player player = {1.0, 1.0, 1.0, 0.0, 0.0};
  int last_input; 

  double newx = player.x;
  double newy = player.y;
  
  do {
    render_2d(&player);
    last_input = getch();


    if(last_input == 'd') {
      player.a += 0.1;
      player.dx = cos(player.a);
      player.dy = sin(player.a);
    }
    if(last_input == 'a') {
      player.a -= 0.1;
      player.dx = cos(player.a);
      player.dy = sin(player.a);
    }
    if(last_input == 'w') {
      newx += player.dx * 0.1;
      newy += player.dy * 0.1;
    }
    if(last_input == 's') {
      newx -= player.dx * 0.1;
      newy -= player.dy * 0.1;
    }

    if(!is_point_in_world(newx, newy)) {
      player.x = newx;
      player.y = newy;
    } else if(!is_point_in_world(player.x, newy)) {
      newx = player.x;
      player.y = newy;
    } else if(!is_point_in_world(newx, player.y)) {
      player.x = newx;
      newy = player.y;
    } else {
      newx = player.x;
      newy = player.y;
    }

  } while(last_input != KEY_F(1));

  endwin();
  return 0;
}

void render_2d(struct Player* player) {
    clear();
    // printw("px: %f\tpy: %f\t pa: %f", player->x, player->y, player->a);

    render_3d(20, 3, 200, 60, player);
    // draw_level(3, 3);
    // draw_player(3, 3, player);

    
    refresh();
}

void draw_line(int sx, int sy, int len, int mode, const chtype ch) {
  if(mode == VERTICAL) {
    for(int i = 0; i < len; i++)
      mvaddch(sy+i,sx, ch);
  } else {
    for(int i = 0; i < len; i++)
      mvaddch(sy,sx+i, ch);
  }
}

void draw_level(int sx, int sy) {
  int colour;
  for(int x = 0; x < LEVEL_WIDTH; x++) {
    for(int y = 0; y < LEVEL_HEIGHT; y++) {
      if((colour = get_level_block(x, y))) {
        attron(COLOR_PAIR(2 * colour));
        mvaddch(y+sy, x*2+sx, ACS_BLOCK);
        mvaddch(y+sy, x*2+1+sx, ACS_BLOCK);
      }
    }
  }
}

void draw_player(int sx, int sy, struct Player* p) {
  int rx = (int) ((p->x/GRID_SIZE) * 2.0);
  int ry = (int) ((p->y/GRID_SIZE));
  attron(COLOR_PAIR(255));
  mvaddch(ry+sy, rx+sx, 'Y');
}

bool is_point_in_world(double x, double y) {
  int rx = (int)((x/GRID_SIZE));
  int ry = (int)((y/GRID_SIZE));
  return get_level_block(rx, ry);
}



void render_3d(int sx, int sy, int width, int height, struct Player* player) {

  double dirX = cos(player->a);
  double dirY = sin(player->a);
  double viewPlaneX = -dirY*tan(FOV/2);
  double viewPlaneY = dirX*tan(FOV/2);

  
  for(int x = 0; x < width; x++) {
    double cameraX = 2 * x/(double)width - 1; // 0 <-> width => -1 <-> 1
    double rayDirX = dirX + viewPlaneX*cameraX;
    double rayDirY = dirY + viewPlaneY*cameraX;
    //printw("%f", (cameraX));

    int mapX = (int) player->x;
    int mapY = (int) player->y;

    //distance from start to first side of grid
    double sideDistX;
    double sideDistY;

    // length of ray that travels between perpendicular grid lines
    double deltaDistX = fabs(1/rayDirX);
    double deltaDistY = fabs(1/rayDirY);
    double perpWallDist;

    int stepX;
    int stepY;

    int hit = 0;
    int side;
    int rayJumps = 0;


    if(rayDirX < 0) {
      stepX = -1;
      sideDistX = (player->x - mapX) * deltaDistX;
    } else {
      stepX = 1;
      sideDistX = (mapX + 1 - player->x) * deltaDistX;
    }
    if(rayDirY < 0) {
      stepY = -1;
      sideDistY = (player->y - mapY) * deltaDistY;
    } else {
      stepY = 1;
      sideDistY = (mapY + 1 - player->y) * deltaDistY;
    }

    while(hit == 0) {
      // jump along the shortest travelled so far, the DDA algorithm for line drawing on pixel grid
      if (sideDistX < sideDistY) {
        sideDistX += deltaDistX;
        mapX += stepX;
        side = 0;
      } else {
        sideDistY += deltaDistY;
        mapY += stepY;
        side = 1;
      }
      if (get_level_block(mapX, mapY) > 0) hit = 1;
      if (rayJumps++ > DOF) break;
    }

    // this is so smart, it only calculates the distance on the X or Y
    // direction then scales by the direction of the ray in that direction so
    // that the more perpendicular they are looking the closer they are to the
    // camera this means points along line will have same distance values
    
    if (side == 0) {
      perpWallDist = (mapX - player->x + (1 - stepX)/2.)/rayDirX;
    } else {
      perpWallDist = (mapY - player->y + (1 - stepY)/2.)/rayDirY;
    }
    //printw("x: %d, y: %d", stepX, stepY);

    int lineHeight = (int) (height/perpWallDist);
    if(lineHeight > height) lineHeight = height;

    attron(COLOR_PAIR(get_level_block(mapX, mapY) * 2 - side));
    // printw("%d", lineHeight);
    draw_line(x+sx, (height-lineHeight)/2 + sy, lineHeight, VERTICAL, ACS_BLOCK);

  }
}


int get_level_block(int x, int y) {
  return level[x + y * LEVEL_WIDTH];
//  if(y%2 == 1) {
//    return 0;
//  }
//  if(x < 0) {
//    return -x % 2;
//  }
//  return x % 2;
}







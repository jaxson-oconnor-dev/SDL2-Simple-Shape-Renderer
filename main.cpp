#include <iostream>
#include <SDL2/SDL.h>
#include <chrono>
#include <cmath>
#include <vector>

#define WIDTH 800
#define HEIGHT 600
#define FOV WIDTH/2

typedef struct vec2 { double x, y; } vec2;
typedef struct vec3 { double x, y, z; } vec3;

using namespace std;

SDL_Window* window = SDL_CreateWindow("Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_BORDERLESS);
SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
SDL_Event event;

namespace shape {
  typedef struct polygon { vector<vec3> points; } polygon;
  typedef struct polyhedron { vector<polygon> polygons; } polyhedron;
};

namespace transform {
  /** @brief ROTATE VEC3 IN 3D SPACE RELATIVE TO ORIGIN */
  void rotate(vec3& target, vec3 origin, double x, double y, double z, bool isDeg) {
    double x_ = isDeg ? x * (M_PI / 180.0f) : x, y_ = isDeg ? y * (M_PI / 180.0f) : y, z_ = isDeg ? z * (M_PI / 180.0f) : z;
    double cosx = cos(x_), sinx = sin(x_), dx = target.x - origin.x,
      cosy = cos(y_), siny = sin(y_), dy = target.y - origin.y,
      cosz = cos(z_), sinz = sin(z_), dz = target.z - origin.z;
    target.x = static_cast<double>((dx * (cosy * cosz)) + (dy * (cosy * sinz)) + (dz * -siny));
    target.y = static_cast<double>((dx * (sinx * siny * cosz - cosx * sinz)) + (dy * (sinx * siny * sinz + cosx * cosz)) + (dz * (sinx * cosy)));
    target.z = static_cast<double>((dx * (cosx * siny * cosz + sinx * sinz)) + (dy * (cosx * siny * sinz - sinx * cosz)) + (dz * (cosx * cosy)));
  }

  /** @brief ROTATE A POLYGON */
  void rotate_polygon(shape::polygon& p, vec3 origin, double x, double y, double z, bool isDeg) { // add second & to shape::polygon if doesn't rotate
    for (vec3& v : p.points) rotate(v, origin, x, y, z, isDeg); }

  /** @brief ROTATE A POLYHEDRON */
  void rotate_polyhedron(shape::polyhedron& p, vec3 origin, double x, double y, double z, bool isDeg) {
    for (shape::polygon& pol : p.polygons) {
      rotate_polygon(pol, origin, x, y, z, isDeg);
    }
  }

  /** @brief TRANSLATE VEC3 IN 3D SPACE */
  void translate(vec3& target, double x, double y, double z) { target.x += x, target.y += y, target.z += z; }
};

namespace canvas {
  chrono::high_resolution_clock::time_point render_start, render_end;
  
  /** @brief TURN VEC3 -> VEC2 BASED ON FOV */
  vec2 get_projected(vec3 vec) { return { (vec.x * FOV) / (vec.z + FOV), (vec.y * FOV) / (vec.z + FOV) }; }

  /** @brief DRAW LINE */
  void draw_line(vec3 begin, vec3 end, SDL_Color color, bool isPerspective) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    vec2 proj_begin { isPerspective ? get_projected(begin).x : begin.x, isPerspective ? get_projected(begin).y : begin.y };
    vec2 proj_end { isPerspective ? get_projected(end).x : end.x, isPerspective ? get_projected(end).y : end.y};
    SDL_RenderDrawLine(renderer, WIDTH/2 + proj_begin.x, HEIGHT/2 + proj_begin.y, WIDTH/2 + proj_end.x, HEIGHT/2 + proj_end.y);
  }

  /** @brief DRAW POLYGON */
  void draw_closed_polygon(shape::polygon p, SDL_Color color, bool isPerspective) {
    for (int i=0; i<p.points.size(); i++) {
      draw_line(p.points.at(i), p.points.at((i + 1) % p.points.size()), color, isPerspective);
    }
  }

  /** @brief DRAW POLYGON */
  void draw_polyhedron(shape::polyhedron p, SDL_Color color, bool isPerspective) {
    for (int i=0; i<p.polygons.size(); i++) {
      draw_closed_polygon(p.polygons.at(i), color, isPerspective);
    }
  }

  /** @brief CLEAR RENDERER */
  void clear(SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderer);
  }
};

int SDL_main(int argc, char *argv[]) {
  double r1 = 50;

  // SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_MUL); // makes cool effect

  // example cube:
  shape::polyhedron cube = {{ 
    {{
      {-r1, r1, r1},
      {r1, r1, r1},
      {r1, -r1, r1},
      {-r1, -r1, r1}
    }},
    {{
      {-r1, r1, -r1},
      {r1, r1, -r1},
      {r1, -r1, -r1},
      {-r1, -r1, -r1}
    }},
    {{
      {-r1, r1, -r1},
      {-r1, r1, r1},
      {-r1, -r1, r1},
      {-r1, -r1, -r1}
    }},
    {{
      {r1, r1, -r1},
      {r1, r1, r1},
      {r1, -r1, r1},
      {r1, -r1, -r1}
    }},
    {{
      {-r1, r1, -r1},
      {r1, r1, -r1},
      {r1, r1, r1},
      {-r1, r1, r1}
    }},
    {{
      {-r1, -r1, -r1},
      {r1, -r1, -r1},
      {r1, -r1, r1},
      {-r1, -r1, r1}
    }},
  }};

  while (true) {
    canvas::clear({20, 20, 20});
  
    // begin draw
    canvas::draw_polyhedron(cube, {255, 255, 255}, true);
    transform::rotate_polyhedron(cube, {0, 0, 0}, 0.3, 0.15, 0.3, true);
    // end draw

    SDL_PollEvent(&event);
    if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) break;

    SDL_Delay(16);
    SDL_RenderPresent(renderer);
  }

  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  SDL_Quit();

  return 0;
}
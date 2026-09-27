#include <iostream>
#include <vector>
#include <SDL3/SDL.h>


constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 800;

constexpr uint32_t BOARD_WIDTH = 20;
constexpr uint32_t BOARD_HEIGHT = 20;

constexpr float H_MARGIN = 10.0f;
constexpr float V_MARGIN = 10.0f;

constexpr float FIELD_SIZE = (WINDOW_WIDTH - 2*H_MARGIN)/BOARD_WIDTH;

struct Pos {
  uint32_t x = -1;
  uint32_t y = -1;
};

enum struct FieldState: uint8_t {
  EMPTY,
  SNAKE,
  SNAKE_HEAD,
  APPLE
};

struct Field{
  FieldState state = FieldState::EMPTY;
  SDL_FRect rect = {-1.0, -1.0, -1.0, -1.0};
  Pos pos;

  Field() = default;

  void init(const Pos new_pos) {
    pos = new_pos;
    rect = {
      H_MARGIN + FIELD_SIZE*static_cast<float>(pos.x),
      V_MARGIN + FIELD_SIZE*static_cast<float>(pos.y),
      FIELD_SIZE,
      FIELD_SIZE
    };
  }

  void draw(SDL_Renderer *renderer) const {
    switch (state) {
      case FieldState::EMPTY:
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, SDL_ALPHA_OPAQUE);
        break;
      case FieldState::APPLE:
        SDL_SetRenderDrawColor(renderer, 200, 20, 20, SDL_ALPHA_OPAQUE);
        break;
      case FieldState::SNAKE:
        SDL_SetRenderDrawColor(renderer, 20, 200, 20, SDL_ALPHA_OPAQUE);
        break;
      case FieldState::SNAKE_HEAD:
        SDL_SetRenderDrawColor(renderer, 10, 100, 10, SDL_ALPHA_OPAQUE);
        break;
    }
    SDL_RenderFillRect(renderer, &rect);

    SDL_SetRenderDrawColor(renderer, 10, 10, 10, SDL_ALPHA_OPAQUE);
    SDL_RenderRect(renderer, &rect);
  }
};

void run(SDL_Window* window, SDL_Renderer *renderer) {

}

int main(int, char**) {
  SDL_Init(SDL_INIT_VIDEO);

  SDL_Window* window{SDL_CreateWindow(
    "Snek", WINDOW_WIDTH, WINDOW_HEIGHT, 0
  )};

  SDL_Renderer *renderer{SDL_CreateRenderer(window, nullptr)};

  Field board[BOARD_HEIGHT][BOARD_WIDTH];
  Field *board_it = &board[0][0];
  for (uint32_t y = 0; y < BOARD_HEIGHT; y++) {
    for (uint32_t x = 0; x < BOARD_WIDTH; x++, board_it++) {
      board_it->init({x, y});
    }
  }

  std::vector<Pos> snake;

  bool is_running = true;
  SDL_Event event;
  while (is_running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        is_running = false;
      }
    }

    SDL_SetRenderDrawColor(renderer, 200, 20, 200, 255);
    SDL_RenderClear(renderer);

    board_it = &board[0][0];
    for (uint32_t y = 0; y < BOARD_HEIGHT; y++) {
      for (uint32_t x = 0; x < BOARD_WIDTH; x++, board_it++) {
        board_it->draw(renderer);
      }
    }

    SDL_RenderPresent(renderer);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
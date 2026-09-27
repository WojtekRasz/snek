#include <iostream>
#include <queue>
#include <random>
#include <set>
#include <vector>
#include <SDL3/SDL.h>


constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 800;

constexpr uint32_t BOARD_WIDTH = 20;
constexpr uint32_t BOARD_HEIGHT = 20;

constexpr float H_MARGIN = 10.0f;
constexpr float V_MARGIN = 10.0f;

constexpr uint32_t MIN_SNAKE_OFFSET = 3;

constexpr float FIELD_SIZE = (WINDOW_WIDTH - 2*H_MARGIN)/BOARD_WIDTH;

struct Pos {
  uint32_t x = -1;
  uint32_t y = -1;

  Pos() = default;
  Pos(const uint32_t x, const uint32_t y) {
    this->x = x;
    this->y = y;
  }


};

enum struct FieldState: uint8_t {
  EMPTY,
  SNAKE,
  SNAKE_HEAD,
  APPLE
};

enum struct Direction: uint8_t {
  UP,
  RIGHT,
  DOWN,
  LEFT
};

Pos move_in_dir(Pos p, const Direction dir) {
  switch (dir) {
    case Direction::UP:
      return {p.x, p.y - 1};
    case Direction::RIGHT:
      return {p.x + 1, p.y};
    case Direction::DOWN:
      return {p.x, p.y + 1};
    case Direction::LEFT:
      return {p.x - 1, p.y};
  }
}

struct Field{
  FieldState state = FieldState::EMPTY;
  SDL_FRect rect = {-1.0, -1.0, -1.0, -1.0};
  Pos pos;

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

class Board{
public:
  Board(const uint32_t w, const uint32_t h, std::mt19937 &g): width(w), height(h), fields(w*h), gen(g), dist_x(0, w-1), dist_y(0, h-1) {
    auto board_it = fields.begin();
    for (uint32_t y = 0; y < height; y++) {
      for (uint32_t x = 0; x < width; x++, ++board_it) {
        board_it->init({x, y});
      }
    }
  }

  void draw(SDL_Renderer* renderer) {
    auto board_it = fields.begin();
    for (uint32_t y = 0; y < height; y++) {
      for (uint32_t x = 0; x < width; x++, ++board_it) {
        board_it->draw(renderer);
      }
    }
  }

  void put_random_apple() {
    Pos p = {dist_x(gen), dist_y(gen)};
    while (get_field(p).state != FieldState::EMPTY) {
      p = {dist_x(gen), dist_y(gen)};
    }
    set_field_state(p, FieldState::APPLE);
  }

  FieldState get_field_state(const Pos pos){ return get_field(pos).state; }
  void set_field_state(const Pos pos, const FieldState state){ get_field(pos).state = state; }

private:
  Field &get_field(const uint32_t x, const uint32_t y) {
    if (y < 0 || y >= height || x < 0 || x >= width) throw std::runtime_error("Outside board!!!");
    return fields[y * width + x];
  }
  Field &get_field(const Pos pos){return get_field(pos.x , pos.y);}

  const uint32_t width;
  const uint32_t height;
  std::vector<Field> fields;
  std::mt19937& gen;
  std::uniform_int_distribution<uint32_t> dist_x;
  std::uniform_int_distribution<uint32_t> dist_y;
};

class Snek{
public:
  explicit Snek(Board &board, Pos init_pos, Direction tail_dir): board(board){
    poss.emplace_back(init_pos);
    poss.emplace_back(move_in_dir(init_pos, tail_dir));
    board.set_field_state(init_pos, FieldState::SNAKE_HEAD);
    board.set_field_state(move_in_dir(init_pos, tail_dir), FieldState::SNAKE);
  }

  void move(Direction dir) {
    const uint32_t tail = (head + 1) % poss.size();
    const Pos new_head_pos = move_in_dir(poss[head], dir);
    try {
      switch (board.get_field_state(new_head_pos)) {
        case FieldState::SNAKE:
        case FieldState::SNAKE_HEAD:
          throw std::runtime_error("Hit snek");
        case FieldState::APPLE:
          board.put_random_apple();
          poss.insert(poss.begin() + tail, poss[tail]);
        case FieldState::EMPTY:
          board.set_field_state(poss[head], FieldState::SNAKE);
          board.set_field_state(poss[tail], FieldState::EMPTY);
          poss[tail] = new_head_pos;
          board.set_field_state(new_head_pos, FieldState::SNAKE_HEAD);
          head = tail;
          break;
      }
    } catch (std::runtime_error &e) {
      std::cout << "Snek: " << e.what();
      throw;
    }


  }

  std::vector<Pos> poss;
  uint32_t head = 0;
  Board& board;

};

int main(int, char**) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint32_t> dist_x(MIN_SNAKE_OFFSET, BOARD_WIDTH - 1 - MIN_SNAKE_OFFSET);
  std::uniform_int_distribution<uint32_t> dist_y(MIN_SNAKE_OFFSET, BOARD_HEIGHT - 1 - MIN_SNAKE_OFFSET);

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window* window{SDL_CreateWindow(
    "Snek", WINDOW_WIDTH, WINDOW_HEIGHT, 0
  )};

  SDL_Renderer *renderer{SDL_CreateRenderer(window, nullptr)};

  Board board{BOARD_WIDTH, BOARD_HEIGHT, gen};
  Direction curr_direction = Direction::UP;

  Snek snek{board, {dist_x(gen), dist_y(gen)}, Direction::DOWN};

  board.put_random_apple();

  bool is_running = true;
  std::queue<Direction> dir_queue;
  SDL_Event event;
  while (is_running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        is_running = false;
      }
      if (event.type == SDL_EVENT_KEY_DOWN) {
        switch (event.key.key) {
          case SDLK_W:
            dir_queue.push(Direction::UP);
            break;
          case SDLK_D:
            dir_queue.push(Direction::RIGHT);
            break;
          case SDLK_S:
            dir_queue.push(Direction::DOWN);
            break;
          case SDLK_A:
            dir_queue.push(Direction::LEFT);
            break;
          default:
            break;
        }
      }
    }
    if (!dir_queue.empty()) {
      curr_direction = dir_queue.front();
      dir_queue.pop();
    }

    snek.move(curr_direction);

    SDL_SetRenderDrawColor(renderer, 200, 20, 200, 255);
    SDL_RenderClear(renderer);

    board.draw(renderer);

    SDL_RenderPresent(renderer);

    SDL_Delay(200);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
#include <emscripten.h>
#include <SDL.h>
#include <iostream>
#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>
#include <emscripten/html5.h>
#endif

enum Draw_Modes {
    FREE,
    LINE,   // disabled for now until regular drawing is working with websockets
    CIRCLE, // disabled for now until lines are working with websockets
    RECT    // not yet implemented
};

std::vector<int> draw_start_x;
std::vector<int> draw_start_y;
std::vector<int> draw_end_x;
std::vector<int> draw_end_y;
std::vector<int> draw_types;
int draw_command_count = 0;
int last_sent_index = 0;

struct GameState {
    bool running;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* drawingTexture = nullptr;

    bool mousePressed = false;

    int lineMode = 0;
    int circleMode = 0;
    int squareMode = 0;

    int screenWidth = 800;
    int screenHeight = 600;

    int lastMouseX = -1;
    int lastMouseY = -1;
};

GameState gameState;

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void add_draw_command(int startX, int startY, int endX, int endY, int type) {
        draw_start_x.push_back(startX);
        draw_start_y.push_back(startY);
        draw_end_x.push_back(endX);
        draw_end_y.push_back(endY);
        draw_types.push_back(type);
        ++draw_command_count;
    }

    EMSCRIPTEN_KEEPALIVE
    int get_draw_command_count() {
        return draw_command_count;
    }

    EMSCRIPTEN_KEEPALIVE
    int get_new_command_count() {
        return draw_command_count - last_sent_index;
    }

    EMSCRIPTEN_KEEPALIVE
    void mark_commands_sent() {
        last_sent_index = draw_command_count;
    }

    // get pointers to the start of all the arrays
    EMSCRIPTEN_KEEPALIVE
    int* get_start_x_array() { return draw_start_x.data(); }
    EMSCRIPTEN_KEEPALIVE
    int* get_start_y_array() { return draw_start_y.data(); }
    EMSCRIPTEN_KEEPALIVE
    int* get_end_x_array() { return draw_end_x.data(); }
    EMSCRIPTEN_KEEPALIVE
    int* get_end_y_array() { return draw_end_y.data(); }
    EMSCRIPTEN_KEEPALIVE
    int* get_draw_types() { return draw_types.data(); }

    EMSCRIPTEN_KEEPALIVE
    void clear_draw_commands() {
        draw_start_x.clear();
        draw_start_y.clear();
        draw_end_x.clear();
        draw_end_y.clear();
        draw_types.clear();
        draw_command_count = 0;
        last_sent_index = 0;
    }

    EMSCRIPTEN_KEEPALIVE
    void draw_line(int startX, int startY, int endX, int endY, int type, bool add_to_stack) {
        SDL_SetRenderTarget(gameState.renderer, gameState.drawingTexture);
        SDL_SetRenderDrawColor(gameState.renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(gameState.renderer, startX, startY, endX, endY);
        SDL_SetRenderTarget(gameState.renderer, NULL);
        if (add_to_stack)
            add_draw_command(startX, startY, endX, endY, type);
    }

    EMSCRIPTEN_KEEPALIVE
    void resize_renderer(int width, int height) {
        std::cout << "Resizing to: " << width << "x" << height << std::endl;
        SDL_Texture* oldTexture = gameState.drawingTexture;
        int oldWidth, oldHeight;
        SDL_QueryTexture(oldTexture, NULL, NULL, &oldWidth, &oldHeight);

        gameState.drawingTexture = SDL_CreateTexture(
            gameState.renderer,
            SDL_PIXELFORMAT_ABGR8888,
            SDL_TEXTUREACCESS_TARGET,
            width,
            height
        );

        SDL_SetRenderTarget(gameState.renderer, gameState.drawingTexture);
        SDL_SetRenderDrawColor(gameState.renderer, 0, 0, 0, 255);
        SDL_RenderClear(gameState.renderer);

        SDL_Rect SrcRect = {0, 0, oldWidth, oldHeight};
        SDL_Rect DstRect = {0, 0, oldWidth, oldHeight};
        SDL_RenderCopy(gameState.renderer, oldTexture, &SrcRect, &DstRect);
        SDL_SetRenderTarget(gameState.renderer, NULL);

        SDL_DestroyTexture(oldTexture);

        gameState.screenWidth = width;
        gameState.screenHeight = height;

        SDL_SetWindowSize(gameState.window, width, height);
        SDL_RenderSetLogicalSize(gameState.renderer, width, height);
    }

    EMSCRIPTEN_KEEPALIVE
    void clear_canvas() {
        SDL_SetRenderTarget(gameState.renderer, gameState.drawingTexture);
        SDL_SetRenderDrawColor(gameState.renderer, 0, 0, 0, 255);
        SDL_RenderClear(gameState.renderer);
        SDL_SetRenderTarget(gameState.renderer, NULL);
        clear_draw_commands();
    }

    EMSCRIPTEN_KEEPALIVE
    int set_line_mode() {
        gameState.lineMode = !gameState.lineMode;
        gameState.circleMode = 0;
        gameState.squareMode = 0;
        return gameState.lineMode;
    }

    EMSCRIPTEN_KEEPALIVE
    int get_line_mode() {
        return gameState.lineMode;
    }

    EMSCRIPTEN_KEEPALIVE
    int set_circle_mode() {
        gameState.lineMode = 0;
        gameState.circleMode = !gameState.circleMode;
        gameState.squareMode = 0;
        return gameState.circleMode;
    }

    EMSCRIPTEN_KEEPALIVE
    int get_circle_mode() {
        return gameState.circleMode;
    }

    EMSCRIPTEN_KEEPALIVE
    int set_square_mode() {
        gameState.lineMode = 0;
        gameState.circleMode = 0;
        gameState.squareMode = !gameState.squareMode;
        return gameState.squareMode;
    }

    EMSCRIPTEN_KEEPALIVE
    int get_square_mode() {
        return gameState.squareMode;
    }

    EMSCRIPTEN_KEEPALIVE
    int get_last_mouse_x() {
        return gameState.lastMouseX;
    }

    EMSCRIPTEN_KEEPALIVE
    int get_last_mouse_y() {
        return gameState.lastMouseY;
    }

}

// midpoint circle algorithm
void draw_circle(SDL_Renderer* renderer, int centerX, int centerY, int radius) {
    int x = radius;
    int y = 0;
    int d = 1 - radius;

    while (x >= y) {
        SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
        SDL_RenderDrawPoint(renderer, centerX + y, centerY + x);
        SDL_RenderDrawPoint(renderer, centerX - y, centerY + x);
        SDL_RenderDrawPoint(renderer, centerX - x, centerY + y);
        SDL_RenderDrawPoint(renderer, centerX - x, centerY - y);
        SDL_RenderDrawPoint(renderer, centerX - y, centerY - x);
        SDL_RenderDrawPoint(renderer, centerX + y, centerY - x);
        SDL_RenderDrawPoint(renderer, centerX + x, centerY - y);

        y += 1;

        if (d <= 0) {
            d += 2 * y + 1;
        } else {
            x -= 1;
            d += 2 * (y - x) + 1;
        }
    }
}

void draw_square(SDL_Renderer* renderer, int startX, int startY, int endX, int endY) {
    int dx = endX - startX;
    int dy = endY - startY;

    int sideLength = std::max(abs(dx), abs(dy));

    // Maintain the original startX, startY as the anchor point
    // and adjust the end points to form a square
    int x2, y2;
    
    // Determine the direction of drag and adjust accordingly
    if (dx >= 0 && dy >= 0) {        // Dragging right and down
        x2 = startX + sideLength;
        y2 = startY + sideLength;
    } else if (dx >= 0 && dy < 0) {  // Dragging right and up
        x2 = startX + sideLength;
        y2 = startY - sideLength;
    } else if (dx < 0 && dy >= 0) {  // Dragging left and down
        x2 = startX - sideLength;
        y2 = startY + sideLength;
    } else {                         // Dragging left and up
        x2 = startX - sideLength;
        y2 = startY - sideLength;
    }

    // Draw the square
    SDL_RenderDrawLine(renderer, startX, startY, x2, startY);     // Top edge
    SDL_RenderDrawLine(renderer, x2, startY, x2, y2);            // Right edge
    SDL_RenderDrawLine(renderer, x2, y2, startX, y2);            // Bottom edge
    SDL_RenderDrawLine(renderer, startX, y2, startX, startY);    // Left edge
}

void game_loop(void* arg) {
    GameState* state = static_cast<GameState*>(arg);
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_MOUSEMOTION:
//                std::cout << "Mouse Motion - Pressed: " << (state->mousePressed ? "true" : "false") 
//                         << " X: " << event.motion.x 
//                         << " Y: " << event.motion.y << std::endl;
                if (state->mousePressed) {
                    if (state->lineMode) {
                        SDL_SetRenderTarget(state->renderer, NULL);
                        SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);
                        SDL_RenderClear(state->renderer);

                        SDL_RenderCopy(state->renderer, state->drawingTexture, NULL, NULL);

                        SDL_SetRenderDrawColor(state->renderer, 255, 255, 255, 255);
                        SDL_RenderDrawLine(state->renderer, state->lastMouseX, state->lastMouseY,
                                event.motion.x, event.motion.y);
                        SDL_RenderPresent(state->renderer);
                    } else if (state->circleMode) {
                        SDL_SetRenderTarget(state->renderer, NULL);
                        SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);
                        SDL_RenderClear(state->renderer);

                        SDL_RenderCopy(state->renderer, state->drawingTexture, NULL, NULL);

                        SDL_SetRenderDrawColor(state->renderer, 255, 255, 255, 255);

                        int centerX = state->lastMouseX;
                        int centerY = state->lastMouseY;
                        int radius = sqrt(pow(event.motion.x - centerX, 2) + pow(event.motion.y - centerY, 2));
                        draw_circle(state->renderer, centerX, centerY, radius);

                        SDL_RenderPresent(state->renderer);
                    } else {
                        draw_line(state->lastMouseX, state->lastMouseY, event.motion.x, event.motion.y, FREE, true);

                        state->lastMouseX = event.motion.x;
                        state->lastMouseY = event.motion.y;
                    }
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    state->mousePressed = true;
                    state->lastMouseX = event.motion.x;
                    state->lastMouseY = event.motion.y;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    state->mousePressed = false;
                    if (state->lineMode) {
                        draw_line(state->lastMouseX, state->lastMouseY, event.motion.x, event.motion.y, LINE, true);
                        state->lineMode = 0;
                    } else if (state->circleMode) {
                        SDL_SetRenderTarget(state->renderer, state->drawingTexture);
                        SDL_SetRenderDrawColor(state->renderer, 255, 255, 255, 255);
                        int centerX = state->lastMouseX;
                        int centerY = state->lastMouseY;
                        int radius = sqrt(pow(event.motion.x - centerX, 2) + pow(event.motion.y - centerY, 2));
                        draw_circle(state->renderer, centerX, centerY, radius);
                        SDL_SetRenderTarget(state->renderer, NULL);
                        state->circleMode = 0;
                    } else if (state->squareMode) {
                        SDL_SetRenderTarget(state->renderer, state->drawingTexture);
                        SDL_SetRenderDrawColor(state->renderer, 255, 255, 255, 255);
                        draw_square(state->renderer, state->lastMouseX, state->lastMouseY, event.motion.x, event.motion.y);
                        SDL_SetRenderTarget(state->renderer, NULL);
                    }
                }
                break;
            case SDL_QUIT:
                state->running = false;
                break;
        }
    } 

    SDL_SetRenderDrawColor(state->renderer, 255, 255, 255, 255);
    SDL_RenderClear(state->renderer);
    SDL_RenderCopy(state->renderer, state->drawingTexture, NULL, NULL);
    if (!(state->lineMode || state->circleMode || state->squareMode)) {
        SDL_RenderPresent(state->renderer);
    }
}

#ifdef __EMSCRIPTEN__
void emscripten_loop(void* arg) {
    game_loop(arg);
}
#endif

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "failed to init SDL_INIT_VIDEO" << std::endl;
        return 1;
    }
    std::cout << "successfully inited SDL_INIT_VIDEO" << std::endl;
    gameState.running = true;
    int width = EM_ASM_INT({ return window.innerWidth });
    int height = EM_ASM_INT({ return window.innerHeight });
    SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#canvas");   
    if (SDL_CreateWindowAndRenderer(width, height, SDL_WINDOW_SHOWN, &gameState.window, &gameState.renderer) < 0) {
        std::cerr << "failed to create window or renderer" << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    gameState.drawingTexture = SDL_CreateTexture(
        gameState.renderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_TARGET,
        width,
        height
    );

    if (!gameState.drawingTexture) {
        std::cout << "failed to init gamestate drawingTexture " << SDL_GetError() << std:: endl;
        return 1;
    }

    SDL_SetRenderTarget(gameState.renderer, gameState.drawingTexture);
    SDL_SetRenderDrawColor(gameState.renderer, 0, 0, 0, 255);
    SDL_RenderClear(gameState.renderer);
    SDL_SetRenderTarget(gameState.renderer, NULL);

    SDL_SetWindowTitle(gameState.window, "wasm draw");


#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(emscripten_loop, &gameState, 0, 1);
#else
    while (gameState.running) {
        game_loop(&gameState);
    }
#endif

    if (gameState.drawingTexture) {
        SDL_DestroyTexture(gameState.drawingTexture);
    }
    SDL_DestroyRenderer(gameState.renderer);
    SDL_DestroyWindow(gameState.window);
    SDL_Quit();

    return 0;
}

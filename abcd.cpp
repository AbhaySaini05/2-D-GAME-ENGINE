#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int MOVEMENT_SPEED = 5;
const int JUMP_VELOCITY = 15;
const int FLOOR_LEVEL = SCREEN_HEIGHT - 50;

bool handleInput(SDL_Event& event, bool& moveLeft, bool& moveRight, bool& jump) {
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            return true;
        } else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    return true;
                case SDLK_a:
                    moveLeft = true;
                    break;
                case SDLK_d:
                    moveRight = true;
                    break;
                case SDLK_w:
                    jump = true;
                    break;
            }
        } else if (event.type == SDL_KEYUP) {
            switch (event.key.keysym.sym) {
                case SDLK_a:
                    moveLeft = false;
                    break;
                case SDLK_d:
                    moveRight = false;
                    break;
                case SDLK_w:
                    jump = false;
                    break;
            }
        }
    }

    return false;
}

SDL_Texture* loadTexture(const char* path, SDL_Renderer* renderer) {
    SDL_Surface* loadedSurface = IMG_Load(path);
    if (!loadedSurface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    SDL_FreeSurface(loadedSurface);

    return texture;
}

int main(int argc, char* args[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Player Image Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Texture* playerTexture = loadTexture("lettuce.jpg", renderer);
    if (!playerTexture) {
        return -1;
    }

    int playerX = SCREEN_WIDTH / 2;
    int playerY = SCREEN_HEIGHT / 2;
    int playerWidth, playerHeight;
    SDL_QueryTexture(playerTexture, nullptr, nullptr, &playerWidth, &playerHeight);
    playerWidth = 100;
    playerHeight = 100;

    int velocityY = 0;
    bool isJumping = false;

    bool moveLeft = false;
    bool moveRight = false;
    bool jump = false;

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        quit = handleInput(e, moveLeft, moveRight, jump);

        if (moveLeft) playerX -= MOVEMENT_SPEED;
        if (moveRight) playerX += MOVEMENT_SPEED;

        if (jump && !isJumping) {
            velocityY = -JUMP_VELOCITY;
            isJumping = true;
        }

        if (isJumping) {
            velocityY += 1;
        }

        playerY += velocityY;

        if (playerY > FLOOR_LEVEL) {
            playerY = FLOOR_LEVEL;
            velocityY = 0;
            isJumping = false;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect floorRect = {0, FLOOR_LEVEL, SCREEN_WIDTH, SCREEN_HEIGHT - FLOOR_LEVEL};
        SDL_RenderFillRect(renderer, &floorRect);

        SDL_Rect playerRect = {playerX, playerY, playerWidth, playerHeight};
        SDL_RenderCopy(renderer, playerTexture, nullptr, &playerRect);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(playerTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}

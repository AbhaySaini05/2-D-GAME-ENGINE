#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <unordered_map>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TILE_SIZE = 32;

using namespace std;

class Player {
private:
    int x, y, SPEED, JUMP_VELOCITY, lives;
public:
    Player() : x(SCREEN_WIDTH/2), y(SCREEN_HEIGHT/2), SPEED(3), JUMP_VELOCITY(15), lives(3) {};
    Player(int X, int Y, int sp, int jv) : x(X), y(Y), SPEED(sp), JUMP_VELOCITY(jv), lives(3) {};
    friend class GameEngine;
};

class GameEngine {
public:
    GameEngine();
    ~GameEngine();

    void Initialize(const char* title, int width, int height);
    void Run();
    void Shutdown();
    void Update();

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    unordered_map<int, SDL_Texture*> tileTexture;
    unordered_map<int, SDL_Texture*> Life;
    TTF_Font *font;  
    SDL_Texture* playerTexture;
    SDL_Texture* bg;
    SDL_Texture* fg;
    SDL_Texture* tt;
    Player py;
    bool isRunning;
    bool left;
    bool right;
    bool jump;
    bool isJumping;
    bool won;
    int velocityX;
    int velocityY;
    int startX, startY;

    vector<vector<int>> levelData;

    void LoadLevelConfiguration(const std::string& configFile);
    void RenderScene();
    void Render();
    int checkCollision(int);
    void handleInput();
    void LoadTextures();
    bool winCheck();
    void win();
};

GameEngine::GameEngine() : window(nullptr), renderer(nullptr), isRunning(false), left(false), right(false), jump(false), isJumping(false), velocityX(0), velocityY(0), won(false) {};

GameEngine::~GameEngine() {
    Shutdown();
}

void GameEngine::Initialize(const char* title, int width, int height) {
    cout << "Init";
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "SDL initialization error: " << SDL_GetError() << std::endl;
        return;
    }
    TTF_Init();

    window = SDL_CreateWindow(title, 50, 50, width, height, SDL_WINDOW_SHOWN);
    if (!window) {
        cerr << "Window creation error: " << SDL_GetError() << std::endl;
        return;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        cerr << "Renderer creation error: " << SDL_GetError() << std::endl;
        return;
    }
    font = TTF_OpenFont("PressStart2P-Regular.ttf", 24);
    LoadLevelConfiguration("level_config.txt");
    LoadTextures();
    isRunning = true;
}

void GameEngine::Run() {
    cout << "Run";
    while (isRunning) {
        if (won) {
            win();
            SDL_Event e;
            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT || e.type == SDL_KEYDOWN) {
                    isRunning = false;
                }
            }
        } else {
            handleInput();
            Update();
            RenderScene();
        }
    }

}

void GameEngine::Shutdown() {
    cout << "Shutdown";
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }

    if (window) {
        SDL_DestroyWindow(window);
    }
    TTF_Quit();
    SDL_Quit();
}

int GameEngine::checkCollision(int choice = 0) {
    int X, Y;
    switch (choice) {
        case 0 :
        X = py.x/TILE_SIZE;
        Y = py.y/TILE_SIZE;
        break;
        case 1 :
        X = py.x/TILE_SIZE;
        Y = py.y/TILE_SIZE + 1;
        break;
        case 2 :
        X = py.x/TILE_SIZE;
        Y = py.y/TILE_SIZE - 1;
        break;
        case 3 :
        X = py.x/TILE_SIZE + 1;
        Y = py.y/TILE_SIZE;
        break;
        case 4 : 
        X = py.x/TILE_SIZE - 1;
        Y = py.y/TILE_SIZE;
        break;
        case 5:
        X = py.x/TILE_SIZE + 1;
        Y = py.y/TILE_SIZE - 1;
        default :
        case 6:
        X = py.x/TILE_SIZE + 1;
        Y = py.y/TILE_SIZE + 1;
        break;
    } 
    if (X >= 0 && X < levelData[0].size() && Y >= 0 && Y < levelData.size()) {
        if (levelData[Y][X] == 1 || levelData[Y][X] == 2) {
            return 1;
        } else {
            return 0;
        }
    }
    return -1;
}

void GameEngine::Update() {
    int flag;
    if (left) {
        if (py.x / TILE_SIZE > 0.5) {
            py.x -= py.SPEED;
            flag = checkCollision();
            if (flag == 1) {
                py.x = ((py.x + TILE_SIZE)/TILE_SIZE) * TILE_SIZE;
            }
        }
    }
    if (right) {
        if (py.x / TILE_SIZE != levelData[0].size() - 1) {
            py.x += py.SPEED;
            if (checkCollision(3) == 1) {
                py.x = (py.x/TILE_SIZE) * TILE_SIZE;
            }
        }
    }

    
    if (isJumping) velocityY += 1;
    py.y += velocityY;

    if (velocityY > 0 && (checkCollision(1) == 1 || checkCollision(6) == 1)) {
        
        py.y = (py.y/ TILE_SIZE) * TILE_SIZE;
        velocityY = 0;
        isJumping = false;
    }

    if (jump && !isJumping) {
        isJumping = true;
        velocityY -= py.JUMP_VELOCITY;
    }

    if (velocityY <= 0 && checkCollision() == 1) {
        py.y = ((py.y + TILE_SIZE)/TILE_SIZE) * TILE_SIZE;
        velocityY = 0;
    }
    if (velocityY == 0 && checkCollision() != 1) {
        isJumping = true;
    }
    if (py.y > SCREEN_HEIGHT + 50) {
        cout << "Death";
        py.lives--;
        if (py.lives == 0) {
            cout << "PermaDeath";
            isRunning = false;
            SDL_Delay(1000);
        } else {
            py.x = startX;
            py.y = startY;
        }
    }
    if (winCheck()) {
        won = true;
    }
}

void GameEngine::handleInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            isRunning = false;
        } else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    isRunning = false;
                    break;
                case SDLK_SPACE :
                    jump = true;
                    break;
                case SDLK_LEFT :
                    left = true;
                    break;
                case SDLK_RIGHT :
                    right = true;
                    break;
                default : 
                    break;
            }
        } else if (event.type == SDL_KEYUP) {
            switch (event.key.keysym.sym){
                case SDLK_SPACE:
                    jump = false;
                    break;
                case SDLK_RIGHT:
                    right = false;
                    break;
                case SDLK_LEFT:
                    left = false;
                    break;
                default:
                    break;
            }
        }
    }
}

void GameEngine::LoadTextures() {
    SDL_Surface* tileSurface = IMG_Load("soil.png");
    tileTexture[1] = SDL_CreateTextureFromSurface(renderer, tileSurface);
    SDL_FreeSurface(tileSurface);


    SDL_Surface* tileSurface2 = IMG_Load("grass.png");
    tileTexture[2] = SDL_CreateTextureFromSurface(renderer, tileSurface2);
    SDL_FreeSurface(tileSurface2);


    SDL_Surface* playerSurface = IMG_Load("playerpic2.png");
    playerTexture = SDL_CreateTextureFromSurface(renderer, playerSurface);
    SDL_FreeSurface(playerSurface);


    SDL_Surface* backg = IMG_Load("bg3.png");
    bg = SDL_CreateTextureFromSurface(renderer, backg);
    SDL_FreeSurface(backg);


    SDL_Surface* fgSur = IMG_Load("flag.png");
    fg = SDL_CreateTextureFromSurface(renderer, fgSur);
    SDL_FreeSurface(fgSur);
    

    SDL_Surface* lifeactive=IMG_Load("lifeActive.png");
    Life[6] = SDL_CreateTextureFromSurface(renderer,lifeactive);
    SDL_FreeSurface(lifeactive);


    SDL_Surface* lifeInactive = IMG_Load("lifeInactive.png");
    Life[7] = SDL_CreateTextureFromSurface(renderer,lifeInactive);
    SDL_FreeSurface(lifeInactive);
}

void GameEngine::LoadLevelConfiguration(const std::string& configFile) {
    ifstream inFile(configFile);

    levelData.clear(); 
    int tileType;
    string line;
    while (getline(inFile, line, '\n')) {
        vector<int> tileRow;
        istringstream ss(line);
        while (ss >> tileType) {
            tileRow.push_back(tileType);
        }
        levelData.push_back(tileRow);
    }

    inFile.close();

    for (int i = 0; i < SCREEN_HEIGHT/TILE_SIZE; i++) {
        for (int j = 0; j < SCREEN_WIDTH/TILE_SIZE; j++) {
            std::cout << levelData[i][j] << " ";
            if (levelData[i][j] == 5) {
                py.x = startX = j*TILE_SIZE;
                py.y = startY = i*TILE_SIZE;
            }
        }
        cout << std::endl;
    }
}

bool GameEngine::winCheck() {
    int X = py.x/TILE_SIZE;
    int Y = py.y/TILE_SIZE;
    if (X >= 0 && X < levelData[0].size() && Y >= 0 && Y < levelData.size()) {
        if (levelData[Y][X] == 3) {
            return true;
        } else {
            return false;
        }
    }
    return false;
}

void GameEngine::win() {
    SDL_Rect menuRect = {SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderFillRect(renderer, &menuRect);
    SDL_Color col = {255, 255, 255, 255};
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Congratulations!!! You won!!!", col);
    tt = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = {SCREEN_WIDTH / 2 - 90, SCREEN_HEIGHT / 2 + 45, 180, 30};
    SDL_RenderCopy(renderer, tt, nullptr, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_RenderPresent(renderer);
}

void GameEngine::RenderScene() {
    int lifeFlag[3] = {1, 1, 1};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    SDL_Rect backGround = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderCopy(renderer, bg, nullptr, &backGround);
    for (int y = 0; y < levelData.size(); ++y) {
        for (int x = 0; x < levelData[y].size(); ++x) {
            SDL_Rect tileRect = {(x * TILE_SIZE), (y * TILE_SIZE), TILE_SIZE, TILE_SIZE};
            switch (levelData[y][x]) {
                case 0:
                    break;
                case 1:
                    SDL_RenderCopy(renderer, tileTexture[1], nullptr, &tileRect);
                    break;
                case 2:
                    SDL_RenderCopy(renderer, tileTexture[2], nullptr, &tileRect);
                    break;
                case 3:
                    SDL_RenderCopy(renderer, fg, nullptr, &tileRect);
                    break;
                case 4:
                    SDL_SetRenderDrawColor(renderer, 0, 0, 160, 255);
                    SDL_RenderFillRect(renderer, &tileRect);
                case 5:
                    break;
                default:
                    break;
            }
        }
    }
    if (py.lives == 2) {
        lifeFlag[0] = 0;
    } else if (py.lives == 1) {
        lifeFlag[0] = lifeFlag[1] = 0;
    }
    for (int i = 0; i < 3; i++) {
        int X = (i + 22)*TILE_SIZE;
        int Y = 0;
        SDL_Texture* lt = lifeFlag[i] ? Life[6] : Life[7];
        SDL_Rect tRect = {X, Y, TILE_SIZE, TILE_SIZE};
        SDL_RenderCopy(renderer, lt, nullptr, &tRect);
    }
    SDL_Rect PlayerRect = {py.x, py.y, TILE_SIZE, TILE_SIZE};
    SDL_RenderCopy(renderer, playerTexture, nullptr, &PlayerRect);
    SDL_RenderPresent(renderer);
    SDL_Delay(8);
}

void GameEngine::Render() {
    SDL_RenderPresent(renderer);
}

int main(int argc, char** argv) {
    GameEngine game;
    game.Initialize("Game Engine", SCREEN_WIDTH, SCREEN_HEIGHT);
    game.Run();
    game.Shutdown();
    return 0;
}



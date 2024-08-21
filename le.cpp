#include <iostream>
#include <SDL2/SDL.h>
#include <sstream>
#include <vector>
#include <fstream>
using namespace std;
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TILE_SIZE = 32;
class LevelEditor {
public:
    LevelEditor();
    ~LevelEditor();
    void Run();
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    vector<vector<int>> levelData;
    bool isRunning;
    int selectedTile;
    void HandleInput();
    void Render();
    void SaveConfiguration();
    void loadConfig(const std::string &);
};
LevelEditor::LevelEditor() : window(nullptr), renderer(nullptr), isRunning(true), selectedTile(1) {
    levelData.resize(SCREEN_HEIGHT / TILE_SIZE, vector<int>(SCREEN_WIDTH / TILE_SIZE, 0));
}
LevelEditor::~LevelEditor() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
void LevelEditor::HandleInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            isRunning = false;
        } else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    isRunning = false;
                    break;
                case SDLK_0:
                    selectedTile = 0;
                    break;
                case SDLK_1:
                    selectedTile = 1;
                    break;
                case SDLK_2:
                    selectedTile = 2;
                    break;
                case SDLK_3:
                    selectedTile = 3;
                    break;
                case SDLK_4:
                    selectedTile = 4;
                    break;
                case SDLK_5:
                    selectedTile = 5;
                    break;
                case SDLK_s:
                    if (SDL_GetModState() & KMOD_CTRL) {
                        SaveConfiguration();
                    }
                    break;
            }
        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                int mouseX = event.button.x / TILE_SIZE;
                int mouseY = event.button.y / TILE_SIZE;

                if (mouseX >= 0 && mouseX < levelData[0].size() && mouseY >= 0 && mouseY < levelData.size()) {
                    levelData[mouseY][mouseX] = selectedTile;
                }
            }
        }
    }
}
void LevelEditor::Render() {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    for (int y = 0; y < levelData.size(); ++y) {
        for (int x = 0; x < levelData[y].size(); ++x) {
            int tileValue = levelData[y][x];
            SDL_Rect tileRect = {(x * TILE_SIZE), (y * TILE_SIZE), TILE_SIZE, TILE_SIZE};
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &tileRect);
            if (tileValue == 0) {
                continue;
            } else if (tileValue == 1) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_RenderFillRect(renderer, &tileRect);
            } else if (tileValue == 2) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                SDL_RenderFillRect(renderer, &tileRect);
            } else if (tileValue == 3) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
                SDL_RenderFillRect(renderer, &tileRect);
            } else if (tileValue == 4) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 150, 255);
                SDL_RenderFillRect(renderer, &tileRect);
            } else if (tileValue == 5) {
                SDL_SetRenderDrawColor(renderer,0,255,255,255);
                SDL_RenderFillRect(renderer,&tileRect);
            }
            // Add more cases for different tile values
        }
    }

    SDL_RenderPresent(renderer);
}
void LevelEditor::SaveConfiguration() {
    fstream outFile;
    outFile.open("level_config.txt", ios::out);
    if (!outFile.is_open()) {
        cerr << "Error: Could not open file for writing." << std::endl;
        return;
    }

    for (int y = 0; y < levelData.size(); ++y) {
        for (int x = 0; x < levelData[y].size(); ++x) {
            outFile << levelData[y][x] << " ";
        }
        outFile << "\n";
    }

    outFile.close();
    cout << "Configuration saved to level_config.txt" << std::endl;
}
void LevelEditor::loadConfig(const std::string &configFile) {
    ifstream inFile(configFile);
    if (!inFile.is_open()) {
        cerr << "Error: Could not open file for reading." << std::endl;
        return;
    }

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
}
void LevelEditor::Run() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        isRunning = false;
        return;
    }

    window = SDL_CreateWindow("Level Editor", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        isRunning = false;
        return;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        isRunning = false;
        return;
    }

    loadConfig("level_config.txt");

    while (isRunning) {
        HandleInput();
        Render();
    }
}
int main(int argc, char** argv) {
    LevelEditor levelEditor;
    levelEditor.Run();
    return 0;
}



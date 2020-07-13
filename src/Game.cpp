#include <iostream>
#include "./Constants.h"
#include "Game.h"
#include "./AssetManager.h"
#include "./Map.h"
#include "./Components/TransformComponent.h"
#include "./Components/SpriteComponent.h"
#include "./Components/KeyboardControlComponent.h"
#include "../lib/glm/glm.hpp"

EntityManager manager;
AssetManager* Game::assetManager = new AssetManager(&manager);
SDL_Renderer* Game::renderer;
SDL_Event Game::event;
Map* map;

Game::Game() {
    isRunning = false;
}

Game::~Game() {}

bool Game::IsRunning() const {
    return isRunning;
}

void Game::Initialize(int width, int height) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "Error initializing SDL." << std::endl;
        return;
    }
    window = SDL_CreateWindow(
        NULL,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_BORDERLESS
    );
    if(!window) {
        std::cerr << "Error creating SDL window." << std::endl;
        return;
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    if(!renderer) {
        std::cerr << "Error creating SDL renderer." << std::endl;
        return;
    }
    LoadLevel(0);

    isRunning = true;
    return;
}

void Game::LoadLevel(int levelNumber) {
    /* start including new assets to assetmanager list */
    assetManager->AddTexture("tank-image", std::string("./assets/images/tank-big-right.png").c_str());
    assetManager->AddTexture("chopper-image", std::string("./assets/images/chopper-spritesheet.png").c_str());
    assetManager->AddTexture("radar-image", std::string("./assets/images/radar.png").c_str());
    assetManager->AddTexture("jungle-tiletexture", std::string("./assets/tilemaps/jungle.png").c_str());

    map = new Map("jungle-tiletexture", 1, 32);
    map->LoadMap("./assets/tilemaps/jungle.map", 25, 20);

    Entity& chopperEntity(manager.AddEntity("chopper", constants::PLAYER_LAYER));
    chopperEntity.AddComponent<TransformComponent>(210, 106, 0, 0, 32, 32, 1);
    chopperEntity.AddComponent<SpriteComponent>("chopper-image", 2, 90, true, false);
    chopperEntity.AddComponent<KeyboardControlComponent>("up", "right", "down", "left", "shoot");

    /* start including entities and components to them */
    Entity& tankEntity(manager.AddEntity("tank", constants::PLAYER_LAYER));
    tankEntity.AddComponent<TransformComponent>(0,0,20,20,32,32,1);
    tankEntity.AddComponent<SpriteComponent>("tank-image");
    
    Entity& radarEntity(manager.AddEntity("Radar", constants::UI_LAYER));
    radarEntity.AddComponent<TransformComponent>(720, 15, 0, 0, 64, 64, 1);
    radarEntity.AddComponent<SpriteComponent>("radar-image", 8, 150, false, true);
}

void Game::ProcessInput() {
    SDL_PollEvent(&event);
    switch(event.type) {
        case SDL_QUIT: {
            isRunning = false;
            break;
        }
        case SDL_KEYDOWN: {
            if (event.key.keysym.sym == SDLK_ESCAPE ) {
                isRunning = false;
            }
        } 
        default: {
            break;
        }
    }  
}

void Game::Update() {

    int timeToWait = constants::FRAME_TARGET_TIME - (SDL_GetTicks() - ticksLastFrame);
    if (timeToWait > 0 && timeToWait <= constants::FRAME_TARGET_TIME) {
        SDL_Delay(timeToWait);
    }
    float deltaTime = (SDL_GetTicks() - ticksLastFrame) / 1000.0f;
    deltaTime = (deltaTime > 0.05f) ? 0.05 : deltaTime;

    ticksLastFrame = SDL_GetTicks();

    //todo: here we call manager update
    manager.Update(deltaTime);
}

void Game::Render() {
    SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
    SDL_RenderClear(renderer);
    
    //todo: we call manager render to render all entities
    if (manager.HasNoEntities()) {
        return;
    }
    manager.Render();

    SDL_RenderPresent(renderer);
}

void Game::Destroy() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
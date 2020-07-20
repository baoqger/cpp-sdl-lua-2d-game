#include <iostream>
#include "./Constants.h"
#include "Game.h"
#include "./AssetManager.h"
#include "./Map.h"
#include "./Components/TransformComponent.h"
#include "./Components/SpriteComponent.h"
#include "./Components/KeyboardControlComponent.h"
#include "./Components/ColliderComponent.h"
#include "./Components/TextLabelComponent.h"
#include "./Components/ProjectileEmitterComponent.h"
#include "../lib/glm/glm.hpp"

EntityManager manager;
AssetManager* Game::assetManager = new AssetManager(&manager);
SDL_Renderer* Game::renderer;
SDL_Event Game::event;
SDL_Rect Game::camera = {0, 0, constants::WINDOW_WIDTH, constants::WINDOW_HEIGHT};
Entity* mainPlayer = NULL;
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
    if (TTF_Init() != 0) {
        std::cerr << "Error initializing TTF." << std::endl;
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
    LoadLevel(1);

    isRunning = true;
    return;
}

void Game::LoadLevel(int levelNumber) {
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::os, sol::lib::math);

    std::string levelName = "Level" + std::to_string(levelNumber);
    lua.script_file("./assets/scripts/" + levelName + ".lua");

    sol::table levelData = lua[levelName];
    sol::table levelAssets = levelData["assets"];   
 
    unsigned int assetIndex = 0;
    while (true) {
        sol::optional<sol::table> existsAssetIndexNode = levelAssets[assetIndex]; 
        if (existsAssetIndexNode == sol::nullopt) {
            break;
        } else {
            sol::table asset = levelAssets[assetIndex];
            std::string assetType = asset["type"];
            if (assetType.compare("texture") == 0) {
                std::string assetId = asset["id"];
                std::string assetFile = asset["file"];
                assetManager->AddTexture(assetId, assetFile.c_str());
            } 
        }
        assetIndex++;
    }

    /*********************************************/
    /* LOADS MAP FROM LUA CONFIG FILE            */
    /*********************************************/
    sol::table levelMap = levelData["map"];
    std::string mapTextureId = levelMap["textureAssetId"];
    std::string mapFile = levelMap["file"];

    map = new Map(
        mapTextureId,
        static_cast<int>(levelMap["scale"]),
        static_cast<int>(levelMap["tileSize"])
    );

    map->LoadMap(
        mapFile,
        static_cast<int>(levelMap["mapSizeX"]),
        static_cast<int>(levelMap["mapSizeY"])
    );
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

    HandleCameraMovement();

    CheckCollisions();
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

void Game::HandleCameraMovement() {
    if (mainPlayer) {
        TransformComponent* mainPlayerTransform = mainPlayer->GetComponent<TransformComponent>();
        camera.x = mainPlayerTransform->position.x - (constants::WINDOW_WIDTH / 2);
        camera.y = mainPlayerTransform->position.y - (constants::WINDOW_HEIGHT / 2);

        camera.x = camera.x < 0 ? 0 : camera.x;
        camera.y = camera.y < 0 ? 0 : camera.y;
        camera.x = camera.x > camera.w ? camera.w : camera.x;
        camera.y = camera.y > camera.h ? camera.h : camera.y;
    }
}

void Game::CheckCollisions() {
    constants::CollisionType collisionType = manager.CheckCollisions();
    if (collisionType == constants::PLAYER_ENEMY_COLLISION) {
        // todo do something when collision is identified with an enemy
        ProcessGameOver();
    }
    if (collisionType == constants::PLAYER_PROJECTILE_COLLISION) {
        // todo do something when collision is identified with an enemy
        ProcessGameOver();
    }    
    if (collisionType == constants::PLAYER_LEVEL_COMPLETE_COLLISION) {
        ProcessNextLevel(1);
    }
}

void Game::ProcessGameOver() {
    isRunning = false;
}

void Game::ProcessNextLevel(int levelNumber) {
    isRunning = false;
}

void Game::Destroy() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
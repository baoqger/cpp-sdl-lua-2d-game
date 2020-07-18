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
    LoadLevel(0);

    isRunning = true;
    return;
}

Entity& player(manager.AddEntity("chopper", constants::PLAYER_LAYER));

void Game::LoadLevel(int levelNumber) {
    /* start including new assets to assetmanager list */
    assetManager->AddTexture("tank-image", std::string("./assets/images/tank-big-right.png").c_str());
    assetManager->AddTexture("chopper-image", std::string("./assets/images/chopper-spritesheet.png").c_str());
    assetManager->AddTexture("radar-image", std::string("./assets/images/radar.png").c_str());
    assetManager->AddTexture("jungle-tiletexture", std::string("./assets/tilemaps/jungle.png").c_str());
    assetManager->AddTexture("heliport-image", std::string("./assets/images/heliport.png").c_str());
    assetManager->AddTexture("projectile-image", std::string("./assets/images/bullet-enemy.png").c_str());
    assetManager->AddFont("charriot-font", std::string("./assets/fonts/charriot.ttf").c_str(), 14);

    map = new Map("jungle-tiletexture", 2, 32);
    map->LoadMap("./assets/tilemaps/jungle.map", 25, 20);


    player.AddComponent<TransformComponent>(240, 106, 0, 0, 32, 32, 1);
    player.AddComponent<SpriteComponent>("chopper-image", 2, 90, true, false);
    player.AddComponent<KeyboardControlComponent>("up", "right", "down", "left", "shoot");
    player.AddComponent<ColliderComponent>("PLAYER", 240, 106, 32, 32);

    /* start including entities and components to them */
    Entity& tankEntity(manager.AddEntity("tank", constants::PLAYER_LAYER));
    tankEntity.AddComponent<TransformComponent>(150,490,0,0,32,32,1);
    tankEntity.AddComponent<SpriteComponent>("tank-image");
    tankEntity.AddComponent<ColliderComponent>("ENEMY", 150, 490, 32, 32);

    Entity& projectile(manager.AddEntity("projectile", constants::PROJECTILE_LAYER));
    projectile.AddComponent<TransformComponent>(150 + 16, 490 + 16, 0, 0, 4, 4, 1);
    projectile.AddComponent<SpriteComponent>("projectile-image");
    projectile.AddComponent<ColliderComponent>("PROJECTILE", 150 + 16, 490 + 16, 4, 4);
    projectile.AddComponent<ProjectileEmitterComponent>(50, 270, 200, true);

    Entity& heliport(manager.AddEntity("Heliport", constants::OBSTACLE_LAYER)); 
    heliport.AddComponent<TransformComponent>(470, 420, 0, 0, 32, 32, 1);
    heliport.AddComponent<SpriteComponent>("heliport-image");
    heliport.AddComponent<ColliderComponent>("LEVEL_COMPLETE", 470, 420, 32, 32);
    
    Entity& radarEntity(manager.AddEntity("Radar", constants::UI_LAYER));
    radarEntity.AddComponent<TransformComponent>(720, 15, 0, 0, 64, 64, 1);
    radarEntity.AddComponent<SpriteComponent>("radar-image", 8, 150, false, true);

    Entity& labelLevelName(manager.AddEntity("LabelLevelName", constants::UI_LAYER));
    labelLevelName.AddComponent<TextLabelCompnent>(10, 10, "First Level...", "charriot-font", constants::WHITE_COLOR);
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
    TransformComponent* mainPlayerTransform = player.GetComponent<TransformComponent>();
    camera.x = mainPlayerTransform->position.x - (constants::WINDOW_WIDTH / 2);
    camera.y = mainPlayerTransform->position.y - (constants::WINDOW_HEIGHT / 2);

    camera.x = camera.x < 0 ? 0 : camera.x;
    camera.y = camera.y < 0 ? 0 : camera.y;
    camera.x = camera.x > camera.w ? camera.w : camera.x;
    camera.y = camera.y > camera.h ? camera.h : camera.y;
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
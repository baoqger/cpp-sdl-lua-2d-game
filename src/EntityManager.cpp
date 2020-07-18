#include "./EntityManager.h"
#include "./Collision.h"
#include "./Components/ColliderComponent.h"

void EntityManager::ClearData() {
    for (auto& entity: entities  ) {
        entity->Destroy();
    }
}

bool EntityManager::HasNoEntities() {
    return entities.size() == 0;
}

void EntityManager::Update(float deltaTime) {
    for (auto& entity: entities) {
        entity->Update(deltaTime);
    }
    DestroyInactiveEntities();
}

void EntityManager::DestroyInactiveEntities() {
    for (int i = 0; i < entities.size(); i++) {
        if (!entities[i]->IsActive()) {
            entities.erase(entities.begin() + i);
        }
    }
}

void EntityManager::Render() {
    for (int layerNumber = 0; layerNumber < constants::NUM_LAYERS; layerNumber++) {
        for (auto &entity: GetEntitiesByLayer(static_cast<constants::LayerType>(layerNumber))) {
            entity->Render();
        }
    }
}

std::vector<Entity*> EntityManager::GetEntitiesByLayer(constants::LayerType layer) const {
    std::vector<Entity*> selectedEntities;
    for (auto &entity: entities) {
        if (entity->layer == layer) {
            selectedEntities.emplace_back(entity);
        }
    }
    return selectedEntities;
}

constants::CollisionType EntityManager::CheckCollisions() const {
    for (auto &thisEntity: entities) {
        if (thisEntity->HasComponent<ColliderComponent>()) {
            ColliderComponent* thisCollider = thisEntity->GetComponent<ColliderComponent>();
            for (auto &thatEntity: entities) {
                if (
                    thatEntity->HasComponent<ColliderComponent>() &&
                    thisEntity->name.compare(thatEntity->name) != 0
                ) {
                    ColliderComponent* thatCollider = thatEntity->GetComponent<ColliderComponent>();
                    if (Collision::CheckRectangleCollision(thisCollider->collider, thatCollider->collider)) {
                        if (
                            thisCollider->colliderTag.compare("PLAYER") == 0 &&
                            thatCollider->colliderTag.compare("ENEMY") == 0
                        ) {
                            return constants::PLAYER_ENEMY_COLLISION;
                        }
                        if (
                            thisCollider->colliderTag.compare("PLAYER") == 0 &&
                            thatCollider->colliderTag.compare("PROJECTILE") == 0
                        ) {
                            return constants::PLAYER_PROJECTILE_COLLISION;
                        }
                        if (
                            thisCollider->colliderTag.compare("ENEMY") == 0 &&
                            thatCollider->colliderTag.compare("FRIENDLY_PROJECTILE") == 0                            
                        ) {
                            return constants::ENEMY_PROJECTILE_COLLISION;
                        }
                        if (
                            thisCollider->colliderTag.compare("PLAYER") == 0 &&
                            thatCollider->colliderTag.compare("LEVEL_COMPLETE") == 0                            
                        ) {
                            return constants::PLAYER_LEVEL_COMPLETE_COLLISION;
                        }                        
                    }
                }
            }
        }
        return constants::NO_COLLISION;
    }
}

// retired method
std::string EntityManager::CheckEntityCollisions(Entity &myEntity) const {
    ColliderComponent* myCollider = myEntity.GetComponent<ColliderComponent>();
    for (auto &entity: entities) {
        if (entity->name.compare(myEntity.name) != 0 && entity->name.compare("Tile") != 0 ) {
            if (entity->HasComponent<ColliderComponent>()) {
                ColliderComponent* otherCollider = entity->GetComponent<ColliderComponent>();
                if (Collision::CheckRectangleCollision(myCollider->collider, otherCollider->collider)) {
                    return otherCollider->colliderTag;
                }
            } 
        }

    }
    return std::string();
}

Entity& EntityManager::AddEntity(std::string entityName, constants::LayerType layer) {
    Entity* newEntity = new Entity(*this, entityName, layer);
    entities.emplace_back(newEntity);
    return *newEntity;
}

std::vector<Entity*> EntityManager::GetEntities() const {
    return entities;
}

unsigned int EntityManager::GetEntityCount() {
    return entities.size();
}
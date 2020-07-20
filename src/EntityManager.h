#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include "./Entity.h"
#include "./Component.h"
#include "./Constants.h"

class EntityManager {
    private:
        std::vector<Entity*> entities;
    public:
        void ClearData();
        void Update(float deltaTime);
        void Render();
        bool HasNoEntities();
        Entity& AddEntity(std::string entityName, constants::LayerType layer);
        std::vector<Entity*> GetEntities() const;
        std::vector<Entity*> GetEntitiesByLayer(constants::LayerType layer) const;
        Entity* GetEntityByName(std::string entityName) const;
        unsigned int GetEntityCount();
        std::string CheckEntityCollisions(Entity& entity) const; // retired method
        constants::CollisionType CheckCollisions() const;
        void DestroyInactiveEntities();
};
#endif
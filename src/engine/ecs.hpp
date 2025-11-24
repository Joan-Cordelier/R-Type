/*
** EPITECH PROJECT, 2025
** local
** File description:
** ECS
*/

#ifndef ECS_HPP
    #define ECS_HPP

    #include <cstdint>
    #include <vector>
    #include <memory>
    #include <typeindex>
    #include <unordered_map>

    using Entity = uint32_t;
    constexpr Entity INVALID_ENTITY = 0;

    struct Position { float x, y; };
    struct Velocity { float vx, vy; };
    struct Stats { int hp; };

    struct IComponentArray {
        virtual ~IComponentArray() = default;
        virtual void remove(Entity e) = 0;
        virtual bool has(Entity e) const = 0;
        virtual std::vector<Entity> entities() const = 0;
    };

    template<typename T>
    class ComponentArray : public IComponentArray {
    public:
        void insert(Entity e, T component);
        void remove(Entity e) override;
        bool has(Entity e) const override;
        T& get(Entity e);
        std::vector<Entity> entities() const override;

    private:
        std::unordered_map<Entity, T> data;
    };

    class EntityManager {
    public:
        EntityManager();

        Entity create();
        void destroy(Entity e);

    private:
        Entity nextId;
        std::vector<Entity> freeIds;
    };

    class Registry {
    public:
        Entity createEntity();
        void destroyEntity(Entity e);

        template<typename T>
        std::shared_ptr<ComponentArray<T>> ensure();

        template<typename T, typename... Args>
        void addComponent(Entity e, Args&&... args);

        template<typename T>
        bool hasComponent(Entity e) const;

        template<typename T>
        T& getComponent(Entity e);

    private:
        EntityManager em;
        std::unordered_map<std::type_index, std::shared_ptr<IComponentArray>> components;

        template<typename T>
        void collectEntities(std::vector<std::vector<Entity>>& lists) const;
    };
#endif

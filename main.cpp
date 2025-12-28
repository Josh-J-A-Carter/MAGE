#include <iostream>
#include <cstdlib>
#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <typeinfo>
#include <cassert>
#include <string>
#include <bitset>



using Identifier = std::size_t;
using Index = std::size_t;

struct Entity {
	Identifier id;
	Index index;
	
	static constexpr Identifier invalid { std::numeric_limits<Identifier>::max() };
};

template<std::size_t Size>
using ComponentMask = std::bitset<Size>;


using Typeid = std::size_t;

template<typename Type>
Typeid type() { return typeid(Type).hash_code(); }


struct IComponentArray {};

template<typename Component, std::size_t MaxEntities>
struct ComponentArray : IComponentArray {

private:
	std::array<Component, MaxEntities> components {};

public:
	Component& operator[] (Index index) {
		return components[index];
	}
};



template<std::size_t MaxEntities, std::size_t MaxComponents>
struct World {

private:

	// Entities
	std::vector<Identifier> entity_identifiers = std::vector<Identifier>( MaxEntities, Entity::invalid );
	std::vector<ComponentMask<MaxComponents>> entity_masks = std::vector<ComponentMask<MaxComponents>>( MaxEntities );
	Identifier next_entity{};

	// Components
	std::unordered_map<Typeid, std::shared_ptr<IComponentArray>> component_arrays{};
	std::unordered_map<Typeid, ComponentMask<MaxComponents>> component_bitmasks{};
	std::size_t next_component{ 0 };

public:

	World() {
		component_arrays.reserve(MaxComponents);
		component_bitmasks.reserve(MaxComponents);
	}

	Entity create_entity() {
		for (std::size_t i{ 0 }; i < MaxEntities; i += 1) {
			if (entity_identifiers[i] != Entity::invalid) continue;

			Entity entity{ .id = next_entity, .index = i };

			entity_identifiers[i] = next_entity;
			entity_masks[i] = 0;

			next_entity += 1;

			return entity;
		}

		assert(false && "Attempted to create more than maximum number of allowed entities.");
		
		return {};
	}

	bool is_alive(Entity entity) {
		if (entity.index >= MaxEntities) return false;
		if (entity.id == Entity::invalid) return false;

		// If the entity is dead, entities[entity.index] was set to Entity::invalid ( != to entity.id )
		// Since then, entities[entity.index] may have been updated to a new ID ( != entity.id )
		return entity_identifiers[entity.index] == entity.id;
	}

	const std::vector<Identifier>& entities() {
		return entity_identifiers;
	}

	bool match(Entity entity, ComponentMask<MaxComponents> mask) {
		return (entity_masks[entity.index] & mask) == mask;
	}

	bool match(Index index, ComponentMask<MaxComponents> mask) {
		return (entity_masks[index] & mask) == mask;
	}

	void destroy_entity(Entity entity) {
		assert(entity.index < MaxEntities && "Attempted to destroy entity beyond the bounds of the entity array.");
		assert(entity.id != Entity::invalid && "Attempted to destroy invalid entity.");

		entity_identifiers[entity.index] = Entity::invalid;
		entity_masks[entity.index] = 0;
	}

	template<typename Component>
	void register_component() {
		assert(!component_arrays.contains(type<Component>()) && "Component registered twice.");
		assert(next_component < MaxComponents && "Registered more than maximum number of allowed components.");

		component_arrays.insert({ type<Component>(), std::make_shared<ComponentArray<Component, MaxEntities>>() });
		component_bitmasks.insert({ type<Component>(), { std::size_t { 1 } << next_component } });

		next_component += 1;
	}

	template<typename Component>
	ComponentMask<MaxComponents> mask() {
		assert(component_bitmasks.contains(type<Component>()) && "Tried to look up bitmask of unregistered component.");

		return component_bitmasks[type<Component>()];
	}

	template<typename Component>
	ComponentArray<Component, MaxEntities>& components() {
		assert(component_arrays.contains(type<Component>()) && "Tried to look up array of unregistered component.");

		return *std::static_pointer_cast<ComponentArray<Component, MaxEntities>>(component_arrays[type<Component>()]);
	}

	template<typename Component>
	void add_component(Entity entity, Component component) {
		assert(entity.index < MaxEntities && "Tried to add component to entity beyond the bounds of component array.");
		assert(entity.id != Entity::invalid && "Tried to add component to invalid entity.");
		assert(((entity_masks[entity.index] & mask<Component>()) != mask<Component>()) && "Tried to add same component type to entity twice.");

		components<Component>()[entity.index] = component;

		entity_masks[entity.index] |= component_bitmasks[type<Component>()];
	}
};


struct Transform {};
struct Name { std::string name; };
struct Rating { int coolness; };


template<std::size_t MaxEntities, std::size_t MaxComponents>
void my_system(World<MaxEntities, MaxComponents>& world) {
	ComponentMask<MaxComponents> mask = world.mask<Name>() | world.mask<Rating>();

	ComponentArray<Name, MaxEntities>& names = world.components<Name>();
	ComponentArray<Rating, MaxEntities>& ratings = world.components<Rating>();

	std::cout << "running the system!" << std::endl;

	for (Index index{ 0 }; index < world.entities().size(); index += 1) {
		if (world.match(index, mask) == false) continue;

		std::cout << "entity at index " << index << " named " << names[index].name
				  << " has rating " << ratings[index].coolness << std::endl;
	}

	std::cout << std::endl;
}


int main() {
	std::cout << "Hello, VS Code" << std::endl;

	constexpr std::size_t MAX_ENTITIES { 1000 };
	constexpr std::size_t MAX_COMPONENTS { 32 };

	World<MAX_ENTITIES, MAX_COMPONENTS> world {};

	world.register_component<Transform>();
	world.register_component<Name>();
	world.register_component<Rating>();

	Entity e1 = world.create_entity();
	world.add_component<Name>(e1, { "Awesome Guy" });
	world.add_component<Rating>(e1, { 1000 });

	Entity e2 = world.create_entity();
	world.add_component<Name>(e2, { "Cringe Guy" });
	world.add_component<Rating>(e2, { -1000 });

	Entity e3 = world.create_entity();
	world.add_component<Name>(e3, { "Mediocre Guy" });
	world.add_component<Rating>(e3, { 0 });

	Entity e4 = world.create_entity();
	world.add_component<Name>(e4, { "Purple Guy" });

	my_system(world);

	std::cout << "deleted entity 'Awesome Guy'" << std::endl;
	world.destroy_entity(e1);

	my_system(world);

	return EXIT_SUCCESS;
}
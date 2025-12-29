#ifndef ECS_H
#define ECS_H

#include <cstdlib>
#include <vector>
#include <array>
#include <unordered_map>
#include <typeinfo>
#include <cassert>
#include <string>
#include <bitset>
#include <memory>


using Identifier = std::size_t;
using Index = std::size_t;

struct Entity {
	Identifier id;
	Index index;

	static constexpr Identifier invalid{ std::numeric_limits<Identifier>::max() };
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
	std::array<Component, MaxEntities> components{};

public:
	Component& operator[] (Index index) {
		return components[index];
	}
};



template<std::size_t MaxEntities, std::size_t MaxComponents>
struct World {

private:

	// Entities
	std::vector<Identifier> entity_identifiers = std::vector<Identifier>(MaxEntities, Entity::invalid);
	std::vector<ComponentMask<MaxComponents>> entity_masks = std::vector<ComponentMask<MaxComponents>>(MaxEntities);
	Identifier next_entity { 0 };
	Index last_entity_index { 0 };

	// Components
	std::unordered_map<Typeid, std::shared_ptr<IComponentArray>> component_arrays {};
	std::unordered_map<Typeid, ComponentMask<MaxComponents>> component_bitmasks {};
	std::size_t next_component { 0 };

	void update_last_entity_index() {
		for (Index i { MaxEntities - 1 } ; i > 0 ; i -= 1) {
			if (entity_identifiers[i] == Entity::invalid) continue;

			last_entity_index = i + 1;
			break;
		}
	}

public:

	World() {
		component_arrays.reserve(MaxComponents);
		component_bitmasks.reserve(MaxComponents);
	}

	Entity create_entity() {
		for (std::size_t i { 0 } ; i < MaxEntities ; i += 1) {
			if (entity_identifiers[i] != Entity::invalid) continue;

			Entity entity{ .id = next_entity, .index = i };

			entity_identifiers[i] = next_entity;
			entity_masks[i] = 0;

			next_entity += 1;
			update_last_entity_index();

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

	Index last() {
		return last_entity_index;
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

		update_last_entity_index();
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

#endif
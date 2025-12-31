#ifndef ECS_H
#define ECS_H

#include "sparse_set.h"

#include <cstdlib>
#include <vector>
#include <array>
#include <unordered_map>
#include <typeinfo>
#include <cassert>
#include <string>
#include <bitset>
#include <memory>
#include <tuple>


using Identifier = std::size_t;
using Index = std::size_t;

struct Entity {
	Identifier id;
	Index index;

	static constexpr Identifier invalid { std::numeric_limits<Identifier>::max() };
};

template<std::size_t Size>
using ComponentMask = std::bitset<Size>;


// According to the C++ standard, it *is* legal to cast a function to void*,
// but calling the resulting pointer is undefined behaviour.
using System = void*;


using Typeid = std::size_t;

template<typename Type>
Typeid type() { return typeid(Type).hash_code(); }



template<std::size_t MaxEntities, std::size_t MaxComponents>
struct ECS {

private:

	// Entities
	std::vector<Identifier> entity_identifiers = std::vector<Identifier>(MaxEntities, Entity::invalid);
	std::vector<ComponentMask<MaxComponents>> entity_masks = std::vector<ComponentMask<MaxComponents>>(MaxEntities);
	Identifier next_entity { 0 };

	// Components
	std::unordered_map<Typeid, std::shared_ptr<ISparseSet>> component_arrays {};
	std::unordered_map<Typeid, ComponentMask<MaxComponents>> component_masks {};
	std::size_t next_component { 0 };

	// Systems
	std::unordered_map<System, std::shared_ptr<SparseSet<Entity>>> system_entities {};
	std::unordered_map<System, ComponentMask<MaxComponents>> system_masks {};

public:

	ECS() {
		component_arrays.reserve(MaxComponents);
		component_masks.reserve(MaxComponents);
	}

	//
	// Entity functions
	//

	Entity create_entity() {
		for (std::size_t i { 0 } ; i < MaxEntities ; i += 1) {
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

	const std::vector<Identifier>& entities_all() {
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

		// Remove the entity from system entity lists
		for (auto& system_pair : system_entities) {
			auto& system_entities = *system_pair.second;
			system_entities.remove_entry(entity.index);
		}

		// Remove the entity from component arrays
		for (auto& component_pair : component_arrays) {
			auto& component_array = *component_pair.second;
			component_array.remove_entry_type_erased(entity.index);
		}
	}

	//
	// Component functions
	//

	template<typename Component>
	void register_component() {
		assert(!component_arrays.contains(type<Component>()) && "Component registered twice.");
		assert(next_component < MaxComponents && "Registered more than maximum number of allowed components.");

		Typeid t { type<Component>() };
		component_arrays[t] = std::make_shared<SparseSet<Component>>(MaxEntities);
		component_masks[t] = { std::size_t { 1 } << next_component };

		next_component += 1;
	}

	template<typename LastComponent>
	ComponentMask<MaxComponents> mask() {
		assert(component_masks.contains(type<LastComponent>()) && "Tried to look up bitmask of unregistered component.");

		return component_masks[type<LastComponent>()];
	}

	template<typename FirstComponent, typename SecondComponent, typename ...RemainingComponents>
	ComponentMask<MaxComponents> mask() {
		assert(component_masks.contains(type<FirstComponent>()) && "Tried to look up bitmask of unregistered component.");

		return component_masks[type<FirstComponent>()] | mask<SecondComponent, RemainingComponents...>();
	}

	template<typename LastComponent>
	auto components() {
		assert(component_arrays.contains(type<LastComponent>()) && "Tried to look up array of unregistered component.");

		std::tuple<const SparseSet<LastComponent>&> this_ref { 
			*std::static_pointer_cast<SparseSet<LastComponent>>(component_arrays[type<LastComponent>()])
		};

		return this_ref;
	}

	template<typename FirstComponent, typename SecondComponent, typename ...RemainingComponents>
	auto components() {
		assert(component_arrays.contains(type<FirstComponent>()) && "Tried to look up array of unregistered component.");
		
		std::tuple<const SparseSet<FirstComponent>&> this_ref {
			*std::static_pointer_cast<SparseSet<FirstComponent>>(component_arrays[type<FirstComponent>()])
		};

		return std::tuple_cat( this_ref, components<SecondComponent, RemainingComponents...>() );
	}

private:
	template<typename Component>
	SparseSet<Component>& components_internal() {
		return *std::static_pointer_cast<SparseSet<Component>>(component_arrays[type<Component>()]);
	}

public:

	//
	// System functions
	//

	template<typename ...ListeningComponents>
	void register_system(System system) {
		assert(!system_masks.contains(system) && "Attempted to register the same system twice.");

		system_entities[system] = std::make_shared<SparseSet<Entity>>(MaxEntities);
		system_masks[system] = mask<ListeningComponents...>();

		auto& system_entities = *this->system_entities[system];
		auto system_mask { system_masks[system] };

		// Add any existing entities to the system's list
		for (Index index { 0 } ; index < MaxEntities ; index += 1) {
			if ((entity_masks[index] & system_mask) != system_mask) continue;

			system_entities.add_entry(index, { .id = entity_identifiers[index], .index = index });
		}
	}

	const SparseSet<Entity>& entities(System system) {
		assert(system_masks.contains(system) && "Attempted to access entities for an unregistered system.");

		return *system_entities[system];
	}

	//
	// Functions that coordinate entities, components, and systems
	//

	template<typename Component>
	void add_component(Entity entity, Component component) {
		assert(entity.index < MaxEntities && "Tried to add component to entity beyond the bounds of component array.");
		assert(entity.id != Entity::invalid && "Tried to add component to invalid entity.");
		assert(((entity_masks[entity.index] & mask<Component>()) != mask<Component>()) && "Tried to add same component type to entity twice.");

		ComponentMask<MaxComponents> comp_mask { component_masks[type<Component>()] };
		ComponentMask<MaxComponents> entity_mask { entity_masks[entity.index] };

		for (auto& s : system_masks) {
			ComponentMask<MaxComponents> sys_mask { s.second };
			if ((sys_mask & (entity_mask | comp_mask)) != sys_mask) continue;
		
			(*system_entities[s.first]).add_entry(entity.index, entity);
		}

		components_internal<Component>().add_entry(entity.index, component);
		entity_masks[entity.index] |= comp_mask;
	}

	template<typename Component>
	void remove_component(Entity entity) {
		assert(entity.index < MaxEntities && "Tried to remove component from entity beyond the bounds of component array.");
		assert(entity.id != Entity::invalid && "Tried to remove component from invalid entity.");
		assert(((entity_masks[entity.index] & mask<Component>()) == mask<Component>()) && "Tried to remove non-existent component from entity.");

		ComponentMask<MaxComponents> comp_mask { component_masks[type<Component>()] };
		ComponentMask<MaxComponents> entity_mask { entity_masks[entity.index] };

		for (auto& s : system_masks) {
			ComponentMask<MaxComponents> sys_mask { s.second };
			if ((sys_mask & comp_mask) != comp_mask) continue;

			(*system_entities[s.first]).remove_entry(entity.index);
		}

		components_internal<Component>().remove_entry(entity.index);
		entity_masks[entity.index] = entity_mask & ~comp_mask;
	}
};

#endif
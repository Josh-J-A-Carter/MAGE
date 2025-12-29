#include "ecs.h"

#include <iostream>


struct Transform {};
struct Name { std::string name; };
struct Rating { int coolness; };


template<std::size_t MaxEntities, std::size_t MaxComponents>
void my_system(World<MaxEntities, MaxComponents>& world) {
	ComponentMask<MaxComponents> mask = world.mask<Name>() | world.mask<Rating>();

	ComponentArray<Name, MaxEntities>& names = world.components<Name>();
	ComponentArray<Rating, MaxEntities>& ratings = world.components<Rating>();

	std::cout << "running the system!" << std::endl;

	for (Index index { 0 } ; index < world.last() ; index += 1) {
		std::cout << "idx: " << index << std::endl;
		if (world.match(index, mask) == false) continue;

		std::cout << "entity at index " << index << " named " << names[index].name
				  << " has rating " << ratings[index].coolness << std::endl;
	}

	std::cout << std::endl;
}


int main() {
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
	world.destroy_entity(e4);

	my_system(world);

	return EXIT_SUCCESS;
}
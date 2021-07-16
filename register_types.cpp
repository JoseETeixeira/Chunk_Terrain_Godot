/* register_types.cpp */

#include "register_types.h"

#include "core/class_db.h"
#include "terrain/chunk_terrain.h"
#include "generator/chunk_generator.h"

void register_chunk_terrain_types() {
    ClassDB::register_class<ChunkGenerator>();
	ClassDB::register_class<ChunkTerrain>();
}

void unregister_chunk_terrain_types() {
   // Nothing to do here in this example.
}

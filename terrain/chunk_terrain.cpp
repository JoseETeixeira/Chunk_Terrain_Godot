/* chunk_terrain.cpp */

#include "chunk_terrain.h"




void ChunkTerrain::_bind_methods() {

	ADD_PROPERTY(PropertyInfo(Variant::INT, "x"), "set_x", "get_x");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "z"), "set_z", "get_z");
}

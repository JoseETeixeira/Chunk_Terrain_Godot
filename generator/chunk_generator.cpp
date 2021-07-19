#include "chunk_generator.h"
#include "../../scene/resources/primitive_meshes.h"
#include "../../core/math/vector2.h"
#include "../../core/math/vector3.h"
#include "../../scene/resources/mesh.h"
#include <core/core_string_names.h>
#include "../terrain/chunk_terrain.h"

ChunkGenerator::ChunkGenerator(){

}

ChunkGenerator::ChunkGenerator( int x, int z){
	_x = x;
	_z = z;

}

void ChunkGenerator::_init( int x, int z, int chunk_size,Ref<OpenSimplexNoise> noise){
	_x = x;
	_z = z;
	_chunk_size = chunk_size;
	_noise = noise;

}


void ChunkGenerator::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY:
			generate_chunk();
			break;


		case NOTIFICATION_PARENTED:
			_parent = Object::cast_to<ChunkTerrain>(get_parent());
			if (_parent != nullptr) {
				set_surface_material(_parent->get_surface_material());
			}
			break;

		case NOTIFICATION_UNPARENTED:
			break;
	}
}
ChunkGenerator::~ChunkGenerator() {
}

void ChunkGenerator::set_noise(Ref<OpenSimplexNoise> noise) {
	if (_noise == noise) {
		return;
	}
	_noise = noise;

}


Ref<OpenSimplexNoise> ChunkGenerator::get_noise()  {
	return _noise;
}

void ChunkGenerator::set_surface_material(Ref<ShaderMaterial> surface_material) {
	if (_surface_material == surface_material) {
		return;
	}
	_surface_material = surface_material;

}



Ref<ShaderMaterial> ChunkGenerator::get_surface_material()  {
	return _surface_material;
}


void ChunkGenerator::set_x(int x){
    _x = x;
}

void ChunkGenerator::set_z(int z){
    _z = z;
}

void ChunkGenerator::set_chunk_size(int chunk_size){
    _chunk_size = chunk_size;
}

void ChunkGenerator::generate_chunk(){

	PlaneMesh *plane_mesh = memnew(PlaneMesh());
	plane_mesh->set_size(Vector2(_chunk_size, _chunk_size));
	plane_mesh->set_subdivide_depth(_chunk_size * 0.5);
	plane_mesh->set_subdivide_width( _chunk_size * 0.5);
	plane_mesh->set_material(_parent->get_surface_material());
	//print_line("-------- GENERATED PLANE MESH----------");
	_data_tool = memnew(MeshDataTool());
	_surface_tool = memnew(SurfaceTool());
	_surface_tool->create_from(plane_mesh, 0);
	Ref<ArrayMesh> array_plane = _surface_tool->commit();
	//print_line("-------- GENERATED ARRAY_PLANE----------");


	Chunk *chunk = memnew(Chunk());
	Error error = _data_tool->create_from_surface(array_plane, 0);

	for (int it = 0; it < _data_tool->get_vertex_count(); it++) {
		Vector3 vertex = _data_tool->get_vertex(it);
		vertex.y =  Math::ceil(_noise->get_noise_3d(vertex.x + _x, vertex.y, vertex.z + _z) * 400);
		_data_tool->set_vertex(it,vertex);
		chunk->grid_positions.push_back(Vector3i(vertex.x,vertex.y,vertex.z));
	}

	for (int it = 0; it < array_plane->get_surface_count(); it++) {
		array_plane->surface_remove(it);
	}


	_data_tool->commit_to_surface(array_plane);
	_surface_tool->begin(Mesh::PrimitiveType::PRIMITIVE_TRIANGLES);
	_surface_tool->create_from(array_plane, 0);
	_surface_tool->generate_normals();
	chunk->mesh_instance = memnew(MeshInstance());
	chunk->mesh_instance->set_mesh(_surface_tool->commit());
	chunk->mesh_instance->create_trimesh_collision();
	_chunks.push_back(chunk);
	add_child(chunk->mesh_instance);

}

int ChunkGenerator::get_x(){
	return _x;
}
int ChunkGenerator::get_z(){
	return _z;
}

void ChunkGenerator::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_noise", "noise"), &ChunkGenerator::set_noise);
	ClassDB::bind_method(D_METHOD("get_noise"), &ChunkGenerator::get_noise);

	ClassDB::bind_method(D_METHOD("get_x"), &ChunkGenerator::get_x);
	ClassDB::bind_method(D_METHOD("get_z"), &ChunkGenerator::get_z);

	ClassDB::bind_method(D_METHOD("set_x", "x"), &ChunkGenerator::set_x);
	ClassDB::bind_method(D_METHOD("set_z", "z"), &ChunkGenerator::set_z);
	ClassDB::bind_method(D_METHOD("set_chunk_size", "chunk_size"), &ChunkGenerator::set_chunk_size);

	ClassDB::bind_method(D_METHOD("_init","x","z","chunk_size","noise"), &ChunkGenerator::_init);

	ClassDB::bind_method(D_METHOD("set_surface_material", "surface_material"), &ChunkGenerator::set_surface_material);
	ClassDB::bind_method(D_METHOD("get_surface_material"), &ChunkGenerator::get_surface_material);

	ClassDB::bind_method(D_METHOD("generate_chunk"), &ChunkGenerator::generate_chunk);


}

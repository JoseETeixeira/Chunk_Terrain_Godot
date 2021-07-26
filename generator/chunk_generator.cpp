/*

Copyright (c) 2021 José Eduardo da Silva Teixeira Junior

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "chunk_generator.h"
#include "../../scene/resources/primitive_meshes.h"
#include "../../core/math/vector2.h"
#include "../../core/math/vector3.h"
#include "../../scene/resources/mesh.h"
#include "../../scene/resources/multimesh.h"
#include <core/core_string_names.h>
#include "../terrain/chunk_terrain.h"

ChunkGenerator::ChunkGenerator(){
	pool = ThreadPool::get_singleton();
}

ChunkGenerator::ChunkGenerator( int x, int z){
	set_x(x);
	set_z(z);
	pool = ThreadPool::get_singleton();
}

void ChunkGenerator::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY:
			if(_parent != nullptr){
				if(_parent->get_should_generate_water()){
					pool->create_execute_job(this, "generate_water");
				}

				pool->create_execute_job(this, "generate_chunk");
				if(_parent->get_should_generate_grass()){
					pool->create_execute_job(this, "generate_grass");
				}
			}
			break;


		case NOTIFICATION_PARENTED:
			_parent = Object::cast_to<ChunkTerrain>(get_parent());
			if (_parent != nullptr) {
                set_chunk_size(_parent->get_chunk_size());
				set_noise(_parent->get_noise());
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
	mtx.lock();
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


	Error error = _data_tool->create_from_surface(array_plane, 0);

	ERR_FAIL_COND(error != OK);

	for (int it = 0; it < _data_tool->get_vertex_count(); it++) {
		Vector3 vertex = _data_tool->get_vertex(it);
		vertex.y =  Math::ceil(_noise->get_noise_3d(vertex.x + _x, vertex.y, vertex.z + _z) * 400);
		_data_tool->set_vertex(it,vertex);
	}

	for (int it = 0; it < array_plane->get_surface_count(); it++) {
		array_plane->surface_remove(it);
	}


	_data_tool->commit_to_surface(array_plane);
	_surface_tool->begin(Mesh::PrimitiveType::PRIMITIVE_TRIANGLES);
	_surface_tool->create_from(array_plane, 0);
	_surface_tool->generate_normals();
	mesh_instance = memnew(MeshInstance());
	mesh_instance->set_mesh(_surface_tool->commit());
	mesh_instance->create_trimesh_collision();
	add_child(mesh_instance);
	mtx.unlock();
}

void ChunkGenerator::generate_water(){
	mtx.lock();
	PlaneMesh *plane_mesh = memnew(PlaneMesh());
	plane_mesh->set_size(Vector2(_chunk_size, _chunk_size));

	plane_mesh->set_material(_parent->get_water_material());

	water_mesh = memnew(MeshInstance());
	water_mesh->set_mesh(plane_mesh);
	water_mesh->set_translation(water_mesh->get_translation()+Vector3(0,0.5,0));

	add_child(water_mesh);
	mtx.unlock();


}

void ChunkGenerator::generate_grass(){
	mtx.lock();
	MultiMesh *_multimesh = memnew(MultiMesh());
	_multimesh->set_mesh(_parent->get_grass_mesh());
	_multimesh->set_transform_format(MultiMesh::TRANSFORM_3D);
	_multimesh->set_instance_count(_chunk_size*_chunk_size);
	_multimesh->set_visible_instance_count(_chunk_size*_chunk_size*0.8);
	grass_multimesh = memnew(MultiMeshInstance());
	for (int x=0; x < _chunk_size; x++){
		for(int y=0; y< _chunk_size; y++){
			if(_data_tool->get_vertex_count()> x*y){
				Vector3 vertex = _data_tool->get_vertex(x*y);
				if (vertex.y > 0){
					_multimesh->set_instance_transform(x*_chunk_size+y,Transform(Basis(), Vector3(vertex)));
				}else{
					_multimesh->set_instance_transform(x*_chunk_size+y,Transform(Basis(), Vector3(vertex.x,-400,vertex.y)));
				}
			}


		}
	}

	grass_multimesh->set_multimesh(_multimesh);
	grass_multimesh->set_material_override(_parent->get_grass_material());

	add_child(grass_multimesh);
	mtx.unlock();


}

int ChunkGenerator::get_x(){
	return _x;
}

int ChunkGenerator::get_z(){
	return _z;
}

void ChunkGenerator::set_should_remove(bool should_remove){
	_should_remove = should_remove;
}


bool ChunkGenerator::get_should_remove(){
	return _should_remove;
}

void ChunkGenerator::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_noise", "noise"), &ChunkGenerator::set_noise);
	ClassDB::bind_method(D_METHOD("get_noise"), &ChunkGenerator::get_noise);


	ClassDB::bind_method(D_METHOD("set_x", "x"), &ChunkGenerator::set_x);
	ClassDB::bind_method(D_METHOD("get_x"), &ChunkGenerator::get_x);

	ClassDB::bind_method(D_METHOD("set_z", "z"), &ChunkGenerator::set_z);
	ClassDB::bind_method(D_METHOD("get_z"), &ChunkGenerator::get_z);

	ClassDB::bind_method(D_METHOD("set_should_remove", "should_remove"), &ChunkGenerator::set_should_remove);
	ClassDB::bind_method(D_METHOD("get_should_remove"), &ChunkGenerator::get_should_remove);

	ClassDB::bind_method(D_METHOD("set_chunk_size", "chunk_size"), &ChunkGenerator::set_chunk_size);

	ClassDB::bind_method(D_METHOD("set_surface_material", "surface_material"), &ChunkGenerator::set_surface_material);
	ClassDB::bind_method(D_METHOD("get_surface_material"), &ChunkGenerator::get_surface_material);

	ClassDB::bind_method(D_METHOD("generate_chunk"), &ChunkGenerator::generate_chunk);
	ClassDB::bind_method(D_METHOD("generate_water"), &ChunkGenerator::generate_water);
	ClassDB::bind_method(D_METHOD("generate_grass"), &ChunkGenerator::generate_grass);


}

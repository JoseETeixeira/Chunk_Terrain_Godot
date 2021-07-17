/* chunk_terrain.cpp */

#include "chunk_terrain.h"
#include "../generator/chunk_generator.h"
#include <core/core_string_names.h>
#include <core/array.h>

ChunkTerrain::ChunkTerrain(){
	set_x(0);
	set_z(0);
	set_chunk_size(0);
	set_chunk_amount(0);
	this->_thread = memnew(Thread());
}

ChunkTerrain::~ChunkTerrain(){
}

void ChunkTerrain::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE :

			break;
		case NOTIFICATION_EXIT_TREE:
			//memdelete(_generator);
			break;
		case NOTIFICATION_PROCESS:
			_process(get_process_delta_time());
			break;

	}
}

void ChunkTerrain::set_x(uint32_t x){
    _x = x;
}

void ChunkTerrain::set_z(uint32_t z){
    _z = z;
}

uint32_t ChunkTerrain::get_x(){
	return _x;
}

uint32_t ChunkTerrain::get_z(){
	return _z;
}

void ChunkTerrain::set_chunk_size(int size){
    _chunk_size = size;
}

int ChunkTerrain::get_chunk_size(){
	return _chunk_size;
}

void ChunkTerrain::set_chunk_amount(int amount){
    _chunk_amount = amount;
}

int ChunkTerrain::get_chunk_amount(){
	return _chunk_amount;
}


void ChunkTerrain::_process(float delta){

}

void ChunkTerrain::set_noise(Ref<OpenSimplexNoise> noise) {
	if (_noise == noise) {
		return;
	}
	_noise = noise;
	if(_generator!=nullptr){
		_generator->set_noise(_noise);
	}

}


Ref<OpenSimplexNoise> ChunkTerrain::get_noise() {
	return _noise;
}
void ChunkTerrain::set_surface_material(Ref<ShaderMaterial> surface_material) {
	if (_surface_material == surface_material) {
		return;
	}
	_surface_material->disconnect(CoreStringNames::get_singleton()->changed, this, "_on_surface_material_changed");
	_surface_material = surface_material;
	_surface_material->connect(CoreStringNames::get_singleton()->changed, this, "_on_surface_material_changed");

}

void ChunkTerrain::_on_surface_material_changed() {
	ERR_FAIL_COND(_surface_material.is_null());
	print_line("-------- SURFACE MATERIAL CHANGED ----------");
	if(_generator!=nullptr){
		_generator->set_surface_material(_surface_material);
	}
    //TODO: CLEAR CHUNKS AND REGENERATE
}

Ref<ShaderMaterial> ChunkTerrain::get_surface_material() {
	return _surface_material;
}

void ChunkTerrain::set_generator(ChunkGenerator *generator){
	if (_generator == generator){
		return;
	}else{
		_generator = generator;
	}
}


void ChunkTerrain::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_x", "x"), &ChunkTerrain::set_x);
	ClassDB::bind_method(D_METHOD("get_x"), &ChunkTerrain::get_x);
	ClassDB::bind_method(D_METHOD("set_z", "z"), &ChunkTerrain::set_z);
	ClassDB::bind_method(D_METHOD("get_z"), &ChunkTerrain::get_z);
	ClassDB::bind_method(D_METHOD("set_chunk_size", "size"), &ChunkTerrain::set_chunk_size);
	ClassDB::bind_method(D_METHOD("get_chunk_size"), &ChunkTerrain::get_chunk_size);
	ClassDB::bind_method(D_METHOD("set_chunk_amount", "amount"), &ChunkTerrain::set_chunk_amount);
	ClassDB::bind_method(D_METHOD("get_chunk_amount"), &ChunkTerrain::get_chunk_amount);

	ClassDB::bind_method(D_METHOD("set_noise", "noise"), &ChunkTerrain::set_noise);
	ClassDB::bind_method(D_METHOD("get_noise"), &ChunkTerrain::get_noise);

	ClassDB::bind_method(D_METHOD("set_surface_material", "surface_material"), &ChunkTerrain::set_surface_material);
	ClassDB::bind_method(D_METHOD("get_surface_material"), &ChunkTerrain::get_surface_material);
	ClassDB::bind_method(D_METHOD("_on_surface_material_changed"), &ChunkTerrain::_on_surface_material_changed);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "noise", PROPERTY_HINT_RESOURCE_TYPE, "OpenSimplexNoise"), "set_noise", "get_noise");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "surface_material", PROPERTY_HINT_RESOURCE_TYPE, "ShaderMaterial"), "set_surface_material", "get_surface_material");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "x"), "set_x", "get_x");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "z"), "set_z", "get_z");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "chunk_size"), "set_chunk_size", "get_chunk_size");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "chunk_amount"), "set_chunk_amount", "get_chunk_amount");
}

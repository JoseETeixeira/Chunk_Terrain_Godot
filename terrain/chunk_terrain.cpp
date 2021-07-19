/* chunk_terrain.cpp */

#include "chunk_terrain.h"
#include "../generator/chunk_generator.h"
#include <core/core_string_names.h>
#include <core/array.h>
#include "../../scene/3d/physics_body.h"

ChunkTerrain::ChunkTerrain(){
	set_x(0);
	set_z(0);
	set_chunk_size(32);
	set_chunk_amount(16);
	set_process(true);
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

void ChunkTerrain::set_x(int x){
    _x = x;
}

void ChunkTerrain::set_z(int z){
    _z = z;
}

int ChunkTerrain::get_x(){
	return _x;
}

int ChunkTerrain::get_z(){
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
	clean_up_chunks();
	reset_chunks();
	if(_player_node!=nullptr){
		update_chunks(_player_node->get_translation());
	}else{
		update_chunks(Vector3(0,0,0));
	}
}


void ChunkTerrain::update_chunks(Vector3 player_translation){

	int p_x =int(player_translation.x) / _chunk_size;
	int p_z = int(player_translation.z) / _chunk_size;

	for (int x =(p_x - _chunk_amount * 0.5) ; x< (p_x + _chunk_amount * 0.5);x++ ){
		for (int z =(p_z - _chunk_amount * 0.5) ; z< (p_z + _chunk_amount * 0.5);z++ ){
			add_chunk(x,z);
			ChunkGenerator* chunk = get_chunk(x,z);
			if (chunk!=nullptr){
				chunk->should_remove = false;
			}
		}
	}



}

void ChunkTerrain::clean_up_chunks(){

}

void ChunkTerrain::reset_chunks(){

}

void ChunkTerrain::set_player(NodePath path) {
	if (_player == path) {
		return;
	}
	_player = path;
	Node *node = this->get_node(_player);
	_player_node = Object::cast_to<Spatial>(node);

}


NodePath ChunkTerrain::get_player() {
	return _player;
}


void ChunkTerrain::set_noise(Ref<OpenSimplexNoise> noise) {
	if (_noise == noise) {
		return;
	}
	_noise = noise;

}


Ref<OpenSimplexNoise> ChunkTerrain::get_noise() {
	return _noise;
}
void ChunkTerrain::set_surface_material(Ref<ShaderMaterial> surface_material) {
	if (_surface_material == surface_material) {
		return;
	}
	_surface_material = surface_material;

}



Ref<ShaderMaterial> ChunkTerrain::get_surface_material() {
	return _surface_material;
}


void ChunkTerrain::add_chunk(int x, int z){

	Vector2 key = Vector2(x,z);

	if (_chunks.count(key)>0 ||_unready_chunks.count(key)>0){
		//already has key so we dont have to do anything
		return;
	}
	//print_line("CHUNK ADDED");

	load_chunk(key);
}


void ChunkTerrain::load_chunk(Vector2 key){
	int x = key.x;
	int z = key.y;
	ChunkGenerator *chunk = memnew(ChunkGenerator());
	chunk->_init(x,z,_chunk_size,_noise);
	chunk->set_translation(Vector3(x*_chunk_size,0,z*_chunk_size));
	//print_line("CHUNK LOADED");
	load_done(chunk);


}

void ChunkTerrain::load_done(ChunkGenerator *generator){
	add_child(generator);
	int x = int(generator->get_x()/_chunk_size);
	int z = int(generator->get_z()/_chunk_size);

	Vector2 key = Vector2(x,z);
	std::pair<std::map<Vector2,ChunkGenerator*>::iterator,bool> ret;
	ret = _chunks.insert ( std::pair<Vector2,ChunkGenerator*>(key,generator) );

	_unready_chunks.erase(key);
}

ChunkGenerator* ChunkTerrain::get_chunk(int x, int z){

	Vector2 key = Vector2(x,z);

	if (_chunks.count(key) == 0 ){
		//already has key so we dont have to do anything
		return nullptr;
	}

	return _chunks.at(key);
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
	ClassDB::bind_method(D_METHOD("_process","delta"), &ChunkTerrain::_process);

	ClassDB::bind_method(D_METHOD("set_player", "path"), &ChunkTerrain::set_player);
	ClassDB::bind_method(D_METHOD("get_player"), &ChunkTerrain::get_player);

	ClassDB::bind_method(D_METHOD("set_surface_material", "surface_material"), &ChunkTerrain::set_surface_material);
	ClassDB::bind_method(D_METHOD("get_surface_material"), &ChunkTerrain::get_surface_material);

	ClassDB::bind_method(D_METHOD("add_chunk","x","z"), &ChunkTerrain::add_chunk);
	ClassDB::bind_method(D_METHOD("get_chunk","x","z"), &ChunkTerrain::get_chunk);

	ClassDB::bind_method(D_METHOD("update_chunks","player_translation"), &ChunkTerrain::update_chunks);
	ClassDB::bind_method(D_METHOD("clean_up_chunks"), &ChunkTerrain::clean_up_chunks);
	ClassDB::bind_method(D_METHOD("reset_chunks"), &ChunkTerrain::reset_chunks);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "noise", PROPERTY_HINT_RESOURCE_TYPE, "OpenSimplexNoise"), "set_noise", "get_noise");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "surface_material", PROPERTY_HINT_RESOURCE_TYPE, "ShaderMaterial"), "set_surface_material", "get_surface_material");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "player", PROPERTY_HINT_RESOURCE_TYPE, "NodePath"), "set_player", "get_player");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "x"), "set_x", "get_x");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "z"), "set_z", "get_z");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "chunk_size"), "set_chunk_size", "get_chunk_size");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "chunk_amount"), "set_chunk_amount", "get_chunk_amount");
}

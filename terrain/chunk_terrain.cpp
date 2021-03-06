/* chunk_terrain.cpp */
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

#include "chunk_terrain.h"
#include "../generator/chunk_generator.h"
#include <core/core_string_names.h>
#include <core/array.h>

template <typename T>
std::vector<String> get_keys(std::map<String,T> dictionary){
	std::vector<String> mapString;
	for(auto const& imap: dictionary)
    	mapString.push_back(imap.first);

	return mapString;
}

ChunkTerrain::ChunkTerrain(){
	set_x(0);
	set_z(0);
	set_chunk_size(32);
	set_chunk_amount(16);
	should_generate = true;
	should_generate_water = true;
	should_generate_grass = true;
	pool = ThreadPool::get_singleton();
	set_process(true);
}

ChunkTerrain::~ChunkTerrain(){
	std::vector<String> chunk_keys = get_keys(chunks);
	for (uint32_t it =0; it<chunk_keys.size(); it++){
		ChunkGenerator *chunk = chunks[chunk_keys[it]];
		if(chunk!= NULL){
			memdelete(chunk);
		}
		
	}
	chunks.clear();
	unready_chunks.clear();
}

void ChunkTerrain::_notification(int p_what) {
	switch (p_what) {
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
	if(_noise!=NULL && _surface_material!=nullptr){
		update_chunks();
		clean_up_chunks();
		reset_chunks();
	}


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


void ChunkTerrain::set_water_material(Ref<ShaderMaterial> water_material) {
	if (_water_material == water_material) {
		return;
	}
	_water_material = water_material;

}



Ref<ShaderMaterial> ChunkTerrain::get_water_material() {
	return _water_material;
}


void ChunkTerrain::set_grass_material(Ref<ShaderMaterial> grass_material) {
	if (_grass_material == grass_material) {
		return;
	}
	_grass_material = grass_material;

}



Ref<ShaderMaterial> ChunkTerrain::get_grass_material() {
	return _grass_material;
}



template <typename T>
std::string NumberToString ( T Number )
{
	std::ostringstream ss;
	ss << Number;
	return ss.str();
}


void ChunkTerrain::add_chunk(int x_local, int z_local){
	String xx =  NumberToString(x_local).c_str();
	String zz =  NumberToString(z_local).c_str();
	String key = xx + "," + zz;
	if (chunks.find(key)!=chunks.end() || unready_chunks.find(key)!=unready_chunks.end()){
		return;
	}


	Array arr;
	arr.push_back(x_local);
	arr.push_back(z_local);
	arr.push_back(_chunk_size);

	pool->create_execute_job(this, "load_chunk", arr);

	unready_chunks[key] = 1;


}

void ChunkTerrain::load_chunk(Array arr){
	int x_local = arr.pop_front();
	int z_local = arr.pop_front();
	int chunk_size = arr.pop_front();

	ChunkGenerator *chunk = memnew(ChunkGenerator(x_local,z_local));
	chunk->set_x(x_local * chunk_size);
	chunk->set_z(z_local * chunk_size);
	chunk->set_chunk_size(chunk_size);
	chunk->set_translation(Vector3(x_local*chunk_size,0,z_local*chunk_size));

	load_done(chunk);

	//call_deferred("load_done",chunk);
}

void ChunkTerrain::load_done(Variant variant){

	ChunkGenerator *chunk = Object::cast_to<ChunkGenerator>(variant);
	if(chunk!=NULL){
		mtx.lock();
		add_child(chunk);
		String xx =  NumberToString(chunk->get_x()/get_chunk_size()).c_str();
		String zz =  NumberToString(chunk->get_z()/get_chunk_size()).c_str();
		String key = xx + "," + zz;
		chunks[key] = chunk;
		unready_chunks.erase(key);
		mtx.unlock();

	}

	call_deferred("_on_load_done",chunk);

	//mtx.unlock();
	//thread.wait_to_finish();

}

void ChunkTerrain::_on_load_done(Variant variant){

	ChunkGenerator *chunk = Object::cast_to<ChunkGenerator>(variant);
	if(chunk!=NULL){
		print_line(String("Chunk Loaded {{0}, {1}}").format(varray(chunk->get_x(), chunk->get_z())));

	}


}

ChunkGenerator* ChunkTerrain::get_chunk(int x_local, int z_local){
	String xx =  NumberToString(x_local).c_str();
	String zz =  NumberToString(z_local).c_str();
	String key = xx + "," + zz;
	if(chunks.find(key)!=chunks.end()){
		return chunks[key];
	}
	return NULL;
}


void ChunkTerrain::update_chunks(){
	Node *player = get_node(player_path);
	if (player!=NULL && player->is_inside_tree() == true){
		Spatial *s_player = Object::cast_to<Spatial>(player);
		if(s_player != NULL){

			set_x(s_player->get_translation().x);


			set_z(s_player->get_translation().z);
		}
	}
	Vector3 player_translation = Vector3(get_x(),0,get_z());
	int p_x = int(player_translation.x) / _chunk_size;
	int p_z = int(player_translation.z) / _chunk_size;

	for (int i = (p_x - _chunk_amount * 0.5); i< (p_x + _chunk_amount * 0.5); i++){
		for (int j = (p_z - _chunk_amount * 0.5);j< (p_z + _chunk_amount * 0.5); j++){

			add_chunk(i, j);
			ChunkGenerator *chunk = get_chunk(i,j);
			if (chunk != NULL){
				chunk->set_should_remove(false);

			}

		}
	}



}


void ChunkTerrain::clean_up_chunks(){

	std::vector<String> chunk_keys = get_keys(chunks);
	for (uint32_t it =0; it<chunk_keys.size(); it++){
		ChunkGenerator* chunk = chunks[chunk_keys[it]];
		if(chunk!=NULL){
			if(chunk->get_should_remove() == true){
				memdelete(chunk);
				chunks.erase(chunk_keys[it]);

			}
		}
	}



}


void ChunkTerrain::reset_chunks(){

	std::vector<String> chunk_keys = get_keys(chunks);
	for (uint32_t it =0; it<chunk_keys.size(); it++){
		ChunkGenerator* chunk = chunks[chunk_keys[it]];
		if(chunk!=NULL){
			chunk->set_should_remove(true);

		}
	}
}

void ChunkTerrain::set_player_path(NodePath path){
	player_path = path;

}
NodePath ChunkTerrain::get_player_path(){
	return player_path;
}

void ChunkTerrain::set_should_generate_water(bool generate_water){
	should_generate_water = generate_water;
}

bool ChunkTerrain::get_should_generate_water(){
	return should_generate_water;
}

void ChunkTerrain::set_grass_mesh(Ref<Mesh> mesh){
	grass_mesh = mesh;
}

Ref<Mesh> ChunkTerrain::get_grass_mesh(){
	return grass_mesh;
}

void ChunkTerrain::set_should_generate_grass(bool generate_grass){
	should_generate_grass = generate_grass;
}

bool ChunkTerrain::get_should_generate_grass(){
	return should_generate_grass;
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

	ClassDB::bind_method(D_METHOD("set_player_path", "path"), &ChunkTerrain::set_player_path);
	ClassDB::bind_method(D_METHOD("get_player_path"), &ChunkTerrain::get_player_path);

	ClassDB::bind_method(D_METHOD("add_chunk", "x","y"), &ChunkTerrain::add_chunk);
	ClassDB::bind_method(D_METHOD("load_chunk", "arr"), &ChunkTerrain::load_chunk);
	ClassDB::bind_method(D_METHOD("load_done", "var"), &ChunkTerrain::load_done);

	ClassDB::bind_method(D_METHOD("_on_load_done","chunk"), &ChunkTerrain::_on_load_done);


	ClassDB::bind_method(D_METHOD("set_surface_material", "surface_material"), &ChunkTerrain::set_surface_material);
	ClassDB::bind_method(D_METHOD("get_surface_material"), &ChunkTerrain::get_surface_material);

	ClassDB::bind_method(D_METHOD("set_water_material", "water_material"), &ChunkTerrain::set_water_material);
	ClassDB::bind_method(D_METHOD("get_water_material"), &ChunkTerrain::get_water_material);

	ClassDB::bind_method(D_METHOD("set_grass_mesh", "grass_mesh"), &ChunkTerrain::set_grass_mesh);
	ClassDB::bind_method(D_METHOD("get_grass_mesh"), &ChunkTerrain::get_grass_mesh);

	ClassDB::bind_method(D_METHOD("set_should_generate_water", "generate_water"), &ChunkTerrain::set_should_generate_water);
	ClassDB::bind_method(D_METHOD("get_should_generate_water"), &ChunkTerrain::get_should_generate_water);

	ClassDB::bind_method(D_METHOD("set_should_generate_grass", "generate_grass"), &ChunkTerrain::set_should_generate_grass);
	ClassDB::bind_method(D_METHOD("get_should_generate_grass"), &ChunkTerrain::get_should_generate_grass);

	ClassDB::bind_method(D_METHOD("set_grass_material", "grass_material"), &ChunkTerrain::set_grass_material);
	ClassDB::bind_method(D_METHOD("get_grass_material"), &ChunkTerrain::get_grass_material);

	ADD_GROUP("Terrain", "");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "player", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Spatial"), "set_player_path", "get_player_path");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "noise", PROPERTY_HINT_RESOURCE_TYPE, "OpenSimplexNoise"), "set_noise", "get_noise");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "surface_material", PROPERTY_HINT_RESOURCE_TYPE, "ShaderMaterial"), "set_surface_material", "get_surface_material");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "chunk_size"), "set_chunk_size", "get_chunk_size");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "chunk_amount"), "set_chunk_amount", "get_chunk_amount");
	ADD_GROUP("Water", "");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "water_material", PROPERTY_HINT_RESOURCE_TYPE, "ShaderMaterial"), "set_water_material", "get_water_material");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "should_generate_water"), "set_should_generate_water", "get_should_generate_water");
	ADD_GROUP("Grass", "");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "grass_mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_grass_mesh", "get_grass_mesh");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "grass_material", PROPERTY_HINT_RESOURCE_TYPE, "ShaderMaterial"), "set_grass_material", "get_grass_material");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "should_generate_grass"), "set_should_generate_grass", "get_should_generate_grass");
	

}

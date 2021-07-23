/* chunk_terrain.cpp */

#include "chunk_terrain.h"
#include "../generator/chunk_generator.h"
#include <core/core_string_names.h>
#include <core/array.h>


ChunkTerrain::ChunkTerrain(){

}

ChunkTerrain::~ChunkTerrain(){
	Array chunk_keys = chunks.keys();
	Array unready_chunks_keys = unready_chunks.keys();
	for (int it =0; it<chunk_keys.size(); it++){
		memdelete(chunks.getptr(chunk_keys[it]));
	}
	for (int it =0; it<unready_chunks_keys.size(); it++){
		memdelete(unready_chunks.getptr(unready_chunks_keys[it]));
	}
	chunks.clear();
	unready_chunks.clear();
}

void ChunkTerrain::_notification(int p_what) {
	switch (p_what) {

		case NOTIFICATION_ENTER_TREE :
			set_x(0);
			set_z(0);
			set_chunk_size(32);
			set_chunk_amount(16);
			pool.set_use_threads(true);
			pool.set_thread_count(8);
			pool.set_thread_fallback_count(4);
			pool.set_max_work_per_frame_percent(20);

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
	if(_noise!=nullptr && _surface_material!=nullptr){
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
	if (chunks.has(key) || unready_chunks.has(key)){
		return;
	}

	Array arr;
	arr.push_back(x_local);
	arr.push_back(z_local);
	arr.push_back(_chunk_size);
	pool.create_execute_job(this, "load_chunk", arr);
	unready_chunks[key] = 1;
	//mtx.unlock();
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

	call_deferred("load_done",chunk);
}

void ChunkTerrain::load_done(Variant variant){
	ChunkGenerator *chunk = Object::cast_to<ChunkGenerator>(variant);
	add_child(chunk);
	String xx =  NumberToString(chunk->get_x()/get_chunk_size()).c_str();
	String zz =  NumberToString(chunk->get_z()/get_chunk_size()).c_str();
	String key = xx + "," + zz;
	//
	chunks[key] = chunk;
	unready_chunks.erase(key);
	//mtx.unlock();
	//thread.wait_to_finish();

}

Variant* ChunkTerrain::get_chunk(int x_local, int z_local){
	String xx =  NumberToString(x_local).c_str();
	String zz =  NumberToString(z_local).c_str();
	String key = xx + "," + zz;
	if(chunks.has(key)){
		return chunks.getptr(key);
	}
	return nullptr;
}


void ChunkTerrain::update_chunks(){
	Vector3 player_translation = Vector3(get_x(),0,get_z());
	int p_x = int(player_translation.x) / _chunk_size;
	int p_z = int(player_translation.z) / _chunk_size;

	for (int i = (p_x - _chunk_amount * 0.5); i< (p_x + _chunk_amount * 0.5); i++){
		for (int j = (p_z - _chunk_amount * 0.5);j< (p_z + _chunk_amount * 0.5); j++){

			add_chunk(i, j);
			Variant *chunk = get_chunk(i,j);
			if (chunk != nullptr){
				ChunkGenerator *gen = Object::cast_to<ChunkGenerator>(*chunk);
				gen->set_should_remove(false);
			}

		}
	}



}


void ChunkTerrain::clean_up_chunks(){
	Array chunk_keys = chunks.keys();
	for (auto it =0; it<chunk_keys.size(); it++){
		ChunkGenerator* chunk = Object::cast_to<ChunkGenerator>(chunks.get_valid(chunk_keys[it]));
		if(chunk->get_should_remove() == true){
			memdelete(chunk);
			chunks.erase(chunk_keys[it]);

		}

	}
}


void ChunkTerrain::reset_chunks(){
	Array chunk_keys = chunks.keys();
	for (auto it =0; it<chunk_keys.size(); it++){
		ChunkGenerator* chunk = Object::cast_to<ChunkGenerator>(chunks.get_valid(chunk_keys[it]));
		chunk->set_should_remove(true);

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

	ClassDB::bind_method(D_METHOD("add_chunk", "x","y"), &ChunkTerrain::add_chunk);
	ClassDB::bind_method(D_METHOD("load_chunk", "arr"), &ChunkTerrain::load_chunk);
	ClassDB::bind_method(D_METHOD("load_done", "var"), &ChunkTerrain::load_done);


	ClassDB::bind_method(D_METHOD("set_surface_material", "surface_material"), &ChunkTerrain::set_surface_material);
	ClassDB::bind_method(D_METHOD("get_surface_material"), &ChunkTerrain::get_surface_material);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "noise", PROPERTY_HINT_RESOURCE_TYPE, "OpenSimplexNoise"), "set_noise", "get_noise");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "surface_material", PROPERTY_HINT_RESOURCE_TYPE, "ShaderMaterial"), "set_surface_material", "get_surface_material");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "x"), "set_x", "get_x");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "z"), "set_z", "get_z");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "chunk_size"), "set_chunk_size", "get_chunk_size");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "chunk_amount"), "set_chunk_amount", "get_chunk_amount");
}

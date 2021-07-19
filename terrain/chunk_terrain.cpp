/* chunk_terrain.cpp */

#include "chunk_terrain.h"
#include "../generator/chunk_generator.h"
#include <core/core_string_names.h>
#include <core/array.h>
#include <core/ustring.h>
#include <sstream>

ChunkTerrain::ChunkTerrain(){
	set_x(0);
	set_y(0);
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


void ChunkTerrain::set_y(int y){
    _y = y;
}

int ChunkTerrain::get_y(){
	return _y;
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
	update_chunks();
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
	_surface_material = surface_material;
	if(_generator!=nullptr){
		_generator->set_surface_material(_surface_material);
	}

}



Ref<ShaderMaterial> ChunkTerrain::get_surface_material() {
	return _surface_material;
}

void ChunkTerrain::set_generator(ChunkGenerator *generator){
	if (_generator == generator){
		return;
	}
	_generator = generator;

}

template <typename T>
std::string NumberToString ( T Number )
{
	std::ostringstream ss;
	ss << Number;
	return ss.str();
}

void ChunkTerrain::add_chunk(int x, int z){
	const char*  xstr = NumberToString(x).c_str();
	const char*  zstr = NumberToString(z).c_str();
	String xstrr= xstr; 
	String zstrr = zstr;
	String out_key = xstrr + "," + zstrr;
	Variant key = memnew(Variant(out_key));
	//if(chunks.has(key) or unready_chunks.has(key)){
	//	return;
	//}
		
	ThreadData data;
	if(!data.thread->is_active()){
		data.x = x;
		data.z = z;
		data.chunk_size = _chunk_size;
		data.terrain = this;
		data.thread->start(this,"load_chunk",&data);
		unready_chunks[key] = 1;
	}
	
		
}

void ChunkTerrain::load_chunk(void *p_data){
	ThreadData &data = *static_cast<ThreadData *>(p_data);
	int x_local = data.x;
	int z_local = data.z;
	
	ChunkGenerator *chunk = memnew(ChunkGenerator());
	chunk->set_x(x_local*data.chunk_size);
	chunk->set_z(z_local*data.chunk_size);
	
	chunk->set_translation(Vector3(x_local*data.chunk_size,0,z_local*data.chunk_size));
	//load_done(chunk);
	data.generator = chunk;
	call_deferred("load_done",data);
}

void ChunkTerrain::load_done(ThreadData data){
	add_child(data.generator);
	const char*  xstr = NumberToString(data.generator->get_x()/data.chunk_size).c_str();
	const char*  zstr = NumberToString(data.generator->get_z()/data.chunk_size).c_str();
	String xstrr= xstr; 
	String zstrr = zstr;
	String out_key = xstrr + "," + zstrr;
	Variant key = memnew(Variant(out_key));
	data.terrain->chunks[key] = data.generator;
	data.terrain->unready_chunks.erase(key);
	data.thread->wait_to_finish();
}

ChunkGenerator* ChunkTerrain::get_chunk(int x,int z){
	const char*  xstr = NumberToString(x).c_str();
	const char*  zstr = NumberToString(z).c_str();
	String xstrr= xstr; 
	String zstrr = zstr;
	String out_key = xstrr + "," + zstrr;
	Variant key = memnew(Variant(out_key));
	if(chunks.getptr(key)!=nullptr){
		Variant var = chunks.get_valid(key);
		ChunkGenerator *generator = Object::cast_to<ChunkGenerator>(var);
		return generator;
	}
	return nullptr;
}

void ChunkTerrain::update_chunks(){
	Vector3 player_translation = Vector3(get_x(),get_y(),get_z());
	int p_x = int(player_translation.x/ _chunk_size) ;
	int p_z = int(player_translation.z/ _chunk_size) ;

	for(int i = (p_x - _chunk_amount * 0.5); i < (p_x + _chunk_amount * 0.5); i++ ){
		for (int j = (p_z - _chunk_amount * 0.5); (j< p_z + _chunk_amount * 0.5); j++ ){
			add_chunk(i,j);
			ChunkGenerator *chunk = get_chunk(i,j);
			if (chunk!=nullptr){
				chunk->set_should_remove(false);
			}
		}
	}
		
	
}

void ChunkTerrain::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_x", "x"), &ChunkTerrain::set_x);
	ClassDB::bind_method(D_METHOD("get_x"), &ChunkTerrain::get_x);
	ClassDB::bind_method(D_METHOD("set_y", "y"), &ChunkTerrain::set_y);
	ClassDB::bind_method(D_METHOD("get_y"), &ChunkTerrain::get_y);
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
	

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "noise", PROPERTY_HINT_RESOURCE_TYPE, "OpenSimplexNoise"), "set_noise", "get_noise");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "surface_material", PROPERTY_HINT_RESOURCE_TYPE, "ShaderMaterial"), "set_surface_material", "get_surface_material");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "x"), "set_x", "get_x");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "y"), "set_y", "get_y");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "z"), "set_z", "get_z");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "chunk_size"), "set_chunk_size", "get_chunk_size");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "chunk_amount"), "set_chunk_amount", "get_chunk_amount");
}

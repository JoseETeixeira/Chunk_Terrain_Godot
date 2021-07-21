/* chunk_terrain.cpp */

#include "chunk_terrain.h"
#include "../generator/chunk_generator.h"
#include <core/core_string_names.h>
#include <core/array.h>
#include <core/ustring.h>
#include <sstream>

namespace {
ChunkTerrain *g_chunk_terrain = nullptr;
}

template <typename Dst_T>
inline Dst_T *must_be_cast(IVoxelTask *src) {
#ifdef TOOLS_ENABLED
	Dst_T *dst = dynamic_cast<Dst_T *>(src);
	CRASH_COND_MSG(dst == nullptr, "Invalid cast");
	return dst;
#else
	return static_cast<Dst_T *>(src);
#endif
}


ChunkTerrain::ChunkTerrain(){
	set_x(0);
	set_y(0);
	set_z(0);
	set_chunk_size(32);
	set_chunk_amount(16);
	_chunk_thread_pool.set_name("Chunk generation");
	_chunk_thread_pool.set_thread_count(1);
	_chunk_thread_pool.set_priority_update_period(300);
	_chunk_thread_pool.set_batch_count(16);
	set_process(true);
}

ChunkTerrain::~ChunkTerrain(){

	wait_and_clear_all_tasks(true);
	_chunks.clear();
	_unready_chunks.clear();
}

void ChunkTerrain::wait_and_clear_all_tasks(bool warn){
	_chunk_thread_pool.wait_for_all_tasks();
	_chunk_thread_pool.dequeue_completed_tasks([warn](IVoxelTask *task) {
		if (warn) {
			WARN_PRINT("Streaming tasks remain on module cleanup, "
					   "this could become a problem if they reference scripts");
		}
		memdelete(task);
	});
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
	_chunk_thread_pool.dequeue_completed_tasks([this](IVoxelTask *task) {
		memdelete(task);
	});
	if(_noise!=nullptr && _surface_material!=nullptr){

		update_chunks();
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

void ChunkTerrain::add_chunk(int x,int z){
	String xvar = NumberToString(x).c_str();
	String zvar = NumberToString(x).c_str();
	String key = xvar + "," + zvar;
	//_chunk_thread_pool.wait_for_all_tasks();

	if(_chunks.has(key) || _unready_chunks.has(key)){
		return;
	}

	ChunkGenerateRequest *r = memnew(ChunkGenerateRequest);
	r->x = x;
	r->z = z;
	r->chunk_size = _chunk_size;
	r->generator = memnew(ChunkGenerator);
	r->terrain = this;
	_chunk_thread_pool.enqueue(r);



}

void ChunkTerrain::ChunkGenerateRequest::run(VoxelTaskContext ctx) {

	generator->set_x(x*chunk_size);
	generator->set_z(z*chunk_size);
	generator->set_translation(Vector3(x*chunk_size,0,z*chunk_size));

	terrain->load_done(this);
	has_run = true;
	String xvar = NumberToString(x).c_str();
	String zvar = NumberToString(x).c_str();
	String key = xvar + "," + zvar;
	terrain->_unready_chunks[key] = 1;

}





void ChunkTerrain::load_done(ChunkGenerateRequest *request){
	add_child(request->generator);
	String xvar = NumberToString(request->generator->get_x()/get_chunk_size()).c_str();
	String zvar = NumberToString(request->generator->get_z()/get_chunk_size()).c_str();
	String key = xvar + "," + zvar;
	_chunks[key] = request->generator;
	_unready_chunks.erase(key);


}

Variant ChunkTerrain::get_chunk(int x,int z){
	String xvar = NumberToString(x).c_str();
	String zvar = NumberToString(z).c_str();
	String key = xvar + "," + zvar;
	if (_chunks.has(key)){
		return _chunks.get_valid(key);
	}
	return Variant();
}


void ChunkTerrain::update_chunks(){
	Vector3 player_translation = Vector3(get_x(),get_y(),get_z());
	int p_x = int(player_translation.x) / _chunk_size;
	int p_z = int(player_translation.z) / _chunk_size;

	for (int i = (p_x - _chunk_amount * 0.5); i<(p_x + _chunk_amount * 0.5); i++ ){
		for (int j = (p_z - _chunk_amount * 0.5); j<(p_z + _chunk_amount * 0.5); j++ ){
			add_chunk(i,j);
			Variant cgenerator = get_chunk(i,j);
			ChunkGenerator *generator = Object::cast_to<ChunkGenerator>(cgenerator);
			if(generator!=nullptr){

				generator->set_should_remove(false);
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

#include "chunk_generator.h"
#include "../../scene/resources/primitive_meshes.h"
#include "../../core/math/vector2.h"
#include "../../core/math/vector3.h"
#include "../../scene/resources/mesh.h"


void ChunkGenerator::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_PARENTED:
			_parent = Object::cast_to<ChunkTerrain>(get_parent());
			if (_parent != nullptr) {
				_parent->set_generator(this);
                set_x(_parent->get_x());
                set_z(_parent->get_z());
                set_chunk_size(_parent->get_chunk_size());

			}
			// TODO may want to reload all instances? Not sure if worth implementing that use case
			break;

		case NOTIFICATION_UNPARENTED:
			if (_parent != nullptr) {
				_parent->set_generator(nullptr);
                set_x(0);
                set_z(0);
                set_chunk_size(0);

			}
			_parent = nullptr;
			break;
	}
}
ChunkGenerator::~ChunkGenerator() {
}

void ChunkGenerator::set_noise(Ref<OpenSimplexNoise> noise) {
	if (_noise == noise) {
		return;
	}
	_noise->disconnect(CoreStringNames::get_singleton()->changed, this, "_on_noise_changed");
	_noise = noise;
	_noise->connect(CoreStringNames::get_singleton()->changed, this, "_on_noise_changed");
	
}

void ChunkGenerator::_on_noise_changed() {
	ERR_FAIL_COND(_noise.is_null());
	print_line("-------- NOISE CHANGED ----------");
    //TODO: CLEAR CHUNKS AND REGENERATE
}

Ref<OpenSimplexNoise> ChunkGenerator::get_noise() const {
	return _noise;
}

void ChunkGenerator::set_surface_material(Ref<ShaderMaterial> surface_material) {
	if (_surface_material == surface_material) {
		return;
	}
	_surface_material->disconnect(CoreStringNames::get_singleton()->changed, this, "_on_surface_material_changed");
	_surface_material = surface_material;
	_surface_material->connect(CoreStringNames::get_singleton()->changed, this, "_on_surface_material_changed");
	
}

void ChunkGenerator::_on_surface_material_changed() {
	ERR_FAIL_COND(_surface_material.is_null());
	print_line("-------- SURFACE MATERIAL CHANGED ----------");
    //TODO: CLEAR CHUNKS AND REGENERATE
}

Ref<ShaderMaterial> ChunkGenerator::get_surface_material() const {
	return _surface_material;
}


void ChunkGenerator::set_x(uint32_t x){
    _x = x;
}

void ChunkGenerator::set_z(uint32_t z){
    _z = z;
}

void ChunkGenerator::set_chunk_size(int chunk_size){
    _chunk_size = chunk_size;
}

virtual void ChunkGenerator::generate_chunk(){
    if(_parent!=nullptr){
        memdelete(_data_tool);
        memdelete(_surface_tool);
        _data_tool = memnew(MeshDataTool());
        _surface_tool = memnew(SurfaceTool());
        Chunk *chunk = memnew(Chunk());
        Ref<PlaneMesh> plane_mesh = memnew(Ref<PlaneMesh>());
        plane_mesh->size = Vector2(_chunk_size, _chunk_size);
        plane_mesh->subdivide_depth = _chunk_size * 0.5;
        plane_mesh->subdivide_width = _chunk_size * 0.5;
        plane_mesh->material =get_surface_material();
        _surface_tool->create_from(plane_mesh, 0);
        Ref<ArrayMesh> array_plane = _surface_tool.commit();
        Error error = _data_tool->create_from_surface(array_plane, 0);

        for (auto it = 0; it < _data_tool->get_vertex_count(); ++it) {
            Vector3 vertex = _data_tool->get_vertex(it);
            vertex.y =  ceil(_noise->get_noise_3d(vertex.x + _x, vertex.y, vertex.z + _z) * 400);
            _data_tool->set_vertex(it,vertex);
            chunk->grid_positions.push_back(Vector3i(vertex.x,vertex.y,vertex.z));
        }

        for (auto it = 0; it < array_plane->get_surface_count(); ++it) {
            array_plane->surface_remove(it);
        }
        
        _data_tool->commit_to_surface(array_plane);
        _surface_tool->begin(Mesh::PrimitiveType::PRIMITIVE_TRIANGLES);
        _surface_tool->create_from(array_plane, 0);
        _surface_tool->generate_normals();
        
        chunk->mesh_instance = memnew(Ref<MeshInstance>());
        
        chunk->mesh_instance->mesh = _surface_tool.commit();

        chunk->mesh_instance->create_trimesh_collision();
        MeshInstance chunk_mesh = chunk->mesh_instance.instance();
        _chunks->push_back(chunk);
        
        add_child(chunk_mesh);
    }
    

}



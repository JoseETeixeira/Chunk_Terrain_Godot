#ifndef CHUNK_GENERATOR_H
#define CHUNK_GENERATOR_H

#include <core/reference.h>
#include "../../scene/3d/mesh_instance.h"
#include "../../scene/resources/mesh_data_tool.h"
#include "../../scene/resources/surface_tool.h"
#include "../../scene/resources/material.h"
#include "../util/math/vector3i.h"
#include "../../modules/opensimplex/open_simplex_noise.h"
#include <scene/3d/spatial.h>

class ChunkTerrain;

class ChunkGenerator : public Spatial {
	GDCLASS(ChunkGenerator, Spatial)
public:
	ChunkGenerator();
    ~ChunkGenerator();

	virtual void generate_chunk();
    //virtual void generate_water();

    void set_noise(Ref<OpenSimplexNoise> noise);
    Ref<OpenSimplexNoise> get_noise()const;

    //TODO: get_chunks
    void set_surface_material(Ref<ShaderMaterial> surface_material);
    Ref<ShaderMaterial> get_surface_material() const;


protected:
	static void _bind_methods();
    void _on_noise_changed();
    void _on_surface_material_changed();
    void _notification(int p_what);

private:
    struct Chunk;

    uint32_t _x;
    uint32_t _z;
    Ref<OpenSimplexNoise> _noise;
    int _chunk_size;
    bool should_remove = true;
    MeshDataTool *_data_tool;
    SurfaceTool *_surface_tool;
    Ref<ShaderMaterial> _surface_material;


    void set_x(uint32_t x);
    void set_z(uint32_t z);
    void set_chunk_size(int chunk_size);

    struct Chunk {
		// Position in mesh block coordinate system
		std::vector<Vector3i> grid_positions;
		Ref<MeshInstance> mesh_instance;
	};
    ChunkTerrain *_parent;
    std::vector<Chunk *> _chunks; // Does not have nulls


};

#endif // CHUNK_GENERATOR_H

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
	ChunkGenerator(int x, int z);
    ~ChunkGenerator();

	void generate_chunk();
    //virtual void generate_water();

    void set_noise(Ref<OpenSimplexNoise> noise);
    Ref<OpenSimplexNoise> get_noise();

    //TODO: get_chunks
    void set_surface_material(Ref<ShaderMaterial> surface_material);
    Ref<ShaderMaterial> get_surface_material() ;

    void set_x(int x);
    void set_z(int z);
    void set_chunk_size(int chunk_size);

    void set_should_remove(bool should_remove);
    bool get_should_remove();

    int get_x();
    int get_z();



protected:
	static void _bind_methods();
    void _notification(int p_what);

private:

    int _x;
    int _z;
    Ref<OpenSimplexNoise> _noise;
    int _chunk_size;
    bool _should_remove = false;
    MeshDataTool *_data_tool;
    SurfaceTool *_surface_tool;
    Ref<ShaderMaterial> _surface_material;

    ChunkTerrain *_parent;


};

#endif // CHUNK_GENERATOR_H

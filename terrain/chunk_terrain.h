/* chunk_terrain.h */
#ifndef CHUNK_TERRAIN_H
#define CHUNK_TERRAIN_H

#include "../../core/reference.h"
#include "../../core/array.h"
#include "../../core/bind/core_bind.h"
#include "../../modules/opensimplex/open_simplex_noise.h"
#include "../../scene/resources/material.h"
#include <scene/3d/spatial.h>
#include "../util/math/vector3i.h"
#include "../../core/dictionary.h"
#include <string>

class ChunkGenerator;

class ChunkTerrain : public Spatial {
    GDCLASS(ChunkTerrain,Spatial);

protected:
    static void _bind_methods();
	void _notification(int p_what);
	void _process(float delta);

public:
    void set_x(int x);
    int get_x();

	void set_y(int y);
    int get_y();

    void set_z(int z);
    int get_z();

    void set_chunk_size(int size);
    int get_chunk_size();

    void set_chunk_amount(int amount);
    int get_chunk_amount();

    void set_generator(ChunkGenerator *generator);

	void set_noise(Ref<OpenSimplexNoise> noise);
    Ref<OpenSimplexNoise> get_noise();

	void set_surface_material(Ref<ShaderMaterial> surface_material);
    Ref<ShaderMaterial> get_surface_material();


    ChunkTerrain();
    ~ChunkTerrain();

private:


    int _x;
	int _y;
    int _z;
    int _chunk_size;
    int _chunk_amount;
    ChunkGenerator *_generator = nullptr;
	Ref<OpenSimplexNoise> _noise;
	Ref<ShaderMaterial> _surface_material;
	Dictionary _chunks;
	Dictionary _unready_chunks;


};

#endif

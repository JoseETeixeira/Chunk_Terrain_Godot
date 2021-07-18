/* chunk_terrain.h */
#ifndef CHUNK_TERRAIN_H
#define CHUNK_TERRAIN_H

#include "../../core/reference.h"
#include "../../core/os/thread.h"
#include "../../modules/opensimplex/open_simplex_noise.h"
#include "../../scene/resources/material.h"
#include "../../core/math/vector2.h"
#include <scene/3d/spatial.h>
#include "../util/math/vector3i.h"
#include <map>
#include <thread>

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

    void set_z(int z);
    int get_z();

    void set_chunk_size(int size);
    int get_chunk_size();

    void set_chunk_amount(int amount);
    int get_chunk_amount();

	void set_noise(Ref<OpenSimplexNoise> noise);
    Ref<OpenSimplexNoise> get_noise();

	void set_surface_material(Ref<ShaderMaterial> surface_material);
    Ref<ShaderMaterial> get_surface_material();

	void add_chunk(int x, int z);





    ChunkTerrain();
    ~ChunkTerrain();

private:

	void load_chunk(Vector2 key);
	void load_done(ChunkGenerator *generator);

    int _x;
    int _z;
    int _chunk_size;
    int _chunk_amount;
	Ref<OpenSimplexNoise> _noise;
	Ref<ShaderMaterial> _surface_material;
	std::map<Vector2,ChunkGenerator*> _chunks;
	std::map<Vector2,ChunkGenerator*> _unready_chunks;


};

#endif

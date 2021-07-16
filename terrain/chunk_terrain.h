/* chunk_terrain.h */
#ifndef CHUNK_TERRAIN_H
#define CHUNK_TERRAIN_H

#include "<core/reference>"
#include "../../core/os/thread.h"
#include "../../modules/opensimplex/open_simplex_noise.h"
#include "../../scene/resources/material.h"


class ChunkGenerator;

class ChunkTerrain : public Spatial {
    GDCLASS(ChunkTerrain,Spatial);

protected:
    static void _bind_methods();
	void _notification(int p_what);
	void _process(float delta);
	void _on_noise_changed();
	void _on_surface_material_changed();

public:
    void set_x(uint32_t x);
    uint32_t get_x();

    void set_z(uint32_t z);
    uint32_t get_z();

    void set_chunk_size(int size);
    int get_chunk_size()const;

    void set_chunk_amount(int amount);
    int get_chunk_amount()const;

    void set_generator(ChunkGenerator *generator);

	void set_noise(Ref<OpenSimplexNoise> noise);
    Ref<OpenSimplexNoise> get_noise()const;

	void set_surface_material(Ref<ShaderMaterial> surface_material);
    Ref<ShaderMaterial> get_surface_material() const;


    ChunkTerrain();
    ~ChunkTerrain();

private:
    uint32_t _x;
    uint32_t _z;
    int _chunk_size;
    int _chunk_amount;
    ChunkGenerator *_generator = nullptr;
	Thread *tread = nullptr;
	Ref<OpenSimplexNoise> _noise;
	Ref<ShaderMaterial> _surface_material;


};

#endif


#ifndef CHUNK_TERRAIN_H
#define CHUNK_TERRAIN_H

/* chunk_terrain.h */
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

#include "../../core/reference.h"
#include "../../core/array.h"
#include "../../core/bind/core_bind.h"
#include "../../modules/opensimplex/open_simplex_noise.h"
#include "../../scene/resources/material.h"
#include <scene/3d/spatial.h>
#include "../util/math/vector3i.h"
#include "../../core/dictionary.h"
#include "../../core/ustring.h"
#include "../../core/os/thread.h"
#include "../../core/os/semaphore.h"
#include "../../core/os/mutex.h"
#include <thread>
#include <mutex>
#include <sstream>
#include "../server/thread_pool.h"

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

    void set_player_path(NodePath path);
    NodePath get_player_path();





    ChunkTerrain();
    ~ChunkTerrain();

private:

	//CHUNK GENERATION
	void add_chunk(int x, int z);
	void load_chunk(Array arr);
	void load_done(Variant variant);
	Variant* get_chunk(int x,int z);


	void update_chunks();
	void clean_up_chunks();
	void reset_chunks();

    int _x;
    int _z;
    int _chunk_size;
    int _chunk_amount;
	Ref<OpenSimplexNoise> _noise;
	Ref<ShaderMaterial> _surface_material;
	Dictionary chunks;
	Dictionary unready_chunks;
    bool should_generate;
    NodePath player_path;


};

#endif

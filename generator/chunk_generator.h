
#ifndef CHUNK_GENERATOR_H
#define CHUNK_GENERATOR_H

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
	void generate_water();
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
    struct Chunk;

    int _x;
    int _z;
    Ref<OpenSimplexNoise> _noise;
    int _chunk_size;
    bool _should_remove = false;
    MeshDataTool *_data_tool;
    SurfaceTool *_surface_tool;
    Ref<ShaderMaterial> _surface_material;
	MeshInstance* mesh_instance;
	MeshInstance* water_mesh;



    ChunkTerrain *_parent;


};

#endif // CHUNK_GENERATOR_H

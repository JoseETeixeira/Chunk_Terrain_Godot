/* chunk_terrain.h */
#ifndef CHUNK_TERRAIN_H
#define CHUNK_TERRAIN_H

#include "<core/reference>"


class ChunkGenerator;

class ChunkTerrain : public Spatial {
    GDCLASS(ChunkTerrain,Spatial);

protected:
    static void _bind_methods();

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

    ChunkTerrain();
    ~ChunkTerrain();

private:
    uint32_t _x;
    uint32_t _z;
    int _chunk_size;
    int _chunk_amount;
    ChunkGenerator *_generator = nullptr;


};

#endif
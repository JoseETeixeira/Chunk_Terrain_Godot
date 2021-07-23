/* register_types.cpp */

#include "register_types.h"

#include "core/version.h"

#if VERSION_MAJOR > 3
#include "core/config/engine.h"
#else
#include "core/engine.h"
#endif


#include "core/class_db.h"
#include "terrain/chunk_terrain.h"
#include "generator/chunk_generator.h"
#include "server/thread_pool.h"
#include "server/thread_pool_execute_job.h"
#include "server/thread_pool_job.h"

static ThreadPool *thread_pool = NULL;

void register_chunk_terrain_types() {
    ClassDB::register_class<ChunkGenerator>();
	ClassDB::register_class<ChunkTerrain>();


    ClassDB::register_class<ThreadPoolJob>();
	ClassDB::register_class<ThreadPoolExecuteJob>();

	thread_pool = memnew(ThreadPool);
	ClassDB::register_class<ThreadPool>();
	Engine::get_singleton()->add_singleton(Engine::Singleton("ThreadPool", ThreadPool::get_singleton()));
}

void unregister_chunk_terrain_types() {
   if (thread_pool) {
		memdelete(thread_pool);
	}
}

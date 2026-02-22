#ifndef BSK_MATERIAL_SERVER_H
#define BSK_MATERIAL_SERVER_H

#include <basilisk/util/includes.h>
#include <basilisk/render/material.h>
#include <basilisk/render/shader.h>
#include <basilisk/render/tbo.h>
#include <basilisk/resource/textureServer.h>
#include <mutex>
#include <unordered_set>

namespace bsk::internal {

class MaterialServer {
    private:
        TextureServer* textureServer;
        TBO* tbo;

        std::unordered_map<Material*, unsigned int> materialMapping;
        std::mutex materialMutex;  // Thread safety for materialMapping
        
        // Deferred update queue - materials that need updating between render frames
        std::unordered_set<Material*> pendingUpdates;
        std::mutex updateMutex;  // Thread safety for pendingUpdates
        
        // Deferred add queue - materials that need to be added between render frames
        // Needed because texture array operations (resizing) must not happen during rendering
        std::unordered_set<Material*> pendingAdds;
        std::mutex addMutex;  // Thread safety for pendingAdds
        
        // Track if we're in active game loop (update() has been called)
        // During initialization (before first update), materials can be added immediately
        bool inGameLoop;
        
        // Guard to prevent recursive calls to processPendingUpdates()
        bool processingUpdates;

        // Internal helper to check if OpenGL context is current
        bool isContextCurrent() const;
        
        // Internal add implementation (assumes mutex is locked and context is current)
        unsigned int addInternal(Material* material);
        
        // Internal update implementation (assumes context is current and mutex is locked)
        void updateInternal(Material* material);

    public:
        MaterialServer(TextureServer* textureServer);
        ~MaterialServer();

        unsigned int add(Material* material);
        unsigned int get(Material* material);
        
        void write(Shader* shader, std::string name, unsigned int startSlot = 0);
        
        // Queue a material for deferred update (thread-safe, can be called from any thread)
        void update(Material* material);
        
        // Process all pending material updates and additions (must be called from render thread with OpenGL context)
        void processPendingUpdates();
        
        // Remove a material from the server (for cleanup)
        void remove(Material* material);
        
        // Reset material server state (clears all mappings and queues)
        // CRITICAL: Call this when starting a new game run to clear stale pointers
        // from previous Python runs. This ensures consistent behavior.
        void reset();
};

}

#endif
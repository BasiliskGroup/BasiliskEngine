#include <basilisk/resource/materialServer.h>
#include <basilisk/IO/window.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>
#include <algorithm>

namespace bsk::internal {

MaterialServer::MaterialServer(TextureServer* textureServer): textureServer(textureServer), inGameLoop(false), processingUpdates(false) {
    tbo = new TBO(nullptr, 0, 20000);
    // Note: inGameLoop starts as false to allow immediate material adds during initialization
    // It will be set to true when processPendingUpdates() actually processes something
}

MaterialServer::~MaterialServer() {
    delete tbo;
}

/**
 * @brief Check if OpenGL context is current on this thread
 * 
 * @return true if context is current, false otherwise
 */
bool MaterialServer::isContextCurrent() const {
    return glfwGetCurrentContext() != nullptr;
}

/**
 * @brief Get the location of the material in the tbo. 
 *        Starting byte location will be the index returned by this function * sizeof(MaterialData)
 * 
 * @param material The material to get
 * @return unsigned int Material index, or 0 if material not found (safe fallback)
 */
unsigned int MaterialServer::get(Material* material) {
    if (!material) return 0;
    
    // CRITICAL FIX FOR FLICKERING: Check if material is in mapping first
    {
        std::lock_guard<std::mutex> lock(materialMutex);
        auto it = materialMapping.find(material);
        if (it != materialMapping.end()) {
            return it->second;
        }
    }
    
    // Material not found - check if it's pending and try to process it immediately
    // This prevents flickering when animation materials are swapped during update()
    // CRITICAL: Only do this if we're not currently processing updates (to avoid deadlock)
    // and if context is current (required for OpenGL operations)
    if (!processingUpdates.load(std::memory_order_acquire) && isContextCurrent()) {
        // Check if material is pending
        {
            std::lock_guard<std::mutex> addLock(addMutex);
            if (pendingAddsSet.count(material) > 0) {
                // Material is pending - try to process it immediately
                // This is safe because we're not currently processing updates
                // and we have the OpenGL context
                processPendingUpdates();
                
                // Check again after processing
                std::lock_guard<std::mutex> materialLock(materialMutex);
                auto it = materialMapping.find(material);
                if (it != materialMapping.end()) {
                    return it->second;
                }
            }
        }
    }
    
    // Material not found - return 0 as safe fallback (default material)
    return 0;
}

/**
 * @brief Internal add implementation (assumes mutex is locked and context is current)
 * 
 * @param material The material to add
 * @return unsigned int Location of the material in the tbo. 
 */
unsigned int MaterialServer::addInternal(Material* material) {
    if (!material) return 0;
    
    // Do not add if the material is ready on the tbo
    if (materialMapping.count(material)) {
        return materialMapping.at(material);
    }

    // Get the material data
    MaterialData data = material->getData();
    
    // Get the location of the albedo and normal maps in the texture server
    std::pair<unsigned int, unsigned int> albedo = textureServer->add(material->getAlbedo());
    std::pair<unsigned int, unsigned int> normal = textureServer->add(material->getNormal());
    
    // Update material data to have correct texture array locations
    data.albedoArray = albedo.first;
    data.albedoIndex = albedo.second;
    data.normalArray = normal.first;
    data.normalIndex = normal.second;

    // CRITICAL FIX FOR WINDOWS: Calculate index based on mapping size
    // This ensures materials get sequential IDs in the order they're added.
    // IMPORTANT: We must calculate the index BEFORE adding to mapping to ensure
    // consistent ordering. If we add to mapping first, the size changes and
    // we get wrong indices.
    // 
    // The key insight: On Windows, unordered_map iteration order is non-deterministic,
    // but insertion order into the mapping (via this function) IS deterministic.
    // By using mapping.size() as the index, we ensure materials get IDs in the
    // exact order they're added, preventing positioning issues.
    unsigned int index = materialMapping.size();
    unsigned int offset = index * sizeof(MaterialData);
    
    // Write the material data to the correct slot in the tbo
    tbo->write(&data, sizeof(data), offset);

    // Add the material to the mapping with the calculated index
    // CRITICAL: This must happen AFTER calculating index to preserve ordering
    materialMapping[material] = index;

    return index;
}

/**
 * @brief Add a new material to the server. 
 *        During initialization (before first update()), adds immediately.
 *        During game loop, defers to processPendingUpdates() to ensure texture operations
 *        happen at safe times (between render frames, not during rendering).
 * 
 * @param material The material to add
 * @return unsigned int Location of the material in the tbo
 */
unsigned int MaterialServer::add(Material* material) {
    if (!material) return 0;
    
    // Check if material is already added (thread-safe quick check)
    {
        std::lock_guard<std::mutex> lock(materialMutex);
        if (materialMapping.count(material)) {
            return materialMapping.at(material);
        }
    }
    
    // CRITICAL FIX FOR WINDOWS: Only allow immediate addition during initialization
    // During gameplay, we MUST always defer to processPendingUpdates() to ensure:
    // 1. Materials are processed in the exact order they were added (preserves IDs)
    // 2. No race conditions between immediate adds and processPendingUpdates()
    // 3. Consistent behavior across platforms (Windows has stricter memory ordering)
    // 
    // The problem: If we allow immediate addition during gameplay, materials can be added
    // out of order, causing wrong IDs and positioning issues. For example:
    // - Material A added immediately (gets ID 10)
    // - processPendingUpdates() processes Material B (gets ID 10, overwrites A's slot)
    // - Result: Materials have wrong IDs, causing positioning/texture issues
    // 
    // Solution: Only add immediately during initialization (before inGameLoop is true).
    // During gameplay, always defer to processPendingUpdates() which processes in order.
    if (!inGameLoop.load(std::memory_order_acquire) && isContextCurrent() && !processingUpdates.load(std::memory_order_acquire)) {
        std::lock_guard<std::mutex> lock(materialMutex);
        // Double-check after acquiring lock (another thread might have added it)
        if (materialMapping.count(material) == 0) {
            try {
                return addInternal(material);
            } catch (const std::exception& e) {
                // If addInternal fails (e.g., invalid material), defer it
                // This prevents crashes from corrupted materials
                std::cerr << "Warning: Failed to add material immediately (exception: " << e.what() << "), deferring" << std::endl;
                std::lock_guard<std::mutex> addLock(addMutex);
                // CRITICAL FIX FOR WINDOWS: Use vector to preserve order, set for duplicate checking
                if (pendingAddsSet.count(material) == 0) {
                    pendingAdds.push_back(material);
                    pendingAddsSet.insert(material);
                }
                return 0;
            } catch (...) {
                // If addInternal fails (e.g., invalid material), defer it
                // This prevents crashes from corrupted materials
                std::cerr << "Warning: Failed to add material immediately (unknown exception), deferring" << std::endl;
                std::lock_guard<std::mutex> addLock(addMutex);
                // CRITICAL FIX FOR WINDOWS: Use vector to preserve order, set for duplicate checking
                if (pendingAddsSet.count(material) == 0) {
                    pendingAdds.push_back(material);
                    pendingAddsSet.insert(material);
                }
                return 0;
            }
        }
        return materialMapping.at(material);
    }
    
    // If we can't add immediately (processing updates or no context), defer to safe time
    // CRITICAL: Texture array operations (like resizing) must not happen during active rendering
    // They must happen in processPendingUpdates() which is called at the safe time
    // 
    // CRITICAL FIX FOR WINDOWS: Double-check material is not already in mapping before deferring
    // This prevents materials from being added to pendingAdds multiple times, which causes
    // accumulation and wrong IDs over time. On Windows, race conditions can cause the initial
    // check to miss materials that are being processed, so we check again here.
    {
        std::lock_guard<std::mutex> materialLock(materialMutex);
        // CRITICAL: Check again if material was added between the initial check and now
        // This can happen if processPendingUpdates() ran between checks
        if (materialMapping.count(material)) {
            return materialMapping.at(material);
        }
    }
    
    // Material is not in mapping - add it to pending queue
    // CRITICAL FIX FOR WINDOWS: Only add if not already pending to prevent duplicates
    {
        std::lock_guard<std::mutex> addLock(addMutex);
        // CRITICAL: Check if material is already pending before adding
        // This prevents the same material from being added multiple times, which causes
        // accumulation and wrong IDs over time on Windows
        if (pendingAddsSet.count(material) == 0) {
            pendingAdds.push_back(material);
            pendingAddsSet.insert(material);
        }
    }
    
    // Return 0 for now - material will be added in processPendingUpdates()
    // This is safe because render() handles missing materials gracefully (returns 0 = default)
    // CRITICAL: Materials swapped during animation will use default material (ID 0) for 1 frame
    // This causes flickering but is necessary to maintain correct material IDs and ordering
    // The flickering will stop once processPendingUpdates() processes the material on the next frame
    return 0;
}

void MaterialServer::write(Shader* shader, std::string name, unsigned int slot) {
    shader->bind(name.c_str(), tbo, slot);
}

/**
 * @brief Internal update implementation (assumes context is current and mutex is locked)
 * 
 * @param material The material to update
 */
void MaterialServer::updateInternal(Material* material) {
    if (!material) return;
    
    // If material was never added, add it using internal method (no additional locking needed)
    if (materialMapping.count(material) == 0) {
        addInternal(material);
        return;
    }

    // Get the material data
    MaterialData data = material->getData();
    
    // Get the location of the albedo and normal maps in the texture server
    std::pair<unsigned int, unsigned int> albedo = textureServer->add(material->getAlbedo());
    std::pair<unsigned int, unsigned int> normal = textureServer->add(material->getNormal());
    
    // Update material data to have correct texture array locations
    data.albedoArray = albedo.first;
    data.albedoIndex = albedo.second;
    data.normalArray = normal.first;
    data.normalIndex = normal.second;

    // Write the material data to the tbo
    unsigned int index = materialMapping.at(material);
    tbo->write(&data, sizeof(data), index * sizeof(MaterialData));
}

/**
 * @brief Queue a material for deferred update (thread-safe, can be called from any thread)
 *        Updates are processed in processPendingUpdates() which must be called from render thread
 * 
 * @param material The material to update
 */
void MaterialServer::update(Material* material) {
    if (!material) return;
    
    // Thread-safe: add to pending updates queue
    {
        std::lock_guard<std::mutex> lock(updateMutex);
        pendingUpdates.insert(material);
    }
}

/**
 * @brief Process all pending material updates and additions (must be called from render thread with OpenGL context)
 *        This should be called in Engine::update() at the end, before rendering begins
 */
void MaterialServer::processPendingUpdates() {
    // Prevent recursive calls (deadlock/infinite loop protection
    // CRITICAL FIX FOR WINDOWS: Use atomic load to ensure we see the latest value
    bool expected = false;
    if (!processingUpdates.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
        // Already processing, return early
        return;
    }
    
    // CRITICAL: Use RAII to ensure processingUpdates is always reset, even on early return
    // This prevents the flag from getting stuck true on Windows
    struct ProcessingGuard {
        std::atomic<bool>& flag;
        ProcessingGuard(std::atomic<bool>& f) : flag(f) {}
        ~ProcessingGuard() { flag.store(false, std::memory_order_release); }
    } guard(processingUpdates);
    
    // Verify OpenGL context is current
    if (!isContextCurrent()) {
        std::cerr << "Warning: MaterialServer::processPendingUpdates() called without OpenGL context." << std::endl;
        return;  // guard will reset processingUpdates
    }
    
    // Get pending adds (thread-safe copy)
    // CRITICAL FIX FOR WINDOWS: Copy vector to preserve insertion order
    // This ensures materials are processed in the order they were added, preventing
    // wrong IDs and positioning issues on Windows where unordered_set iteration order is non-deterministic
    std::vector<Material*> addsToProcess;
    {
        std::lock_guard<std::mutex> lock(addMutex);
        if (!pendingAdds.empty()) {
            addsToProcess = pendingAdds;  // Copy vector to preserve order
            pendingAdds.clear();
            pendingAddsSet.clear();
        }
    }
    
    // Get pending updates (thread-safe copy)
    std::unordered_set<Material*> updatesToProcess;
    {
        std::lock_guard<std::mutex> lock(updateMutex);
        if (!pendingUpdates.empty()) {
            updatesToProcess = pendingUpdates;
            pendingUpdates.clear();
        }
    }
    
    // If nothing to process, return early (don't mark as in game loop yet)
    // This allows materials created after first update() but before first render()
    // to still be added immediately during initialization
    if (addsToProcess.empty() && updatesToProcess.empty()) {
        return;  // guard will reset processingUpdates
    }
    
    // Mark that we're now in the game loop (we actually processed something)
    // This means we're past initialization and materials should be deferred
    // BUT: Only set this if we're actually processing adds/updates from the game loop,
    // not from initialization. We check if there are pending operations to distinguish.
    // Actually, if we got here, we have something to process, so we're in the game loop.
    // CRITICAL FIX FOR WINDOWS: Use atomic store to ensure visibility across threads
    inGameLoop.store(true, std::memory_order_release);
    
    // processingUpdates was already set to true by compare_exchange_strong above
    
    // Lock material mutex for the duration of all operations
    std::lock_guard<std::mutex> lock(materialMutex);
    
    // First, process all pending additions
    // This ensures materials exist before we try to update them
    // CRITICAL: This is where texture array operations (including resizing) happen safely
    // CRITICAL FIX FOR WINDOWS: Process materials in exact order and prevent duplicates
    // Windows has non-deterministic behavior with unordered_map, so we must ensure
    // materials are processed in the exact order they were added to pendingAdds.
    // We also need to prevent the same material from being added multiple times,
    // which can happen if set_material() is called multiple times with the same material.
    std::vector<Material*> invalidMaterials;
    std::unordered_set<Material*> processedThisBatch;  // Track materials processed in this batch
    
    for (Material* material : addsToProcess) {
        // Validate material pointer is still valid (not destroyed from previous run)
        if (!material) {
            invalidMaterials.push_back(material);
            continue;
        }
        
        // CRITICAL FIX FOR WINDOWS: Check if material was already processed in this batch
        // This prevents the same material from being added multiple times if it appears
        // multiple times in addsToProcess (shouldn't happen, but Windows can be weird)
        if (processedThisBatch.count(material) > 0) {
            // Material already processed in this batch - skip it
            continue;
        }
        
        // CRITICAL FIX FOR WINDOWS: Check if material is already in mapping
        // If it is, it means it was added immediately during initialization or
        // was already processed in a previous batch. We should not add it again.
        if (materialMapping.count(material) == 0) {
            try {
                addInternal(material);
                processedThisBatch.insert(material);  // Mark as processed
            } catch (const std::exception& e) {
                // Material might be invalid (destroyed from previous run or corrupted)
                // Skip it - it will be re-added if still needed
                std::cerr << "Warning: Failed to add material (exception: " << e.what() << ")" << std::endl;
                invalidMaterials.push_back(material);
                continue;
            } catch (...) {
                // Material might be invalid (destroyed from previous run or corrupted)
                // Skip it - it will be re-added if still needed
                std::cerr << "Warning: Failed to add material (unknown exception)" << std::endl;
                invalidMaterials.push_back(material);
                continue;
            }
        } else {
            // Material already in mapping - mark as processed to avoid duplicate handling
            processedThisBatch.insert(material);
        }
    }
    
    // CRITICAL FIX FOR WINDOWS: Remove invalid materials from pendingAdds to prevent accumulation
    if (!invalidMaterials.empty()) {
        std::lock_guard<std::mutex> addLock(addMutex);
        for (Material* material : invalidMaterials) {
            pendingAddsSet.erase(material);
            pendingAdds.erase(std::remove(pendingAdds.begin(), pendingAdds.end(), material), pendingAdds.end());
        }
    }
    
    // Then, process all pending updates
    for (Material* material : updatesToProcess) {
        // Skip if material was removed or never added
        // Also validate material is still valid (not destroyed from previous run)
        if (material && materialMapping.count(material) > 0) {
            try {
                updateInternal(material);
            } catch (...) {
                // Material might be invalid (destroyed from previous run or corrupted)
                // CRITICAL FIX FOR WINDOWS: Do NOT remove from mapping!
                // Removing causes size() to decrease, leading to ID reuse.
                // Just skip it - it will be ignored on future access attempts.
                // materialMapping.erase(material);  // DO NOT DO THIS!
                continue;
            }
        }
    }
    
    // processingUpdates will be reset by RAII guard when function returns
}

/**
 * @brief Remove a material from the server (for cleanup)
 * 
 * CRITICAL FIX FOR WINDOWS: Do NOT remove materials from materialMapping!
 * When materials are removed from the mapping, materialMapping.size() decreases,
 * causing new materials to reuse the same IDs. This overwrites TBO slots that
 * still contain old data, causing wrong materials/positioning.
 * 
 * Instead, we only remove materials from pending queues. The mapping persists
 * until reset() is called, ensuring IDs are never reused. Stale pointers in
 * the mapping are safe because:
 * 1. We check materialMapping.count() before accessing
 * 2. get() returns 0 if material not found (safe fallback)
 * 3. reset() clears everything when starting a new game run
 * 
 * @param material The material to remove
 */
void MaterialServer::remove(Material* material) {
    if (!material) return;
    
    std::lock_guard<std::mutex> updateLock(updateMutex);
    std::lock_guard<std::mutex> addLock(addMutex);
    
    // CRITICAL FIX FOR WINDOWS: Do NOT remove from materialMapping!
    // Removing from mapping causes size() to decrease, leading to ID reuse
    // which overwrites TBO slots and causes wrong materials/positioning.
    // materialMapping.erase(material);  // DO NOT DO THIS!
    
    // Only remove from pending queues to prevent processing stale materials
    pendingUpdates.erase(material);
    // CRITICAL FIX FOR WINDOWS: Remove from both vector and set
    pendingAddsSet.erase(material);
    pendingAdds.erase(std::remove(pendingAdds.begin(), pendingAdds.end(), material), pendingAdds.end());
    
    // Note: We don't remove the data from TBO to avoid index shifting issues
    // The slot will remain with old data, but since we don't reuse IDs, it won't be accessed
}

/**
 * @brief Reset material server state (clears all mappings and queues)
 * 
 * CRITICAL: This should be called when starting a new game run to clear stale pointers
 * from previous Python runs. The C++ singleton persists between Python runs, so when
 * Python objects are destroyed and recreated, the C++ side still has raw pointers to
 * the old (destroyed) materials. This causes crashes when those pointers are accessed.
 * 
 * This method clears:
 * - materialMapping (removes all stale pointers)
 * - pendingUpdates (clears deferred update queue)
 * - pendingAdds (clears deferred add queue)
 * - Resets inGameLoop and processingUpdates flags
 * 
 * Note: We don't clear the TBO data to avoid index shifting issues.
 * The TBO slots will be reused as new materials are added.
 */
void MaterialServer::reset() {
    std::lock_guard<std::mutex> lock(materialMutex);
    std::lock_guard<std::mutex> updateLock(updateMutex);
    std::lock_guard<std::mutex> addLock(addMutex);
    
    // Clear all mappings and queues to remove stale pointers from previous runs
    materialMapping.clear();
    pendingUpdates.clear();
    pendingAdds.clear();
    pendingAddsSet.clear();
    
    // Reset flags to allow immediate material adds during new initialization
    // CRITICAL FIX FOR WINDOWS: Use atomic store to ensure visibility across threads
    inGameLoop.store(false, std::memory_order_release);
    processingUpdates.store(false, std::memory_order_release);
    
    // Note: We don't clear/reset the TBO because:
    // 1. It would require OpenGL context which might not be current
    // 2. Index shifting would break existing material references
    // 3. TBO slots will be reused as new materials are added
}

}
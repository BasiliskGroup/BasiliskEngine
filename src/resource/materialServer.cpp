#include <basilisk/resource/materialServer.h>
#include <basilisk/IO/window.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>

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
    
    std::lock_guard<std::mutex> lock(materialMutex);
    auto it = materialMapping.find(material);
    if (it != materialMapping.end()) {
        return it->second;
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

    // CRITICAL FIX: Calculate index FIRST based on mapping size, then use it for byte offset
    // This ensures index and TBO offset are always in sync, even after reset() clears mapping
    // but not TBO data. The index represents the material's position in the array.
    unsigned int index = materialMapping.size();
    unsigned int offset = index * sizeof(MaterialData);
    
    // Write the material data to the correct slot in the tbo
    tbo->write(&data, sizeof(data), offset);

    // Add the material to the mapping with the calculated index
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
    
    // CRITICAL FIX FOR WINDOWS: Only add immediately if we're NOT in the game loop yet
    // Once inGameLoop is true, we MUST defer all additions because:
    // 1. addInternal() calls textureServer->add() which can trigger TextureArray::generate()
    // 2. generate() calls glTexImage3D() which recreates the texture array
    // 3. This is UNSAFE if a frame is already active (after Engine::update() calls frame->use())
    // 4. Materials swapped during scene->update() happen AFTER frame->use(), so they're unsafe
    // 
    // Solution: Only add immediately during initialization (before first processPendingUpdates()).
    // During game loop, always defer to processPendingUpdates() which runs BEFORE frame->use().
    // This means materials swapped during animations will use default material (ID 0) for 1 frame,
    // but this is safe and prevents crashes.
    if (!inGameLoop && isContextCurrent()) {
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
                pendingAdds.insert(material);
                return 0;
            } catch (...) {
                // If addInternal fails (e.g., invalid material), defer it
                // This prevents crashes from corrupted materials
                std::cerr << "Warning: Failed to add material immediately (unknown exception), deferring" << std::endl;
                std::lock_guard<std::mutex> addLock(addMutex);
                pendingAdds.insert(material);
                return 0;
            }
        }
        return materialMapping.at(material);
    }
    
    // If we can't add immediately (processing updates or no context), defer to safe time
    // CRITICAL: Texture array operations (like resizing) must not happen during active rendering
    // They must happen in processPendingUpdates() which is called at the safe time
    {
        std::lock_guard<std::mutex> lock(addMutex);
        pendingAdds.insert(material);
    }
    
    // Return 0 for now - material will be added in processPendingUpdates()
    // This is safe because render() handles missing materials gracefully (returns 0 = default)
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
    if (processingUpdates) {
        return;
    }
    
    // Verify OpenGL context is current
    if (!isContextCurrent()) {
        std::cerr << "Warning: MaterialServer::processPendingUpdates() called without OpenGL context." << std::endl;
        return;
    }
    
    // Get pending adds (thread-safe copy)
    std::unordered_set<Material*> addsToProcess;
    {
        std::lock_guard<std::mutex> lock(addMutex);
        if (!pendingAdds.empty()) {
            addsToProcess = pendingAdds;
            pendingAdds.clear();
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
        return;
    }
    
    // Mark that we're now in the game loop (we actually processed something)
    // This means we're past initialization and materials should be deferred
    // BUT: Only set this if we're actually processing adds/updates from the game loop,
    // not from initialization. We check if there are pending operations to distinguish.
    // Actually, if we got here, we have something to process, so we're in the game loop.
    inGameLoop = true;
    
    // Set processing flag to prevent recursion
    processingUpdates = true;
    
    // Lock material mutex for the duration of all operations
    std::lock_guard<std::mutex> lock(materialMutex);
    
    // First, process all pending additions
    // This ensures materials exist before we try to update them
    // CRITICAL: This is where texture array operations (including resizing) happen safely
    for (Material* material : addsToProcess) {
        // Validate material pointer is still valid (not destroyed from previous run)
        // We can't fully validate a raw pointer, but we check for null and existence
        if (material && materialMapping.count(material) == 0) {
            try {
                addInternal(material);
            } catch (const std::exception& e) {
                // Material might be invalid (destroyed from previous run or corrupted)
                // Skip it - it will be re-added if still needed
                std::cerr << "Warning: Failed to add material (exception: " << e.what() << ")" << std::endl;
                continue;
            } catch (...) {
                // Material might be invalid (destroyed from previous run or corrupted)
                // Skip it - it will be re-added if still needed
                std::cerr << "Warning: Failed to add material (unknown exception)" << std::endl;
                continue;
            }
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
                // Remove it from mapping to prevent future issues
                materialMapping.erase(material);
                continue;
            }
        }
    }
    
    // Clear processing flag
    processingUpdates = false;
}

/**
 * @brief Remove a material from the server (for cleanup)
 * 
 * @param material The material to remove
 */
void MaterialServer::remove(Material* material) {
    if (!material) return;
    
    std::lock_guard<std::mutex> lock(materialMutex);
    std::lock_guard<std::mutex> updateLock(updateMutex);
    std::lock_guard<std::mutex> addLock(addMutex);
    
    materialMapping.erase(material);
    pendingUpdates.erase(material);
    pendingAdds.erase(material);
    // Note: We don't remove the data from TBO to avoid index shifting issues
    // The slot will be reused if needed in the future
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
    
    // Reset flags to allow immediate material adds during new initialization
    inGameLoop = false;
    processingUpdates = false;
    
    // Note: We don't clear/reset the TBO because:
    // 1. It would require OpenGL context which might not be current
    // 2. Index shifting would break existing material references
    // 3. TBO slots will be reused as new materials are added
}

}
#include <basilisk/render/shader.h>
#include <regex>
#include <filesystem>

namespace bsk::internal {


/**
 * @brief Get the component count of an OpenGL type. Not an exhaustive function, only valid for BOOL, INT, and FLOAT type. 
 * 
 * @param type GLenum of the type
 * @return GLint 
 */
inline GLint getGLTypeComponentCount(GLenum type) {
    switch (type) {
        case GL_BOOL:
        case GL_INT:
        case GL_UNSIGNED_INT:
        case GL_FLOAT: return 1;

        case GL_BOOL_VEC2:
        case GL_INT_VEC2:
        case GL_FLOAT_VEC2: return 2;

        case GL_BOOL_VEC3:
        case GL_INT_VEC3:
        case GL_FLOAT_VEC3: return 3;

        case GL_BOOL_VEC4:
        case GL_INT_VEC4:
        case GL_FLOAT_VEC4: return 4;
    }

    return 0;
}

/**
 * @brief Get the size in bytes of an OpenGL type. Not an exhaustive function, only valid for BOOL, INT, and FLOAT type. 
 * 
 * @param type GLenum of the type
 * @return GLsizei 
 */
inline GLsizei getGLTypeSize(GLenum type) {
    switch (type) {
        case GL_BOOL:
        case GL_INT:
        case GL_UNSIGNED_INT:
        case GL_FLOAT: return 4;

        case GL_BOOL_VEC2:
        case GL_INT_VEC2:
        case GL_FLOAT_VEC2: return 8;

        case GL_BOOL_VEC3:
        case GL_INT_VEC3:
        case GL_FLOAT_VEC3: return 12;

        case GL_BOOL_VEC4:
        case GL_INT_VEC4:
        case GL_FLOAT_VEC4: return 16;
    }

    return 0;
}

/**
 * @brief Read a file as a c-string. Unused now (replaced by loadShader Source), but kept in case needed for testing
 * 
 * @param path The absolute or relative path of the file to read
 * @return const char* 
 */
std::string loadFile(const char* path) {
    // Set out string and stream
    std::string content;
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    
    // Attempt to load the file and read to string
    try {
        file.open(path);
        std::stringstream vertexStream, fragmentStream;
        vertexStream << file.rdbuf();
        content = vertexStream.str();
        file.close();
    }
    catch (std::ifstream::failure e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }

    return content;
}


std::string loadShaderSource(std::string filepath, std::unordered_set<std::string>& includedFiles, bool isRootFile) {
    // Mark as included
    std::replace(filepath.begin(), filepath.end(), '\\', '/');
    includedFiles.insert(filepath);
    
    // Open the file
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << filepath << std::endl;
        return "";
    }

    // Get directory of file
    std::filesystem::path currentFilePath(filepath);
    std::filesystem::path currentDir = currentFilePath.parent_path();

    std::string line;
    std::stringstream fullSource;
    
    // Regex for #include "path/to/file.glsl"
    std::regex includeRegex(R"(#include\s+["<](.*)[">])");
    // Regex for #version
    std::regex versionRegex(R"(#version\s+.*)");

    while (std::getline(file, line)) {

        // Filter out #version unless at root
        if (std::regex_match(line, versionRegex)) {
            if (isRootFile) {
                fullSource << line << "\n";
            }
            continue;
        }

        // Add include files if needed
        std::smatch match;
        if (std::regex_search(line, match, includeRegex)) {
            std::string includeName = match[1].str();
            std::filesystem::path includePath = currentDir / includeName;
            std::string includePathStr = includePath.string();
            std::replace(includePathStr.begin(), includePathStr.end(), '\\', '/');
            
            if (!includedFiles.contains(includePathStr)) {
                fullSource << "// == Begin "  << includePathStr << " include ==\n";
                fullSource << "\n" << loadShaderSource(includePathStr, includedFiles, false) << "\n";
                fullSource << "// == End "  << includePathStr << " include ==\n";
            }
        } 
        // Add line as is if not an include
        else {
            fullSource << line << "\n";
        }
    }

    return fullSource.str();
}
std::string loadShaderSource(const std::string& filepath) {
    std::unordered_set<std::string> includedFiles;
    return loadShaderSource(filepath, includedFiles, true);
}

/**
 * @brief Compiles a shader from source code and returns the ID.
 * 
 * @param source C-String containg the shader source code
 * @param shaderType The type of shader. May be GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
 * @return unsigned int of the shader ID
 */
unsigned int loadShader(std::string source, unsigned int shaderType) {
    int success;
    char infoLog[512];

    // Load Shader
    unsigned int shader = glCreateShader(shaderType);
    const char* sourceCode = source.c_str();
    glShaderSource(shader, 1, &sourceCode, NULL);
    glCompileShader(shader);

    // Check for compilation errors
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    return shader;
}

/**
 * @brief Links a shader program given a vertex and fragment shader. Returns program ID. 
 * 
 * @param vertex ID of the vertex shader
 * @param fragment ID of the fragment shader
 * @return unsigned int 
 */
unsigned int loadProgram(unsigned int vertex, unsigned int fragment) {
    int success;
    char infoLog[512];

    // Shader program
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram (program);

    // Check for linking errors
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    return program;
}

/**
 * @brief Construct a new Shader object from vertex and fragment source
 * 
 * @param vertexPath 
 * @param fragmentPath 
 */
Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    //  Load the source code
    std::string vertexShaderSource   = loadShaderSource(vertexPath);
    std::string fragmentShaderSource = loadShaderSource(fragmentPath);

    if (vertexShaderSource.empty()) {
        std::cout << "Failed to load shader from path: " << vertexPath << std::endl;
    }
    if (fragmentShaderSource.empty()) {
        std::cout << "Failed to load shader from path: " << fragmentPath << std::endl;
    }

    // Compile shaders from source
    unsigned int vertex   = loadShader(vertexShaderSource,   GL_VERTEX_SHADER);
    unsigned int fragment = loadShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

    // Shader program
    ID = loadProgram(vertex, fragment);

    // Release hanging shaders
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    // Get all of the active attributes in the shader for VAO use
    loadAttributes();
}

/**
 * @brief Get all of the active attributes in the shader and saves them on the shader
 * 
 */
void Shader::loadAttributes() {
    GLint nAttributes;
    GLint size; 
    GLenum type; 
    const GLsizei bufSize = 256;
    GLchar name[bufSize];
    GLsizei length; 

    glGetProgramiv(ID, GL_ACTIVE_ATTRIBUTES, &nAttributes);

    attributes.resize(nAttributes);

    for (GLint i = 0; i < nAttributes; i++) {
        glGetActiveAttrib(ID, (GLuint)i, bufSize, &length, &size, &type, name);
        GLint location = glGetAttribLocation(ID, name);
        attributes.at(location) = {name, location, getGLTypeComponentCount(type), type, 0};
    }

    stride = 0;
    for (GLint i = 0; i < nAttributes; i++) {
        attributes.at(i).offset = stride;
        stride += getGLTypeSize(attributes.at(i).dataType);
    }
}

/**
 * @brief Destroy the Shader object
 * 
 */
Shader::~Shader() {
    slotBindings.clear();
    glDeleteProgram(ID);
}

/**
 * @brief Uses the shader program for rendering
 * 
 */
void Shader::use() { 
    glUseProgram(ID); 
}

/**
 * @brief General method for binding a texture target and id to a slot
 * 
 * @param texID 
 * @param target 
 * @param slot 
 */
void Shader::bindTextureToSlot(const char* name, GLuint texID, GLenum target, unsigned int slot) {
    use();

    auto it = slotBindings.find(slot);

    // TODO: Implement this logic to work globally, probably with static attributes
    // // If not bound or different, bind it
    // if (it == slotBindings.end() || it->second.id != texID || it->second.target != target) {
    //     glActiveTexture(GL_TEXTURE0 + slot);
    //     glBindTexture(target, texID);

    //     slotBindings[slot] = { texID, target };

    //     setUniform(name, (int)slot);
    // }

    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(target, texID);

    slotBindings[slot] = { texID, target };

    setUniform(name, (int)slot);
}

/**
 * @brief Binds a texture to the shader
 * 
 * @param name Name of the texture on the shader
 * @param texture Pointer to the texture to bind
 * @param slot Slot to bind the texutre to [0-15]
 */
void Shader::bind(const char* name, Texture* texture, unsigned int slot) {
    bindTextureToSlot(name, texture->getID(), GL_TEXTURE_2D, slot);
}

/**
 * @brief Binds a texture array to the shader
 * 
 * @param name Name of the texture on the shader
 * @param textureArray Pointer to the texture array to bind
 * @param slot Slot to bind the texutre to [0-15]
 */
void Shader::bind(const char* name, TextureArray* textureArray, unsigned int slot) {
    bindTextureToSlot(name, textureArray->getID(), GL_TEXTURE_2D_ARRAY, slot);
}

/**
 * @brief Binds a tbo to the shader
 * 
 * @param name Name of the uniform samplerBuffer on the shader
 * @param tbo Pointer to the TBO object 
 * @param slot Slot to bind the tbo [0-15]
 */
void Shader::bind(const char* name, TBO* tbo, unsigned int slot) {
    bindTextureToSlot(name, tbo->getTextureID(), GL_TEXTURE_BUFFER, slot);
}

/**
 * @brief Binds a fbo color attachment to the shader
 * 
 * @param name Name of the uniform samplerBuffer on the shader
 * @param fbo Pointer to the FBO object
 * @param slot Slot to bind the tbo [0-15]
 */
void Shader::bind(const char* name, FBO* fbo, unsigned int slot) {
    bindTextureToSlot(name, fbo->getTextureID(), GL_TEXTURE_2D, slot);
}

/**
 * @brief Get the location of a uniform on this shader. 
 * 
 * @param name C-String name of the uniform 
 * @return int 
 */
int Shader::getUniformLocation(const char* name){
    return glGetUniformLocation(ID, name);
}

/**
 * @brief Set a float uniform value
 * 
 * @param name Name of the uniform on the shader
 * @param value Value to set the uniform
 */
void Shader::setUniform(const char* name, float value) { 
    use();
    glUniform1f(getUniformLocation(name), value); 
}

/**
 * @brief Set a double uniform value
 * 
 * @param name Name of the uniform on the shader
 * @param value Value to set the uniform
 */
void Shader::setUniform(const char* name, double value) {
    Shader::setUniform(name, (float)value);
}

/**
 * @brief Set an int uniform value
 * 
 * @param name Name of the uniform on the shader
 * @param value Value to set the uniform
 */
void Shader::setUniform(const char* name, int value) { 
    use();
    glUniform1i(getUniformLocation(name), value); 
}

void Shader::setUniform(const char* name, glm::vec3 value) { 
    use();
    glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(value)); 
}

/**
 * @brief Set a matrix uniform value
 * 
 * @param name Name of the uniform on the shader
 * @param value Value to set the uniform
 */
void Shader::setUniform(const char* name, glm::mat4 value) { 
    use();
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));  
}

}
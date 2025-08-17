#define STB_IMAGE_IMPLEMENTATION
#include "texture.h"


Texture::Texture(const std::string& path) {
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);

    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nChannels, 0);

    if (!data) { std::cout << "Failed to load texture" << std::endl; }

    glTexImage2D(GL_TEXTURE_2D, 0, getFormat(nChannels), width, height, 0, getFormat(nChannels), GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);
}
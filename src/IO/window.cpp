#include <basilisk/IO/window.h>

namespace bsk::internal {

/**
 * @brief Callback for window resize. 
 *        Sets to the framebuffer size for mac compatibility. 
 * 
 * @param window 
 * @param width 
 * @param height 
 */
void Window::windowResize(GLFWwindow* window, int width, int height) {
    Window* self = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);

    int xsize, ysize;
    glfwGetWindowSize(window, &xsize, &ysize);
    
    if (self) {
        self->width  = xsize;
        self->height = ysize;
        self->windowScaleX = (float)fbWidth / xsize;
        self->windowScaleY = (float)fbHeight / ysize;
    }
}



/**
 * @brief Sets the window hint for ther GL version.
 *        Must be set before the window is created.  
 * 
 * @param major Major version
 * @param minor Sub-Version within the major version
 */
void setGLVersion(unsigned int major, unsigned int minor) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
}


/**
 * @brief Checks that the window was sucessfully created
 * 
 * @param window Pointer to the GLFWwindow to check
 * @return true 
 * @return false 
 */
bool confirmGLFW(GLFWwindow* window) {
    if (window != NULL) { return true; }
    
    // Default GLFW fail message
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return false;
}

/**
 * @brief Checks that the GLAD was sucessfully initialized
 * 
 * @return true 
 * @return false 
 */
bool confirmGLAD() {
    if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { return true; }

    // Default GLAD fail message
    std::cout << "Failed to initialize GLAD" << std::endl;
    return false;
}

/**
 * @brief Construct a new Window object with GLFW.
 * 
 * @param width The initial width of the window in pixels
 * @param height The initial height of the window in pixels
 * @param title 
 */
Window::Window(int width, int height, const char* title): width(width), height(height) {
    // Initialize GLFW with OpenGL
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    setGLVersion(3, 3);

    // Create the window and confirm proper initialization
    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!confirmGLFW(window)) { return; }
    glfwMakeContextCurrent(window);
    if (!confirmGLAD()) { return; }

    // Draws closer objects in front of futher objects
    glEnable(GL_DEPTH_TEST);  
    // Does not draw faces that are not facing the camera
    // glEnable(GL_CULL_FACE);
    // Antialiasing 
    glEnable(GL_MULTISAMPLE);
    // Blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

    // Set the resize callback
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, Window::windowResize);

    // Inital resize
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    windowResize(window, fbWidth, fbHeight);
    use();

    // Delta time config
    previousTime = 0;
    deltaTime = 0;
}

/**
 * @brief Checks if the window close flag has been set.
 * 
 * @return true 
 * @return false 
 */
bool Window::isRunning() {
    // Update delta time
    double currentTime = glfwGetTime();
    deltaTime = currentTime - previousTime;
    previousTime = currentTime;

    // Update the events
    glfwPollEvents();

    return !glfwWindowShouldClose(window);
}

/**
 * @brief Swaps the window buffers, showing all draws in the last frame.
 * 
 */
void Window::render() {
    glfwSwapBuffers(window);
}

/**
 * @brief Clears the window with the given color. 
 * 
 * @param r Red component
 * @param g Green component
 * @param b Blue component
 * @param a Alpha component
 */
void Window::clear(float r, float g, float b, float a) {
    use();
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/**
 * @brief Use the window for render calls. Bind framebuffer to 0. 
 * 
 */
void Window::use() {
    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 
}

/**
 * @brief Destroy the Window object and terminates GLFW
 * 
 */
Window::~Window() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

}
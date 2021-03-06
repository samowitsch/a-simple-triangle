#include "opengl-application.hpp"
#include "../../core/assets.hpp"
#include "../../core/graphics-wrapper.hpp"
#include "../../core/log.hpp"
#include "../../core/perspective-camera.hpp"
#include "../../core/sdl-wrapper.hpp"
#include "opengl-mesh.hpp"
#include "opengl-pipeline.hpp"
#include "opengl-texture.hpp"
#include <string>

using ast::OpenGLApplication;

namespace
{
    SDL_GLContext createContext(SDL_Window* window)
    {
        static const std::string logTag{"ast::OpenGLApplication::createContext"};

        SDL_GLContext context{SDL_GL_CreateContext(window)};

#ifdef WIN32
        glewInit();
#endif

        int viewportWidth;
        int viewportHeight;
        SDL_GL_GetDrawableSize(window, &viewportWidth, &viewportHeight);
        ast::log(logTag, "Created OpenGL context with viewport size: " + std::to_string(viewportWidth) + " x " + std::to_string(viewportHeight));

        glClearDepthf(1.0f);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_CULL_FACE);
        glViewport(0, 0, viewportWidth, viewportHeight);

        return context;
    }

    ast::PerspectiveCamera createCamera()
    {
        std::pair<uint32_t, uint32_t> displaySize{ast::sdl::getDisplaySize()};

        return ast::PerspectiveCamera(static_cast<float>(displaySize.first), static_cast<float>(displaySize.second));
    }

    glm::mat4 createMeshTransform()
    {
        glm::mat4 identity{1.0f};
        glm::vec3 position{0.0f, 0.0f, 0.0f};
        glm::vec3 rotationAxis{0.0f, 1.0f, 0.0f};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};
        float rotationDegrees{45.0f};

        return glm::translate(identity, position) *
               glm::rotate(identity, glm::radians(rotationDegrees), rotationAxis) *
               glm::scale(identity, scale);
    }
} // namespace

struct OpenGLApplication::Internal
{
    SDL_Window* window;
    SDL_GLContext context;
    const ast::PerspectiveCamera camera;
    const ast::OpenGLPipeline defaultPipeline;
    const ast::OpenGLMesh mesh;
    const glm::mat4 meshTransform;
    const ast::OpenGLTexture texture;

    Internal() : window(ast::sdl::createWindow(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI)),
                 context(::createContext(window)),
                 camera(::createCamera()),
                 defaultPipeline(ast::OpenGLPipeline("default")),
                 mesh(ast::OpenGLMesh(ast::assets::loadOBJFile("assets/models/crate.obj"))),
                 meshTransform(::createMeshTransform()),
                 texture(ast::OpenGLTexture(ast::assets::loadBitmap("assets/textures/crate.png"))) {}

    void render()
    {
        SDL_GL_MakeCurrent(window, context);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const glm::mat4 mvp{
            camera.getProjectionMatrix() *
            camera.getViewMatrix() *
            meshTransform};

        defaultPipeline.render(mesh, texture, mvp);

        SDL_GL_SwapWindow(window);
    }

    ~Internal()
    {
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
    }
};

OpenGLApplication::OpenGLApplication() : internal(ast::make_internal_ptr<Internal>()) {}

void OpenGLApplication::render()
{
    internal->render();
}

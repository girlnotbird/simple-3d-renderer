#include <SDL3/SDL_main.h>
#include "common.hpp"
#include "3D_Rendering_Engine.hpp"

ApplicationStatus status {APP_CONTINUE};

int main(int argc, char **argv) {
    status = RenderingEngine::InitializeEngine();

    while (status == APP_CONTINUE) {
        status = RenderingEngine::UpdateObjects();
        if (status != APP_CONTINUE) { break; }
        status = RenderingEngine::TransferVertexData();
        if (status != APP_CONTINUE) { break; }
        status = RenderingEngine::DrawFrame();
        if (status != APP_CONTINUE) { break; }
    }

    RenderingEngine::QuitEngine();

    return status;
}
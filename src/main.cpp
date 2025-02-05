#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <__filesystem/path.h>
#include <complex>
#include <glm/glm.hpp>
#include <iostream>

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtx/rotate_vector.hpp"

// List of structs for rendering Scenes
using TransformMatrix = glm::mat4;
using TriangleIndices = glm::u16vec3;
using VertexNormal = glm::vec3;
struct PositionAndColorVertex {
    glm::vec3 pos{};
    glm::u8vec4 color{};
};
struct Context {
    SDL_Window* Window;
    SDL_GPUDevice* Device;
    SDL_GPUGraphicsPipeline* ScenePipeline;
    SDL_GPUBuffer* VertexBuf;
    SDL_GPUBuffer* NormalBuf;
    SDL_GPUBuffer* IndexBuf;
    SDL_GPUBuffer* DrawBuf;
    SDL_GPUTexture* ColorTexture;
    SDL_GPUTexture* DepthTexture;
};
struct KeyboardState {
    bool w = false;
    bool a = false;
    bool s = false;
    bool d = false;
    bool q = false;
    bool e = false;
    bool f = false;
    bool g = false;
    bool cam_mode = false;
};
struct RenderableObject {
    std::vector<PositionAndColorVertex> vertices;
    std::vector<VertexNormal> normals;
    std::vector<TriangleIndices> indices;
    glm::mat4 model;
};
struct CameraObject {
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 camera_coords;
    glm::vec3 target_coords;
};
struct Scene {
    std::vector<RenderableObject> Objects;
    CameraObject Camera;
};
struct VertexUniformBufferData {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

// Clock Methods
auto GetTimePoint() {
    return std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>(
        std::chrono::duration_cast<std::chrono::milliseconds>((std::chrono::system_clock::now().time_since_epoch())));
}

// Methods for Transforming Model, View, and Projection Matrices before passing to Vertex Shader
auto& TranslateModel(RenderableObject& obj, const glm::vec3 tr) {
    const glm::mat4 translation_matrix{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                       0.0f, 0.0f, 1.0f, 0.0f, tr.x, tr.y, tr.z, 1};
    obj.model = obj.model * translation_matrix;
    return obj.model;
}
auto& RotateModelInPlace(RenderableObject& obj, const float angle, const glm::vec3 axis) {
    const bool is_zero_vector = axis.x == 0.0f && axis.y == 0.0f && axis.z == 0.0f;
    assert(!is_zero_vector);
    const auto normalized_axis = glm::normalize(axis);
    const glm::mat4 rotation_matrix =
        glm::mat4{glm::pow(normalized_axis.x, 2) * (1 - glm::cos(angle)) + glm::cos(angle),
                  normalized_axis.x * normalized_axis.y * (1 - glm::cos(angle)) - normalized_axis.z * glm::sin(angle),
                  normalized_axis.x * normalized_axis.z * (1 - glm::cos(angle)) + normalized_axis.y * glm::sin(angle),
                  0,
                  normalized_axis.x * normalized_axis.y * (1 - glm::cos(angle)) + normalized_axis.z * glm::sin(angle),
                  glm::pow(normalized_axis.y, 2) * (1 - glm::cos(angle)) + glm::cos(angle),
                  normalized_axis.y * normalized_axis.z * (1 - glm::cos(angle)) - normalized_axis.x * glm::sin(angle),
                  0,
                  normalized_axis.x * normalized_axis.z * (1 - glm::cos(angle)) - normalized_axis.y * glm::sin(angle),
                  normalized_axis.y * normalized_axis.z * (1 - glm::cos(angle)) + normalized_axis.x * glm::sin(angle),
                  glm::pow(normalized_axis.z, 2) * (1 - glm::cos(angle)) + glm::cos(angle),
                  0,
                  0,
                  0,
                  0,
                  1};
    obj.model = obj.model * rotation_matrix;
    return obj.model;
}
auto& RotateModelAboutOrigin(RenderableObject& obj, const float angle, const glm::vec3 axis) {
    const bool is_zero_vector = axis.x == 0.0f && axis.y == 0.0f && axis.z == 0.0f;
    assert(!is_zero_vector);
    const auto normalized_axis = glm::normalize(axis);
    const glm::mat4 rotation_matrix =
        glm::mat4{glm::pow(normalized_axis.x, 2) * (1 - glm::cos(angle)) + glm::cos(angle),
                  normalized_axis.x * normalized_axis.y * (1 - glm::cos(angle)) - normalized_axis.z * glm::sin(angle),
                  normalized_axis.x * normalized_axis.z * (1 - glm::cos(angle)) + normalized_axis.y * glm::sin(angle),
                  0,
                  normalized_axis.x * normalized_axis.y * (1 - glm::cos(angle)) + normalized_axis.z * glm::sin(angle),
                  glm::pow(normalized_axis.y, 2) * (1 - glm::cos(angle)) + glm::cos(angle),
                  normalized_axis.y * normalized_axis.z * (1 - glm::cos(angle)) - normalized_axis.x * glm::sin(angle),
                  0,
                  normalized_axis.x * normalized_axis.z * (1 - glm::cos(angle)) - normalized_axis.y * glm::sin(angle),
                  normalized_axis.y * normalized_axis.z * (1 - glm::cos(angle)) + normalized_axis.x * glm::sin(angle),
                  glm::pow(normalized_axis.z, 2) * (1 - glm::cos(angle)) + glm::cos(angle),
                  0,
                  0,
                  0,
                  0,
                  1};
    obj.model = rotation_matrix * obj.model;
    return obj.model;
}
auto& ScaleModel(RenderableObject& obj, const glm::vec3 scalars) {
    const glm::mat4 scale_matrix{
        scalars.x, 0, 0, 0, 0, scalars.y, 0, 0, 0, 0, scalars.z, 0, 0, 0, 0, 1,
    };
    obj.model = obj.model * scale_matrix;
    return obj.model;
}
auto& DollyCamera(CameraObject& cam, const float velocity) {
    const auto translation =
        glm::normalize(cam.target_coords - cam.camera_coords); // gives vec from camera_coords to target_coords
    cam.camera_coords += (velocity * 0.02f * translation);
    return cam.camera_coords;
}
auto& RaiseOrLowerCamera(CameraObject& cam, const float velocity) {
    cam.camera_coords.y += velocity * 0.02;
    return cam.camera_coords;
}
auto& OrbitCameraLaterally(CameraObject& cam, const float velocity) {
    const auto forward =
        glm::normalize(cam.camera_coords - cam.target_coords); // treats the "center" point as the origin.
    const auto right = glm::cross(glm::vec3{0.0f, 1.0f, 0.0f}, forward);
    const auto axis = glm::cross(forward, right);
    const auto angle = velocity * glm::pi<float>() / 256;

    const bool is_zero_vector = axis.x == 0.0f && axis.y == 0.0f && axis.z == 0.0f;
    assert(!is_zero_vector);
    const auto normalized_axis = glm::normalize(axis);
    const glm::mat4 rotation_matrix =
        glm::mat4{glm::pow(normalized_axis.x, 2) * (1 - glm::cos(angle)) + glm::cos(angle),
                  normalized_axis.x * normalized_axis.y * (1 - glm::cos(angle)) - normalized_axis.z * glm::sin(angle),
                  normalized_axis.x * normalized_axis.z * (1 - glm::cos(angle)) + normalized_axis.y * glm::sin(angle),
                  0,
                  normalized_axis.x * normalized_axis.y * (1 - glm::cos(angle)) + normalized_axis.z * glm::sin(angle),
                  glm::pow(normalized_axis.y, 2) * (1 - glm::cos(angle)) + glm::cos(angle),
                  normalized_axis.y * normalized_axis.z * (1 - glm::cos(angle)) - normalized_axis.x * glm::sin(angle),
                  0,
                  normalized_axis.x * normalized_axis.z * (1 - glm::cos(angle)) - normalized_axis.y * glm::sin(angle),
                  normalized_axis.y * normalized_axis.z * (1 - glm::cos(angle)) + normalized_axis.x * glm::sin(angle),
                  glm::pow(normalized_axis.z, 2) * (1 - glm::cos(angle)) + glm::cos(angle),
                  0,
                  0,
                  0,
                  0,
                  1};
    auto cc_vec4 = rotation_matrix * glm::vec4{cam.camera_coords, 1.0f};
    cam.camera_coords = {cc_vec4.x, cc_vec4.y, cc_vec4.z};
    return cam.camera_coords;
}
auto& CalculateVertexNormals(RenderableObject& obj) {

    std::vector<VertexNormal> vertex_normals;
    vertex_normals.resize(obj.vertices.size(), {0.0f, 0.0f, 0.0f});

    for (auto instance_number = 0; instance_number < obj.indices.size(); ++instance_number) {
        auto indices = obj.indices.at(instance_number);

        auto vertex1 = obj.vertices.at(indices[0]);
        auto vertex2 = obj.vertices.at(indices[1]);
        auto vertex3 = obj.vertices.at(indices[2]);

        auto side1 = vertex2.pos - vertex1.pos; // vector from 1 to 2
        auto side2 = vertex3.pos - vertex2.pos; // vector from 2 to 3
        auto normal = glm::cross(side1, side2);

        vertex_normals[indices[0]] += normal;
        vertex_normals[indices[1]] += normal;
        vertex_normals[indices[2]] += normal;
    }

    for (auto vertex_number = 0; vertex_number < obj.vertices.size(); ++vertex_number) {
        vertex_normals[vertex_number] = glm::normalize(-vertex_normals[vertex_number]);
    }

    obj.normals = vertex_normals;
    return obj;
};
auto& LookAt(CameraObject& cam, const glm::vec3 center) {
    const auto forward = glm::normalize(cam.camera_coords - center); // treats the "center" point as the origin.
    const auto right = glm::normalize(glm::cross(glm::vec3{0.0f, 1.0f, 0.0f}, forward));
    const auto up = glm::normalize(glm::cross(forward, right));
    cam.view = glm::mat4{right.x,
                         right.y,
                         right.z,
                         0.0f,
                         up.x,
                         up.y,
                         up.z,
                         0.0f,
                         forward.x,
                         forward.y,
                         forward.z,
                         0.0f,
                         glm::dot(cam.camera_coords, right),
                         glm::dot(cam.camera_coords, up),
                         -glm::dot(cam.camera_coords, forward),
                         1.0f};
    return cam.view;
}
auto& Project(CameraObject& cam, const float vFov, const float aspectRatio, const float near, const float far) {
    const auto focal_length = (glm::tan(1 / (vFov / 2)));
    const auto top = focal_length * near;
    const auto bottom = -top;
    const auto right = top * aspectRatio;
    const auto left = bottom * aspectRatio;
    const auto base_projection_matrix = glm::mat4{
        (2 * near) / (right - left),
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        (2 * near) / (top - bottom),
        0.0f,
        0.0f,
        (right + left) / (right - left),
        (top + bottom) / (top - bottom),
        -(far + near) / (far - near),
        -1.0f,
        0.0f,
        0.0f,
        -(2 * far * near) / (far - near),
        0.0f,
    };
    cam.proj = base_projection_matrix;
    return cam.proj;
}

// High-level scaffolding for SDL, Create Buffers, etc
auto InitContext() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* Window = SDL_CreateWindow(nullptr, 640, 640, 0);
    SDL_GPUDevice* Device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_MSL, true, "metal");
    SDL_ClaimWindowForGPUDevice(Device, Window);

    std::filesystem::path basepath = SDL_GetBasePath();
    while (basepath.parent_path().filename() == "build") {
        basepath = basepath.parent_path().parent_path();
    }

    std::string filename = "MVPUniform.vert.msl";
    std::string vspath = basepath;
    vspath.append("/shaders/compiled/");
    vspath.append(filename);
    std::size_t codesize = 0;
    auto code = static_cast<const unsigned char*>(SDL_LoadFile(vspath.c_str(), &codesize));

    auto vertex_shader_details = SDL_GPUShaderCreateInfo{.stage = SDL_GPU_SHADERSTAGE_VERTEX,
                                                         .format = SDL_GPU_SHADERFORMAT_MSL,
                                                         .entrypoint = "main0",
                                                         .num_samplers = 0,
                                                         .num_storage_buffers = 0,
                                                         .num_storage_textures = 0,
                                                         .num_uniform_buffers = 1,
                                                         .code = code,
                                                         .code_size = codesize};

    filename = "SolidColorDepth.frag.msl";
    // filename = "SolidColor.frag.msl";
    std::string fspath = basepath;
    fspath.append("/shaders/compiled/");
    fspath.append(filename);
    codesize = 0;
    code = static_cast<const unsigned char*>(SDL_LoadFile(fspath.c_str(), &codesize));

    auto fragment_shader_details = SDL_GPUShaderCreateInfo{.stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
                                                           .format = SDL_GPU_SHADERFORMAT_MSL,
                                                           .entrypoint = "main0",
                                                           .num_samplers = 0,
                                                           .num_storage_buffers = 0,
                                                           .num_storage_textures = 0,
                                                           .num_uniform_buffers = 0,
                                                           .code = code,
                                                           .code_size = codesize};

    SDL_GPUShader* vertex_shader = SDL_CreateGPUShader(Device, &vertex_shader_details);
    SDL_GPUShader* fragment_shader = SDL_CreateGPUShader(Device, &fragment_shader_details);

    auto pipeline_info = SDL_GPUGraphicsPipelineCreateInfo{
        .vertex_shader = vertex_shader,
        .fragment_shader = fragment_shader,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .rasterizer_state = {
            .fill_mode = SDL_GPU_FILLMODE_FILL,
            .cull_mode = SDL_GPU_CULLMODE_NONE,
            .front_face = SDL_GPU_FRONTFACE_CLOCKWISE
        },
        .target_info{
            .num_color_targets = 1,
            .color_target_descriptions = (SDL_GPUColorTargetDescription[]){
                {.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM}
            },
            .has_depth_stencil_target = true,
            .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
        },
        .vertex_input_state = {
            .num_vertex_attributes = 3,
            .num_vertex_buffers = 2,
            .vertex_attributes =
                (SDL_GPUVertexAttribute[]){
                    {.buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .location = 0, .offset = 0},
                    {.buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM, .location = 1, .offset = sizeof(float) * 3},
                    {.buffer_slot = 1, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .location = 2, .offset = 0}},
            .vertex_buffer_descriptions =
                (SDL_GPUVertexBufferDescription[]){
                    {.slot = 0, .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX, .instance_step_rate = 0, .pitch = sizeof(PositionAndColorVertex),},
                    {.slot = 1, .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX, .instance_step_rate = 0, .pitch = sizeof(VertexNormal)}},
        },
        .depth_stencil_state = {
            .compare_op = SDL_GPU_COMPAREOP_GREATER,
            .enable_depth_test = true,
            .enable_depth_write = true,
            .enable_stencil_test = false,
            .write_mask = 0xFF
        },
    };
    auto scene_pipeline = SDL_CreateGPUGraphicsPipeline(Device, &pipeline_info);

    SDL_ReleaseGPUShader(Device, vertex_shader);
    SDL_ReleaseGPUShader(Device, fragment_shader);

    auto vertex_buffer_info =
        SDL_GPUBufferCreateInfo{.size = (sizeof(PositionAndColorVertex)) * 1024, .usage = SDL_GPU_BUFFERUSAGE_VERTEX};
    auto vertex_buffer = SDL_CreateGPUBuffer(Device, &vertex_buffer_info);

    auto normal_buffer_info =
        SDL_GPUBufferCreateInfo{.size = (sizeof(VertexNormal)) * 1024, .usage = SDL_GPU_BUFFERUSAGE_VERTEX};
    auto normal_buffer = SDL_CreateGPUBuffer(Device, &normal_buffer_info);

    auto index_buffer_info =
        SDL_GPUBufferCreateInfo{.size = sizeof(glm::u16vec3) * 1024, .usage = SDL_GPU_BUFFERUSAGE_INDEX};
    auto index_buffer = SDL_CreateGPUBuffer(Device, &index_buffer_info);

    auto draw_buffer_info = SDL_GPUBufferCreateInfo{.size = sizeof(SDL_GPUIndexedIndirectDrawCommand) * 1024,
                                                    .usage = SDL_GPU_BUFFERUSAGE_INDIRECT};
    auto draw_buffer = SDL_CreateGPUBuffer(Device, &draw_buffer_info);

    auto depth_texture_info = SDL_GPUTextureCreateInfo {
        .format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
        .num_levels = 1,
        .layer_count_or_depth = 1,
        .height = 640,
        .width = 640,
        .type = SDL_GPU_TEXTURETYPE_2D,
        .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
    };
    auto depth_texture = SDL_CreateGPUTexture(Device, &depth_texture_info);

    auto color_texture_info = SDL_GPUTextureCreateInfo {
        .type = SDL_GPU_TEXTURETYPE_2D,
        .height = 640,
        .width = 640,
        .layer_count_or_depth = 1,
        .num_levels = 1,
        .sample_count = SDL_GPU_SAMPLECOUNT_1,
        .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
        .usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET
    };
    auto color_texture = SDL_CreateGPUTexture(Device, &color_texture_info);

    return Context{Window, Device, scene_pipeline, vertex_buffer, normal_buffer, index_buffer, draw_buffer, color_texture, depth_texture};
}

// Functions for scaffolding a Scene
auto CreateCube() {
    RenderableObject cube{};
    cube.vertices.reserve(8);
    cube.vertices.push_back(PositionAndColorVertex{{-1.0f, -1.0f, -1.0f}, {255, 255, 255, 255}}); // 0
    cube.vertices.push_back(PositionAndColorVertex{{-1.0f, -1.0f, 1.0f}, {255, 255, 255, 255}}); // 1
    cube.vertices.push_back(PositionAndColorVertex{{-1.0f, 1.0f, -1.0f}, {255, 255, 255, 255}}); // 2
    cube.vertices.push_back(PositionAndColorVertex{{-1.0f, 1.0f, 1.0f}, {255, 255, 255, 255}}); // 3
    cube.vertices.push_back(PositionAndColorVertex{{1.0f, -1.0f, -1.0f}, {255, 255, 255, 255}}); // 4
    cube.vertices.push_back(PositionAndColorVertex{{1.0f, -1.0f, 1.0f}, {255, 255, 255, 255}}); // 5
    cube.vertices.push_back(PositionAndColorVertex{{1.0f, 1.0f, -1.0f}, {255, 255, 255, 255}}); // 6
    cube.vertices.push_back(PositionAndColorVertex{{1.0f, 1.0f, 1.0f}, {255, 255, 255, 255}}); // 7

    cube.indices.reserve(12);
    cube.indices.emplace_back(0, 2, 3);
    cube.indices.emplace_back(3, 1, 0);
    cube.indices.emplace_back(3, 2, 6);
    cube.indices.emplace_back(6, 7, 3);
    cube.indices.emplace_back(1, 3, 7);
    cube.indices.emplace_back(7, 5, 1);
    cube.indices.emplace_back(1, 5, 4);
    cube.indices.emplace_back(4, 0, 1);
    cube.indices.emplace_back(7, 6, 4);
    cube.indices.emplace_back(4, 5, 7);
    cube.indices.emplace_back(0, 4, 6);
    cube.indices.emplace_back(6, 2, 0);

    CalculateVertexNormals(cube);

    cube.model = glm::identity<glm::mat4>();
    return cube;
}
auto CreateFlatPlane() {
    RenderableObject floor{};
    floor.vertices.reserve(64);
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            float x = (1.0f * i - 3.5f) * 2;
            float z = (1.0f * j - 3.5f) * 2;
            float y = -1.0f;
            floor.vertices.push_back(PositionAndColorVertex{{x, y, z}, {255, 255, 255, 255}});
        }
    }
    floor.indices.reserve(98);
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
            glm::uint16 target = 8 * i + j;
            floor.indices.emplace_back(target, target + 1, target + 8);
            floor.indices.emplace_back(target + 1, target + 8, target + 9);
        }
    }

    floor.model = glm::identity<glm::mat4>(); // no transform

    CalculateVertexNormals(floor);
    return floor;
}
auto CreateCamera() {
    CameraObject cam{};

    cam.camera_coords = {0.0f, 0.0f, 4.0f};
    cam.target_coords = {0.0f, 0.0f, 0.0f};

    cam.proj = Project(cam, glm::pi<float>() / 6, 1.0, 1.0, 0.0);
    cam.view = LookAt(cam, cam.target_coords);

    return cam;
}
auto UploadTestSceneData(Context* Context, RenderableObject& cube, RenderableObject& floor) {

    SDL_GPUIndexedIndirectDrawCommand draw_cube{
        .first_index = 0, .first_instance = 0, .num_indices = 36, .num_instances = 1, .vertex_offset = 0};
    SDL_GPUIndexedIndirectDrawCommand draw_floor = {
        .first_index = 36, .first_instance = 12, .num_indices = 98 * 3, .num_instances = 1, .vertex_offset = 8};

    const auto transfer_buffer_info = SDL_GPUTransferBufferCreateInfo{
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size =
            (sizeof(PositionAndColorVertex) * (8 + 64)) +
            (sizeof(VertexNormal) * (8 + 64)) +
            (sizeof(TriangleIndices) * (12 + 98)) +
            (sizeof(SDL_GPUIndexedIndirectDrawCommand) * (1 + 1))
    };

    auto transfer_buffer = SDL_CreateGPUTransferBuffer(Context->Device, &transfer_buffer_info);
    auto mapped_buffer = SDL_MapGPUTransferBuffer(Context->Device, transfer_buffer, false);
    auto cursor = static_cast<char*>(mapped_buffer);

    memcpy(cursor, cube.vertices.data(), (sizeof(PositionAndColorVertex)) * 8);
    cursor += (sizeof(PositionAndColorVertex)) * 8;
    memcpy(cursor, floor.vertices.data(), (sizeof(PositionAndColorVertex)) * 64);
    cursor += (sizeof(PositionAndColorVertex)) * 64;

    memcpy(cursor, cube.normals.data(), (sizeof(VertexNormal)) * 8);
    cursor += (sizeof(VertexNormal)) * 8;
    memcpy(cursor, floor.normals.data(), (sizeof(VertexNormal)) * 64);
    cursor += (sizeof(VertexNormal)) * 64;

    memcpy(cursor, cube.indices.data(), sizeof(TriangleIndices) * 12);
    cursor += sizeof(TriangleIndices) * 12;
    memcpy(cursor, floor.indices.data(), sizeof(TriangleIndices) * 98);
    cursor += sizeof(TriangleIndices) * 98;

    memcpy(cursor, &draw_cube, sizeof(SDL_GPUIndexedIndirectDrawCommand) * 1);
    cursor += sizeof(SDL_GPUIndexedIndirectDrawCommand) * 1;
    memcpy(cursor, &draw_floor, sizeof(SDL_GPUIndexedIndirectDrawCommand) * 1);
    cursor += sizeof(SDL_GPUIndexedIndirectDrawCommand) * 1;

    assert(cursor == static_cast<char*>(mapped_buffer) + transfer_buffer_info.size);

    SDL_UnmapGPUTransferBuffer(Context->Device, transfer_buffer);

    auto command_buffer = SDL_AcquireGPUCommandBuffer(Context->Device);
    auto copy_pass = SDL_BeginGPUCopyPass(command_buffer);

    auto transfer_buf_region = SDL_GPUTransferBufferLocation{.offset = 0, .transfer_buffer = transfer_buffer};
    auto vertex_buffer_region =
        SDL_GPUBufferRegion{.buffer = Context->VertexBuf, .offset = 0, .size = (sizeof(PositionAndColorVertex)) * 72};
    SDL_UploadToGPUBuffer(copy_pass, &transfer_buf_region, &vertex_buffer_region, false);

    transfer_buf_region.offset = (sizeof(PositionAndColorVertex)) * 72;
    auto normal_buffer_region =
        SDL_GPUBufferRegion{.buffer = Context->NormalBuf, .offset = 0, .size = sizeof(VertexNormal) * 72};
    SDL_UploadToGPUBuffer(copy_pass, &transfer_buf_region, &normal_buffer_region, false);

    transfer_buf_region.offset = (sizeof(PositionAndColorVertex)) * 72 + (sizeof(VertexNormal)) * 72;
    auto index_buffer_region =
        SDL_GPUBufferRegion{.buffer = Context->IndexBuf, .offset = 0, .size = sizeof(TriangleIndices) * 110};
    SDL_UploadToGPUBuffer(copy_pass, &transfer_buf_region, &index_buffer_region, false);

    transfer_buf_region.offset =
        (sizeof(PositionAndColorVertex)) * 72 + (sizeof(VertexNormal)) * 72 + sizeof(TriangleIndices) * 110;
    auto draw_buffer_region = SDL_GPUBufferRegion{
        .buffer = Context->DrawBuf, .offset = 0, .size = sizeof(SDL_GPUIndexedIndirectDrawCommand) * 2};
    SDL_UploadToGPUBuffer(copy_pass, &transfer_buf_region, &draw_buffer_region, false);

    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(command_buffer);
    SDL_ReleaseGPUTransferBuffer(Context->Device, transfer_buffer);

}
auto InitTestScene(Context* Context) {

    // Define objects
    std::vector<RenderableObject> Objects;

    Objects.emplace_back(CreateCube());
    Objects.emplace_back(CreateFlatPlane());

    auto& cube = Objects[0];
    auto& floor = Objects[1];

    auto Camera = CreateCamera();

    // Upload Scene Data to GPU
    UploadTestSceneData(Context, cube, floor);

    return Scene{Objects, Camera};
}

// Lifecycle methods in Scene Loop
auto HandleEvents(Context& c, Scene& s, KeyboardState& k, int& status) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
            status = 1;
        }
        if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.key == SDLK_W) {
                k.w = true;
            }
            if (event.key.key == SDLK_A) {
                k.a = true;
            }
            if (event.key.key == SDLK_S) {
                k.s = true;
            }
            if (event.key.key == SDLK_D) {
                k.d = true;
            }
            if (event.key.key == SDLK_Q) {
                k.q = true;
            }
            if (event.key.key == SDLK_E) {
                k.e = true;
            }
            if (event.key.key == SDLK_F) {
                k.f = true;
            }
            if (event.key.key == SDLK_G) {
                k.g = true;
            }
            if (event.key.key == SDLK_TAB) {
                k.cam_mode = !k.cam_mode;
            }
            if (event.key.key == SDLK_R) {
                s.Camera.proj = Project(s.Camera, glm::pi<float>() / 6, 1.0, 1.0, 0.0);
                s.Camera.view = LookAt(s.Camera, s.Camera.target_coords);
                s.Camera.camera_coords = {0.0, 0.0, 4.0};
                s.Objects[0].model = glm::identity<glm::mat4>();
            }
            if (event.key.key == SDLK_SPACE) {
                glm::vec4 target_vert {-1.0f, -1.0f, -1.0f, 1.0f};
                target_vert = s.Objects[0].model * target_vert;
                target_vert = s.Camera.view * target_vert;
                target_vert = s.Camera.proj * target_vert;
                std::cout << "Screen Space Coords of Vertex at (-1, -1, -1):\n";
                std::cout << std::format("  {}, {}, {}, {}\n",
                    target_vert.x,
                    target_vert.y,
                    target_vert.z,
                    target_vert.w
                );
            }
        }
        if (event.type == SDL_EVENT_KEY_UP) {
            if (event.key.key == SDLK_W) {
                k.w = false;
            }
            if (event.key.key == SDLK_A) {
                k.a = false;
            }
            if (event.key.key == SDLK_S) {
                k.s = false;
            }
            if (event.key.key == SDLK_D) {
                k.d = false;
            }
            if (event.key.key == SDLK_Q) {
                k.q = false;
            }
            if (event.key.key == SDLK_E) {
                k.e = false;
            }
            if (event.key.key == SDLK_F) {
                k.f = false;
            }
            if (event.key.key == SDLK_G) {
                k.g = false;
            }
        }
    }
}
auto Update(Context& c, Scene& s, KeyboardState& k, int& status, float& fov_scale) {
    auto& [Window, Device, Pipeline, VB, NB, IB, DB, ColorTex, DepthTex] = c;
    auto& [Objects, Camera] = s;

    if (k.cam_mode) {
        if (k.w) {
            DollyCamera(Camera, 1.0);
        }
        if (k.a) {
            OrbitCameraLaterally(Camera, -1.0);
        }
        if (k.s) {
            DollyCamera(Camera, -1.0);
        }
        if (k.d) {
            OrbitCameraLaterally(Camera, 1.0);
        }
        if (k.q) {
            RaiseOrLowerCamera(Camera, 1.0);
        }
        if (k.e) {
            RaiseOrLowerCamera(Camera, -1.0);
        }
        if (k.f) {
            auto fov_scale_percent = 1.0f + 0.1f * (fov_scale - 1.0f);
            if (fov_scale_percent < 1.10) {
                fov_scale += 0.01;
                fov_scale_percent = 1.0f + 0.1f * (fov_scale - 1.0f);
            }
            Project(Camera, fov_scale_percent * glm::pi<float>() / 6, 1, 1, 0);
        }
        if (k.g) {
            auto fov_scale_percent = 1.0f + 0.1f * (fov_scale - 1.0f);
            if (fov_scale_percent > 0.85f) {
                fov_scale -= 0.01;
                fov_scale_percent = 1.0f + 0.1f * (fov_scale - 1.0f);
            }
            Project(Camera, fov_scale_percent * glm::pi<float>() / 6, 1, 1, 0);
        }
    }
    else {
        if (k.w) {
            RotateModelInPlace(Objects[0], glm::pi<float>() / 256, {1.0f, 0.0f, 0.0f});
        }
        if (k.a) {
            RotateModelInPlace(Objects[0], glm::pi<float>() / 256, {0.0f, 1.0f, 0.0f});
        }
        if (k.s) {
            RotateModelInPlace(Objects[0], -glm::pi<float>() / 256, {1.0f, 0.0f, 0.0f});
        }
        if (k.d) {
            RotateModelInPlace(Objects[0], -glm::pi<float>() / 256, {0.0f, 1.0f, 0.0f});
        }
        if (k.q) {
            RotateModelInPlace(Objects[0], -glm::pi<float>() / 256, {0.0f, 0.0f, 1.0f});
        }
        if (k.e) {
            RotateModelInPlace(Objects[0], glm::pi<float>() / 256, {0.0f, 0.0f, 1.0f});
        }
        if (k.f) {
            ScaleModel(Objects[0], {0.99, 0.99, 0.99});
        }
        if (k.g) {
            ScaleModel(Objects[0], {1.01, 1.01, 1.01});
        }
    }

    Camera.view = LookAt(Camera, Camera.target_coords);
}
auto Draw(Context& c, Scene& s, KeyboardState& k, int& status) {
    auto& [Window, Device, Pipeline, VB, NB, IB, DB, ColorTex, DepthTex] = c;
    auto& [Objects, Camera] = s;

    auto cmdbuf = SDL_AcquireGPUCommandBuffer(Device);

    auto swapchain = (SDL_GPUTexture*){nullptr};
    SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, Window, &swapchain, nullptr, nullptr);

    SDL_GPUColorTargetInfo swapchain_target = { nullptr };
    swapchain_target.texture = swapchain;
    swapchain_target.clear_color = (SDL_FColor){ 0.0f, 0.0f, 0.0f, 1.0f };
    swapchain_target.load_op = SDL_GPU_LOADOP_CLEAR;
    swapchain_target.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPUColorTargetInfo color_target = { nullptr };
    color_target.texture = ColorTex;
    color_target.clear_color = SDL_FColor{0.0, 0.0, 0.0, 1.0};
    color_target.load_op = SDL_GPU_LOADOP_CLEAR;
    color_target.store_op = SDL_GPU_STOREOP_STORE;

    auto depth_target = SDL_GPUDepthStencilTargetInfo { nullptr };
    depth_target.texture = DepthTex;
    depth_target.cycle = true;
    depth_target.clear_depth = 0;
    depth_target.clear_stencil = 0;
    depth_target.load_op = SDL_GPU_LOADOP_CLEAR;
    depth_target.store_op = SDL_GPU_STOREOP_STORE;
    depth_target.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
    depth_target.stencil_store_op = SDL_GPU_STOREOP_STORE;

    auto uniform_data = VertexUniformBufferData{.model = Objects[0].model, .view = Camera.view, .proj = Camera.proj};
    SDL_PushGPUVertexUniformData(cmdbuf, 0, &uniform_data, sizeof(VertexUniformBufferData));

    auto rp = SDL_BeginGPURenderPass(cmdbuf, &swapchain_target, 1, &depth_target);

    const auto v_bind = SDL_GPUBufferBinding{.buffer = VB, .offset = 0};
    const auto n_bind = SDL_GPUBufferBinding{.buffer = NB, .offset = 0};
    const auto i_bind = SDL_GPUBufferBinding{.buffer = IB, .offset = 0};
    const SDL_GPUBufferBinding v_bufs[] = {v_bind, n_bind};

    SDL_BindGPUVertexBuffers(rp, 0, v_bufs, 2);
    SDL_BindGPUIndexBuffer(rp, (SDL_GPUBufferBinding[]){i_bind}, SDL_GPU_INDEXELEMENTSIZE_16BIT);
    SDL_BindGPUGraphicsPipeline(rp, Pipeline);

    SDL_DrawGPUIndexedPrimitivesIndirect(rp, DB, 0, 1);

    uniform_data.model = Objects[1].model;
    SDL_PushGPUVertexUniformData(cmdbuf, 0, &uniform_data, sizeof(VertexUniformBufferData));

    SDL_DrawGPUIndexedPrimitivesIndirect(rp, DB, sizeof(SDL_GPUIndexedIndirectDrawCommand), 1);

    SDL_EndGPURenderPass(rp);
    SDL_SubmitGPUCommandBuffer(cmdbuf);
}

// Wrappers for the lifecycle of a unique scene
auto RunTestScene() {
    int status = 0;
    auto Context = InitContext();
    auto Scene = InitTestScene(&Context);

    auto& [Window, Device, Pipeline, VB, NB, IB, DB, ColorTex, DepthTex] = Context;
    auto& [Objects, Camera] = Scene;

    auto time_start = GetTimePoint();

    KeyboardState Inputs{};

    float fov_scale = 1.0f;

    while (status == 0) {
        HandleEvents(Context, Scene, Inputs, status);
        Update(Context, Scene, Inputs, status, fov_scale);
        Draw(Context, Scene, Inputs, status);
    }

    SDL_ReleaseGPUBuffer(Device, VB);
    SDL_ReleaseGPUBuffer(Device, IB);
    SDL_ReleaseGPUBuffer(Device, DB);
    SDL_ReleaseWindowFromGPUDevice(Device, Window);
    SDL_DestroyGPUDevice(Device);
    SDL_DestroyWindow(Window);
    SDL_Quit();

    return 0;
}

int main(int argc, char** argv) {
    try {
        return RunTestScene();
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        return -1;
    }
}

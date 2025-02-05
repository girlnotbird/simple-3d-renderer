#ifndef INC_3D_RENDERING_ENGINE_HPP
#define INC_3D_RENDERING_ENGINE_HPP
#include <cassert>
#include <iostream>

#include "common.hpp"

#include <__filesystem/path.h>
#include <vector>

class CameraObject {};

class RenderableObject {
public:
    std::vector<Vector4> vertex_position_data{};
    // pass in 4 floats carrying pre-transform position data of a vertex
    std::vector<Vector4> vertex_color_data{};
    // pass in 4 floats carrying color data of a vertex

    struct TransformData {
        Vector3 xyz_scaling_data;
        Vector4 xyztheta_rotation_data;
        Vector3 xyz_translation_data;
    };

    void Transform(TransformData transform_data) {
        auto& [scaling_data, axis_angle_rotation_data, translation_data] = transform_data;
        const Matrix4x4 scale_matrix = CreateScalingMatrix(scaling_data.v1, scaling_data.v2, scaling_data.v3);
        const Quaternion rotation_quat = CreateRotationQuaternion(
            Vector4{axis_angle_rotation_data.v1, axis_angle_rotation_data.v2, axis_angle_rotation_data.v3, 0.0f},
            axis_angle_rotation_data.v4);
        const Matrix4x4 translation_matrix =
            CreateTranslationMatrix(translation_data.v1, translation_data.v2, translation_data.v3);
        for (auto& vertex_pos : vertex_position_data) {
            vertex_pos = MultiplyMatrixByVector(scale_matrix, vertex_pos);
            vertex_pos = RotateVectorByQuaternion(rotation_quat, vertex_pos);
            vertex_pos = MultiplyMatrixByVector(translation_matrix, vertex_pos);
        }
    }
};

class RenderingEngine {
public:
    // Type aliases
    using LoadedObjectList = std::vector<RenderableObject>;

    // GPU Device, Pipeline, & Window Handles
    static SDL_GPUDevice* Device;
    static SDL_GPUGraphicsPipeline* Pipeline;
    static SDL_Window* Window;

    // Filesystem entrypoints
    static std::string ShaderBasePath;

    // GPU Buffer Handles & sizes
    static SDL_GPUBuffer* VertexBuffer;
    static SDL_GPUBuffer* IndexBuffer;
    static SDL_GPUBuffer* DrawBuffer;
    static Uint32 VertexBufferSize;
    static Uint32 IndexBufferSize;
    static Uint32 DrawBufferSize;

    // Engine-Tracked State
    static LoadedObjectList Objects;

    static LoadedObjectList::size_type LoadObject(RenderableObject&& new_obj) {
        Objects.emplace_back(std::move(new_obj));
    }

    static SDL_GPUShader* LoadShader(const std::string& shaderFilename, const Uint32 samplerCount,
                                     const Uint32 uniformBufferCount, const Uint32 storageBufferCount,
                                     const Uint32 storageTextureCount) {
        // Auto-detect the shader stage from the file name
        SDL_GPUShaderStage stage;
        if (SDL_strstr(shaderFilename.c_str(), ".vert")) {
            stage = SDL_GPU_SHADERSTAGE_VERTEX;
        }
        else if (SDL_strstr(shaderFilename.c_str(), ".frag")) {
            stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
        }
        else {
            std::cerr << "Invalid shader stage!\n";
            return nullptr;
        }

        std::string fullPath;
        const SDL_GPUShaderFormat legal_device_shader_formats = SDL_GetGPUShaderFormats(Device);
        SDL_GPUShaderFormat shader_format = SDL_GPU_SHADERFORMAT_INVALID;
        std::string entrypoint;

        if (legal_device_shader_formats & SDL_GPU_SHADERFORMAT_MSL) {
            fullPath.append(ShaderBasePath);
            fullPath.append("/shaders/compiled/");
            fullPath.append(shaderFilename);
            fullPath.append(".msl\0");
            shader_format = SDL_GPU_SHADERFORMAT_MSL;
            entrypoint = "main0";
        }
        else {
            // TODO: Add Support for other shader formats through ShaderCross?
            std::cerr << "Incompatible backend shader format!\n";
            return nullptr;
        }

        std::size_t codeSize;
        auto code = static_cast<Uint8*>(SDL_LoadFile(fullPath.c_str(), &codeSize));
        if (code == nullptr) {
            std::cerr << "Failed to load shader from disk @ path <" << fullPath << ">\n";
            return nullptr;
        }

        SDL_GPUShaderCreateInfo shaderInfo = {
            .code_size = codeSize,
            .code = code,
            .entrypoint = entrypoint.c_str(),
            .format = shader_format,
            .stage = stage,
            .num_samplers = samplerCount,
            .num_storage_textures = storageTextureCount,
            .num_storage_buffers = storageBufferCount,
            .num_uniform_buffers = uniformBufferCount,
        };

        SDL_GPUShader* shader = SDL_CreateGPUShader(Device, &shaderInfo);
        SDL_free(code);

        if (shader == nullptr) {
            std::cerr << "Failed to create shader!\n";
            return nullptr;
        }

        return shader;
    };

    static ApplicationStatus InitializeEngine(const bool Debug = true) {
        // Initialize SDL Video Subsystem
        SDL_Init(SDL_INIT_VIDEO);

        // Initialize Shader Base Path
        if (Debug) {
            auto launch_path = std::filesystem::path(SDL_GetBasePath());
            while (SDL_strstr(launch_path.c_str(), "build")) {
                launch_path = launch_path.parent_path();
            }
            ShaderBasePath = launch_path.string();
        }
        else {
            std::cerr << "No Non-Debug Shader Base Path. WIP!\n";
            return APP_FAILURE;
        }

        // Get handle to GPU Device & claim Window
        if (Debug) {
            // Set up device for my personal machine
            if (Device == nullptr) {
                Device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_MSL, true, "metal");
                if (Device == nullptr) {
                    std::cerr << "Error Creating GPU Context:\n" << SDL_GetError() << "\n";
                    return APP_FAILURE;
                }
                if (Window == nullptr) {
                    Window = SDL_CreateWindow("The Bad Decade", 640, 640, 0);
                    if (Window == nullptr) {
                        std::cerr << "Error Creating Window:\n" << SDL_GetError() << "\n";
                        return APP_FAILURE;
                    }
                    const bool gpu_context_has_claimed_window = SDL_ClaimWindowForGPUDevice(Device, Window);
                    if (!gpu_context_has_claimed_window) {
                        std::cerr << "Error Claiming Window for GPU Context:\n" << SDL_GetError() << "\n";
                        return APP_FAILURE;
                    }
                }
            }
        }
        else {
            std::cerr << "No non-debug devices are set up yet! WIP!";
            return APP_FAILURE;
        }

        // Temporarily load shaders into memory
        SDL_GPUShader* VertexShader;
        SDL_GPUShader* FragmentShader;

        // Initialize Shaders
        if (Debug) {
            // Initialize Debug Wireframe Shaders
            VertexShader = LoadShader("DebugDraw.vert", 0, 0, 0, 0);
            if (VertexShader == nullptr) {
                std::cerr << "Failed to create vertex shader!\n";
                return APP_FAILURE;
            }
            FragmentShader = LoadShader("SoloWhite.frag", 0, 0, 0, 0);
            if (FragmentShader == nullptr) {
                std::cerr << "Failed to create fragment shader!\n";
                return APP_FAILURE;
            }
        }
        else {
            std::cerr << "No non-debug shaders are set up yet! WIP!\n";
            return APP_FAILURE;
        }

        // Create the Pipeline
        if (Debug) {
            SDL_GPUGraphicsPipelineCreateInfo pipeline_info{
                .vertex_shader = VertexShader,
                .fragment_shader = FragmentShader,
                .target_info =
                    {
                        .color_target_descriptions = (SDL_GPUColorTargetDescription[]){(SDL_GPUColorTargetDescription){
                            .format = SDL_GetGPUSwapchainTextureFormat(Device, Window),
                        }},
                        .num_color_targets = 1,
                    },
                .vertex_input_state =
                    {
                        .vertex_buffer_descriptions =
                            (SDL_GPUVertexBufferDescription[]){(SDL_GPUVertexBufferDescription){
                                .slot = 0,
                                .pitch = sizeof(Vector3),
                                // only 3 floats for position, no color data
                                .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
                                .instance_step_rate = 0,
                                // not used with VERTEXINPUTRATE_VERTEX
                            }},
                        .num_vertex_buffers = 1,
                        .vertex_attributes = (SDL_GPUVertexAttribute[]){(SDL_GPUVertexAttribute){
                            .location = 0,
                            .buffer_slot = 0,
                            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                            .offset = 0,
                        }},
                        .num_vertex_attributes = 1,
                    },
                .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
                .rasterizer_state = (SDL_GPURasterizerState){.fill_mode = SDL_GPU_FILLMODE_LINE},
            };
            Pipeline = SDL_CreateGPUGraphicsPipeline(Device, &pipeline_info);
            if (Pipeline == nullptr) {
                std::cerr << "Failed to create pipeline!\n";
                return APP_FAILURE;
            }
        }
        else {
            std::cerr << "No non-debug pipeline is set up yet! WIP!\n";
            return APP_FAILURE;
        }

        // Release temporary resources
        SDL_ReleaseGPUShader(Device, VertexShader);
        SDL_ReleaseGPUShader(Device, FragmentShader);

        // Create Buffers
        if (Debug) {
            // Create Vertex Buffer for 1000 vertices
            VertexBufferSize = sizeof(Vector3) * 1000; // 12kb
            SDL_GPUBufferCreateInfo vertex_buffer_info{
                .size = VertexBufferSize,
                .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
            };
            VertexBuffer = SDL_CreateGPUBuffer(Device, &vertex_buffer_info);
            if (VertexBuffer == nullptr) {
                std::cout << "Failed to create Vertex buffer:\n" << SDL_GetError() << "\n";
                return APP_FAILURE;
            }

            // Create Index Buffer for 300 primitives
            IndexBufferSize = sizeof(Uint16) * 3 * 300; // 1.8kb
            SDL_GPUBufferCreateInfo index_buffer_info{
                .size = IndexBufferSize,
                .usage = SDL_GPU_BUFFERUSAGE_INDEX,
            };
            IndexBuffer = SDL_CreateGPUBuffer(Device, &index_buffer_info);
            if (IndexBuffer == nullptr) {
                std::cout << "Failed to create Index buffer:\n" << SDL_GetError() << "\n";
                return APP_FAILURE;
            }

            // Create Draw Buffer for one SDL_GPUIndexedIndirectDrawCommand——
            //     draw a single primitive from 3 indices
            DrawBufferSize = sizeof(SDL_GPUIndexedIndirectDrawCommand) * 1;
            SDL_GPUBufferCreateInfo draw_buffer_info{
                .size = DrawBufferSize,
                .usage = SDL_GPU_BUFFERUSAGE_INDIRECT,
            };
            DrawBuffer = SDL_CreateGPUBuffer(Device, &draw_buffer_info);
            if (DrawBuffer == nullptr) {
                std::cout << "Failed to create Draw Command buffer:\n" << SDL_GetError() << "\n";
                return APP_FAILURE;
            }
        }
        else {
            std::cerr << "No non-debug buffers set up yet! WIP!\n";
            return APP_FAILURE;
        }

        // Create a Transfer Buffer
        SDL_GPUTransferBuffer* TransferBuffer;

        // Initialize Buffer Data
        if (Debug) {
            SDL_GPUTransferBufferCreateInfo transfer_buffer_info{
                .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                .size = VertexBufferSize + IndexBufferSize + DrawBufferSize,
            };
            TransferBuffer = SDL_CreateGPUTransferBuffer(Device, &transfer_buffer_info);
            if (TransferBuffer == nullptr) {
                std::cerr << "Failed to get transfer buffer during init.\n";
                return APP_FAILURE;
            }
            void* MapBufferOrigin = SDL_MapGPUTransferBuffer(Device, TransferBuffer, false);
            if (MapBufferOrigin == nullptr) {
                std::cerr << "Failed to get RAM for mapping transfer buffer.\n";
                return APP_FAILURE;
            }
            void* MapBufferCursor = MapBufferOrigin;

            // Add Data for VertexBuffer
            constexpr auto empty_vert_data = (Vector3){0.0f, 0.0f, 0.0f};
            for (auto i = 0; i < VertexBufferSize;) {
                memcpy(MapBufferCursor, &empty_vert_data, sizeof(empty_vert_data));
                MapBufferCursor = static_cast<char*>(MapBufferCursor) + sizeof(empty_vert_data);
                i += sizeof(empty_vert_data);
            }

            // Add Data for IndexBuffer
            Uint16 empty_index_data = 0;
            for (auto i = 0; i < IndexBufferSize;) {
                memcpy(MapBufferCursor, &empty_index_data, sizeof(empty_index_data));
                MapBufferCursor = static_cast<char*>(MapBufferCursor) + sizeof(empty_index_data);
                i += 1 * sizeof(empty_index_data);
            }

            // Add Data for DrawBuffer
            SDL_GPUIndexedIndirectDrawCommand draw_command{
                .num_indices = 3, .num_instances = 1, .first_index = 0, .vertex_offset = 0, .first_instance = 0};
            memcpy(MapBufferCursor, &draw_command, sizeof(draw_command));

            SDL_UnmapGPUTransferBuffer(Device, TransferBuffer);
        }
        else {
            std::cerr << "No Non-Debug Transfer Buffer Configuration! WIP!\n";
            return APP_FAILURE;
        }

        // Create a Command Buffer & Start a Copy Pass
        SDL_GPUCommandBuffer* CommandBuffer;
        SDL_GPUCopyPass* CopyPass;

        // Write Upload Commands to Command Buffer
        if (Debug) {
            CommandBuffer = SDL_AcquireGPUCommandBuffer(Device);
            CopyPass = SDL_BeginGPUCopyPass(CommandBuffer);

            SDL_GPUTransferBufferLocation transfer_buffer_cursor{.transfer_buffer = TransferBuffer, .offset = 0};

            SDL_GPUBufferRegion vertex_buffer_region{
                .buffer = VertexBuffer,
                .offset = 0,
                .size = VertexBufferSize,
            };
            SDL_UploadToGPUBuffer(CopyPass, &transfer_buffer_cursor, &vertex_buffer_region, false);
            transfer_buffer_cursor.offset += VertexBufferSize;

            SDL_GPUBufferRegion index_buffer_region{
                .buffer = IndexBuffer,
                .offset = 0,
                .size = IndexBufferSize,
            };
            SDL_UploadToGPUBuffer(CopyPass, &transfer_buffer_cursor, &index_buffer_region, false);
            transfer_buffer_cursor.offset += IndexBufferSize;

            SDL_GPUBufferRegion draw_buffer_region{
                .buffer = DrawBuffer,
                .offset = 0,
                .size = DrawBufferSize,
            };
            SDL_UploadToGPUBuffer(CopyPass, &transfer_buffer_cursor, &draw_buffer_region, false);
        }
        else {
            std::cerr << "No Non-Debug Buffer Upload Configuration! WIP!\n";
            return APP_FAILURE;
        }

        // Submit End Pass, Submit Buffer, Release TransferBuffer
        SDL_EndGPUCopyPass(CopyPass);
        SDL_SubmitGPUCommandBuffer(CommandBuffer);
        SDL_ReleaseGPUTransferBuffer(Device, TransferBuffer);

        return APP_CONTINUE;
    }

    static ApplicationStatus TransferVertexData(const bool Debug = true) {
        SDL_GPUTransferBuffer* TransferBuffer;

        Uint32 num_vertices{0};
        Uint32 vertex_region_size{0};
        Uint32 num_indices{0}; // 3 indices per primitive
        Uint32 index_region_size{0};

        void* vertex_data_cursor{nullptr};
        void* index_data_cursor{nullptr};

        // Create & Map a Transfer Buffer
        if (Debug) {
            for (auto& obj : Objects) {
                num_vertices += obj.vertex_position_data.size();
                num_indices += (obj.vertex_position_data.size() - 2) * 3;
                // Every vertex beyond 2 creates a new primitive in a triangle strip
            }
            vertex_region_size = num_vertices * sizeof(Vector3);
            index_region_size = num_indices * sizeof(Uint16);
            SDL_GPUTransferBufferCreateInfo transfer_buffer_info{
                .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                .size = vertex_region_size + index_region_size,
            };
            TransferBuffer = SDL_CreateGPUTransferBuffer(Device, &transfer_buffer_info);
            if (TransferBuffer == nullptr) {
                std::cerr << "Failed to get transfer buffer during UploadObjectToGPU fn.\n";
                return APP_FAILURE;
            }
            void* MapBufferOrigin = SDL_MapGPUTransferBuffer(Device, TransferBuffer, false);
            if (MapBufferOrigin == nullptr) {
                std::cerr << "Failed to get RAM for mapping transfer buffer during UploadObjectToGPU fn.\n";
                return APP_FAILURE;
            }
            vertex_data_cursor = MapBufferOrigin;
            index_data_cursor = static_cast<char*>(MapBufferOrigin) + vertex_region_size;
        }
        else {
            std::cerr << "No Protocol for mapping a non-debug transfer buffer during UploadObjectToGPU! WIP!";
            return APP_FAILURE;
        }

        // Write Object Data into transfer buffer.
        if (Debug) {
            auto object_index{0};
            auto memory_used{0};
            while (memory_used < vertex_region_size + index_region_size) {
                auto& obj = Objects[object_index];
                Uint16 vertex_index{0};
                Uint16 index_index{0};
                while (vertex_index < obj.vertex_position_data.size()) {
                    auto vertex = obj.vertex_position_data[vertex_index];
                    memcpy(vertex_data_cursor, &vertex, sizeof(float) * 3); // copy x, y, and z from vertex
                    memory_used += sizeof(float) * 3;
                    vertex_data_cursor = static_cast<char*>(vertex_data_cursor) + sizeof(float) * 3;
                    if (vertex_index < 3) {
                        memcpy(index_data_cursor, &vertex_index, sizeof(Uint16));
                        memory_used += sizeof(Uint16);
                        index_data_cursor = static_cast<char*>(index_data_cursor) + sizeof(Uint16);
                        vertex_index += 1;
                        index_index += 1;
                        continue;
                    }
                    static_cast<Uint16*>(index_data_cursor)[0] = vertex_index - 2;
                    static_cast<Uint16*>(index_data_cursor)[1] = vertex_index - 1;
                    static_cast<Uint16*>(index_data_cursor)[2] = vertex_index;
                    memory_used += sizeof(Uint16) * 3;
                    index_data_cursor = static_cast<char*>(index_data_cursor) + sizeof(Uint16) * 3;
                    vertex_index += 1;
                    index_index += 3;
                }
                object_index += 1;
                assert(object_index < Objects.size());
                assert(vertex_index < num_vertices);
                assert(index_index < num_indices);
            }
        }
        else {
            std::cerr << "No Non-Debug Transfer Buffer Mapping Procedure! WIP!\n";
            return APP_FAILURE;
        }

        SDL_UnmapGPUTransferBuffer(Device, TransferBuffer);

        // Create a Command Buffer & Start a Copy Pass
        SDL_GPUCommandBuffer* CommandBuffer;
        SDL_GPUCopyPass* CopyPass;

        // Write Upload Commands to Command Buffer
        if (Debug) {
            CommandBuffer = SDL_AcquireGPUCommandBuffer(Device);
            CopyPass = SDL_BeginGPUCopyPass(CommandBuffer);

            SDL_GPUTransferBufferLocation transfer_buffer_cursor{.transfer_buffer = TransferBuffer, .offset = 0};

            SDL_GPUBufferRegion vertex_buffer_region{
                .buffer = VertexBuffer,
                .offset = 0,
                .size = vertex_region_size,
            };
            SDL_UploadToGPUBuffer(CopyPass, &transfer_buffer_cursor, &vertex_buffer_region, false);
            transfer_buffer_cursor.offset += vertex_region_size;

            SDL_GPUBufferRegion index_buffer_region{
                .buffer = IndexBuffer,
                .offset = 0,
                .size = index_region_size,
            };
            SDL_UploadToGPUBuffer(CopyPass, &transfer_buffer_cursor, &index_buffer_region, false);
        }
        else {
            std::cerr << "No Non-Debug Buffer Upload Configuration! WIP!\n";
            return APP_FAILURE;
        }

        // Submit End Pass, Submit Buffer, Release TransferBuffer
        SDL_EndGPUCopyPass(CopyPass);
        SDL_SubmitGPUCommandBuffer(CommandBuffer);
        SDL_ReleaseGPUTransferBuffer(Device, TransferBuffer);

        return APP_CONTINUE;
    }

    static ApplicationStatus DrawFrame(const bool Debug = true) {
        SDL_GPUCommandBuffer* CommandBuffer;
        SDL_GPURenderPass* RenderPass;
        if (Debug) {
            // Get a Command Buffer
            CommandBuffer = SDL_AcquireGPUCommandBuffer(Device);
            if (CommandBuffer == nullptr) {
                std::cerr << "AcquireGPUCommandBuffer failed.\n" << SDL_GetError();
                return APP_FAILURE;
            }

            // Get swapchain texture from Window
            SDL_GPUTexture* swapchainTexture;
            bool successfully_retrieved_swapchain_texture =
                SDL_WaitAndAcquireGPUSwapchainTexture(CommandBuffer, Window, &swapchainTexture, nullptr, nullptr);
            if (!successfully_retrieved_swapchain_texture) {
                std::cerr << "WaitAndAcquireGPUSwapchainTexture failed:\n" << SDL_GetError();
                return APP_FAILURE;
            }
            if (swapchainTexture == nullptr) {
                SDL_SubmitGPUCommandBuffer(CommandBuffer);
                return APP_CONTINUE;
            }

            // Set swapchain texture as rendering target.
            SDL_GPUColorTargetInfo color_target_info = {
                .texture = swapchainTexture,
                .clear_color = {0.0f, 0.0f, 0.0f, 1.0f},
                .load_op = SDL_GPU_LOADOP_CLEAR,
                .store_op = SDL_GPU_STOREOP_STORE,
            };

            SDL_GPUBufferBinding vertex_buffer_binding{
                .buffer = VertexBuffer,
                .offset = 0,
            };
            SDL_GPUBufferBinding index_buffer_binding{
                .buffer = IndexBuffer,
                .offset = 0,
            };

            RenderPass = SDL_BeginGPURenderPass(CommandBuffer, &color_target_info, 1, nullptr);
            SDL_BindGPUGraphicsPipeline(RenderPass, Pipeline);
            SDL_BindGPUVertexBuffers(RenderPass, 0, &vertex_buffer_binding, 1);
            SDL_BindGPUIndexBuffer(RenderPass, &index_buffer_binding, SDL_GPU_INDEXELEMENTSIZE_16BIT);

            Uint16 starting_index = 0;
            Uint16 instance_num = 0;
            for (auto& obj : Objects) {
                SDL_DrawGPUIndexedPrimitives(RenderPass, 3, obj.vertex_position_data.size() - 2, starting_index, 0,
                                             instance_num);
                starting_index += (obj.vertex_position_data.size() - 2) * 3;
                instance_num += obj.vertex_position_data.size() - 2;
            }
        }
        else {
            std::cerr << "No non-debug DrawFrame procedure implemented. WIP!\n";
            return APP_FAILURE;
        }
        SDL_EndGPURenderPass(RenderPass);
        SDL_SubmitGPUCommandBuffer(CommandBuffer);
        return APP_CONTINUE;
    }

    static ApplicationStatus UpdateObjects(const bool Debug = true) {
        return APP_CONTINUE;
    };

    static void QuitEngine() {
        SDL_ReleaseGPUGraphicsPipeline(Device, Pipeline);
        SDL_ReleaseGPUBuffer(Device, VertexBuffer);
        SDL_ReleaseGPUBuffer(Device, IndexBuffer);
        SDL_ReleaseGPUBuffer(Device, DrawBuffer);
        SDL_ReleaseWindowFromGPUDevice(Device, Window);
        SDL_DestroyWindow(Window);
        SDL_DestroyGPUDevice(Device);
        SDL_Quit();
    }
};

#endif // INC_3D_RENDERING_ENGINE_HPP
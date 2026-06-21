#include "metal_backend.hpp"
#include "gpu_render_config.hpp"
#include "colors.hpp"

#define Point AppleLegacyPoint
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#undef Point

#include <algorithm>
#include <array>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

std::string nsstring_to_string(NSString *string) {
  // This stores the UTF-8 byte sequence in a std::string, but non-ASCII
  // characters may still display incorrectly in the current CLI/error path.
  if (string == nil) {
    return std::string{};
  }

  const char *utf8 = [string UTF8String];
  return (utf8 != nullptr) ? std::string{utf8} : std::string{};
}

std::string describe_nserror(NSError *error) {
  if (error == nil) {
    return {};
  }

  std::string description = nsstring_to_string([error localizedDescription]);
  if (!description.empty()) {
    return description;
  }

  return "Unknown Metal error";
}

std::runtime_error metal_error(const std::string &message, NSError *error = nil) {
  const std::string details = describe_nserror(error);
  if (details.empty()) {
    return std::runtime_error{message};
  }

  return std::runtime_error{message + ": " + details};
}

std::filesystem::path executable_directory() {
  @autoreleasepool {
    NSString *executable_path = [[[NSProcessInfo processInfo] arguments] firstObject];
    if (executable_path == nil) {
      return {};
    }

    return std::filesystem::path{nsstring_to_string([executable_path stringByStandardizingPath])}.parent_path();
  }
}

id<MTLBuffer> make_buffer(id<MTLDevice> device, const void *bytes, std::size_t length, NSString *label) {
  id<MTLBuffer> buffer = nil;

  if (length == 0) {
    buffer = [device newBufferWithLength:1u options:MTLResourceStorageModeShared];
  } else {
    buffer = [device newBufferWithBytes:bytes length:length options:MTLResourceStorageModeShared];
  }

  if (buffer == nil) {
    throw std::runtime_error{"Could not create Metal buffer"};
  }

  if (label != nil) {
    buffer.label = label;
  }

  return buffer;
}

template <typename T>
id<MTLBuffer> make_buffer(id<MTLDevice> device, const T &value, NSString *label) {
  return make_buffer(device, &value, sizeof(T), label);
}

template <typename T>
id<MTLBuffer> make_buffer(id<MTLDevice> device, const std::vector<T> &values, NSString *label) {
  return make_buffer(device, values.data(), values.size() * sizeof(T), label);
}

std::unique_ptr<HdrImage> texture_to_hdr_image(id<MTLTexture> texture, std::int32_t width, std::int32_t height) {
  const NSUInteger bytes_per_pixel = 4 * sizeof(float);
  const NSUInteger bytes_per_row = static_cast<NSUInteger>(width) * bytes_per_pixel;

  std::vector<float> pixels(static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4u);
  const MTLRegion region =
      MTLRegionMake2D(0u, 0u, static_cast<NSUInteger>(width), static_cast<NSUInteger>(height));

  [texture getBytes:pixels.data() bytesPerRow:bytes_per_row fromRegion:region mipmapLevel:0u];

  auto image = std::make_unique<HdrImage>(width, height);
  for (std::int32_t row = 0; row < height; ++row) {
    for (std::int32_t col = 0; col < width; ++col) {
      const std::size_t base = 4u * (static_cast<std::size_t>(row) * static_cast<std::size_t>(width) +
                                     static_cast<std::size_t>(col));
      image->set_pixel(col, row,
                       Color{pixels[base], pixels[base + 1u], pixels[base + 2u]});
    }
  }

  return image;
}

} // namespace

namespace mtl {

struct Backend::Impl {
  id<MTLDevice> device = nil;
  id<MTLLibrary> library = nil;
  id<MTLCommandQueue> command_queue = nil;
  id<MTLComputePipelineState> on_off_pipeline = nil;
  id<MTLComputePipelineState> flat_pipeline = nil;
  id<MTLComputePipelineState> path_pipeline = nil;

  Impl();

  id<MTLComputePipelineState> make_pipeline(NSString *function_name);

  std::unique_ptr<HdrImage> render(const gpu::Scene &scene, const ::ModeCfg &cfg,
                                   std::int32_t width, std::int32_t height) const;

  template <typename Cfg>
  std::unique_ptr<HdrImage> render_common(const gpu::Scene &scene, const Cfg &cfg,
                                          std::int32_t width, std::int32_t height, id<MTLComputePipelineState> pipeline) const;
};

Backend::Impl::Impl() {
  @autoreleasepool {
    device = MTLCreateSystemDefaultDevice();
    if (device == nil) {
      throw std::runtime_error{"Metal is not supported on this machine"};
    }

    command_queue = [device newCommandQueue];
    if (command_queue == nil) {
      throw std::runtime_error{"Could not create Metal command queue"};
    }

    NSError *error = nil;
    const std::filesystem::path metallib_path = executable_directory() / "raytrace.metallib";
    NSString *library_path = [NSString stringWithUTF8String:metallib_path.string().c_str()];
    NSURL *library_url = [NSURL fileURLWithPath:library_path];

    library = [device newLibraryWithURL:library_url error:&error];
    if (library == nil) {
      throw metal_error("Could not load raytrace.metallib", error);
    }

    on_off_pipeline = make_pipeline(@"on_off_tracing");
    flat_pipeline = make_pipeline(@"flat_tracing");
    path_pipeline = make_pipeline(@"path_tracing");
  }
}

id<MTLComputePipelineState> Backend::Impl::make_pipeline(NSString *function_name) {
  NSError *error = nil;

  id<MTLFunction> function = [library newFunctionWithName:function_name];
  if (function == nil) {
    const std::string function_name_str = nsstring_to_string(function_name);
    throw std::runtime_error{"Could not find Metal function \"" + function_name_str + "\""};
  }

  id<MTLComputePipelineState> pipeline = [device newComputePipelineStateWithFunction:function error:&error];
  if (pipeline == nil) {
    throw metal_error("Could not create Metal compute pipeline", error);
  }

  return pipeline;
}

std::unique_ptr<HdrImage> Backend::Impl::render(const gpu::Scene &scene, const ::ModeCfg &cfg,
                                               std::int32_t width, std::int32_t height) const {
  // TODO implement progress bar

  switch (cfg.mode) {
  case ::ModeCfg::Mode::ONOFF: {
    ::gpu::OnOffCfg on_off_cfg = ::gpu::lower_on_off_cfg(cfg);
    return render_common(scene, on_off_cfg, width, height, on_off_pipeline);
  }
  case ::ModeCfg::Mode::FLAT: {
    ::gpu::FlatCfg flat_cfg = ::gpu::lower_flat_cfg(cfg);
    return render_common(scene, flat_cfg, width, height, flat_pipeline);
  }
  case ::ModeCfg::Mode::PATH: {
    ::gpu::PathCfg path_cfg = ::gpu::lower_path_cfg(cfg);
    return render_common(scene, path_cfg, width, height, path_pipeline);
  }
  default:
    throw std::runtime_error{"Rendering mode unsupported by the GPU backend"};
  }
}

// TODO maybe declare outside the class
template <typename Cfg>
std::unique_ptr<HdrImage> Backend::Impl::render_common(const gpu::Scene &scene, const Cfg &cfg,
                                                       std::int32_t width, std::int32_t height, id<MTLComputePipelineState> pipeline) const {
  if (width <= 0 || height <= 0) {
    throw std::invalid_argument{"Metal backend requires positive image dimensions"};
  }

  @autoreleasepool {
    id<MTLBuffer> sphere_buffer = make_buffer(device, scene.spheres, @"spheres");
    id<MTLBuffer> plane_buffer = make_buffer(device, scene.planes, @"planes");
    id<MTLBuffer> material_buffer = make_buffer(device, scene.materials, @"materials");
    id<MTLBuffer> scene_counts_buffer = make_buffer(device, scene.counts, @"scene_counts");
    id<MTLBuffer> camera_buffer = make_buffer(device, scene.camera, @"camera");
    id<MTLBuffer> cfg_buffer = make_buffer(device, cfg, @"configuration");

    MTLTextureDescriptor *texture_descriptor =
        [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA32Float
                                                           width:static_cast<NSUInteger>(width)
                                                          height:static_cast<NSUInteger>(height)
                                                       mipmapped:NO];
    texture_descriptor.usage = MTLTextureUsageShaderWrite;
    texture_descriptor.storageMode = [device hasUnifiedMemory] ? MTLStorageModeShared : MTLStorageModeManaged;

    id<MTLTexture> output_texture = [device newTextureWithDescriptor:texture_descriptor];
    if (output_texture == nil) {
      throw std::runtime_error{"Could not create Metal output texture"};
    }

    id<MTLCommandBuffer> command_buffer = [command_queue commandBuffer];
    if (command_buffer == nil) {
      throw std::runtime_error{"Could not create Metal command buffer"};
    }

    id<MTLComputeCommandEncoder> compute_encoder = [command_buffer computeCommandEncoder];
    if (compute_encoder == nil) {
      throw std::runtime_error{"Could not create Metal compute command encoder"};
    }

    [compute_encoder setComputePipelineState:pipeline];
    [compute_encoder setTexture:output_texture atIndex:0u];
    [compute_encoder setBuffer:sphere_buffer offset:0u atIndex:0u];
    [compute_encoder setBuffer:plane_buffer offset:0u atIndex:1u];
    [compute_encoder setBuffer:material_buffer offset:0u atIndex:2u];
    [compute_encoder setBuffer:scene_counts_buffer offset:0u atIndex:3u];
    [compute_encoder setBuffer:camera_buffer offset:0u atIndex:4u];
    [compute_encoder setBuffer:cfg_buffer offset:0u atIndex:5u];

    const MTLSize grid_size = MTLSizeMake(static_cast<NSUInteger>(width), static_cast<NSUInteger>(height), 1u);
    const MTLSize threadgroup_size = MTLSizeMake(16u, 16u, 1u); // TODO expose thread group size in program API
    [compute_encoder dispatchThreads:grid_size threadsPerThreadgroup:threadgroup_size];
    [compute_encoder endEncoding];

    if (output_texture.storageMode == MTLStorageModeManaged) {
      id<MTLBlitCommandEncoder> blit_encoder = [command_buffer blitCommandEncoder];
      if (blit_encoder == nil) {
        throw std::runtime_error{"Could not create Metal blit command encoder for texture readback"};
      }

      [blit_encoder synchronizeResource:output_texture];
      [blit_encoder endEncoding];
    }

    [command_buffer commit];
    [command_buffer waitUntilCompleted];

    if (command_buffer.status != MTLCommandBufferStatusCompleted) {
      throw metal_error("Metal command buffer did not complete", command_buffer.error);
    }

    return texture_to_hdr_image(output_texture, width, height);
  }
}

Backend::Backend() : impl{std::make_unique<Impl>()} {}

Backend::~Backend() = default;

Backend::Backend(Backend &&other) noexcept = default;

Backend &Backend::operator=(Backend &&other) noexcept = default;

std::unique_ptr<HdrImage> Backend::render(const gpu::Scene &scene, const ::ModeCfg &cfg,
                                          std::int32_t width, std::int32_t height) const {
  return impl->render(scene, cfg, width, height);
}

} // namespace mtl

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_RENDERER_PIPELINE_H_
#define FLUTTER_IMPELLER_RENDERER_PIPELINE_H_

#include <future>

#include "compute_pipeline_descriptor.h"
#include "impeller/core/raw_ptr.h"
#include "impeller/renderer/compute_pipeline_builder.h"
#include "impeller/renderer/compute_pipeline_descriptor.h"
#include "impeller/renderer/context.h"
#include "impeller/renderer/pipeline_builder.h"
#include "impeller/renderer/pipeline_descriptor.h"
#include "impeller/renderer/shader_stage_compatibility_checker.h"

namespace impeller {

class PipelineLibrary;
template <typename PipelineDescriptor_>
class Pipeline;

template <typename T>
struct PipelineFuture {
  std::optional<T> descriptor;
  std::shared_future<std::shared_ptr<Pipeline<T>>> future;

  const std::shared_ptr<Pipeline<T>> Get() const { return future.get(); }

  bool IsValid() const { return future.valid(); }
};

//------------------------------------------------------------------------------
/// @brief      Describes the fixed function and programmable aspects of
///             rendering and compute operations performed by commands submitted
///             to the GPU via a command buffer.
///
///             A pipeline handle must be allocated upfront and kept alive for
///             as long as possible. Do not create a pipeline object within a
///             frame workload.
///
///             This pipeline object is almost never used directly as it is
///             untyped. Use reflected shader information generated by the
///             Impeller offline shader compiler to generate a typed pipeline
///             object.
///
template <typename T>
class Pipeline {
 public:
  virtual ~Pipeline();

  virtual bool IsValid() const = 0;

  //----------------------------------------------------------------------------
  /// @brief      Get the descriptor that was responsible for creating this
  ///             pipeline. It may be copied and modified to create a pipeline
  ///             variant.
  ///
  /// @return     The descriptor.
  ///
  const T& GetDescriptor() const;

  PipelineFuture<T> CreateVariant(
      bool async,
      std::function<void(T& desc)> descriptor_callback) const;

 protected:
  const std::weak_ptr<PipelineLibrary> library_;

  const T desc_;

  Pipeline(std::weak_ptr<PipelineLibrary> library, T desc);

 private:
  Pipeline(const Pipeline&) = delete;

  Pipeline& operator=(const Pipeline&) = delete;
};

/// @brief A raw ptr to a pipeline object.
///
/// These pipeline refs are safe to use as the context will keep the
/// pipelines alive throughout rendering.
using PipelineRef = raw_ptr<Pipeline<PipelineDescriptor>>;

extern template class Pipeline<PipelineDescriptor>;
extern template class Pipeline<ComputePipelineDescriptor>;

PipelineFuture<PipelineDescriptor> CreatePipelineFuture(
    const Context& context,
    std::optional<PipelineDescriptor> desc);

PipelineFuture<ComputePipelineDescriptor> CreatePipelineFuture(
    const Context& context,
    std::optional<ComputePipelineDescriptor> desc);

/// Holds a reference to a Pipeline used for rendering while also maintaining
/// the vertex shader and fragment shader types at compile-time.
///
/// See also:
///   - impeller::ContentContext::Variants - the typical container for
///     RenderPipelineHandles.
template <class VertexShader_, class FragmentShader_>
class RenderPipelineHandle {
  static_assert(
      ShaderStageCompatibilityChecker<VertexShader_, FragmentShader_>::Check(),
      "The output slots for the fragment shader don't have matches in the "
      "vertex shader's output slots. This will result in a linker error.");

 public:
  using VertexShader = VertexShader_;
  using FragmentShader = FragmentShader_;
  using Builder = PipelineBuilder<VertexShader, FragmentShader>;

  explicit RenderPipelineHandle(const Context& context)
      : RenderPipelineHandle(CreatePipelineFuture(
            context,
            Builder::MakeDefaultPipelineDescriptor(context))) {}

  explicit RenderPipelineHandle(const Context& context,
                                std::optional<PipelineDescriptor> desc)
      : RenderPipelineHandle(CreatePipelineFuture(context, desc)) {}

  explicit RenderPipelineHandle(PipelineFuture<PipelineDescriptor> future)
      : pipeline_future_(std::move(future)) {}

  std::shared_ptr<Pipeline<PipelineDescriptor>> WaitAndGet() {
    if (did_wait_) {
      return pipeline_;
    }
    did_wait_ = true;
    if (pipeline_future_.IsValid()) {
      pipeline_ = pipeline_future_.Get();
    }
    return pipeline_;
  }

  std::optional<PipelineDescriptor> GetDescriptor() const {
    return pipeline_future_.descriptor;
  }

 private:
  PipelineFuture<PipelineDescriptor> pipeline_future_;
  std::shared_ptr<Pipeline<PipelineDescriptor>> pipeline_;
  bool did_wait_ = false;

  RenderPipelineHandle(const RenderPipelineHandle&) = delete;

  RenderPipelineHandle& operator=(const RenderPipelineHandle&) = delete;
};

template <class ComputeShader_>
class ComputePipelineHandle {
 public:
  using ComputeShader = ComputeShader_;
  using Builder = ComputePipelineBuilder<ComputeShader>;

  explicit ComputePipelineHandle(const Context& context)
      : ComputePipelineHandle(CreatePipelineFuture(
            context,
            Builder::MakeDefaultPipelineDescriptor(context))) {}

  explicit ComputePipelineHandle(
      const Context& context,
      std::optional<ComputePipelineDescriptor> compute_desc)
      : ComputePipelineHandle(CreatePipelineFuture(context, compute_desc)) {}

  explicit ComputePipelineHandle(
      PipelineFuture<ComputePipelineDescriptor> future)
      : pipeline_future_(std::move(future)) {}

  std::shared_ptr<Pipeline<ComputePipelineDescriptor>> WaitAndGet() {
    if (did_wait_) {
      return pipeline_;
    }
    did_wait_ = true;
    if (pipeline_future_.IsValid()) {
      pipeline_ = pipeline_future_.Get();
    }
    return pipeline_;
  }

 private:
  PipelineFuture<ComputePipelineDescriptor> pipeline_future_;
  std::shared_ptr<Pipeline<ComputePipelineDescriptor>> pipeline_;
  bool did_wait_ = false;

  ComputePipelineHandle(const ComputePipelineHandle&) = delete;

  ComputePipelineHandle& operator=(const ComputePipelineHandle&) = delete;
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_RENDERER_PIPELINE_H_

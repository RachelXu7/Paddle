/* Copyright (c) 2016 PaddlePaddle Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#pragma once

#include "paddle/fluid/framework/operator.h"
#include "paddle/phi/kernels/funcs/math/beam_search.h"

namespace paddle {
namespace operators {

template <typename T, typename DeviceContext>
class BeamSearchOpKernel : public framework::OpKernel<T> {
 public:
  void Compute(const framework::ExecutionContext& context) const override {
    auto* ids = context.Input<phi::DenseTensor>("ids");
    auto* scores = context.Input<phi::DenseTensor>("scores");
    auto* pre_ids = context.Input<phi::DenseTensor>("pre_ids");
    auto* pre_scores = context.Input<phi::DenseTensor>("pre_scores");

    PADDLE_ENFORCE_NOT_NULL(
        scores,
        phi::errors::NotFound("Input(scores) of BeamSearchOp is not found."));
    PADDLE_ENFORCE_NOT_NULL(
        pre_ids,
        phi::errors::NotFound("Input(pre_ids) of BeamSearchOp is not found."));
    PADDLE_ENFORCE_NOT_NULL(
        pre_scores,
        phi::errors::NotFound(
            "Input(pre_scores) of BeamSearchOp is not found."));

    size_t level = context.Attr<int>("level");
    size_t beam_size = context.Attr<int>("beam_size");
    int end_id = context.Attr<int>("end_id");
    bool is_accumulated = context.Attr<bool>("is_accumulated");

    auto selected_ids = context.Output<phi::DenseTensor>("selected_ids");
    auto selected_scores = context.Output<phi::DenseTensor>("selected_scores");
    auto* parent_idx = context.Output<phi::DenseTensor>("parent_idx");
    PADDLE_ENFORCE_NOT_NULL(
        selected_ids,
        phi::errors::NotFound(
            "Output(selected_ids) of BeamSearchOp is not found."));
    PADDLE_ENFORCE_NOT_NULL(
        selected_scores,
        phi::errors::NotFound(
            "Output(selected_scores) of BeamSearchOp is not found."));

    phi::math::BeamSearchFunctor<DeviceContext, T> alg;
    alg(context.template device_context<DeviceContext>(),
        pre_ids,
        pre_scores,
        ids,
        scores,
        selected_ids,
        selected_scores,
        parent_idx,
        level,
        beam_size,
        end_id,
        is_accumulated);
  }
};

}  // namespace operators
}  // namespace paddle

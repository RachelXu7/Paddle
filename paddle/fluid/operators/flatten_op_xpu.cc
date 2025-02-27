/* Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#ifdef PADDLE_WITH_XPU

#include "paddle/fluid/operators/flatten_op.h"

namespace ops = paddle::operators;

REGISTER_OP_XPU_KERNEL(flatten2,
                       ops::Flatten2Kernel<phi::XPUContext, float>,
                       ops::Flatten2Kernel<phi::XPUContext, int>,
                       ops::Flatten2Kernel<phi::XPUContext, int8_t>,
                       ops::Flatten2Kernel<phi::XPUContext, int64_t>);
REGISTER_OP_XPU_KERNEL(flatten2_grad,
                       ops::Flatten2GradKernel<phi::XPUContext, float>,
                       ops::Flatten2GradKernel<phi::XPUContext, int>,
                       ops::Flatten2GradKernel<phi::XPUContext, int8_t>,
                       ops::Flatten2GradKernel<phi::XPUContext, int64_t>);
#endif

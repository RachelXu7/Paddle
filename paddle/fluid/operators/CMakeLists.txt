include(operators)

add_subdirectory(generator)

# solve "math constants not defined" problems caused by the order of inclusion
# of <cmath> and the definition of macro _USE_MATH_DEFINES
add_definitions(-D_USE_MATH_DEFINES)

# clean cache and pybind_file content first when rebuild
unset(GLOB_OP_LIB CACHE)
unset(OP_LIBRARY CACHE)
set(pybind_file ${PADDLE_BINARY_DIR}/paddle/fluid/pybind/pybind.h.tmp CACHE INTERNAL "pybind.h file")
set(pybind_file_prune ${PADDLE_BINARY_DIR}/paddle/fluid/pybind/pybind.h.prune CACHE INTERNAL "pybind.h file")
set(pybind_file_final ${PADDLE_BINARY_DIR}/paddle/fluid/pybind/pybind.h)
file(WRITE ${pybind_file} "#include \"paddle/phi/core/kernel_registry.h\" // Generated by the paddle/fluid/operators/CMakeLists.txt.  DO NOT EDIT!\n\n")

add_subdirectory(controlflow)
add_subdirectory(detection)
add_subdirectory(elementwise)
add_subdirectory(fused)
add_subdirectory(optimizers)
add_subdirectory(reduce_ops)
add_subdirectory(sequence_ops)
add_subdirectory(string)


if(WITH_DISTRIBUTE)
    add_subdirectory(collective)
endif()

if (WITH_PSCORE)
    add_subdirectory(pscore)
endif()

add_subdirectory(reader)

if (NOT WIN32)
    add_subdirectory(nccl)
endif()

if (WITH_GPU AND TENSORRT_FOUND)
    add_subdirectory(tensorrt)
endif()

if (WITH_DLNNE)
    add_subdirectory(dlnne)
endif()

if(WITH_CINN)
    add_subdirectory(cinn)
endif()

if(WITH_IPU)
    add_subdirectory(ipu)
endif()

SET(OP_HEADER_DEPS xxhash executor)

if (WITH_GPU)
    if (${CMAKE_CUDA_COMPILER_VERSION} LESS 11.0)
        SET(OP_HEADER_DEPS ${OP_HEADER_DEPS} cub)
    endif()
endif()

if (WITH_POCKETFFT)
    SET(OP_HEADER_DEPS ${OP_HEADER_DEPS} pocketfft)
endif()


SET(OP_MKL_DEPS "")
if (NOT WITH_MKL OR NOT WITH_AVX)
    SET(OP_MKL_DEPS ${OP_MKL_DEPS} match_matrix_tensor_op)
endif()
if(WITH_COVERAGE OR WIN32 OR WITH_NV_JETSON)
    SET(OP_MKL_DEPS ${OP_MKL_DEPS} pyramid_hash_op)
endif()

if(WITH_UNITY_BUILD)
    # Load Unity Build rules for operators in paddle/fluid/operators.
    include(unity_build_rule.cmake)
endif()

set(OP_HEADER_DEPS ${OP_HEADER_DEPS} phi common phi_utils static_prim_api get_expected_kernel_func)

register_operators(EXCLUDES py_func_op dgc_op generated_op1 generated_op2 generated_op3 generated_op4 load_combine_op lstm_op run_program_op quantize_linear_op
        recurrent_op save_combine_op sparse_attention_op sync_batch_norm_op activation_op ${OP_MKL_DEPS} DEPS ${OP_HEADER_DEPS} processgroup_comm_utils)

op_library(generated_op UNITY SRCS generated_op1.cc generated_op2.cc generated_op3.cc generated_op4.cc DEPS ${OP_HEADER_DEPS})
op_library(run_program_op DEPS executor_cache ${OP_HEADER_DEPS})
target_link_libraries(run_program_op cuda_graph_with_memory_pool)
op_library(quantize_linear_op DEPS phi common)
op_library(save_combine_op DEPS string_array phi common)
op_library(load_combine_op DEPS string_array)

if (WITH_GPU OR WITH_ROCM)
    register_cu_kernel(class_center_sample_op SRCS class_center_sample_op.cu DEPS ${OP_HEADER_DEPS})
endif()

if (WITH_GPU OR WITH_ROCM)
    op_library(activation_op SRCS activation_op.cc activation_op.kps soft_relu_op.cu DEPS ${OP_HEADER_DEPS})
elseif (WITH_XPU_KP)
    op_library(activation_op SRCS activation_op.cc activation_op.kps DEPS ${OP_HEADER_DEPS})
else()
    op_library(activation_op SRCS activation_op.cc DEPS ${OP_HEADER_DEPS})
endif()

if (WITH_GPU OR WITH_ROCM)
    op_library(sync_batch_norm_op DEPS processgroup_comm_utils)
    if ((NOT WIN32) AND (NOT WITH_ROCM) AND (NOT PADDLE_WITH_ARM) AND (NOT ${CMAKE_CUDA_COMPILER_VERSION} VERSION_LESS 11.3) )
        op_library(sparse_attention_op DEPS processgroup_comm_utils)
    endif()
endif()

op_library(lstm_op DEPS ${OP_HEADER_DEPS})
if (WITH_ROCM)
    op_library(recurrent_op DEPS executor ${OP_HEADER_DEPS})
else()
    op_library(recurrent_op DEPS ${OP_HEADER_DEPS})
endif()


set(COMMON_OP_DEPS ${OP_HEADER_DEPS})

if (WITH_DGC)
    op_library(dgc_op DEPS dgc)
    set(COMMON_OP_DEPS ${COMMON_OP_DEPS} dgc)
endif()

cc_library(common_infer_shape_functions SRCS common_infer_shape_functions.cc DEPS operator)
cc_library(ops_extra_info SRCS ops_extra_info.cc DEPS attribute phi common)

set(COMMON_OP_DEPS ${COMMON_OP_DEPS} phi common)
set(COMMON_OP_DEPS ${COMMON_OP_DEPS} selected_rows_utils
lod_tensor lod_rank_table executor static_prim_api)
set(COMMON_OP_DEPS ${COMMON_OP_DEPS} dynload_warpctc static_prim_api static_utils static_global_utils prim_utils)
set(COMMON_OP_DEPS ${COMMON_OP_DEPS} box_wrapper ps_gpu_wrapper)
set(COMMON_OP_DEPS ${COMMON_OP_DEPS} common_infer_shape_functions)
set(COMMON_OP_DEPS ${COMMON_OP_DEPS} processgroup_comm_utils)
if(WITH_NCCL OR WITH_RCCL)
  set(COMMON_OP_DEPS ${COMMON_OP_DEPS} process_group_nccl)
endif()
if(WITH_XPU)
  set(COMMON_OP_DEPS ${COMMON_OP_DEPS} xpulib)
endif()
set(COMMON_OP_DEPS ${COMMON_OP_DEPS} layer)
set(COMMON_OP_DEPS ${COMMON_OP_DEPS} op_version_registry)


# FIXME(typhoonzero): operator deps may not needed.
# op_library(unsqueeze_op DEPS reshape_op)
# op_library(squeeze_op DEPS reshape_op)
# op_library(flatten_op DEPS reshape_op)
# op_library(unstack_op DEPS stack_op)
# op_library(tensor_array_to_tensor_op DEPS concat_op)

set(OPERATOR_DEPS ${OPERATOR_DEPS} ${COMMON_OP_DEPS})
set(GLOB_OPERATOR_DEPS ${OPERATOR_DEPS} CACHE INTERNAL "Global Op dependencies")

if (WITH_PYTHON)
  cc_library(py_func_op SRCS py_func_op.cc DEPS op_registry python pybind)
endif()

set(GLOB_OP_LIB ${OP_LIBRARY} CACHE INTERNAL "Global OP library")

if(WITH_UNITY_BUILD)
    # Using Unity Build to compile operators, `register_operator` will cause
    # the unity library to lose some symbols.
    # The specified link dependency needs to be displayed here.
    target_link_libraries(paddle_operators_unity ${OP_HEADER_DEPS} ${COMMON_OP_DEPS})
endif()

copy_if_different(${pybind_file} ${pybind_file_final})

if (WITH_CUSTOM_DEVICE)
cc_library(custom_device_common_op_registry SRCS custom_device_common_op_registry.cc DEPS operator phi common type_info)
endif()

if(NOT "${OP_LIST}" STREQUAL "")
prune_pybind_h()
endif()

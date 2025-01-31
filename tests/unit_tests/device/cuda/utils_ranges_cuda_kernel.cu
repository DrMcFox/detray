/** Detray library, part of the ACTS project (R&D line)
 *
 * (c) 2022 CERN for the benefit of the ACTS project
 *
 * Mozilla Public License Version 2.0
 */

#include "detray/definitions/cuda_definitions.hpp"
#include "utils_ranges_cuda_kernel.hpp"

namespace detray {

//
// single
//
__global__ void single_kernel(const dindex value, dindex* result) {

    // single view should ony add the value 'i' once
    for (auto i : detray::views::single(value)) {
        *result += i;
    }
}

void test_single(const dindex value, dindex& check) {
    dindex* result{nullptr};
    cudaMallocManaged(&result, sizeof(dindex));
    *result = 0u;

    // run the kernel
    single_kernel<<<1, 1>>>(value, result);

    // cuda error check
    DETRAY_CUDA_ERROR_CHECK(cudaGetLastError());
    DETRAY_CUDA_ERROR_CHECK(cudaDeviceSynchronize());

    check = *result;
    cudaFree(result);
}

//
// iota
//
__global__ void iota_kernel(const darray<dindex, 2> range,
                            vecmem::data::vector_view<dindex> check_data) {

    vecmem::device_vector<dindex> check(check_data);

    for (auto i : detray::views::iota(range)) {
        check.push_back(i);
    }
}

void test_iota(const darray<dindex, 2> range,
               vecmem::data::vector_view<dindex> check_data) {

    // run the kernel
    iota_kernel<<<1, 1>>>(range, check_data);

    // cuda error check
    DETRAY_CUDA_ERROR_CHECK(cudaGetLastError());
    DETRAY_CUDA_ERROR_CHECK(cudaDeviceSynchronize());
}

//
// enumerate
//
__global__ void enumerate_kernel(
    vecmem::data::vector_view<uint_holder> seq_data,
    vecmem::data::vector_view<dindex> check_idx_data,
    vecmem::data::vector_view<dindex> check_value_data) {

    vecmem::device_vector<uint_holder> seq(seq_data);
    vecmem::device_vector<dindex> check_idx(check_idx_data);
    vecmem::device_vector<dindex> check_value(check_value_data);

    for (auto [i, v] : detray::views::enumerate(seq)) {
        check_idx.push_back(i);
        check_value.push_back(v.ui);
    }
}

void test_enumerate(vecmem::data::vector_view<uint_holder> seq_data,
                    vecmem::data::vector_view<dindex> check_idx_data,
                    vecmem::data::vector_view<dindex> check_value_data) {

    // run the kernel
    enumerate_kernel<<<1, 1>>>(seq_data, check_idx_data, check_value_data);

    // cuda error check
    DETRAY_CUDA_ERROR_CHECK(cudaGetLastError());
    DETRAY_CUDA_ERROR_CHECK(cudaDeviceSynchronize());
}

//
// pick
//
__global__ void pick_kernel(
    vecmem::data::vector_view<uint_holder> seq_data,
    vecmem::data::vector_view<dindex> idx_data,
    vecmem::data::vector_view<dindex> check_idx_data,
    vecmem::data::vector_view<dindex> check_value_data) {

    vecmem::device_vector<uint_holder> seq(seq_data);
    vecmem::device_vector<dindex> idx(idx_data);
    vecmem::device_vector<dindex> check_idx(check_idx_data);
    vecmem::device_vector<dindex> check_value(check_value_data);

    for (auto [i, v] : detray::views::pick(seq, idx)) {
        check_idx.push_back(i);
        check_value.push_back(v.ui);
    }
}

void test_pick(vecmem::data::vector_view<uint_holder> seq_data,
               vecmem::data::vector_view<dindex> idx_data,
               vecmem::data::vector_view<dindex> check_idx_data,
               vecmem::data::vector_view<dindex> check_value_data) {

    // run the kernel
    pick_kernel<<<1, 1>>>(seq_data, idx_data, check_idx_data, check_value_data);

    // cuda error check
    DETRAY_CUDA_ERROR_CHECK(cudaGetLastError());
    DETRAY_CUDA_ERROR_CHECK(cudaDeviceSynchronize());
}

//
// join
//
__global__ void join_kernel(
    vecmem::data::vector_view<uint_holder> seq_data_1,
    vecmem::data::vector_view<uint_holder> seq_data_2,
    vecmem::data::vector_view<dindex> check_value_data) {

    vecmem::device_vector<uint_holder> seq_1(seq_data_1);
    vecmem::device_vector<uint_holder> seq_2(seq_data_2);
    vecmem::device_vector<dindex> check_value(check_value_data);

    for (auto v : detray::views::join(seq_1, seq_2)) {
        check_value.push_back(v.ui);
    }
}

void test_join(vecmem::data::vector_view<uint_holder> seq_data_1,
               vecmem::data::vector_view<uint_holder> seq_data_2,
               vecmem::data::vector_view<dindex> check_value_data) {

    // run the kernel
    join_kernel<<<1, 1>>>(seq_data_1, seq_data_2, check_value_data);

    // cuda error check
    DETRAY_CUDA_ERROR_CHECK(cudaGetLastError());
    DETRAY_CUDA_ERROR_CHECK(cudaDeviceSynchronize());
}

//
// subrange
//
__global__ void subrange_kernel(vecmem::data::vector_view<int> seq_data,
                                vecmem::data::vector_view<int> check_value_data,
                                const std::size_t begin,
                                const std::size_t end) {

    vecmem::device_vector<int> seq(seq_data);
    vecmem::device_vector<int> check(check_value_data);

    for (const auto& v : detray::ranges::subrange(
             seq, std::array<std::size_t, 2>{begin, end})) {
        check.push_back(v);
    }
}

void test_subrange(vecmem::data::vector_view<int> seq_data,
                   vecmem::data::vector_view<int> check_value_data,
                   const std::size_t begin, const std::size_t end) {

    // run the kernel
    subrange_kernel<<<1, 1>>>(seq_data, check_value_data, begin, end);

    // cuda error check
    DETRAY_CUDA_ERROR_CHECK(cudaGetLastError());
    DETRAY_CUDA_ERROR_CHECK(cudaDeviceSynchronize());
}

}  // namespace detray

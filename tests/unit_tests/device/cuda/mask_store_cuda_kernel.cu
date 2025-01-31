/** Detray library, part of the ACTS project (R&D line)
 *
 * (c) 2021 CERN for the benefit of the ACTS project
 *
 * Mozilla Public License Version 2.0
 */

#include "detray/definitions/cuda_definitions.hpp"
#include "mask_store_cuda_kernel.hpp"

namespace detray {

/// test kernel function to fill the output vector with is_inside function
/// return values
__global__ void mask_test_kernel(
    typename host_store_type::view_type store_data,
    vecmem::data::vector_view<point2> input_point2_data,
    vecmem::data::jagged_vector_view<intersection::status> output_data) {

    /** get mask store **/
    device_store_type store(store_data);

    /** get mask objects **/
    vecmem::device_vector<point2> input_point2(input_point2_data);
    vecmem::jagged_device_vector<intersection::status> output_device(
        output_data);

    const auto& rectangle_mask = store.get<e_rectangle2>()[0];
    const auto& trapezoid_mask = store.get<e_trapezoid2>()[0];
    const auto& ring_mask = store.get<e_ring2>()[0];
    const auto& cylinder_mask = store.get<e_cylinder2>()[0];
    const auto& annulus_mask = store.get<e_annulus2>()[0];

    /** get device results from is_inside function **/
    for (int i = 0; i < n_points; i++) {
        output_device[0].push_back(rectangle_mask.is_inside(input_point2[i]));
        output_device[1].push_back(trapezoid_mask.is_inside(input_point2[i]));
        output_device[2].push_back(ring_mask.is_inside(input_point2[i]));
        output_device[3].push_back(cylinder_mask.is_inside(input_point2[i]));
        output_device[4].push_back(annulus_mask.is_inside(input_point2[i]));
    }
}

void mask_test(
    typename host_store_type::view_type store_data,
    vecmem::data::vector_view<point2> input_point2_data,
    vecmem::data::jagged_vector_view<intersection::status> output_data) {

    int block_dim = 1;
    int thread_dim = 1;

    // run the test kernel
    mask_test_kernel<<<block_dim, thread_dim>>>(store_data, input_point2_data,
                                                output_data);

    // cuda error check
    DETRAY_CUDA_ERROR_CHECK(cudaGetLastError());
    DETRAY_CUDA_ERROR_CHECK(cudaDeviceSynchronize());
}

}  // namespace detray

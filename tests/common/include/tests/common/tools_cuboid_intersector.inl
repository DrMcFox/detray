/** Detray library, part of the ACTS project (R&D line)
 *
 * (c) 2023 CERN for the benefit of the ACTS project
 *
 * Mozilla Public License Version 2.0
 */

// Project include(s)
#include "detray/intersection/bounding_box/cuboid_intersector.hpp"
#include "detray/intersection/detail/trajectories.hpp"
#include "detray/intersection/intersection.hpp"
#include "detray/tools/bounding_volume.hpp"

// GTest include
#include <gtest/gtest.h>

using namespace detray;

namespace {

/// @note __plugin has to be defined with a preprocessor command
using transform3 = __plugin::transform3<detray::scalar>;
using vector3 = typename transform3::vector3;
using point3 = typename transform3::point3;

// cuboid
constexpr scalar x_min{1.f * unit<scalar>::mm};
constexpr scalar x_max{3.f * unit<scalar>::mm};
constexpr scalar y_min{0.f * unit<scalar>::mm};
constexpr scalar y_max{2.f * unit<scalar>::mm};
constexpr scalar z_min{2.f * unit<scalar>::mm};
constexpr scalar z_max{3.f * unit<scalar>::mm};

// envelope around wrapped object (scalor in percent)
constexpr scalar envelope{1.01f};
// test tolerance
constexpr scalar tol{1e-7f};

}  // anonymous namespace

// This test the intersection between ray and cuboid aabb
TEST(ALGEBRA_PLUGIN, cuboid_aabb_intersector) {
    // Test ray
    const point3 pos{2.f, 1.f, 0.f};
    const vector3 mom{0.f, 0.f, 1.f};
    const detail::ray<transform3> r(pos, 0.f, mom, 0.f);

    // The bounding box
    mask<cuboid3D<>> c3{0u, x_min, y_min, z_min, x_max, y_max, z_max};
    axis_aligned_bounding_volume<cuboid3D<>> aabb{c3, 0u, envelope};

    ASSERT_TRUE(aabb.intersect(r));
}

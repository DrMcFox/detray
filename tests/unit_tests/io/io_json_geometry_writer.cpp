/** Detray library, part of the ACTS project (R&D line)
 *
 * (c) 2023 CERN for the benefit of the ACTS project
 *
 * Mozilla Public License Version 2.0
 */

// Project include(s)
#include "detray/definitions/algebra.hpp"
#include "detray/definitions/units.hpp"
#include "detray/detectors/create_telescope_detector.hpp"
#include "detray/detectors/create_toy_geometry.hpp"
#include "detray/io/json/json_geometry_writer.hpp"

// Vecmem include(s)
#include <vecmem/memory/host_memory_resource.hpp>

// GTest include(s)
#include <gtest/gtest.h>

using namespace detray;

/// Test the writing of a telescope detector geometry to json
TEST(io, json_telescope_geometry_writer) {

    using detector_t =
        detector<detector_registry::template telescope_detector<annulus2D<>>>;

    // Use stereo annulus surfaces
    constexpr scalar minR{7.2f * unit<scalar>::mm};
    constexpr scalar maxR{12.0f * unit<scalar>::mm};
    constexpr scalar minPhi{0.74195f};
    constexpr scalar maxPhi{1.33970f};
    constexpr scalar offset_x{-2.f * unit<scalar>::mm};
    constexpr scalar offset_y{-2.f * unit<scalar>::mm};
    mask<annulus2D<>> ann2{0u,     minR,     maxR,     minPhi,
                           maxPhi, offset_x, offset_y, 0.f};

    // Surface positions
    std::vector<scalar> positions = {0.f,   50.f,  100.f, 150.f, 200.f, 250.f,
                                     300.f, 350.f, 400.f, 450.f, 500.f};

    // Telescope detector
    vecmem::host_memory_resource host_mr;
    detector_t det = create_telescope_detector(host_mr, ann2, positions);

    detray::json_geometry_writer<detector_t> geo_writer;
    geo_writer.write(det, "telescope_geometry");
}

/// Test the writing of the toy detector geometry to json
TEST(io, json_toy_geometry_writer) {

    using detector_t = detector<detector_registry::toy_detector>;

    // Toy detector
    vecmem::host_memory_resource host_mr;
    detector_t det = create_toy_geometry(host_mr);

    detray::json_geometry_writer<detector_t> geo_writer;
    geo_writer.write(det, "toy_geometry");
}
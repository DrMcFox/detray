/** Detray library, part of the ACTS project (R&D line)
 *
 * (c) 2022-2023 CERN for the benefit of the ACTS project
 *
 * Mozilla Public License Version 2.0
 */

#include <gtest/gtest.h>

#include <vecmem/memory/host_memory_resource.hpp>

#include "detray/definitions/units.hpp"
#include "detray/detectors/create_telescope_detector.hpp"
#include "detray/masks/masks.hpp"
#include "detray/masks/unbounded.hpp"
#include "detray/propagator/actor_chain.hpp"
#include "detray/propagator/actors/aborters.hpp"
#include "detray/propagator/navigation_policies.hpp"
#include "detray/propagator/navigator.hpp"
#include "detray/propagator/propagator.hpp"
#include "detray/propagator/rk_stepper.hpp"
#include "detray/tracks/tracks.hpp"
#include "tests/common/tools/inspectors.hpp"

// This tests the construction and general methods of the navigator
TEST(ALGEBRA_PLUGIN, guided_navigator) {
    using namespace detray;
    using namespace navigation;
    using transform3_t = __plugin::transform3<scalar>;
    using point3 = typename transform3_t::point3;
    using vector3 = typename transform3_t::vector3;

    vecmem::host_memory_resource host_mr;

    // Use unbounded rectangle surfaces that cannot be missed
    mask<unbounded<rectangle2D<>>> urectangle{0u, 20.f * unit<scalar>::mm,
                                              20.f * unit<scalar>::mm};

    // Module positions along z-axis
    const std::vector<scalar> positions = {0.f,  10.f, 20.f, 30.f, 40.f, 50.f,
                                           60.f, 70.f, 80.f, 90.f, 100.f};

    // Build telescope detector with unbounded rectangles
    const auto telescope_det =
        create_telescope_detector(host_mr, urectangle, positions);

    // Inspectors are optional, of course
    using detector_t = decltype(telescope_det);
    using intersection_t =
        intersection2D<typename detector_t::surface_type, transform3_t>;
    using object_tracer_t =
        object_tracer<intersection_t, dvector, status::e_on_portal,
                      status::e_on_module>;
    using inspector_t = aggregate_inspector<object_tracer_t, print_inspector>;
    using b_field_t = detector_t::bfield_type;
    using runge_kutta_stepper =
        rk_stepper<b_field_t::view_t, transform3_t, unconstrained_step,
                   guided_navigation>;
    using guided_navigator = navigator<detector_t, inspector_t>;
    using actor_chain_t = actor_chain<dtuple, pathlimit_aborter>;
    using propagator_t =
        propagator<runge_kutta_stepper, guided_navigator, actor_chain_t>;

    // track must point into the direction of the telescope
    const point3 pos{0.f, 0.f, 0.f};
    const vector3 mom{0.f, 0.f, 1.f};
    free_track_parameters<transform3_t> track(pos, 0.f, mom, -1.f);
    const vector3 B{0.f, 0.f, 1.f * unit<scalar>::T};
    const b_field_t b_field(
        b_field_t::backend_t::configuration_t{B[0], B[1], B[2]});

    // Actors
    pathlimit_aborter::state pathlimit{200.f * unit<scalar>::cm};

    // Propagator
    propagator_t p(runge_kutta_stepper{}, guided_navigator{});
    propagator_t::state guided_state(track, b_field, telescope_det);

    // Propagate
    p.propagate(guided_state, std::tie(pathlimit));

    auto &nav_state = guided_state._navigation;
    auto &debug_printer = nav_state.inspector().template get<print_inspector>();
    auto &obj_tracer = nav_state.inspector().template get<object_tracer_t>();

    // Check that navigator exited
    ASSERT_TRUE(nav_state.is_complete()) << debug_printer.to_string();

    // Sequence of surface ids we expect to see
    const std::vector<unsigned int> sf_sequence = {0u, 1u, 2u, 3u, 4u,  5u,
                                                   6u, 7u, 8u, 9u, 10u, 11u};
    // Check the surfaces that have been visited by the navigation
    EXPECT_EQ(obj_tracer.object_trace.size(), sf_sequence.size())
        << debug_printer.to_string();
    for (std::size_t i = 0u; i < sf_sequence.size(); ++i) {
        const auto &candidate = obj_tracer.object_trace[i];
        auto bcd = geometry::barcode{};
        bcd.set_volume(0u).set_index(sf_sequence[i]);
        bcd.set_id((i == 11u) ? surface_id::e_portal : surface_id::e_sensitive);
        EXPECT_TRUE(candidate.surface.barcode() == bcd)
            << "error at intersection on surface:\n"
            << "Expected: " << bcd
            << "\nFound: " << candidate.surface.barcode();
    }
}

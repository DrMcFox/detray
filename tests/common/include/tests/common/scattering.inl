/** Detray library, part of the ACTS project (R&D line)
 *
 * (c) 2023 CERN for the benefit of the ACTS project
 *
 * Mozilla Public License Version 2.0
 */

// Project include(s).
#include "detray/propagator/actors/pointwise_material_interactor.hpp"
#include "detray/simulation/random_scatterer.hpp"
#include "detray/simulation/scattering_helper.hpp"
#include "detray/tracks/tracks.hpp"
#include "detray/utils/statistics.hpp"

// google-test include(s).
#include <gtest/gtest.h>

// System include(s).
#include <algorithm>
#include <random>
#include <vector>

using namespace detray;
using transform3 = __plugin::transform3<detray::scalar>;
using matrix_operator = typename transform3::matrix_actor;
using vector3 = typename transform3::vector3;

namespace {
std::random_device rd{};
std::mt19937_64 generator{rd()};
}  // namespace

// Test scattering helper
TEST(scattering, scattering_helper) {

    generator.seed(0u);

    const vector3 dir{2.f, 2.f, 4.f};
    const scalar scattering_angle{0.1f};

    // xaxis of curvilinear plane
    const vector3 u = unit_vectors<vector3>().make_curvilinear_unit_u(dir);

    // normal vector of the plane defined by u and dir
    const vector3 w = vector::cross(dir, u);

    std::vector<scalar> angle_diffs;

    std::size_t n_samples{100000u};
    for (std::size_t i = 0u; i < n_samples; i++) {

        // Scatter the input direction
        const vector3 new_dir =
            scattering_helper<transform3>()(dir, scattering_angle, generator);

        // Assign a plus sign if the dot product of new direction and w vector
        // is plus
        const scalar sign = vector::dot(w, new_dir) > 0 ? 1.f : -1.f;

        // the cosine of angle between original direction and new one
        scalar cos_theta = vector::dot(dir, new_dir) /
                           (getter::norm(dir) * getter::norm(new_dir));

        // To prevent rounding error where cos_theta is out of [-1, 1]
        cos_theta = std::clamp(cos_theta, scalar{-1.f}, scalar{1.f});

        // Geht the angle between original direction and new one
        const scalar angle = sign * std::acos(cos_theta);

        angle_diffs.push_back(angle);
    }

    EXPECT_NEAR(statistics::mean(angle_diffs), 0.f, 1e-3f);

    // Tolerate upto 1% difference
    const scalar stddev = std::sqrt(statistics::rms(angle_diffs, 0.f));
    EXPECT_NEAR((stddev - scattering_angle) / scattering_angle, 0.f, 1e-2f);
}

// Test angle update
TEST(scattering, angle_update) {

    generator.seed(0u);

    // Initial direction
    vector3 dir{1.f, 2.f, 3.f};
    dir = vector::normalize(dir);

    const scalar phi0 = getter::phi(dir);
    const scalar theta0 = getter::theta(dir);

    // Projected scattering angle (Tests will fail with relatively large angles)
    const scalar projected_scattering_angle{0.01f};

    // Navigation in forward direction
    const int direction_sign = 1;

    // Initial bound covariance
    typename bound_track_parameters<transform3>::covariance_type bound_cov =
        matrix_operator().template zero<e_bound_size, e_bound_size>();

    // We are comparing the bound covariances (var[phi] and var[theta]) to the
    // variance of samples taken by random scattering

    // Update the bound covariance with projected scattering angle
    pointwise_material_interactor<transform3>().update_angle_variance(
        bound_cov, dir, projected_scattering_angle, direction_sign);

    // Get the samples of phi and theta after the random scattering
    std::vector<scalar> phis;
    std::vector<scalar> thetas;
    std::size_t n_samples{100000u};
    for (std::size_t i = 0u; i < n_samples; i++) {
        const auto new_dir =
            random_scatterer<pointwise_material_interactor<transform3>>()
                .scatter(dir, projected_scattering_angle, generator);
        phis.push_back(getter::phi(new_dir));
        thetas.push_back(getter::theta(new_dir));
    }

    // Tolerate upto 1% difference
    const auto var_phi = getter::element(bound_cov, e_bound_phi, e_bound_phi);
    const auto var_theta =
        getter::element(bound_cov, e_bound_theta, e_bound_theta);
    EXPECT_NEAR((var_phi - statistics::rms(phis, phi0)) / var_phi, 0.f, 1e-2f);
    EXPECT_NEAR((var_theta - statistics::rms(thetas, theta0)) / var_theta, 0.f,
                1e-2f);
}
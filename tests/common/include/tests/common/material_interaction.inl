/** Detray library, part of the ACTS project (R&D line)
 *
 * (c) 2022-2023 CERN for the benefit of the ACTS project
 *
 * Mozilla Public License Version 2.0
 */

// Project include(s).
#include "detray/definitions/pdg_particle.hpp"
#include "detray/definitions/units.hpp"
#include "detray/detectors/create_telescope_detector.hpp"
#include "detray/masks/masks.hpp"
#include "detray/masks/unbounded.hpp"
#include "detray/materials/interaction.hpp"
#include "detray/materials/material.hpp"
#include "detray/materials/material_slab.hpp"
#include "detray/materials/predefined_materials.hpp"
#include "detray/propagator/actor_chain.hpp"
#include "detray/propagator/actors/aborters.hpp"
#include "detray/propagator/actors/parameter_resetter.hpp"
#include "detray/propagator/actors/parameter_transporter.hpp"
#include "detray/propagator/actors/pointwise_material_interactor.hpp"
#include "detray/propagator/navigator.hpp"
#include "detray/propagator/propagator.hpp"
#include "detray/propagator/rk_stepper.hpp"
#include "detray/simulation/random_scatterer.hpp"
#include "detray/utils/statistics.hpp"
#include "tests/common/tools/inspectors.hpp"

// VecMem include(s).
#include <vecmem/memory/host_memory_resource.hpp>

// GTest include(s).
#include <gtest/gtest.h>

using namespace detray;

using transform3 = __plugin::transform3<scalar>;
using matrix_operator = typename transform3::matrix_actor;

// Material interaction test with telescope Geometry
TEST(material_interaction, telescope_geometry_energy_loss) {

    vecmem::host_memory_resource host_mr;

    // Use rectangle surfaces in telescope
    mask<rectangle2D<>> rectangle{0u, 20.f * unit<scalar>::mm,
                                  20.f * unit<scalar>::mm};

    // Build in x-direction from given module positions
    detail::ray<transform3> traj{{0.f, 0.f, 0.f}, 0.f, {1.f, 0.f, 0.f}, -1.f};
    std::vector<scalar> positions = {0.f,   50.f,  100.f, 150.f, 200.f, 250.f,
                                     300.f, 350.f, 400.f, 450.f, 500.f};

    const auto mat = silicon_tml<scalar>();
    constexpr scalar thickness{0.17f * unit<scalar>::cm};

    const auto det = create_telescope_detector(host_mr, rectangle, positions,
                                               mat, thickness, traj);

    using navigator_t = navigator<decltype(det)>;
    using stepper_t = line_stepper<transform3>;
    using interactor_t = pointwise_material_interactor<transform3>;
    using actor_chain_t =
        actor_chain<dtuple, propagation::print_inspector, pathlimit_aborter,
                    parameter_transporter<transform3>, interactor_t,
                    parameter_resetter<transform3>>;
    using propagator_t = propagator<stepper_t, navigator_t, actor_chain_t>;

    // Propagator is built from the stepper and navigator
    propagator_t p({}, {});

    constexpr scalar q{-1.f};
    constexpr scalar iniP{10.f * unit<scalar>::GeV};

    typename bound_track_parameters<transform3>::vector_type bound_vector;
    getter::element(bound_vector, e_bound_loc0, 0) = 0.f;
    getter::element(bound_vector, e_bound_loc1, 0) = 0.f;
    getter::element(bound_vector, e_bound_phi, 0) = 0.f;
    getter::element(bound_vector, e_bound_theta, 0) = constant<scalar>::pi_2;
    getter::element(bound_vector, e_bound_qoverp, 0) = q / iniP;
    getter::element(bound_vector, e_bound_time, 0) = 0.f;
    typename bound_track_parameters<transform3>::covariance_type bound_cov =
        matrix_operator().template zero<e_bound_size, e_bound_size>();

    // bound track parameter at first physical plane
    const bound_track_parameters<transform3> bound_param(
        geometry::barcode{}.set_index(0u), bound_vector, bound_cov);

    propagation::print_inspector::state print_insp_state{};
    pathlimit_aborter::state aborter_state{};
    parameter_transporter<transform3>::state bound_updater{};
    interactor_t::state interactor_state{};
    parameter_resetter<transform3>::state parameter_resetter_state{};

    // Create actor states tuples
    auto actor_states = std::tie(print_insp_state, aborter_state, bound_updater,
                                 interactor_state, parameter_resetter_state);

    propagator_t::state state(bound_param, det);

    // Propagate the entire detector
    ASSERT_TRUE(p.propagate(state, actor_states))
        << print_insp_state.to_string() << std::endl;

    // muon
    const int pdg{interactor_state.pdg};

    // mass
    const scalar mass{interactor_state.mass};

    // new momentum
    const scalar newP{state._stepping._bound_params.charge() /
                      state._stepping._bound_params.qop()};

    // new energy
    const scalar newE{std::hypot(newP, mass)};

    // Initial energy
    const scalar iniE{std::hypot(iniP, mass)};

    // New qop variance
    const scalar new_var_qop{
        matrix_operator().element(state._stepping._bound_params.covariance(),
                                  e_bound_qoverp, e_bound_qoverp)};

    // Interaction object
    interaction<scalar> I;

    // intersection with a zero incidence angle
    intersection2D<typename decltype(det)::surface_type, transform3> is;
    is.cos_incidence_angle = 1.f;

    // Same material used for default telescope detector
    material_slab<scalar> slab(mat, thickness);

    // Expected Bethe Stopping power for telescope geometry is estimated
    // as (number of planes * energy loss per plane assuming 1 GeV muon).
    // It is not perfectly precise as the track loses its energy during
    // propagation. However, since the energy loss << the track momentum,
    // the assumption is not very bad
    const scalar dE{
        I.compute_energy_loss_bethe(is, slab, pdg, mass, q / iniP, q) *
        static_cast<scalar>(positions.size())};

    // Check if the new energy after propagation is enough close to the
    // expected value
    EXPECT_NEAR(newE, iniE - dE, 1e-5f);

    const scalar sigma_qop{I.compute_energy_loss_landau_sigma_QOverP(
        is, slab, pdg, mass, q / iniP, q)};

    const scalar dvar_qop{sigma_qop * sigma_qop *
                          static_cast<scalar>(positions.size() - 1u)};

    EXPECT_NEAR(new_var_qop, dvar_qop, 1e-10f);

    /********************************
     * Test with next_surface_aborter
     *********************************/

    using alt_actor_chain_t =
        actor_chain<dtuple, propagation::print_inspector, pathlimit_aborter,
                    parameter_transporter<transform3>, next_surface_aborter,
                    interactor_t, parameter_resetter<transform3>>;
    using alt_propagator_t =
        propagator<stepper_t, navigator_t, alt_actor_chain_t>;

    bound_track_parameters<transform3> alt_bound_param(
        geometry::barcode{}.set_index(0u), bound_vector, bound_cov);

    scalar altE(0);

    unsigned int surface_count = 0;
    while (surface_count < 1e4) {
        surface_count++;

        // Create actor states tuples
        propagation::print_inspector::state alt_print_insp_state{};
        pathlimit_aborter::state alt_aborter_state{};
        next_surface_aborter::state next_surface_aborter_state{
            0.1f * unit<scalar>::mm};

        auto alt_actor_states =
            std::tie(alt_print_insp_state, alt_aborter_state, bound_updater,
                     next_surface_aborter_state, interactor_state,
                     parameter_resetter_state);

        // Propagator and its state
        alt_propagator_t alt_p({}, {});
        alt_propagator_t::state alt_state(alt_bound_param, det);

        // Propagate
        alt_p.propagate(alt_state, alt_actor_states);

        alt_bound_param = alt_state._stepping._bound_params;

        // Terminate the propagation if the next sensitive surface was not found
        if (!next_surface_aborter_state.success) {
            // if (alt_state._navigation.is_complete()){
            const scalar altP = alt_state._stepping._bound_params.charge() /
                                alt_state._stepping._bound_params.qop();
            altE = std::hypot(altP, mass);
            break;
        }
    }

    EXPECT_EQ(surface_count, positions.size());
    EXPECT_EQ(altE, newE);

    // @todo: Validate the backward direction case as well?
}

// Material interaction test with telescope Geometry
TEST(material_interaction, telescope_geometry_scattering_angle) {
    vecmem::host_memory_resource host_mr;

    // Build in x-direction from given module positions
    detail::ray<transform3> traj{{0.f, 0.f, 0.f}, 0.f, {1.f, 0.f, 0.f}, -1.f};
    std::vector<scalar> positions = {0.f};

    // To make sure that someone won't put more planes than one by accident
    EXPECT_EQ(positions.size(), 1u);

    // Material
    const auto mat = silicon_tml<scalar>();
    const scalar thickness = 100.f * unit<scalar>::cm;

    // Use rectangle surfaces in the telescope
    mask<rectangle2D<>> rectangle{0u, 2000.f * unit<scalar>::mm,
                                  2000.f * unit<scalar>::mm};

    // Create telescope geometry
    const auto det = create_telescope_detector(host_mr, rectangle, positions,
                                               mat, thickness, traj);

    using navigator_t = navigator<decltype(det)>;
    using stepper_t = line_stepper<transform3>;
    using interactor_t = pointwise_material_interactor<transform3>;
    using simulator_t = random_scatterer<interactor_t>;
    using material_actor_t = composite_actor<dtuple, interactor_t, simulator_t>;
    using actor_chain_t =
        actor_chain<dtuple, propagation::print_inspector, pathlimit_aborter,
                    parameter_transporter<transform3>, material_actor_t,
                    parameter_resetter<transform3>>;
    using propagator_t = propagator<stepper_t, navigator_t, actor_chain_t>;

    // Propagator is built from the stepper and navigator
    propagator_t p({}, {});

    constexpr scalar q{-1.f};
    constexpr scalar iniP{10.f * unit<scalar>::GeV};

    // Initial track parameters directing x-axis
    typename bound_track_parameters<transform3>::vector_type bound_vector =
        matrix_operator().template zero<e_bound_size, 1>();
    getter::element(bound_vector, e_bound_theta, 0) = constant<scalar>::pi_2;
    getter::element(bound_vector, e_bound_qoverp, 0) = q / iniP;

    typename bound_track_parameters<transform3>::covariance_type bound_cov =
        matrix_operator().template zero<e_bound_size, e_bound_size>();

    // bound track parameter
    const bound_track_parameters<transform3> bound_param(
        geometry::barcode{}.set_index(0u), bound_vector, bound_cov);

    std::size_t n_samples{100000u};
    std::vector<scalar> phis;
    std::vector<scalar> thetas;

    scalar ref_phi_variance{0.f};
    scalar ref_theta_variance{0.f};

    for (std::size_t i = 0u; i < n_samples; i++) {

        propagation::print_inspector::state print_insp_state{};
        pathlimit_aborter::state aborter_state{};
        parameter_transporter<transform3>::state bound_updater{};
        interactor_t::state interactor_state{};
        // No energy loss
        interactor_state.do_energy_loss = false;
        // Seed = sample id
        simulator_t::state simulator_state{i};
        parameter_resetter<transform3>::state parameter_resetter_state{};

        // Create actor states tuples
        auto actor_states = std::tie(print_insp_state, aborter_state,
                                     bound_updater, interactor_state,
                                     simulator_state, parameter_resetter_state);

        propagator_t::state state(bound_param, det);

        // Propagate the entire detector
        ASSERT_TRUE(p.propagate(state, actor_states))
            << print_insp_state.to_string() << std::endl;

        const auto& final_param = state._stepping._bound_params;

        // Get the updated phi and theta after scattering (Every sample should
        // have equal values)
        if (i == 0u) {
            const auto& covariance = final_param.covariance();
            ref_phi_variance =
                matrix_operator().element(covariance, e_bound_phi, e_bound_phi);
            ref_theta_variance = matrix_operator().element(
                covariance, e_bound_theta, e_bound_theta);
        }

        phis.push_back(final_param.phi());
        thetas.push_back(final_param.theta());
    }

    scalar phi_variance{statistics::rms(phis, bound_param.phi())};
    scalar theta_variance{statistics::rms(thetas, bound_param.theta())};

    // Tolerate upto 1% difference
    EXPECT_NEAR((phi_variance - ref_phi_variance) / ref_phi_variance, 0.f,
                1e-2f);
    EXPECT_NEAR((theta_variance - ref_theta_variance) / ref_theta_variance, 0.f,
                1e-2f);

    // To make sure that the variances are not zero
    EXPECT_TRUE(ref_phi_variance > 1e-9f && ref_theta_variance > 1e-9f);
}

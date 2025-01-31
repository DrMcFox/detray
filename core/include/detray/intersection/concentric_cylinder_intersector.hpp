/** Detray library, part of the ACTS project (R&D line)
 *
 * (c) 2022-2023 CERN for the benefit of the ACTS project
 *
 * Mozilla Public License Version 2.0
 */

#pragma once

// Project include(s)
#include "detray/coordinates/cylindrical2.hpp"
#include "detray/definitions/math.hpp"
#include "detray/definitions/qualifiers.hpp"
#include "detray/intersection/detail/trajectories.hpp"
#include "detray/intersection/intersection.hpp"
#include "detray/utils/quadratic_equation.hpp"

// System include(s)
#include <cmath>
#include <type_traits>

namespace detray {

/// A functor to find intersections between trajectory and concentric cylinder
/// mask
template <typename transform3_t>
struct concentric_cylinder_intersector {

    /// linear algebra types
    /// @{
    using scalar_type = typename transform3_t::scalar_type;
    using point3 = typename transform3_t::point3;
    using point2 = typename transform3_t::point2;
    using vector3 = typename transform3_t::vector3;
    /// @}
    using ray_type = detail::ray<transform3_t>;

    /// Operator function to find intersections between ray and concentric
    /// cylinder mask
    ///
    /// @tparam mask_t is the input mask type
    /// @tparam surface_t is the type of surface handle
    ///
    /// @param ray is the input ray trajectory
    /// @param sf the surface handle the mask is associated with
    /// @param mask is the input mask that defines the surface extent
    /// @param trf is the surface placement transform
    /// @param mask_tolerance is the tolerance for mask edges
    ///
    /// @return the intersection
    template <
        typename mask_t, typename surface_t,
        std::enable_if_t<std::is_same_v<typename mask_t::measurement_frame_type,
                                        cylindrical2<transform3_t>>,
                         bool> = true>
    DETRAY_HOST_DEVICE inline intersection2D<surface_t, transform3_t>
    operator()(const ray_type &ray, const surface_t sf, const mask_t &mask,
               const transform3_t & /*trf*/,
               const scalar_type mask_tolerance = 0.f) const {

        using intersection_t = intersection2D<surface_t, transform3_t>;

        intersection_t is;

        const scalar_type r{mask[mask_t::shape::e_r]};
        // Two points on the line, these are in the cylinder frame
        const point3 &ro = ray.pos();
        const vector3 &rd = ray.dir();
        const point3 &l0 = ro;
        const point3 l1 = ro + rd;

        // swap coorinates x/y for numerical stability
        const bool swap_x_y = std::abs(rd[0]) < 1e-3f;

        unsigned int _x = swap_x_y ? 1u : 0u;
        unsigned int _y = swap_x_y ? 0u : 1u;
        const scalar_type k{(l0[_y] - l1[_y]) / (l0[_x] - l1[_x])};
        const scalar_type d{l1[_y] - k * l1[_x]};

        detail::quadratic_equation<scalar_type> qe{(1.f + k * k), 2.f * k * d,
                                                   d * d - r * r};

        if (qe.solutions() > 0) {
            const scalar_type overstep_tolerance{ray.overstep_tolerance()};
            std::array<point3, 2> candidates;
            std::array<scalar_type, 2> t01 = {0.f, 0.f};

            candidates[0][_x] = qe.smaller();
            candidates[0][_y] = k * qe.smaller() + d;
            t01[0] = (candidates[0][_x] - ro[_x]) / rd[_x];
            candidates[0][2] = ro[2] + t01[0] * rd[2];

            candidates[1][_x] = qe.larger();
            candidates[1][_y] = k * qe.larger() + d;
            t01[1] = (candidates[1][_x] - ro[_x]) / rd[_x];
            candidates[1][2] = ro[2] + t01[1] * rd[2];

            // Chose the index, take the smaller positive one
            const unsigned int cindex =
                (t01[0] < t01[1] and t01[0] > overstep_tolerance)
                    ? 0u
                    : (t01[0] < overstep_tolerance and
                               t01[1] > overstep_tolerance
                           ? 1u
                           : 0u);
            if (t01[0] > overstep_tolerance or t01[1] > overstep_tolerance) {
                is.p3 = candidates[cindex];
                is.path = t01[cindex];

                const scalar_type phi{getter::phi(is.p3)};
                is.p2 = point2{r * phi, is.p3[2]};
                // In this case, the point has to be in cylinder3 coordinates
                // for the r-check
                if constexpr (mask_t::shape::check_radius) {
                    is.status = mask.is_inside(is.p3, mask_tolerance);
                } else {
                    is.status = mask.is_inside(is.p2, mask_tolerance);
                }

                // prepare some additional information in case the intersection
                // is valid
                if (is.status == intersection::status::e_inside) {
                    is.surface = sf;
                    is.direction = std::signbit(is.path)
                                       ? intersection::direction::e_opposite
                                       : intersection::direction::e_along;
                    is.volume_link = mask.volume_link();

                    // Get incidence angle
                    const vector3 normal = {std::cos(phi), std::sin(phi), 0.f};
                    is.cos_incidence_angle = vector::dot(rd, normal);
                }
            }
        }
        return is;
    }

    /// Operator function to find intersections between a ray and a 2D cylinder
    ///
    /// @tparam mask_t is the input mask type
    /// @tparam surface_t is the type of surface handle
    ///
    /// @param ray is the input ray trajectory
    /// @param sfi the intersection to be updated
    /// @param mask is the input mask that defines the surface extent
    /// @param trf is the surface placement transform
    /// @param mask_tolerance is the tolerance for mask edges
    template <
        typename mask_t, typename surface_t,
        std::enable_if_t<std::is_same_v<typename mask_t::measurement_frame_type,
                                        cylindrical2<transform3_t>>,
                         bool> = true>
    DETRAY_HOST_DEVICE inline void update(
        const ray_type &ray, intersection2D<surface_t, transform3_t> &sfi,
        const mask_t &mask, const transform3_t &trf,
        const scalar_type mask_tolerance = 0.f) const {
        sfi = this->operator()(ray, sfi.surface, mask, trf, mask_tolerance)[0];
    }
};

}  // namespace detray

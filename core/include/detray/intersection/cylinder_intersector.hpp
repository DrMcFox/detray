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

/// A functor to find intersections between a ray and a 2D cylinder mask
template <typename transform3_t>
struct cylinder_intersector {

    /// linear algebra types
    /// @{
    using scalar_type = typename transform3_t::scalar_type;
    using point3 = typename transform3_t::point3;
    using point2 = typename transform3_t::point2;
    using vector3 = typename transform3_t::vector3;
    /// @}
    using ray_type = detail::ray<transform3_t>;

    /// Operator function to find intersections between a ray and a 2D cylinder
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
    /// @return the intersections.
    template <
        typename mask_t, typename surface_t,
        std::enable_if_t<std::is_same_v<typename mask_t::measurement_frame_type,
                                        cylindrical2<transform3_t>>,
                         bool> = true>
    DETRAY_HOST_DEVICE inline std::array<
        intersection2D<surface_t, transform3_t>, 2>
    operator()(const ray_type &ray, const surface_t sf, const mask_t &mask,
               const transform3_t &trf,
               const scalar_type mask_tolerance = 0.f) const {

        using intersection_t = intersection2D<surface_t, transform3_t>;

        // One or both of these solutions might be invalid
        const auto qe = solve_intersection(ray, mask, trf);

        std::array<intersection_t, 2> ret;
        switch (qe.solutions()) {
            case 2:
                ret[1] = build_candidate<intersection_t>(
                    ray, mask, trf, qe.larger(), mask_tolerance);
                ret[1].surface = sf;
                // If there are two solutions, reuse the case for a single
                // solution to setup the intersection with the smaller path
                // in ret[0]
                [[fallthrough]];
            case 1:
                ret[0] = build_candidate<intersection_t>(
                    ray, mask, trf, qe.smaller(), mask_tolerance);
                ret[0].surface = sf;
                break;
            case 0:
                ret[0].status = intersection::status::e_missed;
                ret[1].status = intersection::status::e_missed;
        };

        // Even if there are two geometrically valid solutions, the smaller one
        // might not be passed on if it is below the overstepping tolerance:
        // see 'build_candidate'
        return ret;
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

        using intersection_t = intersection2D<surface_t, transform3_t>;

        // One or both of these solutions might be invalid
        const auto qe = solve_intersection(ray, mask, trf);

        switch (qe.solutions()) {
            case 1:
                sfi = build_candidate<intersection_t>(
                    ray, mask, trf, qe.smaller(), mask_tolerance);
                break;
            case 0:
                sfi.status = intersection::status::e_missed;
        };
    }

    protected:
    /// Calculates the distance to the (two) intersection points on the
    /// cylinder in global coordinates.
    ///
    /// @returns a quadratic equation object that contains the solution(s).
    template <typename mask_t>
    DETRAY_HOST_DEVICE inline detail::quadratic_equation<scalar_type>
    solve_intersection(const ray_type &ray, const mask_t &mask,
                       const transform3_t &trf) const {
        const scalar_type r{mask[mask_t::shape::e_r]};
        const auto &m = trf.matrix();
        const vector3 sz = getter::vector<3>(m, 0u, 2u);
        const vector3 sc = getter::vector<3>(m, 0u, 3u);

        const point3 &ro = ray.pos();
        const vector3 &rd = ray.dir();

        const auto pc_cross_sz = vector::cross(ro - sc, sz);
        const auto rd_cross_sz = vector::cross(rd, sz);
        const scalar_type a{vector::dot(rd_cross_sz, rd_cross_sz)};
        const scalar_type b{2.f * vector::dot(rd_cross_sz, pc_cross_sz)};
        const scalar_type c{vector::dot(pc_cross_sz, pc_cross_sz) - (r * r)};

        return detail::quadratic_equation<scalar_type>{a, b, c};
    }

    /// From the intersection path, construct an intersection candidate and
    /// check it against the surface boundaries (mask).
    ///
    /// @returns the intersection candidate. Might be (partially) uninitialized
    /// if the overstepping tolerance is not met or the intersection lies
    /// outside of the mask.
    template <typename intersection_t, typename mask_t>
    DETRAY_HOST_DEVICE inline intersection_t build_candidate(
        const ray_type &ray, const mask_t &mask, const transform3_t &trf,
        const scalar_type path, const scalar_type mask_tolerance = 0.f) const {

        intersection_t is;

        // Construct the candidate only when needed
        if (path >= ray.overstep_tolerance()) {

            const point3 &ro = ray.pos();
            const vector3 &rd = ray.dir();

            is.path = path;
            is.p3 = ro + is.path * rd;

            // The point has to be in cylinder3 coordinates for the r-check
            if constexpr (mask_t::shape::check_radius) {
                const auto loc3D = mask.to_local_frame(trf, is.p3);
                is.status = mask.is_inside(loc3D, mask_tolerance);
                // Go from local to measurement frame
                is.p2 = point2{loc3D[0] * loc3D[1], loc3D[2]};
            } else {
                // local frame and measurement frame are identical
                is.p2 = mask.to_measurement_frame(trf, is.p3);
                is.status = mask.is_inside(is.p2, mask_tolerance);
            }

            // prepare some additional information in case the intersection
            // is valid
            if (is.status == intersection::status::e_inside) {
                is.direction = std::signbit(is.path)
                                   ? intersection::direction::e_opposite
                                   : intersection::direction::e_along;
                is.volume_link = mask.volume_link();

                // Get incidence angle
                const scalar_type phi{is.p2[0] / mask[mask_t::shape::e_r]};
                const vector3 normal = {math_ns::cos(phi), math_ns::sin(phi),
                                        0.f};
                is.cos_incidence_angle = vector::dot(rd, normal);
            }
        } else {
            is.status = intersection::status::e_missed;
        }

        return is;
    }
};

}  // namespace detray

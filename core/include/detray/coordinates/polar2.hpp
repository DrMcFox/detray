/** Detray library, part of the ACTS project
 *
 * (c) 2022-2023 CERN for the benefit of the ACTS project
 *
 * Mozilla Public License Version 2.0
 */

#pragma once

// Project include(s).
#include "detray/coordinates/coordinate_base.hpp"
#include "detray/definitions/math.hpp"
#include "detray/definitions/qualifiers.hpp"

// System include(s).
#include <cmath>

namespace detray {

template <typename transform3_t>
struct polar2 : public coordinate_base<polar2, transform3_t> {

    /// @name Type definitions for the struct
    /// @{

    // Base type
    using base_type = coordinate_base<polar2, transform3_t>;
    // Sclar type
    using scalar_type = typename base_type::scalar_type;
    // Point in 2D space
    using point2 = typename base_type::point2;
    // Point in 3D space
    using point3 = typename base_type::point3;
    // Vector in 3D space
    using vector3 = typename base_type::vector3;
    /// Matrix actor
    using matrix_operator = typename base_type::matrix_operator;
    /// Matrix size type
    using size_type = typename base_type::size_type;
    // 2D matrix type
    template <size_type ROWS, size_type COLS>
    using matrix_type = typename base_type::template matrix_type<ROWS, COLS>;
    // Rotation Matrix
    using rotation_matrix = typename base_type::rotation_matrix;
    // Vector types
    using bound_vector = typename base_type::bound_vector;
    using free_vector = typename base_type::free_vector;
    // Track Helper
    using track_helper = typename base_type::track_helper;
    // Matrix types
    using free_to_bound_matrix = typename base_type::free_to_bound_matrix;
    using bound_to_free_matrix = typename base_type::bound_to_free_matrix;

    // Local point type in polar coordinates
    using loc_point = point2;

    /** This method transform from a point from 2D cartesian frame to a 2D
     * polar point */
    DETRAY_HOST_DEVICE inline point2 operator()(const point2 &p) const {

        return {getter::perp(p), getter::phi(p)};
    }

    /** This method transform from a point in 3D cartesian frame to a 2D
     * polar point */
    DETRAY_HOST_DEVICE inline point2 operator()(const point3 &p) const {

        return {getter::perp(p), getter::phi(p)};
    }

    /** This method transform from a point from global cartesian 3D frame to a
     * local 2D polar point */
    DETRAY_HOST_DEVICE
    inline point2 global_to_local(const transform3_t &trf, const point3 &p,
                                  const vector3 & /*d*/) const {
        const auto local3 = trf.point_to_local(p);
        return this->operator()(local3);
    }

    /** This method transform from a local 2D polar point to a point global
     * cartesian 3D frame*/
    template <typename mask_t>
    DETRAY_HOST_DEVICE inline point3 local_to_global(
        const transform3_t &trf, const mask_t & /*mask*/, const point2 &p,
        const vector3 & /*d*/) const {
        const scalar_type x = p[0] * math_ns::cos(p[1]);
        const scalar_type y = p[0] * math_ns::sin(p[1]);

        return trf.point_to_global(point3{x, y, 0.f});
    }

    template <typename mask_t>
    DETRAY_HOST_DEVICE inline vector3 normal(const transform3_t &trf3,
                                             const mask_t & /*mask*/,
                                             const point3 & /*pos*/,
                                             const vector3 & /*dir*/) const {
        vector3 ret;
        const auto n =
            matrix_operator().template block<3, 1>(trf3.matrix(), 0u, 2u);
        ret[0] = matrix_operator().element(n, 0u, 0u);
        ret[1] = matrix_operator().element(n, 1u, 0u);
        ret[2] = matrix_operator().element(n, 2u, 0u);
        return ret;
    }

    template <typename mask_t>
    DETRAY_HOST_DEVICE inline rotation_matrix reference_frame(
        const transform3_t &trf3, const mask_t & /*mask*/,
        const point3 & /*pos*/, const vector3 & /*dir*/) const {
        return trf3.rotation();
    }

    template <typename mask_t>
    DETRAY_HOST_DEVICE inline void set_bound_pos_to_free_pos_derivative(
        bound_to_free_matrix &bound_to_free_jacobian, const transform3_t &trf3,
        const mask_t &mask, const point3 &pos, const vector3 &dir) const {

        matrix_type<3, 2> bound_pos_to_free_pos_derivative =
            matrix_operator().template zero<3, 2>();

        const point2 local2 = this->global_to_local(trf3, pos, dir);
        const scalar_type lrad{local2[0]};
        const scalar_type lphi{local2[1]};

        const scalar_type lcos_phi{math_ns::cos(lphi)};
        const scalar_type lsin_phi{math_ns::sin(lphi)};

        // reference matrix
        const auto frame = reference_frame(trf3, mask, pos, dir);

        // dxdu = d(x,y,z)/du
        const matrix_type<3, 1> dxdL =
            matrix_operator().template block<3, 1>(frame, 0u, 0u);
        // dxdv = d(x,y,z)/dv
        const matrix_type<3, 1> dydL =
            matrix_operator().template block<3, 1>(frame, 0u, 1u);

        const matrix_type<3, 1> col0 = dxdL * lcos_phi + dydL * lsin_phi;
        const matrix_type<3, 1> col1 =
            (dydL * lcos_phi - dxdL * lsin_phi) * lrad;

        matrix_operator().template set_block<3, 1>(
            bound_pos_to_free_pos_derivative, col0, e_free_pos0, e_bound_loc0);
        matrix_operator().template set_block<3, 1>(
            bound_pos_to_free_pos_derivative, col1, e_free_pos0, e_bound_loc1);

        matrix_operator().template set_block(bound_to_free_jacobian,
                                             bound_pos_to_free_pos_derivative,
                                             e_free_pos0, e_bound_loc0);
    }

    template <typename mask_t>
    DETRAY_HOST_DEVICE inline void set_free_pos_to_bound_pos_derivative(
        free_to_bound_matrix &free_to_bound_jacobian, const transform3_t &trf3,
        const mask_t &mask, const point3 &pos, const vector3 &dir) const {

        matrix_type<2, 3> free_pos_to_bound_pos_derivative =
            matrix_operator().template zero<2, 3>();

        const auto local = this->global_to_local(trf3, pos, dir);

        const scalar_type lrad{local[0]};
        const scalar_type lphi{local[1]};

        const scalar_type lcos_phi{math_ns::cos(lphi)};
        const scalar_type lsin_phi{math_ns::sin(lphi)};

        // reference matrix
        const auto frame = reference_frame(trf3, mask, pos, dir);
        const auto frameT = matrix_operator().transpose(frame);

        // dudG = du/d(x,y,z)
        const matrix_type<1, 3> dudG =
            matrix_operator().template block<1, 3>(frameT, 0u, 0u);
        // dvdG = dv/d(x,y,z)
        const matrix_type<1, 3> dvdG =
            matrix_operator().template block<1, 3>(frameT, 1u, 0u);

        const matrix_type<1, 3> row0 = dudG * lcos_phi + dvdG * lsin_phi;
        const matrix_type<1, 3> row1 =
            1. / lrad * (lcos_phi * dvdG - lsin_phi * dudG);

        matrix_operator().template set_block<1, 3>(
            free_pos_to_bound_pos_derivative, row0, e_bound_loc0, e_free_pos0);
        matrix_operator().template set_block<1, 3>(
            free_pos_to_bound_pos_derivative, row1, e_bound_loc1, e_free_pos0);

        matrix_operator().template set_block(free_to_bound_jacobian,
                                             free_pos_to_bound_pos_derivative,
                                             e_bound_loc0, e_free_pos0);
    }

    template <typename mask_t>
    DETRAY_HOST_DEVICE inline void set_bound_angle_to_free_pos_derivative(
        bound_to_free_matrix & /*bound_to_free_jacobian*/,
        const transform3_t & /*trf3*/, const mask_t & /*mask*/,
        const point3 & /*pos*/, const vector3 & /*dir*/) const {
        // Do nothing
    }
};

}  // namespace detray
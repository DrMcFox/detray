/** Algebra plugins library, part of the ACTS project
 *
 * (c) 2022-2023 CERN for the benefit of the ACTS project
 *
 * Mozilla Public License Version 2.0
 */

#pragma once

// Project include(s).
#include "detray/definitions/indexing.hpp"
#include "detray/definitions/qualifiers.hpp"
#include "detray/definitions/track_parametrization.hpp"
#include "detray/geometry/barcode.hpp"
#include "detray/tracks/detail/track_helper.hpp"

namespace detray {

template <typename transform3_t>
struct bound_track_parameters {

    /// @name Type definitions for the struct
    /// @{

    using transform3_type = transform3_t;
    using matrix_operator = typename transform3_type::matrix_actor;
    using size_type = typename transform3_type::size_type;
    using scalar_type = typename transform3_type::scalar_type;
    template <size_type ROWS, size_type COLS>
    using matrix_type =
        typename transform3_type::matrix_actor::template matrix_type<ROWS,
                                                                     COLS>;
    using vector3 = typename transform3_type::vector3;
    using point3 = typename transform3_type::point3;
    using point2 = typename transform3_type::point2;

    // Shorthand vector/matrix types related to bound track parameters.
    using vector_type = matrix_type<e_bound_size, 1>;
    using covariance_type = matrix_type<e_bound_size, e_bound_size>;

    // Track helper
    using track_helper = detail::track_helper<matrix_operator>;

    /// @}

    DETRAY_HOST_DEVICE
    bound_track_parameters()
        : m_barcode(),
          m_vector(matrix_operator().template zero<e_bound_size, 1>()),
          m_covariance(
              matrix_operator().template zero<e_bound_size, e_bound_size>()) {}

    DETRAY_HOST_DEVICE
    bound_track_parameters(const geometry::barcode sf_idx,
                           const vector_type& vec, const covariance_type& cov)
        : m_barcode(sf_idx), m_vector(vec), m_covariance(cov) {}

    /** @param rhs is the left hand side params for comparison
     **/
    DETRAY_HOST_DEVICE
    bool operator==(const bound_track_parameters& rhs) const {
        if (m_barcode != rhs.surface_link()) {
            return false;
        }

        for (unsigned int i = 0u; i < e_bound_size; i++) {
            const auto lhs_val = matrix_operator().element(m_vector, i, 0u);
            const auto rhs_val = matrix_operator().element(rhs.vector(), i, 0u);

            if (std::abs(lhs_val - rhs_val) >
                std::numeric_limits<scalar_type>::epsilon()) {
                return false;
            }
        }
        for (unsigned int i = 0u; i < e_bound_size; i++) {
            for (unsigned int j = 0u; j < e_bound_size; j++) {
                const auto lhs_val =
                    matrix_operator().element(m_covariance, i, j);
                const auto rhs_val =
                    matrix_operator().element(rhs.covariance(), i, j);

                if (std::abs(lhs_val - rhs_val) >
                    std::numeric_limits<scalar_type>::epsilon()) {
                    return false;
                }
            }
        }
        return true;
    }

    DETRAY_HOST_DEVICE
    const geometry::barcode& surface_link() const { return m_barcode; }

    DETRAY_HOST_DEVICE
    void set_surface_link(geometry::barcode link) { m_barcode = link; }

    DETRAY_HOST_DEVICE
    vector_type& vector() { return m_vector; }

    DETRAY_HOST_DEVICE
    const vector_type& vector() const { return m_vector; }

    DETRAY_HOST_DEVICE
    void set_vector(const vector_type& v) { m_vector = v; }

    DETRAY_HOST_DEVICE
    covariance_type& covariance() { return m_covariance; }

    DETRAY_HOST_DEVICE
    const covariance_type& covariance() const { return m_covariance; }

    DETRAY_HOST_DEVICE
    void set_covariance(const covariance_type& c) { m_covariance = c; }

    DETRAY_HOST_DEVICE
    point2 local() const { return track_helper().local(m_vector); }

    DETRAY_HOST_DEVICE
    scalar_type phi() const {
        return matrix_operator().element(m_vector, e_bound_phi, 0u);
    }

    DETRAY_HOST_DEVICE
    scalar_type theta() const {
        return matrix_operator().element(m_vector, e_bound_theta, 0u);
    }

    DETRAY_HOST_DEVICE
    vector3 dir() const { return track_helper().dir(m_vector); }

    DETRAY_HOST_DEVICE
    scalar_type time() const {
        return matrix_operator().element(m_vector, e_bound_time, 0u);
    }

    DETRAY_HOST_DEVICE
    scalar_type charge() const {
        if (std::signbit(
                matrix_operator().element(m_vector, e_bound_qoverp, 0u))) {
            return -1.f;
        } else {
            return 1.f;
        }
    }

    DETRAY_HOST_DEVICE
    scalar_type qop() const {
        return matrix_operator().element(m_vector, e_bound_qoverp, 0u);
    }

    DETRAY_HOST_DEVICE
    void set_qop(const scalar_type qop) {
        matrix_operator().element(m_vector, e_bound_qoverp, 0u) = qop;
    }

    DETRAY_HOST_DEVICE
    scalar_type p() const { return charge() / qop(); }

    DETRAY_HOST_DEVICE
    vector3 mom() const { return this->p() * this->dir(); }

    private:
    geometry::barcode m_barcode;
    vector_type m_vector;
    covariance_type m_covariance;
};

}  // namespace detray
/** Detray library, part of the ACTS project (R&D line)
 *
 * (c) 2022-2023 CERN for the benefit of the ACTS project
 *
 * Mozilla Public License Version 2.0
 */

#pragma once

// Project include(s)
#include "detray/definitions/algebra.hpp"
#include "detray/definitions/units.hpp"
#include "detray/materials/material.hpp"

// System include(s)
#include <limits>

namespace detray {

/**
 * Elements Declaration
 * @note: Values from
 * https://pdg.lbl.gov/2020/AtomicNuclearProperties/index.html (Last revised 04
 * June 2020)
 */
// Vacuum
DETRAY_DECLARE_MATERIAL(vacuum, std::numeric_limits<scalar>::infinity(),
                        std::numeric_limits<scalar>::infinity(), 0.f, 0.f, 0.f,
                        material_state::e_gas);

// H₂ (1): Hydrogen Gas
DETRAY_DECLARE_MATERIAL(hydrogen_gas, 7.526E3f * unit<scalar>::m,
                        6.209E3f * unit<scalar>::m, 2.016f, 2.f,
                        static_cast<scalar>(8.376E-5 * unit<double>::g /
                                            unit<double>::cm3),
                        material_state::e_gas);

// H₂ (1): Hydrogen Liquid
DETRAY_DECLARE_MATERIAL(hydrogen_liquid, 8.904f * unit<scalar>::m,
                        7.346f * unit<scalar>::m, 2.016f, 2.f,
                        static_cast<scalar>(0.07080f * unit<double>::g /
                                            unit<double>::cm3),
                        material_state::e_liquid);

// He (2): Helium Gas
DETRAY_DECLARE_MATERIAL(helium_gas, 5.671E3f * unit<scalar>::m,
                        4.269E3f * unit<scalar>::m, 4.003f, 2.f,
                        static_cast<scalar>(1.663E-4 * unit<double>::g /
                                            unit<double>::cm3),
                        material_state::e_gas);

// Be (4)
DETRAY_DECLARE_MATERIAL(beryllium, 352.8f * unit<scalar>::mm,
                        421.0f * unit<scalar>::mm, 9.012f, 4.f,
                        static_cast<scalar>(1.848 * unit<double>::g /
                                            unit<double>::cm3),
                        material_state::e_solid);

// C (6): Carbon (amorphous)
DETRAY_DECLARE_MATERIAL(carbon_gas, 213.5f * unit<scalar>::mm,
                        429.0f * unit<scalar>::mm, 12.01f, 6.f,
                        static_cast<scalar>(2.0 * unit<double>::g /
                                            unit<double>::cm3),
                        material_state::e_gas);

// N₂ (7): Nitrogen Gas
DETRAY_DECLARE_MATERIAL(nitrogen_gas, 3.260E+02f * unit<scalar>::m,
                        7.696E+02f * unit<scalar>::m, 28.014f, 14.f,
                        static_cast<scalar>(1.165E-03 * unit<double>::g /
                                            unit<double>::cm3),
                        material_state::e_gas);

// O₂ (8): Oxygen Gas
DETRAY_DECLARE_MATERIAL(oxygen_gas, 2.571E+02f * unit<scalar>::m,
                        6.772E+02f * unit<scalar>::m, 31.998f, 16.f,
                        static_cast<scalar>(1.332E-3 * unit<double>::g /
                                            unit<double>::cm3),
                        material_state::e_gas);

// O₂ (8): Oxygen liquid
DETRAY_DECLARE_MATERIAL(oxygen_liquid, 300.1f * unit<scalar>::mm,
                        790.3f * unit<scalar>::mm, 31.998f, 16.f,
                        static_cast<scalar>(1.141 * unit<double>::g /
                                            unit<double>::cm3),
                        material_state::e_liquid);

// Al (13)
DETRAY_DECLARE_MATERIAL(aluminium, 88.97f * unit<scalar>::mm,
                        397.0f * unit<scalar>::mm, 26.98f, 13.f,
                        static_cast<scalar>(2.699 * unit<double>::g /
                                            unit<double>::cm3),
                        material_state::e_solid);

// Si (14)
DETRAY_DECLARE_MATERIAL(silicon, 93.7f * unit<scalar>::mm,
                        465.2f * unit<scalar>::mm, 28.0855f, 14.f,
                        static_cast<scalar>(2.329 * unit<double>::g /
                                            unit<double>::cm3),
                        material_state::e_solid);

// Ar (18): Argon gas
DETRAY_DECLARE_MATERIAL(argon_gas, 1.176E+02f * unit<scalar>::m,
                        7.204E+02f * unit<scalar>::m, 39.948f, 18.f,
                        static_cast<scalar>(1.662E-03 * unit<double>::g /
                                            unit<double>::cm3),
                        material_state::e_gas);

// W (74)
DETRAY_DECLARE_MATERIAL(tungsten, 3.504f * unit<scalar>::mm,
                        99.46f * unit<scalar>::mm, 183.84f, 74.f,
                        static_cast<scalar>(19.3 * unit<double>::g /
                                            unit<double>::cm3),
                        material_state::e_solid);

// Au (79)
DETRAY_DECLARE_MATERIAL(gold, 3.344f * unit<scalar>::mm,
                        101.6f * unit<scalar>::mm, 196.97f, 79.f,
                        static_cast<scalar>(19.32 * unit<double>::g /
                                            unit<double>::cm3),
                        material_state::e_solid);

/**
 * Elements Declaration for ACTS Generic detector
 * @note: Values taken from BuildGenericDetector.hpp in ACTS
 */

// Be (4)
DETRAY_DECLARE_MATERIAL(beryllium_tml, 352.8f * unit<scalar>::mm,
                        407.f * unit<scalar>::mm, 9.012f, 4.f,
                        static_cast<scalar>(1.848 * unit<double>::g /
                                            unit<double>::cm3),
                        material_state::e_solid);

// Si (14)
DETRAY_DECLARE_MATERIAL(silicon_tml, 95.7f * unit<scalar>::mm,
                        465.2f * unit<scalar>::mm, 28.03f, 14.f,
                        static_cast<scalar>(2.32 * unit<double>::g /
                                            unit<double>::cm3),
                        material_state::e_solid);

/**
 * Mixtures or Compounds
 */

// Air (dry, 1 atm)
// @note:
// https://pdg.lbl.gov/2020/AtomicNuclearProperties/HTML/air_dry_1_atm.html
// @note: Ar from Wikipedia (https://en.wikipedia.org/wiki/Molar_mass)
DETRAY_DECLARE_MATERIAL(air, 3.039E+02f * unit<scalar>::m,
                        7.477E+02f * unit<scalar>::m, 28.97f, 14.46f,
                        static_cast<scalar>(1.205E-03 * unit<double>::g /
                                            unit<double>::cm3),
                        material_state::e_gas);

// (CH3)2CHCH3 Gas
// @note: (X0, L0, mass_rho) from https://pdg.lbl.gov/2005/reviews/atomicrpp.pdf
// @note: Ar from Wikipedia (https://en.wikipedia.org/wiki/Isobutane)
// @note: Z was caculated by simply summing the number of atoms. Surprisingly
// it seems the right value because Z/A is 0.58496, which is the same with <Z/A>
// in the pdg refernce
DETRAY_DECLARE_MATERIAL(isobutane, 1693E+02f * unit<scalar>::mm,
                        288.3f * unit<scalar>::mm, 58.124f, 34.f,
                        static_cast<scalar>(2.67 * unit<double>::g /
                                            unit<double>::cm3),
                        material_state::e_gas);

// C3H8 Gas
// @note: (X0, L0, mass_rho) from
// https://pdg.lbl.gov/2020/AtomicNuclearProperties/HTML/propane.html
// @note: Ar from Wikipedia (https://en.wikipedia.org/wiki/Propane)
DETRAY_DECLARE_MATERIAL(propane, 2.429E+02f * unit<scalar>::m,
                        4.106E+02f * unit<scalar>::m, 44.097f, 26.f,
                        static_cast<scalar>(1.868E-03 * unit<double>::g /
                                            unit<double>::cm3),
                        material_state::e_gas);

}  // namespace detray
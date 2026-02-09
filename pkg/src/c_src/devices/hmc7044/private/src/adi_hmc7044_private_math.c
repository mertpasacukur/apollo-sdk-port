/*!
 * \brief     Private HMC7044 Math Functions
 *
 * \copyright copyright(c) 2023 Analog Devices, Inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated Analog Devices software license agreement.
 */

#include "adi_hmc7044_private_math.h"

static void rational_gcd(uint64_t n1, uint64_t d1, uint64_t n2, uint64_t d2, uint64_t *n, uint64_t *d);

uint64_t adi_hmc7044_private_math_gcd(uint64_t a, uint64_t b)
{
    if (a == 0) {
        return b;
    }

    if (b == 0) {
        return a;
    }

    return a > b ? adi_hmc7044_private_math_gcd(a % b, b)
                 : adi_hmc7044_private_math_gcd(a, b % a);
}

uint64_t adi_hmc7044_private_math_lcm(uint64_t a, uint64_t b)
{
    return a && b ? (a / adi_hmc7044_private_math_gcd(a, b)) * b
                  : 0;
}

void adi_hmc7044_private_math_rational_reduce(uint64_t *n, uint64_t *d)
{
    uint64_t gcd = adi_hmc7044_private_math_gcd(*n, *d);
    *n /= gcd;
    *d /= gcd;
}

void adi_hmc7044_private_math_rational_mod(uint64_t n1, uint64_t d1, uint64_t n2, uint64_t d2, uint64_t *n, uint64_t *d)
{
    *n = (n1 * d2) - (((n1 * d2) / (d1 * n2)) * d1 * n2);
    *d = (d1 * d2);
    adi_hmc7044_private_math_rational_reduce(n, d);
}

void adi_hmc7044_private_math_rational_gcd(uint64_t n1, uint64_t d1, uint64_t n2, uint64_t d2, uint64_t *n, uint64_t *d)
{
    adi_hmc7044_private_math_rational_reduce(&n1, &d1);
    adi_hmc7044_private_math_rational_reduce(&n2, &d2);
    rational_gcd(n1, d1, n2, d2, n, d);
}

void adi_hmc7044_private_math_rational_lcm(uint64_t n1, uint64_t d1, uint64_t n2, uint64_t d2, uint64_t *n, uint64_t *d)
{
    uint64_t gcd_n = 0, gcd_d = 0;
    adi_hmc7044_private_math_rational_reduce(&n1, &d1);
    adi_hmc7044_private_math_rational_reduce(&n2, &d2);

    if (n1 == 0 || n2 == 0) {
        *n = 0;
        *d = 1;
    } else {
        rational_gcd(n1, d1, n2, d2, &gcd_n, &gcd_d);
        *n = n1 * n2 * gcd_d;
        *d = d1 * d2 * gcd_n;
        adi_hmc7044_private_math_rational_reduce(n, d);
    }
}

static void rational_gcd(uint64_t n1, uint64_t d1, uint64_t n2, uint64_t d2, uint64_t *n, uint64_t *d)
{
    uint64_t mod_n = 0, mod_d = 0;

    if (!n1) {
        *n = n2;
        *d = d2;
        return;
    }

    if (!n2) {
        *n = n1;
        *d = d1;
        return;
    }

    if ((n1 * d2) > (n2 * d1)) {
        adi_hmc7044_private_math_rational_mod(n1, d1, n2, d2, &mod_n, &mod_d);
        rational_gcd(mod_n, mod_d, n2, d2, n, d);
    } else {
        adi_hmc7044_private_math_rational_mod(n2, d2, n1, d1, &mod_n, &mod_d);
        rational_gcd(n1, d1, mod_n, mod_d, n, d);
    }
}

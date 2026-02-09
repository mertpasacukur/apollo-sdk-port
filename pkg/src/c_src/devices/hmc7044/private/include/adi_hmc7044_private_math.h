/*!
 * \brief     HMC7044 Types
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_HMC7044_PRIVATE_MATH_H__
#define __ADI_HMC7044_PRIVATE_MATH_H__

#ifdef __KERNEL__
#include <linux/kernel.h>
#else
#include <stdint.h>
#endif

uint64_t adi_hmc7044_private_math_gcd(uint64_t a, uint64_t b);

uint64_t adi_hmc7044_private_math_lcm(uint64_t a, uint64_t b);

void adi_hmc7044_private_math_rational_mod(uint64_t n1, uint64_t d1, uint64_t n2, uint64_t d2, uint64_t *n, uint64_t *d);

void adi_hmc7044_private_math_rational_gcd(uint64_t n1, uint64_t d1, uint64_t n2, uint64_t d2, uint64_t *n, uint64_t *d);

void adi_hmc7044_private_math_rational_lcm(uint64_t n1, uint64_t d1, uint64_t n2, uint64_t d2, uint64_t *n, uint64_t *d);

void adi_hmc7044_private_math_rational_reduce(uint64_t *n, uint64_t *d);

#endif

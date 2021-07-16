#ifndef _3DMATH_H
#define _3DMATH_H

#pragma once

namespace Matematica {
	typedef float real, r;

	static const real EPSILON=static_cast<real>(0.00001f);
	static const real SCALAR_TINY=static_cast<real>(1.0e-6);
	static const real SCALAR_HUGE=static_cast<real>(1.0e6);

	inline real Clamp(const real n, const real r0, const real r1)
	{
		return (n < r0) ? r0 : (n > r1) ? r1 : n;
	}

	inline bool equal(const real a, const real b)
	{
		const real d = a - b;
		if (d<EPSILON && d>-EPSILON) 
			return true;
		else 
			return false;
	}

	/// Indicates if the input is finite
	inline bool IsFinite(real val)
	{
		return ((val < SCALAR_HUGE) && (val > -SCALAR_HUGE));
	}
}

#endif
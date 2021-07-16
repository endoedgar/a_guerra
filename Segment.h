#ifndef _SEGMENT_H
#define _SEGMENT_H

#include "Vector.h"

#pragma once

namespace Matematica {
	class Segment
	{
	public:
		Vector mOrigin, mDelta;

		Segment() { }
		Segment(const Vector &origin, const Vector &delta) : mOrigin(origin), mDelta(delta) {}

		Vector getPoint(const real t) const { return mOrigin + t * mDelta; }
		Vector getEnd(void) const { return mOrigin + mDelta; }
	};
}

#endif

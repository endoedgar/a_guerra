#ifndef _POINT_H
#define _POINT_H

#pragma once

namespace Matematica {
	namespace Geometry {
		class Point : public GeometryAbstract
		{
		public:
			Point()
			{ }

			bool Intersect(const Point *another) const
			{
				return DistanceSq(this->GetPosition(), another->GetPosition()) <= EPSILON;
			}
		};
	}
}

#endif
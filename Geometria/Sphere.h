#ifndef _SPHERE_H
#define _SPHERE_H

#pragma once

#include "Point.h"

namespace Matematica {
	namespace Geometry {
		class Sphere : public GeometryAbstract
		{
			real r, rsq;
		public:
			Sphere()
			{ }

			Sphere(const real Radius):r(Radius), rsq(Radius*Radius)
			{ }

			void SetRadius(const real Radius)
			{
				this->r = Radius;
				this->rsq = Radius*Radius;
			}

			real GetRadius() const
			{
				return this->r;
			}

			inline bool Intersect(const Sphere *another) const
			{
				return DistanceSq(this->GetPosition(), another->GetPosition()) <= max(this->rsq, another->rsq);
			}

			inline bool Intersect(const Point *another) const
			{
				return DistanceSq(this->GetPosition(), another->GetPosition()) <= this->rsq;
			}
		};
	}
}
#endif
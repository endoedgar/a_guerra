#ifndef _GEOMETRY_H
#define _GEOMETRY_H

#pragma once

namespace Matematica {
	namespace Geometry {
		class GeometryAbstract abstract 
		{
			const Vector *pos;
		public:
			void SetPosition(const Vector *pos)
			{ this->pos = pos; }

			const Vector *GetPosition() const
			{ return pos; }
		};

		#include "Point.h"
		#include "Sphere.h"
	}
}
#endif
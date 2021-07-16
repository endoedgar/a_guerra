#ifndef _VECTOR_H
#define _VECTOR_H

#pragma once

namespace Matematica {
	class Vector {
	public:
		real x, y, z;

		Vector()
		{ }

		Vector(const real x, const real y, const real z):x(x), y(y), z(z)
		{ }

		Vector(const Vector &right):x(right.x), y(right.y), z(right.z)
		{ }

		Vector(const Vector *right):x(right->x), y(right->y), z(right->z)
		{ }

		inline Vector maximum(const Vector &b) const {
			Vector tmp;
			if(fabs(this->x) > fabs(b.x))
				tmp.x = this->x;
			else
				tmp.x = b.x;
			if(fabs(this->y) > fabs(b.y))
				tmp.y = this->y;
			else
				tmp.y = b.y;
			if(fabs(this->z) > fabs(b.z))
				tmp.z = this->z;
			else
				tmp.z = b.z;
			return tmp;
		}

		inline real maximumF(void) const
		{ return max(this->x, max(this->y, this->z)); }
		inline void clear()
		{ this->x = this->y = this->z = 0; }
		inline operator real*()
		{ return &x; }
		inline operator const real*()
		{ return &x; }
		inline real &operator[](const unsigned int n)
		{ return (&x)[n]; }
		inline real operator[](const unsigned int n) const
		{ return (&x)[n]; }
		static const Vector &zero() {
			static const Vector z(0, 0, 0);
			return z;
		}
		inline Vector operator-() const
		{ return Vector(-x, -y, -z); }
		inline Vector operator*( const real scale ) const
		{ return Vector( x*scale,y*scale,z*scale ); }
		inline Vector operator*( const Vector &q ) const
		{ return Vector( x*q.x,y*q.y,z*q.z ); }
		inline Vector operator/( const real scale ) const
		{ return Vector( x/scale,y/scale,z/scale ); }
		inline Vector operator/( const Vector &q ) const
		{ return Vector( x/q.x,y/q.y,z/q.z ); }
		inline Vector operator+( const Vector &q ) const
		{ return Vector( x+q.x,y+q.y,z+q.z ); }
		inline Vector operator+( const unsigned int i ) const
		{ return Vector( x+i,y+i,z+i ); }
		inline Vector operator+( const int i ) const
		{ return Vector( x+i,y+i,z+i ); }
		inline Vector operator+( const real i ) const
		{ return Vector( x+i,y+i,z+i ); }
		inline Vector operator-( const Vector &q ) const
		{ return Vector( x-q.x,y-q.y,z-q.z ); }
		inline Vector &operator*=( const real scale )
		{ x*=scale;y*=scale;z*=scale;return *this; }
		inline Vector &operator*=( const Vector &q )
		{ x*=q.x;y*=q.y;z*=q.z;return *this;}
		inline Vector &operator/=( const real scale )
		{
			ASSERT(!equal(scale, 0))
			const real invScale = static_cast<real>(1)/scale;
			x*=invScale;y*=invScale;z*=invScale;
			return *this;
		}
		inline Vector &operator/=( const Vector &q )
		{ x/=q.x;y/=q.y;z/=q.z;return *this; }
		inline Vector &operator+=( const Vector &q )
		{ x+=q.x;y+=q.y;z+=q.z;return *this; }
		inline Vector &operator-=( const Vector &q )
		{ x-=q.x;y-=q.y;z-=q.z;return *this; }
	    inline bool operator ==(const Vector &other) const
	    { return (equal(x,other.x) && equal(y,other.y) && equal(z,other.z)); }
	    inline bool operator !=(const Vector &other) const
	    { return !(*this==other); }
		inline real dot( const Vector &q ) const
		{ return x*q.x+y*q.y+z*q.z; }
		inline Vector cross( const Vector &q ) const
		{ return Vector( y*q.z - z*q.y , z*q.x - x*q.z, x*q.y - y*q.x ); }
		inline real lengthsq()const
		{ return x*x+y*y+z*z; }
		inline real length()const
		{ return sqrt(this->lengthsq()); }
		inline real distance( const Vector &q )const
		{ const real dx=x-q.x,dy=y-q.y,dz=z-q.z;return sqrt(dx*dx+dy*dy+dz*dz); }
		inline Vector normalized()const
		{ Vector tmp; return getNormalized(tmp); }
		inline void normalize()
		{ const real l=1/length();x*=l;y*=l;z*=l; }
		inline Vector &getNormalized(Vector &tmp) const {
			const real l=1/length();
			tmp.x*=l; tmp.y*=l; tmp.z*=l;
			return tmp;
		}
		inline Vector Transformed( const D3DXMATRIX *m )const
		{ return Vector( this->x*m->_11 + m->_21*this->y + m->_31*this->z,this->x * m->_12 + m->_22*this->y + m->_32*this->z,this->x*m->_13+m->_23*this->y+m->_33*this->z ); }
		inline real pitch() const
		{ return r(-atan2( this->y,sqrt( this->x*this->x+this->z*this->z ))); }
		inline real yaw() const
		{ return r(atan2( this->x,this->z )); }
		inline bool IsSensible(void) const
		{ return IsFinite(this->x) && IsFinite(this->y) && IsFinite(this->z); }
	};

	inline Vector operator*( const real scale, const Vector vec ) 
	{ return Vector( vec.x*scale,vec.y*scale,vec.z*scale ); }

	inline Vector operator/( const real scale, const Vector vec )
	{
		ASSERT(!equal(scale, 0))
		const real invScale = static_cast<real>(1)/scale;

		return vec*invScale;
	}

	inline Vector operator+( const real number, const Vector &q )
	{ return Vector( number+q.x,number+q.y,number+q.z ); }

	inline Vector operator-( const real number, const Vector &q )
	{ return Vector( number-q.x,number-q.y,number-q.z ); }

	inline real DistanceSq(const Vector *pos1, const Vector *pos2)
	{ return (pos1->x-pos2->x)*(pos1->x-pos2->x) + (pos1->y-pos2->y)*(pos1->y-pos2->y) + (pos1->z-pos2->z)*(pos1->z-pos2->z); }

	inline real Distance(const Vector *pos1, const Vector *pos2)
	{ return sqrt(DistanceSq(pos1, pos2)); }
}



#endif
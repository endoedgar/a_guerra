#ifndef _3DMATH_H
#error("Inclua 3DMath.h");
#endif

#ifndef _QUATERNION_H
#define _QUATERNION_H

#pragma once

namespace Matematica {
	class CQuaternion {	
	public:
		real x, y, z, w;

		CQuaternion()
		{ }

		CQuaternion(const real x, const real y, const real z, const real w):x(x), y(y), z(z), w(w)
		{ }

		CQuaternion(const real pitch, const real yaw, const real roll)
		{ this->setEulerAngles(pitch, yaw, roll); }
		
		inline void identity()
		{
			this->x = this->y = this->z = 0;
			this->w = 1;
		}

		inline void setEulerAngles(const real pitch, const real yaw, const real roll)
		{
			const real po2 = pitch/2, yo2 = yaw/2, ro2 = roll/2;

			const real cpo2 = cos(po2), spo2 = sin(po2);
			const real cyo2 = cos(yo2), syo2 = sin(yo2);
			const real cro2 = cos(ro2), sro2 = sin(ro2);

			this->w = cro2*cpo2*cyo2 + sro2*spo2*syo2;
			this->z = sro2*cpo2*cyo2 - cro2*spo2*syo2;
			this->x = cro2*spo2*cyo2 + sro2*cpo2*syo2;
			this->y = cro2*cpo2*syo2 - sro2*spo2*cyo2;
		}

		inline real pitch() const
		{
			return asin(2*(this->w*this->x - this->y*this->z));
		}

		inline real yaw() const
		{
			return atan2(2*(this->w*this->y + this->z*this->x), 1-2*(this->x*this->x + this->y*this->y));
		}

		inline real roll() const
		{
			return atan2(2*(this->w*this->z + this->x*this->y), 1-2*(this->z*this->z + this->x*this->x));
		}


		inline CQuaternion& operator+=(const CQuaternion &b)
		{
			this->w += b.w;
			this->x += b.x;
			this->y += b.y;
			this->z += b.z;
			return *this;
		}

		inline CQuaternion operator -() const {
			return CQuaternion(-x, -y, -z, w);
		}

		inline CQuaternion operator +(const CQuaternion& q) const {
			return CQuaternion(this->x+q.x, this->y+q.y, this->z+q.z, this->w+q.w);
		}

		inline CQuaternion operator -(const CQuaternion& q) const {
			return CQuaternion(this->x-q.x, this->y-q.y, this->z-q.z, this->w-q.w);
		}

		inline CQuaternion operator *(const CQuaternion &q ) const
		{
			const real rx = (q.y*this->z - q.z*this->y) + q.x *this->w+this->x*q.w;
			const real ry = (q.z*this->x - q.x*this->z) + q.y *this->w+this->y*q.w;
			const real rz = (q.x*this->y - q.y*this->x) + q.z *this->w+this->z*q.w;
			const real rw = this->w*q.w-(this->x*q.x+this->y*q.y+this->z*q.z);
			return CQuaternion(rx, ry, rz, rw);
		}

		inline CQuaternion operator *(const CQuaternion *q) const
		{
			return *this * *q; 
		}

		inline CQuaternion &operator=(const CQuaternion &b)
		{
			this->x = b.x;
			this->y = b.y;
			this->z = b.z;
			this->w = b.w;
			return *this;
		}

		inline CQuaternion &operator*=(const CQuaternion &b)
		{
			*this = (*this) * b;
			return *this;
		}

		inline CQuaternion operator*( const real q )const{
			return CQuaternion( this->x*q, this->y*q, this->z*q, this->w*q);
		}

		inline CQuaternion operator/( const real q )const{
			return CQuaternion( this->x/q, this->y/q, this->z/q, this->w/q );
		}

		//inline CQuaternion& operator *=(CQuaternion &a, const CQuaternion &b) {
		//	return (a * q);
		//}

		inline real dot( const CQuaternion &q )const{
			return this->x*q.x+this->y*q.y+this->z*q.z+this->w*q.w;
		}

		inline real normal() const {
			return this->w*this->w+this->x*this->x+this->y*this->y+this->z*this->z;
		}

		inline real length() const {
			return sqrt( this->normal() );
		}

		inline void normalize(){
			*this=*this/length();
		}

		inline CQuaternion normalized()const{
			return *this/length();
		}

		CQuaternion *slerp(CQuaternion *qOut, const CQuaternion *q, real t)const{
			ASSERT(qOut)
			if(t >= 1) {
				*qOut = *q;
			} else if(t <= 0) {
				*qOut = *this;
			} else {
				real flip = 1;

				real cosine = w*q->w + x*q->x + y*q->y + z*q->z;
			
				if (cosine<0) { 
					cosine = -cosine; 
					flip = -1; 
				} 
			
				if ((1-cosine)>=EPSILON) {			
					const real theta = acos(cosine); 
					const real sine = 1/sin(theta); 
					const real beta = sin((1-t)*theta) * sine; 
					const real alpha = sin(t*theta) * sine * flip; 
			
					*qOut = *this * beta + *q * alpha;
				} else
					*qOut = *this * (1-t) + *q * (t*flip);
			}
			return qOut;
		}

		inline CQuaternion inverse() const
		{
			const real n = this->normal();
			ASSERT(n!=0);
			return CQuaternion(-x/n, -y/n, -z/n, w/n); 
		}

		/// calculate inverse of quaternion and store result in parameter.
		inline void conjugate(CQuaternion &result) const
		{
			result = CQuaternion(-x, -y, -z, w);
		}

		inline void inverse(CQuaternion &result) const
		{
			const real n = this->normal();
			result = CQuaternion(-x/n, -y/n, -z/n, w/n);
		}

		inline Vector *apply(Vector *out, const Vector *in) const
		{
			CQuaternion v(in->x, in->y, in->z, 0), i, t;

			ASSERT(!((x == 0.0) && (y == 0.0) && (z == 0.0) && (w == 0.0)));

			i = this->inverse();
			t = i * v;
			v = t * this;

			out->x = v.x;
			out->y = v.y;
			out->z = v.z;

			return out;
		}

		D3DXMATRIX *rotationMatrix(D3DXMATRIX *mOut) const
		{
			mOut->m[0][0] = float(1 - 2 * ((this->y) * (this->y)) - 2 * ((this->z) * (this->z)));
			mOut->m[1][0] = float(2*this->x*this->y - 2*this->w*this->z);
			mOut->m[2][0] = float(2*this->x*this->z + 2*this->w*this->y);
			mOut->m[3][0] = float(0);
			mOut->m[0][1] = float(2*this->x*this->y + 2*this->w*this->z);
			mOut->m[1][1] = float(1 - 2 * ((this->x) * (this->x)) - 2 * ((this->z) * (this->z)));
			mOut->m[2][1] = float(2*this->y*this->z - 2*this->w*this->x);
			mOut->m[3][1] = float(0);
			mOut->m[0][2] = float(2*this->x*this->z - 2*this->w*this->y);
			mOut->m[1][2] = float(2*this->y*this->z + 2*this->w*this->x);
			mOut->m[2][2] = float(1 - 2 * ((this->x) * (this->x)) - 2 * ((this->y) * (this->y)));
			mOut->m[3][2] = float(0);
			mOut->m[0][3] = float(0);
			mOut->m[1][3] = float(0);
			mOut->m[2][3] = float(0);
			mOut->m[3][3] = float(1);

			return mOut;
		}

		inline bool operator ==(const CQuaternion &other) const
		{
			if ( equal(w,other.w) && equal(x,other.x) && equal(y,other.y) && equal(z,other.z)) 
				return true;
			else 
				return false;
		}

		inline bool IsSensible(void) const
		{ return IsFinite(this->x) && IsFinite(this->y) && IsFinite(this->z) && IsFinite(this->w); }
	};

	inline CQuaternion operator*(const real s, const CQuaternion &a)
	{
		return CQuaternion(a.x*s, a.y*s, a.z*s, a.w*s);
	}

	const CQuaternion gIdentityQuaternion(0, 0, 0, 1);
}

#endif
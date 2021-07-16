#ifndef _PLANE_H
#define _PLANE_H

#pragma once

namespace Matematica {
	class Vector;

	class Plane{
	public:
		Vector n;
		real d;

		Plane(){
		}
		Plane( const real a, const real b, const real c, const real d ):n(a,b,c),d(d){
		}
		Plane( const Vector &n,real d ):n(n),d(d){
		}
		//point/normal form
		Plane( const Vector &p,const Vector &n ):n(n),d(-n.dot(p)){
		}
		//create plane from tri
		Plane( const Vector &v0,const Vector &v1,const Vector &v2 ){
			n=(v1-v0).cross(v2-v0).normalized();d=-n.dot(v0);
		}
		Plane operator-()const{
			return Plane( -n,-d );
		}
		/*float t_intersect( const Line &q )const{
			return -distance(q.o)/n.dot(q.d);
		}
		Vector intersect( const Line &q )const{
			return q*t_intersect(q);
		}
		Line intersect( const Plane &q )const{
			Vector lv=n.cross( q.n ).normalized();
			return Line( q.intersect( Line( nearest( n*-d ),n.cross(lv) ) ),lv );
		}*/
		Vector nearest( const Vector *q )const{
			return *q-n*distance(q);
		}

		void negate(){
			n=-n;d=-d;
		}
		
		inline real distance( const Vector *q )const{
			return n.dot(*q)+d;
		}

		real lengthsq() const{
			return this->n.lengthsq()+d*d;
		}
		real length()const{
			return sqrt(this->lengthsq());
		}

		Plane normalized()const{
			real l=length();return Plane( this->n.x/l,this->n.y/l,this->n.z/l, this->d);
		}
		void normalize(){
			real l=length();n.x/=l;n.y/=l;n.z/=l;d/=l;
		}
	};
}
#endif
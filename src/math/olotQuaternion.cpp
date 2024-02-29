/**
 * MIT License
 * 
 * Copyright (c) 2024 DragonDreams (info@dragondreams.ch)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <math.h>
#include "olotQuaternion.h"
#include "../exceptions/exceptions.h"



// Class olotQuaternion
/////////////////////////

// Constructor
////////////////

olotQuaternion::olotQuaternion() :
x( 0.0f ),
y( 0.0f ),
z( 0.0f ),
w( 1.0f ){
}

olotQuaternion::olotQuaternion( float nx, float ny, float nz, float nw ) :
x( nx ),
y( ny ),
z( nz ),
w( nw ){
}

olotQuaternion::olotQuaternion( const olotQuaternion &q ) :
x( q.x ),
y( q.y ),
z( q.z ),
w( q.w ){
}



// Quaternion creation
////////////////////////

olotQuaternion olotQuaternion::CreateFromEuler( float rx, float ry, float rz ){
	olotQuaternion q;
	q.SetFromEuler( rx, ry, rz );
	return q;
}

olotQuaternion olotQuaternion::CreateFromEulerX( float angle ){
	olotQuaternion q;
	q.SetFromEulerX( angle );
	return q;
}

olotQuaternion olotQuaternion::CreateFromEulerY( float angle ){
	olotQuaternion q;
	q.SetFromEulerY( angle );
	return q;
}

olotQuaternion olotQuaternion::CreateFromEulerZ( float angle ){
	olotQuaternion q;
	q.SetFromEulerZ( angle );
	return q;
}



// Management
///////////////

float olotQuaternion::Length() const{
	return sqrtf( x * x + y * y + z * z + w * w );
}

olotQuaternion olotQuaternion::Conjugate() const{
	return olotQuaternion( -x, -y, -z, w );
}

const olotQuaternion olotQuaternion::Absolute() const{
	return olotQuaternion( fabsf( x ), fabsf( y ), fabsf( z ), fabsf( w ) );
}

float olotQuaternion::Dot( const olotQuaternion &q ) const{
	return x * q.x + y * q.y + z * q.z + w * q.w;
}

olotQuaternion olotQuaternion::Slerp( const olotQuaternion &other, float factor ) const{
	float omega, cosom, sinom, scale0, scale1;
	float qx, qy, qz, qw;
	
	cosom = x * other.x + y * other.y + z * other.z + w * other.w;
	
	if( cosom < 0.0f ){
		cosom = -cosom;
		qx = -other.x;
		qy = -other.y;
		qz = -other.z;
		qw = -other.w;
		
	}else{
		qx = other.x;
		qy = other.y;
		qz = other.z;
		qw = other.w;
	}
	
	if( ( 1.0f - cosom ) > 0.001f ){
		omega = acosf( cosom );
		sinom = 1.0f / sinf( omega );
		scale0 = sinf( omega * ( 1.0f - factor ) ) * sinom;
		scale1 = sinf( omega * factor ) * sinom;
		
	}else{
		scale0 = 1.0f - factor;
		scale1 = factor;
	}
	
	return olotQuaternion( x * scale0 + qx * scale1, y * scale0 + qy * scale1, z * scale0 + qz * scale1, w * scale0 + qw * scale1 );
}

void olotQuaternion::SetZero(){
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
	w = 1.0f;
}

void olotQuaternion::Set( float nx, float ny, float nz, float nw ){
	x = nx;
	y = ny;
	z = nz;
	w = nw;
}

void olotQuaternion::Set( const olotQuaternion &q ){
	x = q.x;
	y = q.y;
	z = q.z;
	w = q.w;
}



void olotQuaternion::SetFromEuler( float rx, float ry, float rz ){
	const float a = sinf( rx );
	const float b = cosf( rx );
	const float c = sinf( ry );
	const float d = cosf( ry );
	const float e = sinf( rz );
	const float f = cosf( rz );
	const float g = d * f;
	const float h = c * f;
	const float i = c * e;
	const float j = d * e;
	
	const float a11 = g - a * i;
	const float a12 = a * h + j;
	const float a13 = -c * b;
	const float a21 = -b * e;
	const float a22 = b * f;
	const float a23 = a;
	const float a31 = h + a * j;
	const float a32 = -a * g + i;
	const float a33 = b * d;
	
	const float trace = a11 + a22 + a33 + 1.0f;
	
	if( trace > 0.0001f ) { // do not choose this too small or the numerical errors explode the result
		const float s = 0.5f / sqrtf( trace );
		x = ( a32 - a23 ) * s;
		y = ( a13 - a31 ) * s;
		z = ( a21 - a12 ) * s;
		w = 0.25f / s;
		
	}else if( a11 > a22 && a11 > a33 ){
		const float s = 0.5f / sqrtf( 1.0f + a11 - a22 - a33 );
		x = 0.25f / s;
		y = ( a12 + a21 ) * s;
		z = ( a13 + a31 ) * s;
		w = ( a23 - a32 ) * s;
		
	}else if( a22 > a33 ){
		const float s = 0.5f / sqrtf( 1.0f + a22 - a11 - a33 );
		x = ( a12 + a21 ) * s;
		y = 0.25f / s;
		z = ( a23 + a32 ) * s;
		w = ( a13 - a31 ) * s;
		
	}else{
		const float s = 0.5f / sqrtf( 1.0f + a33 - a11 - a22 );
		x = ( a13 + a31 ) * s;
		y = ( a23 + a32 ) * s;
		z = 0.25f / s;
		w = ( a12 - a21 ) * s;
	}
}

void olotQuaternion::SetFromEulerX( float angle ){
	const float s = sinf( angle );
	const float c = cosf( angle );
	const float trace = c + c + 2.0f;
	
	if( trace > 0.0001f ) { // do not choose this too small or the numerical errors explode the result
		const float t = sqrtf( trace );
		x = -s / t;
		y = 0.0f;
		z = 0.0f;
		w = 0.5f * t;
		
	}else{
		const float t = sqrtf( 2.0f - c - c ); // 4.0f - trace
		x = 0.5f * t;
		y = 0.0f;
		z = 0.0f;
		w = s / t;
	}
}

void olotQuaternion::SetFromEulerY( float angle ){
	const float s = sinf( angle );
	const float c = cosf( angle );
	const float trace = c + c + 2.0f;
	
	if( trace > 0.0001f ) { // do not choose this too small or the numerical errors explode the result
		const float t = sqrtf( trace );
		x = 0.0f;
		y = -s / t;
		z = 0.0f;
		w = 0.5f * t;
		
	}else{
		const float t = sqrtf( 2.0f - c - c ); // 4.0f - trace
		x = 0.0f;
		y = 0.5f * t;
		z = 0.0f;
		w = -s / t;
	}
}

void olotQuaternion::SetFromEulerZ( float angle ){
	const float s = sinf( angle );
	const float c = cosf( angle );
	const float trace = c + c + 2.0f;
	
	if( trace > 0.0001f ) { // do not choose this too small or the numerical errors explode the result
		const float t = sqrtf( trace );
		x = 0.0f;
		y = 0.0f;
		z = -s / t;
		w = 0.5f * t;
		
	}else{
		const float t = sqrtf( 2.0f - c - c ); // 4.0f - trace
		x = 0.0f;
		y = 0.0f;
		z = 0.5f * t;
		w = s / t;
	}
}



// Operators
//////////////

olotQuaternion olotQuaternion::operator-() const{
	return olotQuaternion( -x, -y, -z, -w );
}

olotQuaternion &olotQuaternion::operator=( const olotQuaternion &q ){
	x = q.x;
	y = q.y;
	z = q.z;
	w = q.w;
	return *this;
}

olotQuaternion &olotQuaternion::operator+=( const olotQuaternion &q ){
	x += q.x;
	y += q.y;
	z += q.z;
	w += q.w;
	return *this;
}

olotQuaternion &olotQuaternion::operator-=( const olotQuaternion &q ){
	x -= q.x;
	y -= q.y;
	z -= q.z;
	w -= q.w;
	return *this;
}

olotQuaternion &olotQuaternion::operator*=( float k ){
	x *= k;
	y *= k;
	z *= k;
	w *= k;
	return *this;
}

olotQuaternion &olotQuaternion::operator*=( const olotQuaternion &q ){
	const float nx = q.x * w + q.y * z - q.z * y + q.w * x;
	const float ny = -q.x * z + q.y * w + q.z * x + q.w * y;
	const float nz = q.x * y - q.y * x + q.z * w + q.w * z;
	const float nw = -q.x * x - q.y * y - q.z * z + q.w * w;
	
	x = nx;
	y = ny;
	z = nz;
	w = nw;
	
	return *this;
}

olotQuaternion olotQuaternion::operator+( const olotQuaternion &q ) const{
	return olotQuaternion( x + q.x, y + q.y, z + q.z, w + q.w );
}

olotQuaternion olotQuaternion::operator-( const olotQuaternion &q ) const{
	return olotQuaternion( x - q.x, y - q.y, z - q.z, w - q.w );
}

olotQuaternion olotQuaternion::operator*( float k ) const{
	return olotQuaternion( x * k, y * k, z * k, w * k );
}

olotQuaternion olotQuaternion::operator*( const olotQuaternion &q ) const{
	return olotQuaternion(
			q.x * w + q.y * z - q.z * y + q.w * x,
		-q.x * z + q.y * w + q.z * x + q.w * y,
			q.x * y - q.y * x + q.z * w + q.w * z,
		-q.x * x - q.y * y - q.z * z + q.w * w );
}

bool olotQuaternion::operator<( const olotQuaternion &q ) const{
	return x < q.x && y < q.y && z < q.z && w < q.w;
}

bool olotQuaternion::operator>( const olotQuaternion &q ) const{
	return x > q.x && y > q.y && z > q.z && w > q.w;
}

bool olotQuaternion::operator<=( const olotQuaternion &q ) const{
	return x <= q.x && y <= q.y && z <= q.z && w <= q.w;
}

bool olotQuaternion::operator>=( const olotQuaternion &q ) const{
	return x >= q.x && y >= q.y && z >= q.z && w >= q.w;
}

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

#ifndef _OLOTQUATERNION_H_
#define _OLOTQUATERNION_H_

/**
 * \brief 4 Component Quaternion.
 */
class olotQuaternion{
public:
	/** \brief X Component of quaternion. */
	float x;
	
	/** \brief Y Component of quaternion. */
	float y;
	
	/** \brief Z Component of quaternion. */
	float z;
	
	/** \brief W Component of quaternion. */
	float w;
	
	
	
public:
	/** \name Constructors and Destructors */
	/*@{*/
	/** \brief Create new quaternion initialized to no rotation. */
	olotQuaternion();
	
	/** \brief Create new quaternion with the given values. */
	olotQuaternion( float nx, float ny, float nz, float nw );
	
	/** \brief Create new quaternion with the values of another quaternion. */
	olotQuaternion( const olotQuaternion &q );
	/*@}*/
	
	
	
	/** \name Quaternion Creation */
	/*@{*/
	/** \brief Create new quaternion from euler angles. */
	static olotQuaternion CreateFromEuler( float rx, float ry, float rz );
	
	/** \brief Create new quaternion from an euler angle around the x axis. */
	static olotQuaternion CreateFromEulerX( float angle );
	
	/** \brief Create new quaternion from an euler angle around the y axis. */
	static olotQuaternion CreateFromEulerY( float angle );
	
	/** \brief Create new quaternion from an euler angle around the z axis. */
	static olotQuaternion CreateFromEulerZ( float angle );
	/*@}*/
	
	
	
	/** \name Management */
	/*@{*/
	/** \brief Length of the quaternion. */
	float Length() const;
	
	/**
	 * \brief Conjugation of this quaternion.
	 * 
	 * This yields the inverse rotation of the quaterion.
	 */
	olotQuaternion Conjugate() const;
	
	/** \brief Retrieves absolute quaternion. */
	const olotQuaternion Absolute() const;
	
	/** \brief Dot-product. */
	float Dot( const olotQuaternion &q ) const;
	
	/** \brief Calculates the slerp of this quaternion with another given an interpolation factor. */
	olotQuaternion Slerp( const olotQuaternion &other, float factor ) const;
	
	/** \brief Sets all components to 0 and w to 1. */
	void SetZero();
	
	/** \brief Set components to the given values. */
	void Set( float nx, float ny, float nz, float nw );
	
	/** \brief Set quaternion from another one. */
	void Set( const olotQuaternion &q );
	
	/** \brief Set from euler angles. */
	void SetFromEuler( float rx, float ry, float rz );
	
	/** \brief Set from an euler angle around the x axis. */
	void SetFromEulerX( float angle );
	
	/** \brief Set from an euler angle around the y axis. */
	void SetFromEulerY( float angle );
	
	/** \brief Set from an euler angle around the z axis. */
	void SetFromEulerZ( float angle );
	/*@}*/
	
	
	
	/** \name Operators */
	/*@{*/
	/** \brief Negation of this quaternion. */
	olotQuaternion operator-() const;
	
	/** \brief Set components of this quaternion to the values of another one. */
	olotQuaternion &operator=( const olotQuaternion &q );
	
	/** \brief Adds the components of another quaternion to this one. */
	olotQuaternion &operator+=( const olotQuaternion &q );
	
	/** \brief Subtracts the components of another quaternion from this quaternion. */
	olotQuaternion &operator-=( const olotQuaternion &q );
	
	/** \brief Multiplies the components of this quaternion with a value k. */
	olotQuaternion &operator*=( float k );
	
	/** \brief Multiply this quaternion with another quaternion. */
	olotQuaternion &operator*=( const olotQuaternion &q );
	
	/** \brief Quaternion with the sum of this quaternion with another one. */
	olotQuaternion operator+( const olotQuaternion &q ) const;
	
	/** \brief Quaternion with the difference of this quaternion to another one. */
	olotQuaternion operator-( const olotQuaternion &q ) const;
	
	/** \brief Quaternion scaled by k. */
	olotQuaternion operator*( float k ) const;
	
	/** \brief Quaternion multiplied by another quaternion. */
	olotQuaternion operator*( const olotQuaternion &q ) const;
	
	/** \brief All components are less then the components of another one. */
	bool operator<( const olotQuaternion &q ) const;
	
	/** \brief All components are greater then the components of another one. */
	bool operator>( const olotQuaternion &q ) const;
	
	/** \brief All components are less then or equal to the components of another one. */
	bool operator<=( const olotQuaternion &q ) const;
	
	/** \brief All components are greater then or equal to the components of another one. */
	bool operator>=( const olotQuaternion &q ) const;
	/*@}*/
};

#endif

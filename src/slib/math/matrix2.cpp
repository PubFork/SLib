/*
 *   Copyright (c) 2008-2018 SLIBIO <https://github.com/SLIBIO>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *   THE SOFTWARE.
 */

#include "slib/math/matrix2.h"

namespace slib
{

	template <class T>
	SLIB_ALIGN(8) T Matrix2T<T>::_zero[4] = {
		0, 0
		, 0, 0 };

	template <class T>
	SLIB_ALIGN(8) T Matrix2T<T>::_one[4] = {
		1, 1
		, 1, 1 };

	template <class T>
	SLIB_ALIGN(8) T Matrix2T<T>::_identity[4] = {
		1, 0
		, 0, 1 };

	template <class T>
	Vector2T<T> Matrix2T<T>::getRow0() const noexcept
	{
		return {m00, m01};
	}

	template <class T>
	void Matrix2T<T>::setRow0(const Vector2T<T>& v) noexcept
	{
		m00 = v.x;
		m01 = v.y;
	}

	template <class T>
	Vector2T<T> Matrix2T<T>::getRow1() const noexcept
	{
		return {m10, m11};
	}

	template <class T>
	void Matrix2T<T>::setRow1(const Vector2T<T>& v) noexcept
	{
		m10 = v.x;
		m11 = v.y;
	}

	template <class T>
	Vector2T<T> Matrix2T<T>::getRow(sl_uint32 index) const noexcept
	{
		const T* t = &m00 + (index << 1);
		return {t[0], t[1]};
	}

	template <class T>
	void Matrix2T<T>::setRow(sl_uint32 index, const Vector2T<T>& v) noexcept
	{
		T* t = &m00 + (index << 1);
		t[0] = v.x;
		t[1] = v.y;
	}

	template <class T>
	Vector2T<T> Matrix2T<T>::getColumn0() const noexcept
	{
		return {m00, m10};
	}

	template <class T>
	void Matrix2T<T>::setColumn0(const Vector2T<T>& v) noexcept
	{
		m00 = v.x;
		m10 = v.y;
	}

	template <class T>
	Vector2T<T> Matrix2T<T>::getColumn1() const noexcept
	{
		return {m01, m11};
	}

	template <class T>
	void Matrix2T<T>::setColumn1(const Vector2T<T>& v) noexcept
	{
		m01 = v.x;
		m11 = v.y;
	}

	template <class T>
	Vector2T<T> Matrix2T<T>::getColumn(sl_uint32 index) const noexcept
	{
		const T* t = &m00 + index;
		return {t[0], t[2]};
	}

	template <class T>
	void Matrix2T<T>::setColumn(sl_uint32 index, const Vector2T<T>& v) noexcept
	{
		T* t = &m00 + index;
		t[0] = v.x;
		t[2] = v.y;
	}

	template <class T>
	T Matrix2T<T>::getElement(sl_uint32 row, sl_uint32 column) const noexcept
	{
		return (&m00)[(row << 1) + column];
	}

	template <class T>
	void Matrix2T<T>::setElement(sl_uint32 row, sl_uint32 column, const T& v) noexcept
	{
		(&m00)[(row << 1) + column] = v;
	}

	template <class T>
	void Matrix2T<T>::add(const Matrix2T<T>& other) noexcept
	{
		m00 += other.m00; m01 += other.m01;
		m10 += other.m10; m11 += other.m11;
	}

	template <class T>
	void Matrix2T<T>::subtract(const Matrix2T<T>& other) noexcept
	{
		m00 -= other.m00; m01 -= other.m01;
		m10 -= other.m10; m11 -= other.m11;
	}

	template <class T>
	void Matrix2T<T>::multiply(T value) noexcept
	{
		m00 *= value; m01 *= value;
		m10 *= value; m11 *= value;
	}

	template <class T>
	void Matrix2T<T>::divide(T value) noexcept
	{
		m00 /= value; m01 /= value;
		m10 /= value; m11 /= value;
	}

	template <class T>
	Vector2T<T> Matrix2T<T>::multiplyLeft(const Vector2T<T>& v) const noexcept
	{
		T _x = v.x * m00 + v.y * m10;
		T _y = v.x * m01 + v.y * m11;
		return {_x, _y};
	}

	template <class T>
	Vector2T<T> Matrix2T<T>::multiplyRight(const Vector2T<T>& v) const noexcept
	{
		T _x = m00 * v.x + m01 * v.y;
		T _y = m10 * v.x + m11 * v.y;
		return {_x, _y};
	}

	template <class T>
	void Matrix2T<T>::multiply(const Matrix2T<T>& m) noexcept
	{
		T v0, v1;
		v0 = m00 * m.m00 + m01 * m.m10;
		v1 = m00 * m.m01 + m01 * m.m11;
		m00 = v0; m01 = v1;
		v0 = m10 * m.m00 + m11 * m.m10;
		v1 = m10 * m.m01 + m11 * m.m11;
		m10 = v0; m11 = v1;
	}

	template <class T>
	T Matrix2T<T>::getDeterminant() const noexcept
	{
		return SLIB_MATH_MATRIX_DETERMINANT2(m00, m01, m10, m11);
	}

	template <class T>
	void Matrix2T<T>::makeInverse() noexcept
	{
		T A00 = m11;
		T A01 = -m10;
		T A10 = -m01;
		T A11 = m00;
		T D = 1 / SLIB_MATH_MATRIX_DETERMINANT2(m00, m01, m10, m11);
		m00 = D * A00; m01 = D * A10;
		m10 = D * A01; m11 = D * A11;
	}

	template <class T>
	Matrix2T<T> Matrix2T<T>::inverse() const noexcept
	{
		Matrix2T<T> ret(*this);
		ret.makeInverse();
		return ret;
	}

	template <class T>
	void Matrix2T<T>::makeTranspose() noexcept
	{
		T	f;
		f = m01; m01 = m10; m10 = f;
	}

	template <class T>
	Matrix2T<T> Matrix2T<T>::transpose() const noexcept
	{
		Matrix2T<T> ret(*this);
		ret.makeTranspose();
		return ret;
	}

	template <class T>
	void Matrix2T<T>::makeInverseTranspose() noexcept
	{
		makeInverse();
		makeTranspose();
	}

	template <class T>
	Matrix2T<T> Matrix2T<T>::inverseTranspose() const noexcept
	{
		Matrix2T<T> ret(*this);
		ret.makeInverse();
		ret.makeTranspose();
		return ret;
	}

	template <class T>
	Matrix2T<T> Matrix2T<T>::lerp(const Matrix2T<T>& target, float factor) const noexcept
	{
		return {
			SLIB_LERP(m00, target.m00, factor), SLIB_LERP(m01, target.m01, factor),
			SLIB_LERP(m10, target.m10, factor), SLIB_LERP(m11, target.m11, factor)};
	}

	template <class T>
	Matrix2T<T> Matrix2T<T>::operator+(const Matrix2T<T>& other) const noexcept
	{
		Matrix2T<T> ret(*this);
		ret.add(other);
		return ret;
	}

	template <class T>
	Matrix2T<T>& Matrix2T<T>::operator+=(const Matrix2T<T>& other) noexcept
	{
		add(other);
		return *this;
	}

	template <class T>
	Matrix2T<T> Matrix2T<T>::operator-(const Matrix2T<T>& other) const noexcept
	{
		Matrix2T<T> ret(*this);
		ret.subtract(other);
		return ret;
	}

	template <class T>
	Matrix2T<T>& Matrix2T<T>::operator-=(const Matrix2T<T>& other) noexcept
	{
		subtract(other);
		return *this;
	}

	template <class T>
	Matrix2T<T> Matrix2T<T>::operator-() const noexcept
	{
		Matrix2T<T> ret(Matrix2T<T>::zero());
		ret.subtract(*this);
		return ret;
	}

	template <class T>
	Matrix2T<T> Matrix2T<T>::operator*(T value) const noexcept
	{
		Matrix2T<T> ret(*this);
		ret.multiply(value);
		return ret;
	}

	template <class T>
	Matrix2T<T>& Matrix2T<T>::operator*=(T value) noexcept
	{
		multiply(value);
		return *this;
	}

	template <class T>
	Matrix2T<T> Matrix2T<T>::operator/(T value) const noexcept
	{
		Matrix2T<T> ret(*this);
		ret.divide(value);
		return ret;
	}

	template <class T>
	Matrix2T<T>& Matrix2T<T>::operator/=(T value) noexcept
	{
		divide(value);
		return *this;
	}

	template <class T>
	Vector2T<T> Matrix2T<T>::operator*(const Vector2T<T>& v) const noexcept
	{
		return multiplyRight(v);
	}

	template <class T>
	Matrix2T<T> Matrix2T<T>::operator*(const Matrix2T<T>& other) const noexcept
	{
		Matrix2T<T> ret(*this);
		ret.multiply(other);
		return ret;
	}

	template <class T>
	Matrix2T<T>& Matrix2T<T>::operator*=(const Matrix2T<T>& other) noexcept
	{
		multiply(other);
		return *this;
	}

	template <class T>
	sl_bool Matrix2T<T>::operator==(const Matrix2T<T>& other) const noexcept
	{
		return m00 == other.m00 && m01 == other.m01 &&
			m10 == other.m10 && m11 == other.m11;
	}

	template <class T>
	sl_bool Matrix2T<T>::operator!=(const Matrix2T<T>& other) const noexcept
	{
		return m00 != other.m00 || m01 != other.m01 ||
			m10 != other.m10 || m11 != other.m11;
	}

	SLIB_DEFINE_GEOMETRY_TYPE(Matrix2)

}

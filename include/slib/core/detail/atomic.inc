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

namespace slib
{
	
	template <class T>
	Atomic<T>::Atomic()
	{
	}
	
	template <class T>
	Atomic<T>::Atomic(const T& value)
	: m_value(value)
	{
	}
	
	template <class T>
	Atomic<T>::Atomic(T&& value)
	: m_value(Move(value))
	{
	}
	
	template <class T>
	Atomic<T>& Atomic<T>::operator=(const T& other)
	{
		SLIB_ALIGN(8) char t[sizeof(T)];
		{
			SpinLocker lock(&m_lock);
			((T*)t)->T::T(Move(m_value));
			m_value = other;
		}
		((T*)t)->~T();
		return *this;
	}
	
	template <class T>
	Atomic<T>& Atomic<T>::operator=(T&& other)
	{
		SLIB_ALIGN(8) char t[sizeof(T)];
		{
			SpinLocker lock(&m_lock);
			((T*)t)->T::T(Move(m_value));
			m_value = Move(other);
		}
		((T*)t)->~T();
		return *this;
	}
	
	template <class T>
	Atomic<T>::operator T() const
	{
		SLIB_ALIGN(8) char t[sizeof(T)];
		{
			SpinLocker lock(&m_lock);
			((T*)t)->T::T(m_value);
		}
		return *((T*)t);
	}

}

/*
 * Winamp httpQ Plugin
 * Copyright (C) 1999-2003 Kosta Arvanitis
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Kosta Arvanitis (karvanitis@hotmail.com)
 */
#ifndef __ARRAY_H
#define __ARRAY_H

//--------------------------------------------------
// Class Definition
//--------------------------------------------------

template <class T>
class Array
{
public:
    enum
    {
        DefaultMaxSize = 128
    };

    Array();
    Array(int size);
    ~Array();

	T operator[](const unsigned int n) const;
    Array<T>& operator=(const Array<T>& array);

    inline unsigned int Count() const { return _index; }
    inline unsigned int MaxSize() const { return _size; };
    inline bool IsEmpty() const { return (_index == 0) };
    inline void Clear() { _index = 0; };

    bool Contains(T type) const;
    unsigned int Add(T type);
    unsigned int Remove(T type);

private:
    unsigned int _index;
    unsigned int _size;
    T *_array;
};


//--------------------------------------------------
// Class Implementation
//--------------------------------------------------

template <class T>
Array<T>::Array()
{
    _index = 0;
    _size = DefaultMaxSize;
    _array = new T[_size];
}

template <class T>
Array<T>::Array(int size)
{
    _index = 0;
    _size = size;
    _array = new T[_size];
}

template <class T>
Array<T>::~Array()
{
    delete[] _array;
}

template <class T>
bool Array<T>::Contains(T type) const
{
	for(unsigned int i = 0; i < _index; i++)
    {
		if(_array[i] == type)
			return true;
    }
	return false;
}

template <class T>
T Array<T>::operator [](const unsigned int n) const
{
    _ASSERT((n >= 0 && n < _size));
    return _array[n];
}

template <class T>
Array<T>& Array<T>::operator=(const Array<T>& array)
{
    if (this == &array) 
        return *this;

    _ASSERT(this->_size == array._size);

    this->_index = array._index;
    for(unsigned int i = 0; i < this->_size; i++) 
        this->_array[i] = array[i];

    return *this;
}

template <class T>
unsigned int Array<T>::Add(T type)
{
    if (_index >= _size)
        return -1;
    
    _array[_index] = type;
    return _index++;
}

template <class T>
unsigned int Array<T>::Remove(T type)
{
    unsigned int n = -1;

    for(unsigned int i = 0; i < this->_size; i++)
    {
        if (_array[i] == type)
        {
            n = i;
            break;
        }
    }

    if(n != -1)
    {
        for(unsigned int i = n; i < this->_size - 1; i++)
            _array[i] = _array[i + 1];
        _index--;
    }

    return n;
}


#endif // __ARRAY_H
/*
 * iterators.h
 *
 * Copyright 2019 Vincent Cadet <vincent.cadet@hepl.be>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef __ITERATORS_H__
#define __ITERATORS_H__

#include <ostream>
#include <iterator>

/**
 * \brief Output stream iterator (alternate version)
 *
 * This iterator only outputs the separator when the first chunk of data has
 * been sent. It also prevents the separator from appearing at the end of the
 * output line. One possible application is to output CSV-like data.
 *
 * See https://codereview.stackexchange.com/questions/13176/infix-iterator-code
 */
template <class T, class charT = char, class traits = std::char_traits<charT> >
class ostream_iterator
{
public:
	typedef charT char_type;
	typedef traits traits_type;
	typedef std::basic_ostream<charT, traits> ostream_type;

	// C++17 style iterator declarations:
	using iterator_category = std::output_iterator_tag;
	using value_type = void;
	using difference_type = void;
	using pointer = void;
	using reference = void;

	ostream_iterator(ostream_type& s) : os(&s), delimiter(0), count(0) {}
	ostream_iterator(ostream_type& s, charT const* d) :
			os(&s), delimiter(d), count(0) {}

	// Output iterator minimum required members
	auto& operator = (T const& item);
	auto& operator * ();
	auto& operator ++ ();
	auto operator ++ (int);

private:
	ostream_type* os;
	charT const* delimiter;
	std::size_t count;
};

template <class T, class charT, class traits>
inline auto& ostream_iterator<T, charT, traits>::operator = (T const& item)
{
	// Only output the separator if this iterator has been incremented,
	// i.e. typically after the first item
	if (count && delimiter) *os << delimiter;
	*os << item;
	return *this;
}

template <class T, class charT, class traits>
inline auto& ostream_iterator<T, charT, traits>::operator * ()
{
	// NOOP
	return *this;
}

template <class T, class charT, class traits>
inline auto& ostream_iterator<T, charT, traits>::operator ++ ()
{
	count++;
	return *this;
}

template <class T, class charT, class traits>
inline auto ostream_iterator<T, charT, traits>::operator ++ (int)
{
	auto tmp = *this;
	operator ++ ();
	return tmp;
}


#endif /* if __ITERATORS_H__ */

/*!
 * @file       NTSerialize.hpp
 * @brief      Defines the serialization facilities
 * @author     Alexander Alexeev &lt;ntllct@protonmail.com&gt;
 * @date       Oct 15, 2018
 * @copyright  Copyright &copy; 2018 Alexander Alexeev.
 *             This project is released under the GNU Lesser
 *             General Public License Version 3.
 */

/*********************************************************************
 * MIT License
 * 
 * Copyright (c) 2017 Alexander Alexeev [ntllct@protonmail.com] 
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *********************************************************************/

#pragma once

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <type_traits>
#include <string>
#include <set>
#include <map>
#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <queue>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <mutex>

namespace ntllct {

enum class ntsdirective : unsigned char {
	clear,		// Clear internal buffer
	posstart,	// IO pointer to start
	posend,		// IO pointer to end
	debug,		// Enable debug mode
	nodebug		// Disable debug mode
};

class NTSerialize {
public:
	// Clear stringstream buffer
	void clear() {
		_buffer.clear();
		_buffer.str( std::string() );
	}
	// Eval command
	NTSerialize& operator<<( const ntsdirective command ) {
		if( command == ntsdirective::clear ) {
			clear();
		} else if( command == ntsdirective::posstart ) {
			_buffer.seekp( 0, std::ios::beg );
		} else if( command == ntsdirective::posend ) {
			_buffer.seekp( 0, std::ios::end );
		} else if( command == ntsdirective::debug ) {
			_is_debug = true;
		} else if( command == ntsdirective::nodebug ) {
			_is_debug = false;
		}
		return( *this );
	}
	// Serialize data
	template<typename T>
	typename std::enable_if<std::is_fundamental<T>::value,
							NTSerialize&>::type
	operator<<( const T data ) {
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout 	<< "DEBUG write: stringstream::good() = "
						<< std::boolalpha << _buffer.good()
						<< " data: " << data << std::endl;
		}
		_buffer.write(	reinterpret_cast<const char*>( &data ),
						sizeof( T ) );
		return( *this );
	}
	template<typename T>
	typename std::enable_if<std::is_fundamental<T>::value,
							NTSerialize&>::type
	operator>>( T& data ) {
		_buffer.read( reinterpret_cast<char*>( &data ), sizeof( T ) );
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout 	<< "DEBUG read: stringstream::good() = "
						<< std::boolalpha << _buffer.good()
						<< " data: " << data << std::endl;
		}
		return( *this );
	}
	template<typename T>
	typename std::enable_if<std::is_class<T>::value, NTSerialize&>::type
	operator<<( const T data ) {
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout 	<< "DEBUG write: stringstream::good() = "
						<< std::boolalpha << _buffer.good()
						<< " data: [class]" << std::endl;
		}
		_buffer.write(	reinterpret_cast<const char*>( &data ),
						sizeof( T ) );
		return( *this );
	}
	template<typename T>
	typename std::enable_if<std::is_class<T>::value, NTSerialize&>::type
	operator>>( T& data ) {
		_buffer.read( reinterpret_cast<char*>( &data ), sizeof( T ) );
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout 	<< "DEBUG read: stringstream::good() = "
						<< std::boolalpha << _buffer.good()
						<< " data: [class]" << std::endl;
		}
		return( *this );
	}
	// Serialize STL containers
	NTSerialize& operator<<( const std::string& data ) {
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout	<< "DEBUG write string: stringstream::good() = "
						<< std::boolalpha << _buffer.good()
						<< " data: " << data << std::endl;
		}
		size_t size_ = data.size();
		_buffer.write(	reinterpret_cast<const char*>( &size_ ),
						sizeof( size_t ) );
		_buffer.write(	reinterpret_cast<const char*>( data.c_str() ),
						size_ );
		return( *this );
	}
	NTSerialize& operator>>( std::string& data ) {
		size_t size_ = 0;
		_buffer.read(	reinterpret_cast<char*>( &size_ ),
						sizeof( size_t ) );
		data.resize( size_ );
		_buffer.read( const_cast<char*>( data.c_str() ), size_ );
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout 	<< "DEBUG read string: stringstream::good() = "
						<< std::boolalpha << _buffer.good()
						<< " data: " << data << std::endl;
		}
		return( *this );
	}
	template<typename T>
	NTSerialize& operator<<( const std::vector<T>& data ) {
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout 	<< "DEBUG write vector: stringstream::good() = "
						<< std::boolalpha << _buffer.good()
						<< " data size: " << data.size() << std::endl;
		}
		size_t size_ = data.size();
		_buffer.write(	reinterpret_cast<const char*>( &size_ ),
						sizeof( size_t ) );
		
		for( auto it = data.cbegin(); it != data.cend(); ++it )
			*this << *it;
		
		return( *this );
	}
	template<typename T>
	NTSerialize& operator>>( std::vector<T>& data ) {
		size_t size_ = 0;
		_buffer.read(	reinterpret_cast<char*>( &size_ ),
						sizeof( size_t ) );
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout 	<< "DEBUG read vector: stringstream::good() = "
						<< std::boolalpha << _buffer.good()
						<< " data size: " << size_ << std::endl;
		}
		data.resize( size_ );
		for( size_t i = 0; i < size_; ++i )
			*this >> data[i];
		
		return( *this );
	}
	NTSerialize& operator<<( const std::vector<bool>& data ) {
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout
				<< "DEBUG write vector<bool>: stringstream::good() = "
				<< std::boolalpha << _buffer.good()
				<< " data size: " << data.size() << std::endl;
		}
		size_t size_ = data.size();
		_buffer.write(	reinterpret_cast<const char*>( &size_ ),
						sizeof( size_t ) );
		
		for( auto it = data.cbegin(); it != data.cend(); ++it )
			*this << *it;
		
		return( *this );
	}
	NTSerialize& operator>>( std::vector<bool>& data ) {
		size_t size_ = 0;
		_buffer.read(	reinterpret_cast<char*>( &size_ ),
						sizeof( size_t ) );
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout
				<< "DEBUG read vector<bool>: stringstream::good() = "
				<< std::boolalpha << _buffer.good()
				<< " data size: " << size_ << std::endl;
		}
		data.resize( size_ );
		for( size_t i = 0; i < size_; ++i ) {
			bool val_;
			*this >> val_;
			data[i] = val_;
		}
		
		return( *this );
	}
	template<typename T>
	NTSerialize& operator<<( const std::deque<T>& data ) {
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout 	<< "DEBUG write deque: stringstream::good() = "
						<< std::boolalpha << _buffer.good()
						<< " data size: " << data.size() << std::endl;
		}
		size_t size_ = data.size();
		_buffer.write(	reinterpret_cast<const char*>( &size_ ),
						sizeof( size_t ) );
		
		for( auto it = data.cbegin(); it != data.cend(); ++it )
			*this << *it;
		
		return( *this );
	}
	template<typename T>
	NTSerialize& operator>>( std::deque<T>& data ) {
		size_t size_ = 0;
		_buffer.read(	reinterpret_cast<char*>( &size_ ),
						sizeof( size_t ) );
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout 	<< "DEBUG read deque: stringstream::good() = "
						<< std::boolalpha << _buffer.good()
						<< " data size: " << size_ << std::endl;
		}
		data.resize( size_ );
		for( size_t i = 0; i < size_; ++i )
			*this >> data[i];
		
		return( *this );
	}
	template<typename T>
	NTSerialize& operator<<( const std::forward_list<T>& data ) {
		size_t size_ = data.size();
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout
				<< "DEBUG write forward_list: stringstream::good() = "
				<< std::boolalpha << _buffer.good()
				<< " data size: " << size_ << std::endl;
		}
		_buffer.write(	reinterpret_cast<const char*>( &size_ ),
						sizeof( size_t ) );
		
		for( auto it = data.cbegin(); it != data.cend(); ++it )
			*this << *it;
		
		return( *this );
	}
	template<typename T>
	NTSerialize& operator>>( std::forward_list<T>& data ) {
		size_t size_ = 0;
		_buffer.read(	reinterpret_cast<char*>( &size_ ),
						sizeof( size_t ) );
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout
				<< "DEBUG read forward_list: stringstream::good() = "
				<< std::boolalpha << _buffer.good()
				<< " data size: " << size_ << std::endl;
		}
		data.resize( size_ );
		for( auto it = data.begin(); it != data.end(); ++it )
			*this >> *it;
		
		return( *this );
	}
	template<typename T>
	NTSerialize& operator<<( const std::list<T>& data ) {
		size_t size_ = data.size();
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout 	<< "DEBUG write list: stringstream::good() = "
						<< std::boolalpha << _buffer.good()
						<< " data size: " << size_ << std::endl;
		}
		_buffer.write(	reinterpret_cast<const char*>( &size_ ),
						sizeof( size_t ) );
		
		for( auto it = data.cbegin(); it != data.cend(); ++it )
			*this << *it;
		
		return( *this );
	}
	template<typename T>
	NTSerialize& operator>>( std::list<T>& data ) {
		size_t size_ = 0;
		_buffer.read(	reinterpret_cast<char*>( &size_ ),
						sizeof( size_t ) );
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout 	<< "DEBUG read list: stringstream::good() = "
						<< std::boolalpha << _buffer.good()
						<< " data size: " << size_ << std::endl;
		}
		data.resize( size_ );
		for( auto it = data.begin(); it != data.end(); ++it )
			*this >> *it;
		
		return( *this );
	}
	template<typename T>
	NTSerialize& operator<<( const std::queue<T>& data ) {
		size_t size_ = data.size();
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout 	<< "DEBUG write queue: stringstream::good() = "
						<< std::boolalpha << _buffer.good()
						<< " data size: " << size_ << std::endl;
		}
		_buffer.write(	reinterpret_cast<const char*>( &size_ ),
						sizeof( size_t ) );
		
		for( size_t i = 0; i < size_; ++i ) {
			*this << data.front();
			data.pop();
		}
		
		return( *this );
	}
	template<typename T>
	NTSerialize& operator>>( std::queue<T>& data ) {
		size_t size_ = 0;
		_buffer.read(	reinterpret_cast<char*>( &size_ ),
						sizeof( size_t ) );
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout 	<< "DEBUG read queue: stringstream::good() = "
						<< std::boolalpha << _buffer.good()
						<< " data size: " << size_ << std::endl;
		}
		for( size_t i = 0; i < size_; ++i ) {
			T val_;
			*this >> val_;
			data.push( val_ );
		}
		
		return( *this );
	}
	template<typename T>
	NTSerialize& operator<<( const std::priority_queue<T>& data ) {
		size_t size_ = data.size();
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout
				<< "DEBUG write priority_queue: stringstream::good() = "
				<< std::boolalpha << _buffer.good()
				<< " data size: " << size_ << std::endl;
		}
		_buffer.write(	reinterpret_cast<const char*>( &size_ ),
						sizeof( size_t ) );
		
		for( size_t i = 0; i < size_; ++i ) {
			*this << data.top();
			data.pop();
		}
		
		return( *this );
	}
	template<typename T>
	NTSerialize& operator>>( std::priority_queue<T>& data ) {
		size_t size_ = 0;
		_buffer.read(	reinterpret_cast<char*>( &size_ ),
						sizeof( size_t ) );
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout
				<< "DEBUG read priority_queue: stringstream::good() = "
				<< std::boolalpha << _buffer.good()
				<< " data size: " << size_ << std::endl;
		}
		for( size_t i = 0; i < size_; ++i ) {
			T val_;
			*this >> val_;
			data.push( val_ );
		}
		
		return( *this );
	}
	template<typename T>
	NTSerialize& operator<<( std::stack<T>& data ) {
		size_t size_ = data.size();
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout 	<< "DEBUG write stack: stringstream::good() = "
						<< std::boolalpha << _buffer.good()
						<< " data size: " << size_ << std::endl;
		}
		_buffer.write(	reinterpret_cast<const char*>( &size_ ),
						sizeof( size_t ) );
		std::vector<T> temp_;
		temp_.resize( size_ );
		
		for( size_t i = 0; i < size_; ++i ) {
			*this << data.top();
			temp_[i] = data.top();
			data.pop();
		}
		for( auto it = temp_.crbegin(); it != temp_.crend(); ++it ) {
			data.push( *it );
		}
		
		return( *this );
	}
	template<typename T>
	NTSerialize& operator>>( std::stack<T>& data ) {
		size_t size_ = 0;
		_buffer.read(	reinterpret_cast<char*>( &size_ ),
						sizeof( size_t ) );
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout 	<< "DEBUG read stack: stringstream::good() = "
						<< std::boolalpha << _buffer.good()
						<< " data size: " << size_ << std::endl;
		}
		std::vector<T> temp_;
		temp_.resize( size_ );
		
		for( size_t i = 0; i < size_; ++i ) {
			T val_;
			*this >> val_;
			temp_[i] = val_;
		}
		
		for( auto it = temp_.crbegin(); it != temp_.crend(); ++it )
			data.push( *it );
		
		return( *this );
	}
	template<typename T, size_t N>
	NTSerialize& operator<<( const std::array<T, N>& data ) {
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout 	<< "DEBUG write array: stringstream::good() = "
						<< std::boolalpha << _buffer.good()
						<< " data size: " << N << std::endl;
		}
		for( size_t i = 0; i < N; ++i )
			*this << data[i];
		
		return( *this );
	}
	template<typename T, size_t N>
	NTSerialize& operator>>( std::array<T, N>& data ) {
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout 	<< "DEBUG read array: stringstream::good() = "
						<< std::boolalpha << _buffer.good()
						<< " data size: " << N << std::endl;
		}
		for( size_t i = 0; i < N; ++i )
			*this >> data[i];
		
		return( *this );
	}
	template<typename T, size_t N>
	NTSerialize& operator<<( const T (&data)[N] ) {
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout 	<< "DEBUG write []: stringstream::good() = "
						<< std::boolalpha << _buffer.good()
						<< " data size: " << N << std::endl;
		}
		for( size_t i = 0; i < N; ++i )
			*this << data[i];
		return( *this );
	}
	template<typename T, size_t N>
	NTSerialize& operator>>( T (&data)[N] ) {
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout 	<< "DEBUG read []: stringstream::good() = "
						<< std::boolalpha << _buffer.good()
						<< " data size: " << N << std::endl;
		}
		for( size_t i = 0; i < N; ++i )
			*this >> data[i];
		return( *this );
	}
	template<typename T>
	NTSerialize& operator<<( const std::set<T>& data ) {
		size_t size_ = data.size();
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout 	<< "DEBUG write set: stringstream::good() = "
						<< std::boolalpha << _buffer.good()
						<< " data size: " << size_ << std::endl;
		}
		_buffer.write(	reinterpret_cast<const char*>( &size_ ),
						sizeof( size_t ) );
		
		for( auto it = data.cbegin(); it != data.cend(); ++it )
			*this << *it;
		
		return( *this );
	}
	template<typename T>
	NTSerialize& operator>>( std::set<T>& data ) {
		size_t size_ = 0;
		_buffer.read(	reinterpret_cast<char*>( &size_ ),
						sizeof( size_t ) );
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout 	<< "DEBUG read set: stringstream::good() = "
						<< std::boolalpha << _buffer.good()
						<< " data size: " << size_ << std::endl;
		}
		for( size_t i = 0; i < size_; ++i ) {
			T val_;
			*this >> val_;
			data.insert( val_ );
		}
		return( *this );
	}
	template<typename T>
	NTSerialize& operator<<( const std::multiset<T>& data ) {
		size_t size_ = data.size();
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout
				<< "DEBUG write multiset: stringstream::good() = "
				<< std::boolalpha << _buffer.good()
				<< " data size: " << size_ << std::endl;
		}
		_buffer.write(	reinterpret_cast<const char*>( &size_ ),
						sizeof( size_t ) );
		
		for( auto it = data.cbegin(); it != data.cend(); ++it )
			*this << *it;
		
		return( *this );
	}
	template<typename T>
	NTSerialize& operator>>( std::multiset<T>& data ) {
		size_t size_ = 0;
		_buffer.read(	reinterpret_cast<char*>( &size_ ),
						sizeof( size_t ) );
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout
					<< "DEBUG read multiset: stringstream::good() = "
					<< std::boolalpha << _buffer.good()
					<< " data size: " << size_ << std::endl;
		}
		for( size_t i = 0; i < size_; ++i ) {
			T val_;
			*this >> val_;
			data.insert( val_ );
		}
		
		return( *this );
	}
	template<typename T>
	NTSerialize& operator<<( const std::unordered_set<T>& data ) {
		size_t size_ = data.size();
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout
				<< "DEBUG write unordered_set: stringstream::good() = "
				<< std::boolalpha << _buffer.good()
				<< " data size: " << size_ << std::endl;
		}
		_buffer.write(	reinterpret_cast<const char*>( &size_ ),
						sizeof( size_t ) );
		
		for( auto it = data.cbegin(); it != data.cend(); ++it )
			*this << *it;
		
		return( *this );
	}
	template<typename T>
	NTSerialize& operator>>( std::unordered_set<T>& data ) {
		size_t size_ = 0;
		_buffer.read(	reinterpret_cast<char*>( &size_ ),
						sizeof( size_t ) );
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout
				<< "DEBUG read unordered_set: stringstream::good() = "
				<< std::boolalpha << _buffer.good()
				<< " data size: " << size_ << std::endl;
		}
		for( size_t i = 0; i < size_; ++i ) {
			T val_;
			*this >> val_;
			data.insert( val_ );
		}
		return( *this );
	}
	template<typename T>
	NTSerialize& operator<<( const std::unordered_multiset<T>& data ) {
		size_t size_ = data.size();
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout
			<< "DEBUG write unordered_multiset: stringstream::good() = "
			<< std::boolalpha << _buffer.good()
			<< " data size: " << size_ << std::endl;
		}
		_buffer.write(	reinterpret_cast<const char*>( &size_ ),
						sizeof( size_t ) );
		
		for( auto it = data.cbegin(); it != data.cend(); ++it )
			*this << *it;
		
		return( *this );
	}
	template<typename T>
	NTSerialize& operator>>( std::unordered_multiset<T>& data ) {
		size_t size_ = 0;
		_buffer.read(	reinterpret_cast<char*>( &size_ ),
						sizeof( size_t ) );
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout
			<< "DEBUG read unordered_multiset: stringstream::good() = "
			<< std::boolalpha << _buffer.good()
			<< " data size: " << size_ << std::endl;
		}
		for( size_t i = 0; i < size_; ++i ) {
			T val_;
			*this >> val_;
			data.insert( val_ );
		}
		
		return( *this );
	}
	template<typename T1, typename T2>
	NTSerialize& operator<<( const std::pair<T1, T2>& data ) {
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout 	<< "DEBUG write pair: stringstream::good() = "
						<< std::boolalpha << _buffer.good()
						<< std::endl;
		}
		*this << data.first;
		*this << data.second;
		
		return( *this );
	}
	template<typename T1, typename T2>
	NTSerialize& operator>>( std::pair<T1, T2>& data ) {
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout 	<< "DEBUG read pair: stringstream::good() = "
						<< std::boolalpha << _buffer.good()
						<< std::endl;
		}
		*this >> data.first;
		*this >> data.second;
		
		return( *this );
	}
	template<typename T1, typename T2>
	NTSerialize& operator<<( const std::map<T1, T2>& data ) {
		size_t size_ = data.size();
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout 	<< "DEBUG write map: stringstream::good() = "
						<< std::boolalpha << _buffer.good()
						<< " data size: " << size_ << std::endl;
		}
		_buffer.write(	reinterpret_cast<const char*>( &size_ ),
						sizeof( size_t ) );
		
		for( auto it = data.cbegin(); it != data.cend(); ++it )
			*this << *it;
		
		return( *this );
	}
	template<typename T1, typename T2>
	NTSerialize& operator>>( std::map<T1, T2>& data ) {
		size_t size_ = 0;
		_buffer.read(	reinterpret_cast<char*>( &size_ ),
						sizeof( size_t ) );
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout 	<< "DEBUG read map: stringstream::good() = "
						<< std::boolalpha << _buffer.good()
						<< " data size: " << size_ << std::endl;
		}
		for( size_t i = 0; i < size_; ++i ) {
			std::pair<T1, T2> val_;
			*this >> val_;
			data.insert( val_ );
		}
		
		return( *this );
	}
	template<typename T1, typename T2>
	NTSerialize& operator<<( const std::multimap<T1, T2>& data ) {
		size_t size_ = data.size();
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout
				<< "DEBUG write multimap: stringstream::good() = "
				<< std::boolalpha << _buffer.good()
				<< " data size: " << size_ << std::endl;
		}
		_buffer.write(	reinterpret_cast<const char*>( &size_ ),
						sizeof( size_t ) );
		
		for( auto it = data.cbegin(); it != data.cend(); ++it )
			*this << *it;
		
		return( *this );
	}
	template<typename T1, typename T2>
	NTSerialize& operator>>( std::multimap<T1, T2>& data ) {
		size_t size_ = 0;
		_buffer.read(	reinterpret_cast<char*>( &size_ ),
						sizeof( size_t ) );
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout
				<< "DEBUG read multimap: stringstream::good() = "
				<< std::boolalpha << _buffer.good()
				<< " data size: " << size_ << std::endl;
		}
		for( size_t i = 0; i < size_; ++i ) {
			std::pair<T1, T2> val_;
			*this >> val_;
			data.insert( val_ );
		}
		
		return( *this );
	}
	template<typename T1, typename T2>
	NTSerialize& operator<<( const std::unordered_map<T1, T2>& data ) {
		size_t size_ = data.size();
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout
				<< "DEBUG write unordered_map: stringstream::good() = "
				<< std::boolalpha << _buffer.good()
				<< " data size: " << size_ << std::endl;
		}
		_buffer.write(	reinterpret_cast<const char*>( &size_ ),
						sizeof( size_t ) );
		
		for( auto it = data.cbegin(); it != data.cend(); ++it )
			*this << *it;
		
		return( *this );
	}
	template<typename T1, typename T2>
	NTSerialize& operator>>( std::unordered_map<T1, T2>& data ) {
		size_t size_ = 0;
		_buffer.read(	reinterpret_cast<char*>( &size_ ),
						sizeof( size_t ) );
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout
				<< "DEBUG read unordered_map: stringstream::good() = "
				<< std::boolalpha << _buffer.good()
				<< " data size: " << size_ << std::endl;
		}
		for( size_t i = 0; i < size_; ++i ) {
			std::pair<T1, T2> val_;
			*this >> val_;
			data.insert( val_ );
		}
		
		return( *this );
	}
	template<typename T1, typename T2>
	NTSerialize& operator<<(const std::unordered_multimap<T1,T2>& data){
		size_t size_ = data.size();
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout
			<< "DEBUG write unordered_multimap: stringstream::good() = "
			<< std::boolalpha << _buffer.good()
			<< " data size: " << size_ << std::endl;
		}
		_buffer.write(	reinterpret_cast<const char*>( &size_ ),
						sizeof( size_t ) );
		
		for( auto it = data.cbegin(); it != data.cend(); ++it )
			*this << *it;
		
		return( *this );
	}
	template<typename T1, typename T2>
	NTSerialize& operator>>( std::unordered_multimap<T1, T2>& data ) {
		size_t size_ = 0;
		_buffer.read(	reinterpret_cast<char*>( &size_ ),
						sizeof( size_t ) );
		if( _is_debug ) {
			std::lock_guard<std::mutex> lck_( _console_mtx );
			std::cout
			<< "DEBUG read unordered_multimap: stringstream::good() = "
			<< std::boolalpha << _buffer.good()
			<< " data size: " << size_ << std::endl;
		}
		for( size_t i = 0; i < size_; ++i ) {
			std::pair<T1, T2> val_;
			*this >> val_;
			data.insert( val_ );
		}
		
		return( *this );
	}
	
	std::stringstream& get() {
		return( _buffer );
	}
	std::streampos pos() {
		return( _buffer.tellg() );
	}
	void pos( size_t pos, std::ios_base::seekdir way ) {
		_buffer.seekg( pos, way );
	}
	
	bool save( const char* filename ) {
		std::ofstream ofs_( filename,
							std::ofstream::out | std::ofstream::trunc
							| std::ifstream::binary );
		if( !ofs_.is_open() )
			return( false );
		ofs_ << _buffer.str();//.rdbuf();
		ofs_.close();
		return( ofs_.good() );
	}
	bool load( const char* filename ) {
		std::ifstream ifs_( filename,
							std::ifstream::in | std::ifstream::binary );
		if( !ifs_.is_open() )
			return( false );
		_buffer << ifs_.rdbuf();
		ifs_.close();
		_buffer.seekp( 0, std::ios::beg );
		return( ifs_.good() );
	}
	
	NTSerialize( std::mutex& mtx ) : _console_mtx( mtx ) {
		
	}
	~NTSerialize() {
		
	}

private:
	std::stringstream	_buffer;
	bool _is_debug{false};
	std::mutex& _console_mtx;
}; // class NTSerialize

} // ntllct



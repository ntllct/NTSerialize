// Copyright (c) 2017 Alexander Alexeev [ntllct@protonmail.com] 
// Compilation: g++ -std=c++14 -m64 -O2 NTSerialize_test.cpp -o NTStest

#include "NTSerialize.hpp"
#include <cstddef>
#include <algorithm>

using namespace ntllct;

std::mutex console_mtx;

struct TestStruct1 {
	unsigned int x1;
	unsigned int x2;
};

struct TestStruct2 {
	unsigned int x1;
	unsigned int x2;
	std::fstream fs;
	
	friend NTSerialize& operator<<( NTSerialize& bnz,
									const TestStruct2& ts2 ) {
		bnz << ts2.x1 << ts2.x2;
		return( bnz );
	}
	friend NTSerialize& operator>>( NTSerialize& bnz,
									TestStruct2& ts2 ) {
		bnz >> ts2.x1 >> ts2.x2;
		return( bnz );
	}
};

void test_easy() {
	NTSerialize ser_out( console_mtx );
	size_t val_out_ = 123;
	std::string text_out_ = "Some text...";
	ser_out << val_out_ << text_out_;
	ser_out.save( "test_easy.bin" );
	
	NTSerialize ser_in( console_mtx );
	ser_in.load( "test_easy.bin" );
	size_t val_in_ = 0;
	std::string text_in_ = "...";
	ser_in >> val_in_ >> text_in_;
	
	std::lock_guard<std::mutex> lck_( console_mtx );
	if( val_in_ == val_out_ && text_in_ == text_out_ ) {
		std::cout << "test_easy: OK!" << std::endl;
	} else {
		std::cout << "test_easy: error!" << std::endl;
	}
}
void test_struct() {
	NTSerialize ser_out( console_mtx );
	TestStruct1 struct_out1_ = { 2, 7 };
	TestStruct2 struct_out2_ = { 6, 11 };
	ser_out << struct_out1_ << struct_out2_;
	ser_out.save( "test_struct.bin" );
	
	NTSerialize ser_in( console_mtx );
	ser_in.load( "test_struct.bin" );
	TestStruct1 struct_in1_ = { 3, 8 };
	TestStruct2 struct_in2_ = { 5, 10 };
	ser_in >> struct_in1_ >> struct_in2_;
	
	std::lock_guard<std::mutex> lck_( console_mtx );
	if( struct_in1_.x1 == struct_out1_.x1
		&& struct_in1_.x2 == struct_out1_.x2
		&& struct_in2_.x1 == struct_out2_.x1
		&& struct_in2_.x2 == struct_out2_.x2 ) {
		
		std::cout << "test_struct: OK!" << std::endl;
	} else {
		std::cout << "test_struct: error!" << std::endl;
	}
}
void test_vector() {
	NTSerialize ser_out( console_mtx );
	ser_out << ntsdirective::debug;
	std::vector<unsigned int> vec_out_{ 10, 20, 30 };
	ser_out << vec_out_;
	ser_out.save( "test_vector.bin" );
	
	NTSerialize ser_in( console_mtx );
	ser_in << ntsdirective::debug;
	ser_in.load( "test_vector.bin" );
	std::vector<unsigned int> vec_in_;
	ser_in >> vec_in_;
	
	std::lock_guard<std::mutex> lck_( console_mtx );
	if( vec_out_.size() == vec_in_.size()
		&& vec_out_[0] == vec_in_[0]
		&& vec_out_[1] == vec_in_[1]
		&& vec_out_[2] == vec_in_[2] ) {
		
		std::cout << "test_vector: OK!" << std::endl;
	} else {
		std::cout << "test_vector: error!" << std::endl;
	}
}
void test_stack() {
	NTSerialize ser_out( console_mtx );
	ser_out << ntsdirective::debug;
	std::stack<unsigned int> stack_out_;
	stack_out_.push( 10 );
	stack_out_.push( 20 );
	stack_out_.push( 30 );
	ser_out << stack_out_;
	ser_out.save( "test_stack.bin" );
	
	NTSerialize ser_in( console_mtx );
	ser_in << ntsdirective::debug;
	ser_in.load( "test_stack.bin" );
	std::stack<unsigned int> stack_in_;
	ser_in >> stack_in_;
	
	std::lock_guard<std::mutex> lck_( console_mtx );
	std::vector<unsigned int> data_;
	for( unsigned int i = 0; i < 3; ++i ) {
		if( stack_out_.empty() || stack_in_.empty() )
			break;
		data_.push_back( stack_out_.top() );
		data_.push_back( stack_in_.top() );
		stack_out_.pop();
		stack_in_.pop();
	}
	if( data_.size() == 6
		&& data_[0] == data_[1]
		&& data_[2] == data_[3]
		&& data_[4] == data_[5] ) {
		
		std::cout << "test_stack: OK!" << std::endl;
	} else {
		std::cout << "test_stack: error!" << std::endl;
	}
}
void test_set() {
	NTSerialize ser_out( console_mtx );
	ser_out << ntsdirective::debug;
	std::set<unsigned int> set_out_{ 10, 20, 30 };
	ser_out << set_out_;
	ser_out.save( "test_set.bin" );
	
	NTSerialize ser_in( console_mtx );
	ser_in << ntsdirective::debug;
	ser_in.load( "test_set.bin" );
	std::set<unsigned int> set_in_;
	ser_in >> set_in_;
	
	auto set_out_it_ = set_out_.begin();
	auto set_in_it_ = set_in_.begin();
	std::lock_guard<std::mutex> lck_( console_mtx );
	if( set_out_.size() == set_in_.size()
		&& *set_out_it_++ == *set_in_it_++
		&& *set_out_it_++ == *set_in_it_++
		&& *set_out_it_++ == *set_in_it_++ ) {
		
		std::cout << "test_set: OK!" << std::endl;
	} else {
		std::cout << "test_set: error!" << std::endl;
	}
}
void test_map() {
	NTSerialize ser_out( console_mtx );
	ser_out << ntsdirective::debug;
	std::map<unsigned int, unsigned int> map_out_{
													{ 10, 1 },
													{ 20, 2 },
													{ 30, 3 }
												};
	ser_out << map_out_;
	ser_out.save( "test_map.bin" );
	
	NTSerialize ser_in( console_mtx );
	ser_in << ntsdirective::debug;
	ser_in.load( "test_map.bin" );
	std::map<unsigned int, unsigned int> map_in_;
	ser_in >> map_in_;
	
	auto map_out_it_ = map_out_.begin();
	auto map_in_it_ = map_in_.begin();
	std::lock_guard<std::mutex> lck_( console_mtx );
	if( map_out_.size() == map_in_.size()
		&& *map_out_it_++ == *map_in_it_++
		&& *map_out_it_++ == *map_in_it_++
		&& *map_out_it_++ == *map_in_it_++ ) {
		
		std::cout << "test_map: OK!" << std::endl;
	} else {
		std::cout << "test_map: error!" << std::endl;
	}
}
void test_unordered_multimap() {
	NTSerialize ser_out( console_mtx );
	ser_out << ntsdirective::debug;
	std::unordered_multimap<unsigned int, unsigned int> ummap_out_{
													{ 10, 1 },
													{ 20, 2 },
													{ 20, 5 },
													{ 30, 3 }
												};
	ser_out << ummap_out_;
	ser_out.save( "test_unordered_multimap.bin" );
	
	NTSerialize ser_in( console_mtx );
	ser_in << ntsdirective::debug;
	ser_in.load( "test_unordered_multimap.bin" );
	std::unordered_multimap<unsigned int, unsigned int> ummap_in_;
	ser_in >> ummap_in_;
	
	auto ummap_out_it_ = ummap_out_.begin();
	auto ummap_in_it_ = ummap_in_.begin();
	std::vector<unsigned int> data_;
	if( ummap_out_.size() == ummap_in_.size() ) {
		for( unsigned int i = 0; i < 4; ++i ) {
			data_.push_back( ummap_out_it_->first );
			data_.push_back( ummap_out_it_->second );
			data_.push_back( ummap_in_it_->first );
			data_.push_back( ummap_in_it_->second );
			ummap_out_it_++;
			ummap_in_it_++;
		}
		std::sort( data_.begin(), data_.end() );
	}
	std::lock_guard<std::mutex> lck_( console_mtx );
	if( data_.size() == 16
		&& data_[0] == 1
		&& data_[1] == 1
		&& data_[2] == 2
		&& data_[3] == 2
		&& data_[4] == 3
		&& data_[5] == 3
		&& data_[6] == 5
		&& data_[7] == 5
		&& data_[8] == 10
		&& data_[9] == 10
		&& data_[10] == 20
		&& data_[11] == 20
		&& data_[12] == 20
		&& data_[13] == 20
		&& data_[14] == 30
		&& data_[15] == 30 ) {
		
		std::cout << "test_unordered_multimap: OK!" << std::endl;
	} else {
		std::cout << "test_unordered_multimap: error!" << std::endl;
	}
}

int main() {
	test_easy();
	test_struct();
	test_vector();
	test_stack();
	test_set();
	test_map();
	test_unordered_multimap();
	return( EXIT_SUCCESS );
}


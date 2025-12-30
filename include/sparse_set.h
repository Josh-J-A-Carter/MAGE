#ifndef SPARSE_SET_H
#define SPARSE_SET_H

#include <vector>
#include <optional>
#include <cassert>
#include <utility>
#include <unordered_map>
#include <algorithm>

//#include <iostream>

// shows transformations between index "spaces"
// SparseIndex --> PageIndex --> DenseIndex
using SparseIndex = std::size_t;
using PageIndex = std::size_t;
using DenseIndex = std::size_t;

struct Page {
	static constexpr std::size_t default_size { 128 };
	static constexpr DenseIndex invalid_index { std::numeric_limits<DenseIndex>::max() };

	DenseIndex operator[](PageIndex page_index) {
		if (page_to_dense.empty()) return invalid_index;

		assert(page_index < page_to_dense.size() && "Page index exceeds page bounds");

		return page_to_dense[page_index];
	}

	void add_entry(PageIndex page_index, DenseIndex dense_index, std::size_t page_size) {
		if (page_to_dense.empty()) page_to_dense = std::vector<DenseIndex>( page_size, invalid_index );

		page_to_dense[page_index] = dense_index;
	}

	void remove_entry(PageIndex page_index) {
		page_to_dense[page_index] = invalid_index;

		// could free memory again if completely empty, though this is more of a hassle than it may be worth
		//if (std::all_of(page_to_dense.begin(), page_to_dense.end(), [](DenseIndex d) { d == invalid_index; });
	}

	//void print() {
	//	if (page_to_dense.empty()) {
	//		std::cout << "----" << std::endl;
	//		return;
	//	}

	//	for (DenseIndex idx : page_to_dense) {
	//		if (idx == invalid_index) std::cout << "-1\n";
	//		else std::cout << idx << "\n";
	//	}
	//}

private:
	std::vector<DenseIndex> page_to_dense {};
};


template<typename T>
struct SparseSet {


private:
	std::vector<Page> sparse;
	std::vector<T> dense;
	std::unordered_map<DenseIndex, SparseIndex> dense_to_sparse;

	std::size_t page_size;
	std::size_t max_count;

	std::pair<PageIndex, PageIndex> sparse_to_page_indices(SparseIndex sparse_index) {
		assert(sparse_index < max_count && "Sparse index exceeds sparse set bounds");

		std::size_t page_number = sparse_index / page_size; // which page does this correspond to?
		PageIndex page_index = sparse_index % page_size; // which index does this correspond to, inside the given page?

		assert(page_number < sparse.size() && "Sparse index maps to non-existent page");
		assert(page_index < page_size && "Sparse index maps to page index beyond the bounds of a single page");

		return { page_number, page_index };
	}

public:
	SparseSet(std::size_t max_count, std::size_t page_size = Page::default_size)
		: max_count { max_count }, page_size { page_size },
		// if page_size doesn't divide max_count, need an extra page since integer division truncates fractional components
		sparse ( max_count / page_size + (max_count % page_size == 0 ? 0 : 1) ) {

		// This reserve size is rather arbitrary - should be adjusted as needed
		dense_to_sparse.reserve(page_size);
	}

	T& operator[](SparseIndex sparse_index) {
		auto [ page_number, page_index ] = sparse_to_page_indices(sparse_index);

		DenseIndex dense_index = sparse[page_number][page_index];

		assert(dense_index != Page::invalid_index && "Attempted to access a non-existent entry");

		return dense[dense_index];
	}

	void add_entry(SparseIndex sparse_index, T entry) {
		auto [ page_number, page_index ] = sparse_to_page_indices(sparse_index);

		assert(sparse[page_number][page_index] == Page::invalid_index && "Attempted to add entry, but this sparse index already has one");

		DenseIndex dense_index = dense.size();
		dense.emplace_back(entry);
		sparse[page_number].add_entry(page_index, dense_index, page_size);
		dense_to_sparse[dense_index] = sparse_index;
	}

	void remove_entry(SparseIndex sparse_index0) {
		// Find the dense index of the entry we want to remove
		auto [ page_number0, page_index0 ] = sparse_to_page_indices(sparse_index0);
		DenseIndex dense_index0 = sparse[page_number0][page_index0];

		assert(dense_index0 != Page::invalid_index && "Attempted to remove entry, but this sparse index has none associated");
		assert(dense.size() > 0 && "Attempted to remove entry, but no entries are currently stored.");

		// Find the sparse index of the last dense entry
		DenseIndex dense_index1 = dense.size() - 1;
		assert(dense_to_sparse.contains(dense_index1) && "Final element of dense vector does not have reverse dense_to_sparse mapping.");
		SparseIndex sparse_index1 = dense_to_sparse[dense_index1];
		auto [ page_number1, page_index1 ] = sparse_to_page_indices(sparse_index1);

		// Swap the entries, in the dense array
		dense[dense_index0] = dense[dense_index1];
		dense.pop_back();

		// Book-keeping with pages; remove the old entries for both sparse indices, then remap the swapped one to the deleted dense index
		sparse[page_number0].remove_entry(page_index0);
		sparse[page_number1].remove_entry(page_index1);
		if (sparse_index0 != sparse_index1) sparse[page_number1].add_entry(page_index1, dense_index0, page_size);
	}

	//void print_sparse() {
	//	std::cout << "Sparse:" << std::endl;
	//	for (std::size_t i { 0 } ; i < sparse.size() ; i += 1) {
	//		std::cout << "Page " << i << "\n";
	//		sparse[i].print();
	//	}
	//}

	//void print_dense() {
	//	std::cout << "Dense:" << std::endl;
	//	for (T& element : dense) std::cout << element << "\n";
	//}
};

#endif


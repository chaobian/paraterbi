

#ifndef __MATRIXV_HPP__
#define __MATRIXV_HPP__

#include <memory>
#include <vector>

#include <Vc/Vc>

template<typename vector_T>
class MatrixV {
public:
//	using floatv = Vc::Vector<value_type>;
	using floatv = vector_T;
	using value_type = typename floatv::value_type;
	//	using array_type = std::vector<floatv, Vc::allocator>;
	using array_type = Vc::Memory<floatv>;
	using matrix_type = MatrixV<floatv>;

public:
	MatrixV() = delete;
	MatrixV(int rows, int columns, value_type defaultValue)
		: m_rows(rows), m_data() {
		init(columns, defaultValue);
	}

	MatrixV(const matrix_type& other) = default;
	MatrixV(matrix_type&& other) = default;

	matrix_type& operator=(const matrix_type& rhs) = default;
	matrix_type& operator=(matrix_type&& rhs) = default;
	
/*
	MatrixV(MatrixV&& other)
		: m_rows(other.m_rows), m_data(std::move(other.m_data)) {}
*/

	// return type must be auto, since Vc::memory returns complicated wrapper
	// types here
inline 	auto& vector(int row, int column) {
	return m_data[column].vector(row);
	}

inline 	const auto& vector(int row, int column) const {
	return m_data[column].vector(row);
	}

	value_type& operator()(int row, int column) {
		return m_data[column][row];
	}

	const value_type& operator()(int row, int column) const {
		return m_data[column][row];
	}

array_type& column(size_t column) {
	return m_data[column];
}

	const array_type& column(size_t column) const {
		return m_data[column];
	}

	size_t vectorsCountPerColumn() const {
		if(m_data.size() == 0) {
			return 0;
		}
		
		return m_data[0].vectorsCount();
	}

	size_t columns() const { return m_data.size(); }

	size_t rows() const { return m_rows; }

	void reserve(size_t columns) {
		const auto c = m_data.size();
		if(columns <= c) {
			return;
		}

		for(size_t j = 0; j < columns - c; ++j) {
			m_data.emplace_back(m_rows);
		}

	}


private:

	inline void init(size_t columns, const value_type& value) {
		const auto tmp = floatv(value);
		m_data.reserve(columns);
		for(size_t j = 0; j < columns; ++j) {
			m_data.emplace_back(m_rows);
			auto& mem = m_data[j];
			for(size_t i = 0; i < mem.vectorsCount(); ++i) {
				mem.vector(i) = tmp;
			}
		}
	}

/*
	inline void init(const value_type& value) {
		const floatv tmp = value;
		for (size_t i = 0; i < m_data.vectorsCount(); ++i) {
			m_data.vector(i) = tmp;
		}
	}
*/
private:
	size_t m_rows;
	std::vector<array_type> m_data;
};  // end class MatrixV

/*
inline void matrixTest(int a, int b) {
	using floatv = Vc::Vector<int>;
	using matrix_type = MatrixV<floatv>;

	auto m = matrix_type{a, b, 123};

//	for(int i = 0; i < (a + floatv::Size - 1) / floatv::Size; ++i) {
	for(int i = 0; i < (a + floatv::Size - 1) / floatv::Size; ++i) {
		for(int j = 0; j < b; ++j) {
			m.vector(i, j) = floatv(0);
		}
	}

	for(int i = 0; i < a; ++i) {
		for(int j = 0; j < b; ++j) {
			if(m(i,j) == 123)
				std::cerr << "Matrix(" << i << ", " << j << ") = 123" << std::endl;
		}
	}
}
*/

#endif

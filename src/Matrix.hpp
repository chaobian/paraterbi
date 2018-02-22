

#ifndef __MATRIX_HPP__
#define __MATRIX_HPP__

#include <vector>

template <typename value_T>
class Matrix {
public:
	using value_type = value_T;
	using size_type = typename std::vector<value_type>::size_type;

public:
	Matrix() = delete;
	Matrix(int rows, int columns, value_type defaultValue)
		: m_rows(rows),
		  m_columns(columns),
		  m_data(rows * columns, defaultValue) {}

	Matrix(const Matrix& other)
		: m_rows(other.m_rows),
		  m_columns(other.m_columns),
		  m_data(other.m_data) {}
	Matrix(Matrix&& other)
		: m_rows(other.m_rows),
		  m_columns(other.m_columns),
		  m_data(std::move(other.m_data)) {}

	value_type& operator()(int row, int column) {
		return m_data[row + (column * m_rows)];
	}

	const value_type& operator()(int row, int column) const {
		return m_data[row + (column * m_rows)];
	}

	void reserve(size_type nm) { m_data.reserve(nm); }

private:
	int m_rows;
	int m_columns;
	std::vector<value_type> m_data;
};  // end class matrix
#endif

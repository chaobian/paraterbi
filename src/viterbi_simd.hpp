

#ifndef PARATERBI__VITERBI_SIMD_HPP__
#define PARATERBI__VITERBI_SIMD_HPP__

#include "MatrixV.hpp"
#include "utility.hpp"
#include <iostream>

#include <limits>
#include <algorithm>
#include <vector>

template <typename probability_T, typename label_T = int,
		  typename emission_T = int>
class Viterbi {
public:
	using Label_type = label_T;
	using Emission_type = emission_T;
	using Probability_type = probability_T;
	using floatv = Vc::Vector<Probability_type>;
	using Matrix_type = MatrixV<floatv>;
	using Viterbi_type = Viterbi<Probability_type, Label_type, Emission_type>;

	struct Model {
		using Label_type = Viterbi_type::Label_type;
		using Emission_type = Viterbi_type::Emission_type;
		using Probability_type = Viterbi_type::Probability_type;
		using Matrix_type = Viterbi_type::Matrix_type;

		friend Viterbi_type;

	public:
		Model() = delete;
		Model(int labels, int emissions, Probability_type defaultValue)
			: start(labels, 1, defaultValue),
			  transitions(labels, labels, defaultValue),
			  emissions(labels, emissions, defaultValue) {}

		Model(const Model& other) = default;
		Model(Model&& other) = default;
		Model& operator=(const Model& rhs) = default;
		Model& operator=(Model&& rhs) = default;

	public:
		inline void setStart(Label_type i, Probability_type value) {
			start(i, 0) = value;
		}

		inline void setTransition(Label_type from, Label_type to,
								  Probability_type value) {
			transitions(to, from) = value;
		}

		inline void setEmission(Label_type label, Emission_type emission,
								Probability_type value) {
			emissions(label, emission) = value;
		}

	private:
		Matrix_type start;
		Matrix_type transitions;
		Matrix_type emissions;
	};  // end class Model

public:
	Viterbi() = delete;
	Viterbi(const Model& m)
		: m_model(m),
		  trellis(m.start.rows(), 8,
				  -(std::numeric_limits<Probability_type>::infinity())),
		  backpointers(m.start.rows(), 8, 0) {}

	Viterbi(const Viterbi_type& other) = default;
	Viterbi(Viterbi_type&& other) = default;
	Viterbi_type& operator=(const Viterbi_type& rhs) = default;
	Viterbi_type& operator=(Viterbi_type&& rhs) = default;

	std::vector<Label_type> infer(std::vector<Emission_type> ts) {
		const int n = ts.size();

		if (n == 0) return std::vector<Label_type>{};

		const int labelCount = m_model.start.rows();
		const int labelVectorsCount = m_model.start.vectorsCountPerColumn();
		trellis.reserve(n);
		backpointers.reserve(n);

		// first column
		for (auto i = 0; i < labelVectorsCount; ++i) {
			const floatv s = m_model.start.vector(i, 0);
			const floatv em = m_model.emissions.vector(i, ts[0]);
			trellis.vector(i, 0) = em + s;
		}

		// other columns
		for (auto j = 1; j < n; ++j) {
			for (int i = 0; i < labelVectorsCount; ++i) {
				auto winnerProb = floatv(
					-(std::numeric_limits<Probability_type>::infinity()));
				auto winner = typename floatv::IndexType{0};
				const auto emProb = floatv{m_model.emissions.vector(i, ts[j])};

				for (int prev = 0; prev < labelCount; ++prev) {
					const auto p = trellis(prev, j - 1);
					const auto t = floatv{m_model.transitions.vector(i, prev)};
					const auto candidate = p + t;
					const auto mask = winnerProb < candidate;

					winner =
						Vc::iif(mask, typename floatv::IndexType(prev), winner);
					winnerProb = Vc::iif(mask, candidate, winnerProb);
				}
				trellis.vector(i, j) = winnerProb + emProb;
				backpointers.vector(i, j) = winner;
			}
		}

		// check last column separately for highest likelihood
		const auto maxIndex = std::distance(
			&(trellis(0, n - 1)),
			std::max_element(&(trellis(0, n - 1)),
							 std::next(&(trellis(labelCount - 1, n - 1)))));

		// now retrace the backpointers to find the best label sequence
		auto best = std::vector<Label_type>(n, maxIndex);
		for (auto j = n - 1; j > 0; --j) {
			best[j - 1] = backpointers(best[j], j);
		}

		return best;
	}  // end infer

private:
	Model m_model;
	Matrix_type trellis;
	MatrixV<typename floatv::IndexType> backpointers;
};  // end class Viterbi

#endif

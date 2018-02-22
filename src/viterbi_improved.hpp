

#ifndef __VITERBI_BASELINE_HPP__
#define __VITERBI_BASELINE_HPP__

#include "Matrix.hpp"
#include "utility.hpp"
#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>

template <typename probability_T>
class Viterbi {
public:
	using Label_type = int;
	using Emission_type = int;
	using Probability_type = probability_T;
	using Matrix_type = Matrix<Probability_type>;
	using Viterbi_type = Viterbi<Probability_type>;
	struct Model {
		using Label_type = Viterbi_type::Label_type;
		using Emission_type = Viterbi_type::Emission_type;
		using Probability_type = Viterbi_type::Probability_type;
		using Matrix_type = Viterbi_type::Matrix_type;

		friend Viterbi_type;
	public:
		Model() = delete;
		Model(int labels, int emissions, Probability_type defaultValue)
			: start(labels, defaultValue),
			  transitions(labels, labels, defaultValue),
			  emissions(labels, emissions, defaultValue) {}
	public:
		inline void setStart(Label_type i, Probability_type value) { start[i] = value; }
		inline void setTransition(Label_type from, Label_type to, Probability_type value) { transitions(from, to) = value; }
		inline void setEmission(Label_type label, Emission_type emission, Probability_type value) { emissions(label, emission) = value; }

	private:
		std::vector<Probability_type> start;
		Matrix_type transitions;
		Matrix_type emissions;

	};  // end class Model

public:
	Viterbi() = delete;
	Viterbi(const Model& m)
		: m_model(m),
		  trellis(m.start.size(), 8, std::log(0)),
		  backpointers(m.start.size(), 8, 0) {}
	Viterbi(Model&& m)
		: m_model(std::move(m)),
		  trellis(m_model.start.size(), 8, std::log(0)),
		  backpointers(m_model.start.size(), 8, 0) {}

	std::vector<Label_type> infer(std::vector<Emission_type> ts) {
		const int n(ts.size());
		const int labelCount(m_model.start.size());
		trellis.reserve(labelCount * n);
		backpointers.reserve(labelCount * n);

		// first column
		for (auto i = 0; i < labelCount; ++i) {
			trellis(i, 0) = m_model.start[i] + m_model.emissions(i, ts[0]);
		}

		// other columns
		for (auto j = 1; j < n; ++j) {
			for (auto i = 0; i < labelCount; ++i) {
				auto winnerProb = std::log(0);
				auto winner = 0;

				for (auto prev = 0; prev < labelCount; ++prev) {

					auto candidate = trellis(prev, j - 1) +
									 m_model.transitions(prev, i) +
									 m_model.emissions(i, ts[j]);

					winner = winnerProb < candidate ? prev : winner;
					winnerProb =
						winnerProb < candidate ? candidate : winnerProb;

				}
				trellis(i, j) = winnerProb;
				backpointers(i, j) = winner;

			}
		}

		// check last column separately for highest likelihood
/*
		auto lastColumnWinner = 0;
		auto lastColumnWinnerProb = trellis(0, n-1);
		for(auto i = 0; i < labelCount; ++i) {
			lastColumnWinner = lastColumnWinnerProb < trellis(i, n-1) ? i : lastColumnWinner;
			lastColumnWinnerProb = lastColumnWinnerProb < trellis(i, n-1) ? trellis(i, n-1) : lastColumnWinnerProb;
		}
*/
		
		auto lastColumnWinner =
			std::make_pair<Probability_type, Label_type>(std::log(0.0), 0);
		for (auto i = 0; i < labelCount; ++i) {
			lastColumnWinner = std::max(lastColumnWinner,
										std::make_pair(trellis(i, n - 1), i));
		}


		// now retrace the backpointers to find the best label sequence
		auto best = std::vector<Label_type>(n, lastColumnWinner.second);
		// subsumes 		best[n - 1] = lastColumnWinner.second; (could not
		// measure
		// any performance difference)
		for (auto j = n - 1; j > 0; --j) {
			best[j - 1] = backpointers(best[j], j);
		}

		return best;
	}  // end infer

private:
	Model m_model;
	Matrix_type trellis;
	Matrix<int> backpointers;
};  // end class Viterbi

#endif

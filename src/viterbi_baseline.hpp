

#ifndef __VITERBI_BASELINE_HPP__
#define __VITERBI_BASELINE_HPP__

#include "utility.hpp"
#include <iostream>

#include <algorithm>
#include <vector>

template <typename probability_T, template <typename> class matrix_T>
class Viterbi {
public:
	using Label_type = int;
	using Emission_type = int;
	using Probability_type = probability_T;
	using Matrix_type = matrix_T<Probability_type>;

	struct Model {
		using Label_type = Label_type;
		using Emission_type = Emission_type;
		using Probability_type = Probability_type;
		using Matrix_type = Matrix_type;

		 std::vector<Probability_type> start;
 Matrix_type transitions;
 Matrix_type emissions;

		Model() = delete;
		Model(int labels, int emissions, Probability_type defaultValue) : start(labels, defaultValue), transitions(labels, labels, defaultValue), emissions(labels, emissions, defaultValue) {}
	};  // end class Model

public:
	Viterbi() = delete;
	Viterbi(const Model& m) : m_model(m) {}
	Viterbi(Model&& m) : m_model(std::move(m)) {}

	std::vector<Label_type> infer(std::vector<Emission_type> ts) {

		auto labelCount = m_model.start.size();
		auto trellis = Matrix_type(labelCount, ts.size(), log(0));
		auto backpointers = matrix_T<int>(labelCount, ts.size(), 0);

		for (auto i = 0; i < labelCount; ++i) {
			trellis(i, 0) = m_model.start[i] + m_model.emissions(i, ts[0]);
		}

		for (auto j = 1; j < ts.size(); ++j) {
			for (auto i = 0; i < labelCount; ++i) {
				auto winner =
					std::make_pair<Probability_type, Label_type>(log(0.0), 0);
				auto candidate = winner;
				for (auto prev = 0; prev < labelCount; ++prev) {
					candidate.first = trellis(prev, j - 1) +
									  m_model.transitions(prev, i) +
									  m_model.emissions(i, ts[j]);
					candidate.second = prev;
					winner = std::max(winner, candidate);
				}

				trellis(i, j) = winner.first;
				backpointers(i, j) = winner.second;
			}
		}

		auto best = std::vector<Label_type>(ts.size(), 0);
		auto tmp = std::make_pair<Probability_type, Label_type>(log(0.0), 0);
		for (auto i = 0; i < labelCount; ++i) {
			tmp = std::max(tmp, std::make_pair(trellis(i, ts.size() - 1), i));
		}


		best[ts.size() - 1] = tmp.second;
		for (auto j = ts.size() - 1; j > 0; --j) {
			best[j - 1] = backpointers(best[j], j);
		}

		return std::move(best);
	}  // end infer

private:
	Model m_model;
};  // end class Viterbi

#endif

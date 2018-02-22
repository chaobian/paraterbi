

#ifndef PARATERBI__VITERBI_HPP__
#define PARATERBI__VITERBI_HPP__

#include "MatrixV.hpp"
#include "utility.hpp"
#include <iostream>

#include <limits>
#include <algorithm>
#include <vector>
#include <thread>
#include <atomic>
template <typename label_T, typename emission_T>
struct Options {
	using Label_type = label_T;
	using Emission_type = emission_T;
	static const short AutoDetermineChildThreads = -1;
};

using DefaultOptions = Options<int, int>;

template <typename probability_T, typename options = DefaultOptions>
class Viterbi {
public:
	using Label_type = typename options::Label_type;
	using Emission_type = typename options::Emission_type;
	using Probability_type = probability_T;
	using floatv = Vc::Vector<Probability_type>;
	using Matrix_type = MatrixV<floatv>;
	using Viterbi_type = Viterbi<Probability_type, options>;

	struct Model {
	public:
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
	// constructors
	Viterbi() = delete;
	Viterbi(const Model& m,
			short childThreads = options::AutoDetermineChildThreads)
		: m_model(m),
		  trellis(m.start.rows(), 8,
				  -(std::numeric_limits<Probability_type>::infinity())),
		  backpointers(m.start.rows(), 8, 0),
		  m_tsStackPtr(nullptr),
		  m_spawn(*this, childThreads) {}

	Viterbi(const Viterbi_type& other)
		: m_model(other.m_model),
		  trellis(other.trellis),
		  backpointers(other.backpointers),
		  m_tsStackPtr(nullptr),
		  m_spawn(*this, other.m_spawn.m_workers) {}

	Viterbi_type& operator=(const Viterbi_type& rhs) {
		m_model = rhs.m_model;
		trellis = rhs.trellis;
		backpointers = rhs.backpointers;
		m_tsStackPtr = nullptr;
		m_spawn.m_workersDone = 0;
		m_spawn.m_targetColumn = -1;  // signal for workers to die
		while (m_spawn.m_workersDone > -m_spawn.M_workers) {
			// sleep
		}
		// all workers are now dead
		m_spawn.m_targetColumn = 0;
		m_spawn.m_workers = m_spawn.spawn(*this, rhs.m_workers);
		m_spawn.m_workersDone = m_spawn.m_workers;
	}

	~Viterbi() {
				m_spawn.m_targetColumn = -1;  // signal for workers to die
		while (m_spawn.m_workersDone > -m_spawn.m_workers) {
			// sleep
		}
		// all workers are now dead
	}

	inline int labelCount() const { return m_model.start.rows(); }
	inline int labelVectorCount() const {
		return m_model.start.vectorsCountPerColumn();
	}

	std::vector<Label_type> infer(std::vector<Emission_type> ts) {
		const int n = ts.size();
		if (n == 0) return std::vector<Label_type>{};

		const int labelCount = this->labelCount();
		const int labelVectorsCount = this->labelVectorCount();

		trellis.reserve(n);
		backpointers.reserve(n);

		// first column
		for (auto i = 0; i < labelVectorsCount; ++i) {
			const floatv s = m_model.start.vector(i, 0);
			const floatv em = m_model.emissions.vector(i, ts[0]);
			trellis.vector(i, 0) = em + s;
		}

		// other columns
		m_tsStackPtr = ts.data();

		for(int i = 1; i < n; ++i) {
			m_spawn.m_workersDone = 0;
			++m_tsStackPtr;
//			std::cerr << i << std::endl;
			m_spawn.m_targetColumn = i;
			while(m_spawn.m_workersDone != m_spawn.m_workers) {}
		}
		m_spawn.m_workersDone = 0;
//		m_spawn.m_targetColumn = 0;
//		m_spawn.m_workersDone = 0;

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
	class WorkerSpawn {
	public:
		static void worker(Viterbi_type& parent, const Label_type begin,
						   const Label_type end) {
			auto myColumn = int{0};
			auto& pool = parent.m_spawn;
			const int labelCount = parent.labelCount();

//			std::cerr << "my range: " << begin << " : " << end << std::endl;
			while (pool.m_targetColumn != -1) {

				while(myColumn == pool.m_targetColumn) {
					// sleep
				}

				if (pool.m_targetColumn == -1) {
					--pool.m_workersDone;
					return;
				}






//				myColumn = pool.m_targetColumn == 1 ? 1 : myColumn+1;
				const int j = pool.m_targetColumn;
				myColumn = j;
				for (int i = begin; i < end; ++i) {
//					std::cerr << "Current " << j << ", " << i << ", my: " << myColumn << std::endl;
					auto winner = typename floatv::IndexType{0};
					auto winnerProb = floatv(
						-(std::numeric_limits<Probability_type>::infinity()));
					const auto emProb = floatv{parent.m_model.emissions.vector(
						i, *(parent.m_tsStackPtr))};
					for (int prev = 0; prev < labelCount; ++prev) {
						const auto p = parent.trellis(prev, j - 1);
						const auto t =
							floatv{parent.m_model.transitions.vector(i, prev)};
						const auto candidate = p + t;
						const auto mask = winnerProb < candidate;

						winner = Vc::iif(mask, typename floatv::IndexType(prev),
										 winner);
						winnerProb = Vc::iif(mask, candidate, winnerProb);
					}
					parent.trellis.vector(i, j) = winnerProb + emProb;
					parent.backpointers.vector(i, j) = winner;
				}  // end outer for
				++pool.m_workersDone;
//				std::cerr << begin << "," << end << " Done: " << pool.m_workersDone << std::endl;
			}
			--pool.m_workersDone;
		}  // end worker

	public:
		friend Viterbi_type;

		WorkerSpawn(Viterbi_type& parent, short desiredWorkers)
			: m_targetColumn(0),
			  m_workers(spawn(parent, desiredWorkers)),
			  m_workersDone(m_workers) {}

		short spawn(Viterbi_type& parent, short desired) {
			const short n = desired == options::AutoDetermineChildThreads
								? std::thread::hardware_concurrency()
								: desired;
			const auto k = parent.labelVectorCount();
			const short rowsPerChild = (k + n - 1) / n;
//			std::cerr << rowsPerChild << " = " << k << " / " << n << std::endl;
			for (auto i = 0; i < n - 1; ++i) {
				auto child =
					std::thread{WorkerSpawn::worker, std::ref(parent),
								i * rowsPerChild, (i + 1) * rowsPerChild};
				child.detach();
			}
			// this may be the only thread
			// or the last one (handling the last rows). Since the last rows are
			// padded/might take a couple cycles more due to SIMD lanes, the
			// last thread gets a couple fewer rows in case of k mod n != 0
			auto child = std::thread{WorkerSpawn::worker, std::ref(parent),
									 (n - 1) * rowsPerChild,
									 k};
//									 ((n - 1) * rowsPerChild) + (k / n)};
			child.detach();

			return n;
		}

	private:
		std::atomic<int> m_targetColumn;
		const short m_workers;
		std::atomic<int> m_workersDone;
	};  // end class WorkerSpawn

private:
	Model m_model;
	Matrix_type trellis;
	MatrixV<typename floatv::IndexType> backpointers;
	int* m_tsStackPtr;
	WorkerSpawn m_spawn;
};  // end class Viterbi

#endif

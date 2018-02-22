

#if VITERBI_DEVEL_ITERATION==1
#include "viterbi_improved.hpp"
#endif
#if VITERBI_DEVEL_ITERATION==2
#include "viterbi_simd.hpp"
#endif
#if VITERBI_DEVEL_ITERATION==3
#include "viterbi.hpp"
#endif

#include <chrono>
#include "HMM.hpp"

#include <sstream>

int main() {

//	std::ios::sync_with_stdio(false);
	auto hmm = HMM<Viterbi<double>>{"../data/corpus.txt"};
	auto line = std::string{};
	auto ws = std::vector<std::string>{};
	ws.reserve(10);
	auto out = std::ostringstream{};
	// meaure wall time

//	auto before = std::chrono::high_resolution_clock::now();
#ifdef DO_PROFILING
ProfilerStart("profile.log");
#endif
	while (std::getline(std::cin, line).good()) {
		if (line == "") {
//			auto before = std::chrono::high_resolution_clock::now();
			auto tags = hmm.infer(ws);
//			total += std::chrono::high_resolution_clock::now() - before;
			for (decltype(ws.size()) i = 0; i < ws.size(); ++i) {
				out << ws[i] << "\t" << tags[i] << std::endl;
			}
			out << std::endl;
			std::cout << out.str();
			ws.clear();
			out.str("");
		} else {
			ws.push_back(line);
		}
	}  // end while
#ifdef DO_PROFILING
ProfilerStop();
#endif


//	auto after = std::chrono::high_resolution_clock::now();
//	std::cerr << std::chrono::duration<double/*, std::milli*/>(after - before)
//					 .count() << std::endl;
	std::cerr << total.count() << std::endl;


	return 0;
}

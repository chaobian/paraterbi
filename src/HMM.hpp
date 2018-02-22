

#ifndef __HMM_HPP__
#define __HMM_HPP__

#include "utility.hpp"
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <boost/functional/hash.hpp>
#include <boost/bimap.hpp>
#include <math.h>


	auto total = std::chrono::duration<double, std::milli>{0};


template <typename viterbi_T>
class HMM {
public:
	using Viterbi_type = viterbi_T;
	using Model_type = typename Viterbi_type::Model;

public:
	HMM() = delete;
	HMM(const std::string& filename) : m_viterbi(mlTrain(filename)) {}

	HMM(const HMM& other) = default;
	HMM& operator=(const HMM& rhs) = default;
	std::vector<std::string> infer(const std::vector<std::string>& ws) {
		auto is =
			std::vector<typename Viterbi_type::Emission_type>(ws.size(), 0);

		std::transform(ws.cbegin(), ws.cend(), is.begin(),
					   [this](const std::string& w) {
						   auto i = emissionBijection.left.find(w);
						   if (i == emissionBijection.left.end()) {
							   std::cerr << w << " not covered.\n";
							   return 0;
						   }
						   return i->second;
					   });

		auto before = std::chrono::high_resolution_clock::now();

		auto iouts = m_viterbi.infer(is);

		total += std::chrono::high_resolution_clock::now() - before;

		auto outs = std::vector<std::string>(ws.size(), "");

		std::transform(iouts.cbegin(), iouts.cend(), outs.begin(),
					   [this](const typename Viterbi_type::Label_type& l) {
						   return labelBijection.right.find(l)->second;
					   });

		return outs;
	}  // end infer

private:
	using stringmap = std::unordered_map<std::string, int>;
	using stringpair = std::pair<std::string, std::string>;
	using pairmap =
		std::unordered_map<stringpair, int, boost::hash<stringpair>>;
	using bijection = boost::bimap<std::string, int>;
	using bijectionPair = bijection::value_type;

private:
	Model_type mlTrain(const std::string& filename) {
		auto file = std::ifstream{filename};

		bool isNewSent = true;
		stringmap initialLabelCount;
		stringmap labelCount;
		pairmap transitionCount;
		pairmap emissionCount;
		std::string prevLabel;
		int nextLabel = 0;
		int nextEmission = 0;
		int numSents = 0;

		for (std::string label, emission = "";
			 std::getline(file, emission) && std::getline(file, label);) {
			// if its two new lines in the file emission will be empty
			// note: we require two newlines at end of file (or numSents is
			// wrong)
			if (emission == "") {
				isNewSent = true;
				numSents += 1;
				emission = label;
				if (!std::getline(file, label)) break;
//			} else {
			}

				// bijections
				if (labelBijection.left.find(label) ==
					labelBijection.left.end()) {
					labelBijection.insert(bijectionPair(label, nextLabel++));
				}

				if (emissionBijection.left.find(emission) ==
					emissionBijection.left.end()) {
					emissionBijection.insert(
						bijectionPair(emission, nextEmission++));
				}

				if (isNewSent) {
					isNewSent = false;
					initialLabelCount[label] += 1;
					labelCount[label] += 1;
					prevLabel = label;
				} else {
					// it is not a new sentence, so prevLabel is valid
					transitionCount[std::make_pair(prevLabel, label)] += 1;
					labelCount[label] += 1;
				}

				// we can always count emissions
				emissionCount[std::make_pair(label, emission)] += 1;
				prevLabel = label;
		}  // end for

		// now comes the ml-estimate step
		auto maxLabel = nextLabel;
		auto maxEmission = nextEmission;
/*		std::cout << "Labels: " << maxLabel << "\nEmissions: " << maxEmission
				  << "\nSentences: " << numSents << std::endl;
*/
		auto m = Model_type{maxLabel, maxEmission, std::log(0.0f)};

/*		for (auto i = 0; i < maxLabel; ++i) {
			m.start[i] =
				log(initialLabelCount[labelBijection.right.find(i)->second]) -
				log(numSents);
		}
*/

		for(const auto& l : initialLabelCount) {
			m.setStart(labelBijection.left.find(l.first)->second, log(l.second) - log(numSents));
		}


		// transitions
		for(const auto& t : transitionCount) {
			auto i = labelBijection.left.find(t.first.first)->second;
			auto j = labelBijection.left.find(t.first.second)->second;
			m.setTransition(i, j, log(t.second) - log(labelCount[t.first.first]));
		}

/*
		for (auto i = 0; i < maxLabel; ++i) {
			for (auto j = 0; j < maxLabel; ++j) {
				m.setTransition(i, j, 
					log(transitionCount[std::make_pair(
						labelBijection.right.find(i)->second,
						labelBijection.right.find(j)->second)]) -
					log(labelCount[labelBijection.right.find(i)->second]));
			}
		}
*/
		// emissions
/*
		for(auto i = 0; i < maxLabel; ++i) {
		for(auto j = 0; j < nextEmission-1; ++j) {
			m.emissions(i, j) = log(emissionCount[std::make_pair(labelBijection.right.find(i)->second, emissionBijection.right.find(j)->second)]) - log(labelCount[labelBijection.right.find(i)->second]);
		}
		}
*/
		for ( auto e = emissionCount.begin(); e != emissionCount.end(); ++e) {
			m.setEmission(labelBijection.left.find(e->first.first)->second,
						emissionBijection.left.find(e->first.second)->second,
						log(e->second) - log(labelCount[e->first.first]));
		}

		return std::move(m);
	}

private:
	bijection labelBijection;
	bijection emissionBijection;
	Viterbi_type m_viterbi;
};  // end class HMM

#endif

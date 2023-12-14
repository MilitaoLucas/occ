// Generated as part of Libecpint, Copyright 2017 Robert A Shaw
#include "qgen.hpp"
namespace libecpint {
namespace qgen {
void Q1_1_5(const ECP& U, const GaussianShell& shellA, const GaussianShell& shellB, const FiveIndex<double> &CA, const FiveIndex<double> &CB, const TwoIndex<double> &SA, const TwoIndex<double> &SB, const double Am, const double Bm, const RadialIntegral &radint, const AngularIntegral& angint, const RadialIntegral::Parameters& parameters, ThreeIndex<double> &values) {

	std::vector<Triple> radial_triples_A = {
		Triple{0, 5, 5},
		Triple{1, 4, 5},
		Triple{1, 5, 6},
		Triple{2, 4, 4},
		Triple{2, 4, 6},
		Triple{2, 6, 6}	};

	ThreeIndex<double> radials(8, 7, 7);
	radint.type2(radial_triples_A, 7, 5, U, shellA, shellB, Am, Bm, radials);

	std::vector<Triple> radial_triples_B = {
		Triple{1, 4, 5},
		Triple{1, 5, 6},
		Triple{2, 4, 6}	};

	ThreeIndex<double> radials_B(8, 7, 7);
	radint.type2(radial_triples_B, 7, 5, U, shellB, shellA, Bm, Am, radials_B);
	for (Triple& t : radial_triples_B) radials(std::get<0>(t), std::get<2>(t), std::get<1>(t)) = radials_B(std::get<0>(t), std::get<1>(t), std::get<2>(t));

	rolled_up(5, 1, 1, radials, CA, CB, SA, SB, angint, values);
}
}
}

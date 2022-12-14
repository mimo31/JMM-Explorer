#include "testing.hpp"

#include <algorithm>
#include <memory>
#include <sstream>
#include <variant>

#include "analysis.hpp"

namespace JMMExplorer
{

struct TestCase
{
	/// true iff our program is expected not only to produce a set of legals outputs, but to also produce the complete set of all legal results
	bool require_all;

	/// each entry of this vector is the source code for one thread
	vec<std::string> sources;
	
	/// all possible execution results allowed by the JMM -- should be precomputed by hand, so that we can check against this
	vec<ExecutionResult> results;
};

void run_all_tests()
{
	const vec<TestCase> tcases = {
		// 0
		TestCase{ true, { "print(42);" }, {
			{ RegularExecutionResult{ { 42 } } }
		} },
		// 1
		TestCase{ true, { "print(42);print(41+1);print(6*7);print(65%7+400/10);" }, {
			{ RegularExecutionResult{ { 42, 42, 42, 42 } } }
		} },
		// 2
		TestCase{ true, { "local = 41;local++;print(local);" }, {
			{ RegularExecutionResult{ { 42 } } }
		} },
		// 3
		TestCase{ true, { "print(shared);", "shared=42;" }, {
			{ RegularExecutionResult{ { 0 }, { } } },
			{ RegularExecutionResult{ { 42 }, { } } }
		} },
		// 4
		TestCase{ true, { "shared=42;", "print(shared);print(shared);print(shared);" }, {
			{ RegularExecutionResult{ { }, { 0, 0, 0 } } },
			{ RegularExecutionResult{ { }, { 0, 0, 42 } } },
			{ RegularExecutionResult{ { }, { 0, 42, 0 } } },
			{ RegularExecutionResult{ { }, { 0, 42, 42 } } },
			{ RegularExecutionResult{ { }, { 42, 0, 0 } } },
			{ RegularExecutionResult{ { }, { 42, 0, 42 } } },
			{ RegularExecutionResult{ { }, { 42, 42, 0 } } },
			{ RegularExecutionResult{ { }, { 42, 42, 42 } } }
		} },
		// 5
		TestCase{ true, { "vol=42;", "print(vol);print(vol);print(vol);" }, {
			{ RegularExecutionResult{ { }, { 0, 0, 0 } } },
			{ RegularExecutionResult{ { }, { 0, 0, 42 } } },
			{ RegularExecutionResult{ { }, { 0, 42, 42 } } },
			{ RegularExecutionResult{ { }, { 42, 42, 42 } } }
		} },
		// 6
		TestCase{ true, { "print(sx);sy=1;", "l2=sy;print(l2);sx=l2;" }, {
			{ RegularExecutionResult{ { 0 }, { 0 } } },
			{ RegularExecutionResult{ { 0 }, { 1 } } },
			{ RegularExecutionResult{ { 1 }, { 1 } } }
		} },
		// 7
		TestCase{ true, { "l1=sx;sy=l1;print(l1);", "l2=sy;sx=l2;print(l2);", "sz=42;", "l0=sz;sx=l0;print(l0);" }, {
			{ RegularExecutionResult{ { 0 }, { 0 }, { }, { 0 } } },
			{ RegularExecutionResult{ { 0 }, { 0 }, { }, { 42 } } },
			{ RegularExecutionResult{ { 42 }, { 0 }, { }, { 42 } } },
			{ RegularExecutionResult{ { 42 }, { 42 }, { }, { 42 } } }
		} },
		// 8
		TestCase{ true, { "l1=sx;sy=l1;print(l1);", "l2=sy;sx=l2;print(l2);" }, {
			{ RegularExecutionResult{ { 0 }, { 0 } } }
		} },
		// 9
		TestCase{ true, { "scounter++;scounter++;scounter++;print(scounter);", "scounter++;scounter++;scounter++;print(scounter);" }, {
			{ RegularExecutionResult{ { 1 }, { 1 } } },
			{ RegularExecutionResult{ { 1 }, { 2 } } },
			{ RegularExecutionResult{ { 1 }, { 3 } } },
			{ RegularExecutionResult{ { 1 }, { 4 } } },
			{ RegularExecutionResult{ { 1 }, { 5 } } },
			{ RegularExecutionResult{ { 1 }, { 6 } } },
			{ RegularExecutionResult{ { 2 }, { 1 } } },
			{ RegularExecutionResult{ { 2 }, { 2 } } },
			{ RegularExecutionResult{ { 2 }, { 3 } } },
			{ RegularExecutionResult{ { 2 }, { 4 } } },
			{ RegularExecutionResult{ { 2 }, { 5 } } },
			{ RegularExecutionResult{ { 2 }, { 6 } } },
			{ RegularExecutionResult{ { 3 }, { 1 } } },
			{ RegularExecutionResult{ { 3 }, { 2 } } },
			{ RegularExecutionResult{ { 3 }, { 3 } } },
			{ RegularExecutionResult{ { 3 }, { 4 } } },
			{ RegularExecutionResult{ { 3 }, { 5 } } },
			{ RegularExecutionResult{ { 3 }, { 6 } } },
			{ RegularExecutionResult{ { 4 }, { 1 } } },
			{ RegularExecutionResult{ { 4 }, { 2 } } },
			{ RegularExecutionResult{ { 4 }, { 3 } } },
			{ RegularExecutionResult{ { 4 }, { 4 } } },
			{ RegularExecutionResult{ { 4 }, { 5 } } },
			{ RegularExecutionResult{ { 4 }, { 6 } } },
			{ RegularExecutionResult{ { 5 }, { 1 } } },
			{ RegularExecutionResult{ { 5 }, { 2 } } },
			{ RegularExecutionResult{ { 5 }, { 3 } } },
			{ RegularExecutionResult{ { 5 }, { 4 } } },
			{ RegularExecutionResult{ { 5 }, { 5 } } },
			{ RegularExecutionResult{ { 5 }, { 6 } } },
			{ RegularExecutionResult{ { 6 }, { 1 } } },
			{ RegularExecutionResult{ { 6 }, { 2 } } },
			{ RegularExecutionResult{ { 6 }, { 3 } } },
			{ RegularExecutionResult{ { 6 }, { 4 } } },
			{ RegularExecutionResult{ { 6 }, { 5 } } },
			{ RegularExecutionResult{ { 6 }, { 6 } } }
		} },
		// 10
		TestCase{ true, { "vcounter++;vcounter++;vcounter++;print(vcounter);", "vcounter++;vcounter++;vcounter++;print(vcounter);" }, {
			{ RegularExecutionResult{ { 1 }, { 3 } } },
			{ RegularExecutionResult{ { 2 }, { 2 } } },
			{ RegularExecutionResult{ { 2 }, { 3 } } },
			{ RegularExecutionResult{ { 2 }, { 4 } } },
			{ RegularExecutionResult{ { 3 }, { 1 } } },
			{ RegularExecutionResult{ { 3 }, { 2 } } },
			{ RegularExecutionResult{ { 4 }, { 2 } } },
			{ RegularExecutionResult{ { 3 }, { 3 } } },
			{ RegularExecutionResult{ { 3 }, { 4 } } },
			{ RegularExecutionResult{ { 3 }, { 5 } } },
			{ RegularExecutionResult{ { 3 }, { 6 } } },
			{ RegularExecutionResult{ { 4 }, { 3 } } },
			{ RegularExecutionResult{ { 4 }, { 4 } } },
			{ RegularExecutionResult{ { 4 }, { 5 } } },
			{ RegularExecutionResult{ { 4 }, { 6 } } },
			{ RegularExecutionResult{ { 5 }, { 3 } } },
			{ RegularExecutionResult{ { 5 }, { 4 } } },
			{ RegularExecutionResult{ { 5 }, { 5 } } },
			{ RegularExecutionResult{ { 5 }, { 6 } } },
			{ RegularExecutionResult{ { 6 }, { 3 } } },
			{ RegularExecutionResult{ { 6 }, { 4 } } },
			{ RegularExecutionResult{ { 6 }, { 5 } } },
			{ RegularExecutionResult{ { 6 }, { 6 } } }
		} },
		// 11 
		TestCase{ true, { "monitor.lock();scounter++;monitor.unlock();monitor.lock();scounter++;monitor.unlock();monitor.lock();scounter++;monitor.unlock();print(scounter);", "monitor.lock();scounter++;monitor.unlock();monitor.lock();scounter++;monitor.unlock();monitor.lock();scounter++;monitor.unlock();print(scounter);" }, {
			{ RegularExecutionResult{ { 3 }, { 6 } } },
			{ RegularExecutionResult{ { 4 }, { 6 } } },
			{ RegularExecutionResult{ { 5 }, { 6 } } },
			{ RegularExecutionResult{ { 6 }, { 3 } } },
			{ RegularExecutionResult{ { 6 }, { 4 } } },
			{ RegularExecutionResult{ { 6 }, { 5 } } },
			{ RegularExecutionResult{ { 6 }, { 6 } } }
		} },
		// 12
		TestCase{ false, { "l1=sx;l2=l1|1;sy=l2;print(l1);print(l2);", "l3=sy;sx=l3;print(l3);" }, {
			{ RegularExecutionResult{ { 0, 1 }, { 1 } } },
			{ RegularExecutionResult{ { 0, 1 }, { 0 } } },
			{ RegularExecutionResult{ { 1, 1 }, { 1 } } }
		} },
		// 13
		TestCase{ true, { "print(1/shared);", "shared=1;" }, {
			{ RegularExecutionResult{ { 1 }, { } } },
			{ ExceptedExecutionResult{ 0, 1 } }
		} },
		// 14
		TestCase{ true, { "print(1/sx);sy=1;", "print(1/sy);sx=1;" }, {
			{ RegularExecutionResult{ { 1 }, { 1 } } },
			{ ExceptedExecutionResult{ 0, 1 } },
			{ ExceptedExecutionResult{ 1, 1 } },
		} },
		// 15
		TestCase{ true, { "m.lock();print(sx);sy=1;m.unlock();", "m.lock();print(sy);sx=1;m.unlock();" }, {
			{ RegularExecutionResult{ { 0 }, { 1 } } },
			{ RegularExecutionResult{ { 1 }, { 0 } } }
		} },
		// 16
		TestCase{ true, { "m0.lock();print(sx);sy=1;m0.unlock();", "m1.lock();print(sy);sx=1;m1.unlock();" }, {
			{ RegularExecutionResult{ { 0 }, { 0 } } },
			{ RegularExecutionResult{ { 0 }, { 1 } } },
			{ RegularExecutionResult{ { 1 }, { 0 } } },
			{ RegularExecutionResult{ { 1 }, { 1 } } }
		} },
		// 17
		TestCase{ true, { "mother.lock();mother.unlock();m.lock();m.lock();m.lock();print(sx);m.unlock();sy=1;m.unlock();m.unlock();", "moo.lock();m.lock();moo.unlock();print(sy);sx=1;moo.lock();m.unlock();moo.unlock();" }, {
			{ RegularExecutionResult{ { 0 }, { 1 } } },
			{ RegularExecutionResult{ { 1 }, { 0 } } }
		} },
		// 18
		TestCase{ true, { "print(-12*3|1);print(9^3);", "print(6/2*(1+2));" }, {
			{ RegularExecutionResult{ { -35, 10 }, { 9 } } }
		} },
		// 19
		TestCase{ true, { "print(s);", "s=1;" }, {
			{ RegularExecutionResult{ { 0 }, { } } },
			{ RegularExecutionResult{ { 1 }, { } } }
		} },
		// 20
		TestCase{ true, { "s++;s++;print(s);", "s++;s++;print(s);" }, {
			{ RegularExecutionResult{ { 1 }, { 1 } } },
			{ RegularExecutionResult{ { 1 }, { 2 } } },
			{ RegularExecutionResult{ { 1 }, { 3 } } },
			{ RegularExecutionResult{ { 1 }, { 4 } } },
			{ RegularExecutionResult{ { 2 }, { 1 } } },
			{ RegularExecutionResult{ { 2 }, { 2 } } },
			{ RegularExecutionResult{ { 2 }, { 3 } } },
			{ RegularExecutionResult{ { 2 }, { 4 } } },
			{ RegularExecutionResult{ { 3 }, { 1 } } },
			{ RegularExecutionResult{ { 3 }, { 2 } } },
			{ RegularExecutionResult{ { 3 }, { 3 } } },
			{ RegularExecutionResult{ { 3 }, { 4 } } },
			{ RegularExecutionResult{ { 4 }, { 1 } } },
			{ RegularExecutionResult{ { 4 }, { 2 } } },
			{ RegularExecutionResult{ { 4 }, { 3 } } },
			{ RegularExecutionResult{ { 4 }, { 4 } } }
		} },
		// 21
		TestCase{ true, { "v++;v++;print(v);", "v++;v++;print(v);" }, {
			{ RegularExecutionResult{ { 1 }, { 2 } } },
			{ RegularExecutionResult{ { 2 }, { 1 } } },
			{ RegularExecutionResult{ { 2 }, { 2 } } },
			{ RegularExecutionResult{ { 2 }, { 3 } } },
			{ RegularExecutionResult{ { 2 }, { 4 } } },
			{ RegularExecutionResult{ { 3 }, { 2 } } },
			{ RegularExecutionResult{ { 3 }, { 3 } } },
			{ RegularExecutionResult{ { 3 }, { 4 } } },
			{ RegularExecutionResult{ { 4 }, { 2 } } },
			{ RegularExecutionResult{ { 4 }, { 3 } } },
			{ RegularExecutionResult{ { 4 }, { 4 } } }
		} },
		// 22
		TestCase{ true, { "m.lock();s++;m.unlock();m.lock();s++;m.unlock();m.lock();print(s);m.unlock();", "m.lock();s++;m.unlock();m.lock();s++;m.unlock();m.lock();print(s);m.unlock();" }, {
			{ RegularExecutionResult{ { 2 }, { 4 } } },
			{ RegularExecutionResult{ { 3 }, { 4 } } },
			{ RegularExecutionResult{ { 4 }, { 2 } } },
			{ RegularExecutionResult{ { 4 }, { 3 } } },
			{ RegularExecutionResult{ { 4 }, { 4 } } }
		} },
		// 23
		TestCase{ true, { "l=32;l_s=s;l-=7*l_s*(l_s-5);print(l);", "l++;l*=2;l*=l;v=l|3;l*=l;print(l);", "l=3;s=5;l%=6;l*=l+l*l;print(l);" }, {
			{ RegularExecutionResult{ { 32 }, { 16 }, { 36 } } }
		} },
		// 24
		TestCase{ true, { "print(s*s);", "s=-1;s=1;" }, {
			{ RegularExecutionResult{ { -1 }, { } } },
			{ RegularExecutionResult{ { 0 }, { } } },
			{ RegularExecutionResult{ { 1 }, { } } }
		} },
		// 25
		TestCase{ true, { "print(v0/v1);", "v0+=563;v1+=7;" }, {
			{ RegularExecutionResult{ { 0 }, { } } },
			{ RegularExecutionResult{ { 80 }, { } } },
			{ ExceptedExecutionResult{ 0, 1 } }
		} }
	};

	// number of cases where our program considered the source code to be ill formed (it is never supposed to be, so this count should remain at zero if our program is correct)
	uint32_t errored_count = 0;

	// number of cases where our program didn't consider the source code to be ill formed, but it produced an incorrect set of possible results
	uint32_t wrong_answer_count = 0;

	for (uint32_t i = 0; i < tcases.size(); i++)
	{
		const TestCase& tcase = tcases[i];
		vec<std::string> filenames;
		vec<std::unique_ptr<std::stringstream>> uq_inputs;
		vec<std::istream*> inputs;
		for (uint32_t j = 0; j < tcase.sources.size(); j++)
		{
			filenames.push_back("thread " + std::to_string(j));
			uq_inputs.push_back(std::make_unique<std::stringstream>(tcase.sources[j]));
			inputs.push_back(uq_inputs.back().get());
		}
		std::cout << "[[ TEST CASE " << i << " ]]" << std::endl;
		vec<ExecutionResult> results;
		if (analyze(filenames, inputs, results, std::cerr))
		{
			errored_count++;
			continue;
		}
		bool wrong = false;
		for (const ExecutionResult& res : results)
			if (!std::any_of(tcase.results.begin(), tcase.results.end(), [&res](const ExecutionResult& other){ return res == other; }))
			{
				wrong = true;
				std::cout << "the analysis produced \"";
				res.print(std::cout, [&filenames](const uint32_t thread_i){ return filenames[thread_i]; });
				std::cout << "\", which is not a legal output" << std::endl;
			}
		if (tcase.require_all)
			for (const ExecutionResult& res : tcase.results)
				if (!std::any_of(results.begin(), results.end(), [&res](const ExecutionResult& other){ return res == other; }))
				{
					wrong = true;
					std::cout << "the analysis missed the output \"";
					res.print(std::cout, [&filenames](const uint32_t thread_i){ return filenames[thread_i]; });
					std::cout << "\"" << std::endl;
				}
		wrong_answer_count += wrong;
	}
	std::cout << "RUN " << tcases.size() << " TEST CASES\n";
	if (!errored_count && !wrong_answer_count)
		std::cout << "ALL PASSED\n";
	else
   		std::cout << errored_count << " RETURNED AN ERROR\n" << wrong_answer_count << " GAVE A WRONG ANSWER\n" << tcases.size() - errored_count - wrong_answer_count << " PASSED\n";
}

}

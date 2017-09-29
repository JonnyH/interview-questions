// Problem: Given two integers, what is the bitwise and for those and all integers between then
//

#include <cassert>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <random>
#include <strings.h>
#include <vector>

uint32_t simple_solution(uint32_t a, uint32_t b)
{
	assert(a <= b);
	uint32_t result = a;

	for (uint32_t c = a; c <= b; c++)
		result &= c;

	return result;
}

uint32_t clz(uint32_t a) { return __builtin_clz(a); }

uint32_t more_clever_solution(uint32_t a, uint32_t b)
{
	assert(a <= b);
	uint32_t result = 0;
	// It seems that what we are looking for is the contiguous set bits shared between both numbers
	// from the highest set bit of B Possible methods: bitwise-and a & b, find the first non-set bit
	// after the highest bit, zero any bits after that
	//
	int highest_bit = (sizeof(a) * 8) - clz(b);
	for (int i = highest_bit; i > 0; i--)
	{
		if (1 << (i - 1) & a & b)
			result |= 1 << (i - 1);
		else
			return result;
	}

	return result;
}

struct example_value
{
	uint32_t a;
	uint32_t b;
	uint32_t result;

	example_value(uint32_t a = 0, uint32_t b = 0, uint32_t result = 0) : a(a), b(b), result(result)
	{
	}
};

std::vector<example_value> test_values = {
    {0b0, 0b1, 0b0},
    {0b1, 0b1, 0b1},
    {0b10, 0b10, 0b10},
    {0b0100, 0b1011, 0b00},
    {0b11, 0b100, 0b00},
    {0b11, 0b11, 0b11},
    {0b11000, 0b11011, 0b11000},
    {0b01000, 0b11011, 0b00000},
    {0b10100, 0b11111, 0b10000},
};

std::vector<uint32_t> perf_test(const std::function<uint32_t(uint32_t, uint32_t)> &fn,
                                const std::vector<example_value> &values)
{
	auto count = values.size();
	std::vector<uint32_t> result(count);
	auto timeStart = std::chrono::high_resolution_clock::now();

	for (size_t i = 0; i < count; i++)
	{
		result[i] = fn(values[i].a, values[i].b);
	}

	auto timeEnd = std::chrono::high_resolution_clock::now();

	uint64_t timeNS = std::chrono::duration<uint64_t, std::nano>(timeEnd - timeStart).count();
	std::cout << "Time: " << std::dec << timeNS << "ns\n";
	return result;
}

int main(int argc, char **argv)
{
	int ret = EXIT_SUCCESS;
	for (auto &e : test_values)
	{
		auto result = simple_solution(e.a, e.b);
		if (result != e.result)
		{
			std::cerr << "Verification failed: fn(0x" << std::hex << e.a << ", 0x" << e.b
			          << ") = 0x" << result << "\nExpected: 0x" << e.result << "\n";
			ret = EXIT_FAILURE;
		}
		result = more_clever_solution(e.a, e.b);
		if (result != e.result)
		{
			std::cerr << "Verification failed: fn2(0x" << std::hex << e.a << ", 0x" << e.b
			          << ") = 0x" << result << "\nExpected: 0x" << e.result << "\n";
			ret = EXIT_FAILURE;
		}
	}

	// Performance test
	// Generate N random uint32_t pairs

	constexpr unsigned count = 20;
	std::vector<example_value> values;
	values.resize(count);

	constexpr int seed = 55;
	auto rng = std::minstd_rand{seed};
	auto dist = std::uniform_int_distribution<uint32_t>{};

	std::cout << "Generating integers\n";

	for (unsigned i = 0; i < count; i++)
	{
		auto a = dist(rng);
		auto b = dist(rng);

		auto min = std::min(a, b);
		auto max = std::max(a, b);

		auto expected = simple_solution(min, max);
		values[i] = {min, max, expected};
		auto result = more_clever_solution(min, max);
		// Some extra function validation while we're at it
		if (result != expected)
		{
			std::cerr << "Verification failed: fn2(0x" << std::hex << min << ", 0x" << max
			          << ") = 0x" << result << "\nExpected: 0x" << expected << "\n";
			ret = EXIT_FAILURE;
		}
	}

	std::cout << "Running simple perf test\n";

	auto result_simple = perf_test(simple_solution, values);
	std::cout << "Running clever perf test\n";
	auto result_clever = perf_test(more_clever_solution, values);

	return ret;
}

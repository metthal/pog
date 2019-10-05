#include <gtest/gtest.h>

#include <pog/filter_view.h>

class TestFilterView : public ::testing::Test {};

TEST_F(TestFilterView,
EmptyContainer) {
	std::vector<int> v;

	FilterView fv(v.begin(), v.end(), [](int x) { return x & 1; });

	std::vector<int> actual;
	for (auto x : fv)
		actual.push_back(x);

	EXPECT_EQ(actual, (std::vector<int>{}));
}

TEST_F(TestFilterView,
BasicFilter) {
	std::vector<int> v = {0, 1, 2, 3, 4, 5, 6};

	FilterView fv(v.begin(), v.end(), [](int x) { return x & 1; });

	std::vector<int> actual;
	for (auto x : fv)
		actual.push_back(x);

	EXPECT_EQ(actual, (std::vector<int>{1, 3, 5}));
}

TEST_F(TestFilterView,
SharedEnd) {
	std::vector<int> v = {0, 1, 2, 3, 4, 5};

	FilterView fv(v.begin(), v.end(), [](int x) { return x & 1; });

	std::vector<int> actual;
	for (auto x : fv)
		actual.push_back(x);

	EXPECT_EQ(actual, (std::vector<int>{1, 3, 5}));
}

TEST_F(TestFilterView,
SharedBegin) {
	std::vector<int> v = {1, 2, 3, 4, 5, 6};

	FilterView fv(v.begin(), v.end(), [](int x) { return x & 1; });

	std::vector<int> actual;
	for (auto x : fv)
		actual.push_back(x);

	EXPECT_EQ(actual, (std::vector<int>{1, 3, 5}));
}

TEST_F(TestFilterView,
SharedBeginAndEnd) {
	std::vector<int> v = {1, 2, 3, 4, 5};

	FilterView fv(v.begin(), v.end(), [](int x) { return x & 1; });

	std::vector<int> actual;
	for (auto x : fv)
		actual.push_back(x);

	EXPECT_EQ(actual, (std::vector<int>{1, 3, 5}));
}

TEST_F(TestFilterView,
NoElements) {
	std::vector<int> v = {0, 1, 2, 3, 4, 5, 6};

	FilterView fv(v.begin(), v.end(), [](int x) { return x > 10; });

	std::vector<int> actual;
	for (auto x : fv)
		actual.push_back(x);

	EXPECT_EQ(actual, (std::vector<int>{}));
}

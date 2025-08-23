#pragma once
#include "gtest/gtest.h"
#include "Serializer.hpp"
#include "unordered_set"

enum Test_Enum : uint8_t
{
	test_1,
	test_2
};

enum class Test_Enum_Class
{
	test_class_1,
	test_class_2
};

TEST(TestSerializer, SerializePrimitive)
{
	int value1 = 42;
	std::string serialized1 = Serializer::Serialize(value1);
	int deserialized1 = Serializer::DeSerialize<int>(serialized1);
	EXPECT_EQ(deserialized1, value1);
	double value2 = 4.0;
	std::string serialized2 = Serializer::Serialize(value2);
	double deserialized2 = Serializer::DeSerialize<double>(serialized2);
	EXPECT_EQ(deserialized2, value2);
	std::string value3 = "abc123";
	std::string serialized3 = Serializer::Serialize(value3);
	std::string deserialized3 = Serializer::DeSerialize<std::string>(serialized3);
	EXPECT_EQ(deserialized3, value3);
	std::string value4 = "";
	std::string serialized4 = Serializer::Serialize(value4);
	std::string deserialized4 = Serializer::DeSerialize<std::string>(serialized4);
	EXPECT_EQ(deserialized4, value4);
	Test_Enum value5 = test_1;
	std::string serialized5 = Serializer::Serialize(value5);
	Test_Enum deserialized5 = Serializer::DeSerialize<Test_Enum>(serialized5);
	EXPECT_EQ(deserialized5, value5);
	Test_Enum_Class value6 = Test_Enum_Class::test_class_2;
	std::string serialized6 = Serializer::Serialize(value6);
	Test_Enum_Class deserialized6 = Serializer::DeSerialize<Test_Enum_Class>(serialized6);
	EXPECT_EQ(deserialized6, value6);
	int value7 = -123;
	std::string serialized7 = Serializer::Serialize(value7);
	int deserialized7 = Serializer::DeSerialize<int>(serialized7);
	EXPECT_EQ(deserialized7, value7);
	double value8 = 0.0;
	std::string serialized8 = Serializer::Serialize(value8);
	double deserialized8 = Serializer::DeSerialize<double>(serialized8);
	EXPECT_EQ(deserialized8, value8);
}

TEST(TestSerializer, SerializeContainer)
{
	std::vector<int> values1 = {1, 2, 3, 4};
	std::string serialized1 = Serializer::Serialize(values1);
	std::vector<int> deserialized1 = Serializer::DeSerialize<std::vector<int>>(serialized1);
	EXPECT_EQ(deserialized1, values1);
	std::map<int, int> values2 = {{1, 10}, {2, 20}, {3, 30}, {4, 40}};
	std::string serialized2 = Serializer::Serialize(values2);
	std::map<int, int> deserialized2 = Serializer::DeSerialize<std::map<int, int>>(serialized2);
	EXPECT_EQ(deserialized2, values2);
	std::set<int> values3 = {1, 2, 3, 4};
	std::string serialized3 = Serializer::Serialize(values3);
	std::set<int> deserialized3 = Serializer::DeSerialize<std::set<int>>(serialized3);
	EXPECT_EQ(deserialized3, values3);
	std::unordered_map<int, std::string> values4 = {{1, "one"}, {2, "two"}, {3, "three"}};
	std::string serialized4 = Serializer::Serialize(values4);
	std::unordered_map<int, std::string> deserialized4 = Serializer::DeSerialize<std::unordered_map<int, std::string>>(serialized4);
	EXPECT_EQ(deserialized4, values4);
	std::unordered_set<int> values5 = {1, 2, 3, 4};
	std::string serialized5 = Serializer::Serialize(values5);
	std::unordered_set<int> deserialized5 = Serializer::DeSerialize<std::unordered_set<int>>(serialized5);
	EXPECT_EQ(deserialized5, values5);
	std::deque<int> values6 = {1, 2, 3, 4};
	std::string serialized6 = Serializer::Serialize(values6);
	std::deque<int> deserialized6 = Serializer::DeSerialize<std::deque<int>>(serialized6);
	EXPECT_EQ(deserialized6, values6);
	std::list<int> values7 = {1, 2, 3, 4};
	std::string serialized7 = Serializer::Serialize(values7);
	std::list<int> deserialized7 = Serializer::DeSerialize<std::list<int>>(serialized7);
	EXPECT_EQ(deserialized7, values7);
}

TEST(TestSerializer, SerializeContainerWithComplexTypes)
{
	std::vector<std::pair<int, std::string>> values = {{1, "one"}, {2, "two"}, {3, "three"}};
	std::string serialized = Serializer::Serialize(values);
	std::vector<std::pair<int, std::string>> deserialized = Serializer::DeSerialize<std::vector<std::pair<int, std::string>>>(serialized);
	EXPECT_EQ(deserialized, values);

	std::vector<std::map<int, std::string>> nested_map = {
		{{1, "a"}, {2, "b"}},
		{{3, "c"}, {4, "d"}}};
	std::string serialized_map = Serializer::Serialize(nested_map);
	std::vector<std::map<int, std::string>> deserialized_map = Serializer::DeSerialize<std::vector<std::map<int, std::string>>>(serialized_map);
	EXPECT_EQ(deserialized_map, nested_map);
}

TEST(TestSerializer, SerializeEmptyContainer)
{
	std::vector<int> empty_vector = {};
	std::string serialized = Serializer::Serialize(empty_vector);
	std::vector<int> deserialized = Serializer::DeSerialize<std::vector<int>>(serialized);
	EXPECT_EQ(deserialized, empty_vector);

	std::map<int, int> empty_map = {};
	std::string serialized_map = Serializer::Serialize(empty_map);
	std::map<int, int> deserialized_map = Serializer::DeSerialize<std::map<int, int>>(serialized_map);
	EXPECT_EQ(deserialized_map, empty_map);
}

class TestCustomSerializeClass
{
public:
	int id;
	std::string name;

	TestCustomSerializeClass() : id(0), name("") {}
	TestCustomSerializeClass(int id, const std::string &name) : id(id), name(name) {}

	std::string Serialize() const
	{
		return std::to_string(id) + "|" + name;
	}

	size_t DeSerialize(const std::string &value)
	{
		size_t sep = value.find('|');
		if (sep == std::string::npos)
		{
			throw std::invalid_argument("Invalid format: missing separator");
		}
		id = std::stoi(value.substr(0, sep));
		name = value.substr(sep + 1);
		return value.size();
	}

	auto Tie()
	{
		return std::tie(id, name);
	}

	auto Tie() const
	{
		return std::tie(id, name);
	}
};

TEST(TestSerializer, SerializeCustomSerializeClass)
{
	TestCustomSerializeClass original(1, "TestCustomSerializeClass");
	std::string serialized = Serializer::Serialize(original);
	TestCustomSerializeClass deserialized = Serializer::DeSerialize<TestCustomSerializeClass>(serialized);
	EXPECT_EQ(deserialized.id, original.id);
	EXPECT_EQ(deserialized.name, original.name);
}

class TestClassWithTie
{
public:
	int id;
	std::string name;

	TestClassWithTie() : id(0), name("") {}
	TestClassWithTie(int id, const std::string &name) : id(id), name(name) {}

	auto Tie() & { return std::tie(id, name); }
	auto Tie() const & { return std::tie(id, name); }
};

TEST(TestSerializer, SerializeClassWithTie)
{
	TestClassWithTie original(42, "TestWithTie");
	std::string serialized = Serializer::Serialize(original);
	TestClassWithTie deserialized = Serializer::DeSerialize<TestClassWithTie>(serialized);
	EXPECT_EQ(deserialized.id, original.id);
	EXPECT_EQ(deserialized.name, original.name);
}
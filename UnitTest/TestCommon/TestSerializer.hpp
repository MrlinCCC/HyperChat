#pragma once
#include "gtest/gtest.h"
#include "Serializer.hpp"
#include "unordered_set"

TEST(TestSerializer, SerializePrimitive)
{
    int value1 = 42;
    std::string serialized1 = Serializer::Serialize(value1);
    int deserialized1 = Serializer::DeSerialize<int>(serialized1);
    ASSERT_EQ(deserialized1, value1);
    double value2 = 4.0;
    std::string serialized2 = Serializer::Serialize(value2);
    double deserialized2 = Serializer::DeSerialize<double>(serialized2);
    ASSERT_EQ(deserialized2, value2);
    std::string value3 = "abc123";
    std::string serialized3 = Serializer::Serialize(value3);
    std::string deserialized3 = Serializer::DeSerialize<std::string>(serialized3);
    ASSERT_EQ(deserialized3, value3);
    int value4 = -123;
    std::string serialized4 = Serializer::Serialize(value4);
    int deserialized4 = Serializer::DeSerialize<int>(serialized4);
    ASSERT_EQ(deserialized4, value4);
    double value5 = 0.0;
    std::string serialized5 = Serializer::Serialize(value5);
    double deserialized5 = Serializer::DeSerialize<double>(serialized5);
    ASSERT_EQ(deserialized5, value5);
}

TEST(TestSerializer, SerializeContainer)
{
    std::vector<int> values1 = {1, 2, 3, 4};
    std::string serialized1 = Serializer::Serialize(values1);
    std::vector<int> deserialized1 = Serializer::DeSerialize<std::vector<int>>(serialized1);
    ASSERT_EQ(deserialized1, values1);
    std::map<int, int> values2 = {{1, 10}, {2, 20}, {3, 30}, {4, 40}};
    std::string serialized2 = Serializer::Serialize(values2);
    std::map<int, int> deserialized2 = Serializer::DeSerialize<std::map<int, int>>(serialized2);
    ASSERT_EQ(deserialized2, values2);
    std::set<int> values3 = {1, 2, 3, 4};
    std::string serialized3 = Serializer::Serialize(values3);
    std::set<int> deserialized3 = Serializer::DeSerialize<std::set<int>>(serialized3);
    ASSERT_EQ(deserialized3, values3);
    std::unordered_map<int, std::string> values4 = {{1, "one"}, {2, "two"}, {3, "three"}};
    std::string serialized4 = Serializer::Serialize(values4);
    std::unordered_map<int, std::string> deserialized4 = Serializer::DeSerialize<std::unordered_map<int, std::string>>(serialized4);
    ASSERT_EQ(deserialized4, values4);
    std::unordered_set<int> values5 = {1, 2, 3, 4};
    std::string serialized5 = Serializer::Serialize(values5);
    std::unordered_set<int> deserialized5 = Serializer::DeSerialize<std::unordered_set<int>>(serialized5);
    ASSERT_EQ(deserialized5, values5);
    std::deque<int> values6 = {1, 2, 3, 4};
    std::string serialized6 = Serializer::Serialize(values6);
    std::deque<int> deserialized6 = Serializer::DeSerialize<std::deque<int>>(serialized6);
    ASSERT_EQ(deserialized6, values6);
    std::list<int> values7 = {1, 2, 3, 4};
    std::string serialized7 = Serializer::Serialize(values7);
    std::list<int> deserialized7 = Serializer::DeSerialize<std::list<int>>(serialized7);
    ASSERT_EQ(deserialized7, values7);
}

TEST(TestSerializer, SerializeContainerWithComplexTypes)
{
    std::vector<std::pair<int, std::string>> values = {{1, "one"}, {2, "two"}, {3, "three"}};
    std::string serialized = Serializer::Serialize(values);
    std::vector<std::pair<int, std::string>> deserialized = Serializer::DeSerialize<std::vector<std::pair<int, std::string>>>(serialized);
    ASSERT_EQ(deserialized, values);

    std::vector<std::map<int, std::string>> nested_map = {
        {{1, "a"}, {2, "b"}},
        {{3, "c"}, {4, "d"}}};
    std::string serialized_map = Serializer::Serialize(nested_map);
    std::vector<std::map<int, std::string>> deserialized_map = Serializer::DeSerialize<std::vector<std::map<int, std::string>>>(serialized_map);
    ASSERT_EQ(deserialized_map, nested_map);
}

TEST(TestSerializer, SerializeEmptyContainer)
{
    std::vector<int> empty_vector = {};
    std::string serialized = Serializer::Serialize(empty_vector);
    std::vector<int> deserialized = Serializer::DeSerialize<std::vector<int>>(serialized);
    ASSERT_EQ(deserialized, empty_vector);

    std::map<int, int> empty_map = {};
    std::string serialized_map = Serializer::Serialize(empty_map);
    std::map<int, int> deserialized_map = Serializer::DeSerialize<std::map<int, int>>(serialized_map);
    ASSERT_EQ(deserialized_map, empty_map);
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
};

TEST(TestSerializer, SerializeCustomSerializeClass)
{
    TestCustomSerializeClass original(1, "TestCustomSerializeClass");
    std::string serialized = Serializer::Serialize(original);
    TestCustomSerializeClass deserialized = Serializer::DeSerialize<TestCustomSerializeClass>(serialized);
    ASSERT_EQ(deserialized.id, original.id);
    ASSERT_EQ(deserialized.name, original.name);
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
    ASSERT_EQ(deserialized.id, original.id);
    ASSERT_EQ(deserialized.name, original.name);
}
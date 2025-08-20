#pragma once
#include <iostream>

template <typename T, typename = void>
struct HasSerialize : std::false_type
{
};

template <typename T>
struct HasSerialize<T, std::void_t<decltype(std::declval<const T>().Serialize())>>
	: std::bool_constant<std::is_same_v<decltype(std::declval<const T>().Serialize()), std::string>>
{
};

template <typename T>
inline constexpr bool HasSerializeV = HasSerialize<T>::value;

template <typename T, typename = void>
struct HasDeSerialize : std::false_type
{
};

template <typename T>
struct HasDeSerialize<T, std::void_t<decltype(std::declval<T>().DeSerialize(std::declval<const std::string&>()))>>
	: std::bool_constant<std::is_same_v<decltype(std::declval<T>().DeSerialize(std::declval<const std::string&>())), size_t>>
{
};

template <typename T>
inline constexpr bool HasDeSerializeV = HasDeSerialize<T>::value;

template <typename T, typename = void>
struct IsContainer : std::false_type
{
};

template <typename T>
struct IsContainer<T,
	std::void_t<typename T::value_type, decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>>
	: std::true_type
{
};

template <typename T, typename = void>
struct IsMapContainer : std::false_type
{
};

template <typename T>
struct IsMapContainer<T, std::void_t<typename T::key_type, typename T::mapped_type>> : std::true_type
{
};

template <typename T>
inline constexpr bool IsMapContainerV = IsMapContainer<T>::value;

template <typename T>
inline constexpr bool IsContainerV = IsContainer<T>::value;

template <typename T, typename = void>
struct HasTie : std::false_type
{
};

template <typename T>
struct HasTie<T, std::void_t<decltype(std::declval<T>().Tie())>>
	: std::bool_constant<std::is_base_of_v<
	std::tuple<>,
	std::remove_cv_t<std::remove_reference_t<decltype(std::declval<T>().Tie())>>>>
{
};

template <typename T>
inline constexpr bool HasTieV = HasTie<T>::value;

class Serializer
{
public:
	template <typename T>
	static std::string Serialize(const T& value)
	{
		try
		{
			return SerializeImp(value);
		}
		catch (const std::exception& e)
		{
			throw std::runtime_error("SerializeImp failed: " + std::string(e.what()));
		}
	}

	template <typename T>
	static T DeSerialize(const std::string& value)
	{
		if (value.empty())
		{
			throw std::runtime_error("DeSerialize called with empty string.");
		}
		try
		{
			T data;
			DeSerializeImp(data, value);
			return data;
		}
		catch (const std::exception& e)
		{
			throw std::runtime_error("DeSerializeImp failed: " + std::string(e.what()));
		}
	}

private:
	template <typename T>
	static std::enable_if_t<std::is_arithmetic_v<T>, std::string>
		SerializeImp(const T& data)
	{
		return std::string(reinterpret_cast<const char*>(&data), sizeof(T));
	}

	template <typename T>
	static std::enable_if_t<std::is_enum_v<T>, std::string>
		SerializeImp(const T& data)
	{
		return SerializeImp(static_cast<std::underlying_type_t<T>>(data));
	}

	template <typename Container>
	static std::enable_if_t<IsContainerV<Container>, std::string>
		SerializeImp(const Container& container)
	{
		std::string result;
		uint64_t size = static_cast<uint64_t>(container.size());
		result.append(reinterpret_cast<const char*>(&size), sizeof(size));
		for (const auto& item : container)
		{
			result += SerializeImp(item);
		}
		return result;
	}

	template <typename T1, typename T2>
	static std::string SerializeImp(const std::pair<T1, T2>& pair)
	{
		std::string result;
		result += SerializeImp(pair.first);
		result += SerializeImp(pair.second);
		return result;
	}

	template <typename Cls>
	static std::enable_if_t<HasSerializeV<Cls>, std::string>
		SerializeImp(const Cls& obj)
	{
		return obj.Serialize();
	}

	template <typename Cls>
	static std::enable_if_t<!HasSerializeV<Cls>&& HasTieV<Cls>, std::string>
		SerializeImp(const Cls& obj)
	{
		std::string result;
		std::apply([&](auto &...members)
			{ (void)std::initializer_list<int>{([&]
				{ result += SerializeImp(members); }(), 0)...}; }, obj.Tie());
		return result;
	}

	template <typename T>
	static std::enable_if_t<std::is_arithmetic_v<T>, size_t>
		DeSerializeImp(T& data, const std::string& value)
	{
		if (value.size() >= sizeof(T))
		{
			std::memcpy(&data, value.data(), sizeof(T));
			return sizeof(T);
		}
		else
		{
			throw std::runtime_error("String value is too small to hold the data.");
		}
	}

	template <typename T>
	static std::enable_if_t<std::is_enum_v<T>, size_t>
		DeSerializeImp(T& data, const std::string& value)
	{
		std::underlying_type_t<T> underlying;
		size_t offset = DeSerializeImp(underlying, value);
		data = static_cast<T>(underlying);
		return offset;
	}

	template <typename Container>
	static std::enable_if_t<IsContainerV<Container>, size_t>
		DeSerializeImp(Container& data, const std::string& value)
	{
		using ValueType = typename Container::value_type;
		uint64_t size64;
		memcpy(&size64, value.data(), sizeof(uint64_t));
		if (size64 > SIZE_MAX)
		{
			throw std::overflow_error("Size too large to fit into size_t");
		}
		size_t size = static_cast<size_t>(size64);
		data.clear();

		size_t offset = sizeof(uint64_t);
		if constexpr (IsMapContainerV<Container>)
		{
			for (size_t i = 0; i < size; ++i)
			{
				typename Container::key_type key;
				typename Container::mapped_type val;
				offset += DeSerializeImp(key, value.substr(offset));
				offset += DeSerializeImp(val, value.substr(offset));
				data.insert(std::make_pair(std::move(key), std::move(val)));
			}
		}
		else
		{
			for (size_t i = 0; i < size; ++i)
			{
				ValueType element;
				offset += DeSerializeImp(element, value.substr(offset));
				InsertElement(data, std::move(element));
			}
		}
		return offset;
	}

	template <typename T1, typename T2>
	static size_t DeSerializeImp(std::pair<T1, T2>& pair, const std::string& value)
	{
		size_t offset = 0;
		offset += DeSerializeImp(pair.first, value.substr(offset));
		offset += DeSerializeImp(pair.second, value.substr(offset));
		return offset;
	}

	template <typename Cls>
	static std::enable_if_t<HasDeSerializeV<Cls>, size_t>
		DeSerializeImp(Cls& obj, const std::string& value)
	{
		return obj.DeSerialize(value);
	}

	template <typename Cls>
	static std::enable_if_t<!HasDeSerializeV<Cls>&& HasTieV<Cls>, size_t>
		DeSerializeImp(Cls& obj, const std::string& value)
	{
		size_t offset = 0;
		std::apply([&](auto &...members)
			{ (void)std::initializer_list<int>{([&]()
				{ offset += DeSerializeImp(members, value.substr(offset)); }(), 0)...}; }, obj.Tie());
		return offset;
	}

	template <typename Container, typename T>
	static auto InsertElement(Container& c, T&& val) -> decltype(c.push_back(std::forward<T>(val)), void())
	{
		c.push_back(std::forward<T>(val));
	}

	template <typename Container, typename T>
	static auto InsertElement(Container& c, T&& val) -> decltype(c.insert(std::forward<T>(val)), void())
	{
		c.insert(std::forward<T>(val));
	}
};
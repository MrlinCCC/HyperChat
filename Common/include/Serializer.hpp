#pragma once
#include <iostream>

template <typename T, typename = void>
struct HasSerialize : std::false_type
{
};

template <typename T>
struct HasSerialize<T, std::void_t<decltype(std::declval<const T>().Serialize())>>
	: std::bool_constant<std::is_same_v<decltype(std::declval<const T>().Serialize()), std::vector<char>>>
{
};

template <typename T>
inline constexpr bool HasSerializeV = HasSerialize<T>::value;

template <typename T, typename = void>
struct HasDeSerialize : std::false_type
{
};

template <typename T>
struct HasDeSerialize<T, std::void_t<decltype(std::declval<T>().DeSerialize(std::declval<const std::vector<char> &>(), std::declval<size_t &>()))>>
	: std::bool_constant<std::is_same_v<decltype(std::declval<T>().DeSerialize(std::declval<const std::vector<char> &>(), std::declval<size_t &>())), void>>
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

template <typename Container>
static auto TryReserve(Container &c, size_t size, int)
	-> decltype(c.reserve(size), void())
{
	c.reserve(size);
}

template <typename Container>
static void TryReserve(Container &, size_t, ...)
{
	// do nothing
}

// Calculate size
template <typename T>
struct IsFixedSize
	: std::bool_constant<
		  (std::is_arithmetic_v<T> || std::is_enum_v<T>)>
{
};

template <typename T>
std::enable_if_t<std::is_arithmetic_v<T>, size_t>
SerializedSize(const T &)
{
	return sizeof(T);
}

template <typename T>
std::enable_if_t<std::is_enum_v<T>, size_t>
SerializedSize(const T &value)
{
	return SerializedSize(static_cast<std::underlying_type_t<T>>(value));
}

template <typename Container>
std::enable_if_t<IsContainerV<Container>, size_t>
SerializedSize(const Container &container)
{
	size_t size = sizeof(uint64_t);

	if constexpr (IsFixedSize<typename Container::value_type>::value)
	{
		if (container.size() > 0)
			size += container.size() * SerializedSize(*container.begin());
	}
	else
	{
		for (auto &item : container)
			size += SerializedSize(item);
	}
	return size;
}

template <typename T1, typename T2>
size_t SerializedSize(const std::pair<T1, T2> &pair)
{
	return SerializedSize(pair.first) + SerializedSize(pair.second);
}

template <typename Cls>
std::enable_if_t<!HasSerializeV<Cls> && HasTieV<Cls>, size_t>
SerializedSize(const Cls &obj)
{
	size_t size = 0;
	std::apply([&](auto &...members)
			   { (void)std::initializer_list<int>{(size += SerializedSize(members), 0)...}; }, obj.Tie());
	return size;
}

template <typename Cls>
std::enable_if_t<HasSerializeV<Cls>, size_t>
SerializedSize(const Cls &obj)
{
	// estimate size not exact
	return sizeof(obj);
}

// Serializer
class Serializer
{
public:
	template <typename T>
	static std::string Serialize(const T &value)
	{
		try
		{
			std::vector<char> buffer;
			size_t initSize = SerializedSize(value);
			buffer.reserve(initSize);
			SerializeImp(value, buffer);
			return std::string(buffer.begin(), buffer.end());
		}
		catch (const std::exception &e)
		{
			throw std::runtime_error("SerializeImp failed: " + std::string(e.what()));
		}
	}

	template <typename T>
	static T DeSerialize(const std::string &value)
	{
		if (value.empty())
		{
			throw std::runtime_error("DeSerialize called with empty string.");
		}
		try
		{
			std::vector<char> buffer(value.begin(), value.end());
			T data;
			size_t size = 0;
			DeSerializeImp(data, buffer, size);
			return data;
		}
		catch (const std::exception &e)
		{
			throw std::runtime_error("DeSerializeImp failed: " + std::string(e.what()));
		}
	}

private:
	template <typename T>
	static std::enable_if_t<std::is_arithmetic_v<T>, void>
	SerializeImp(const T &data, std::vector<char> &out)
	{
		const char *ptr = reinterpret_cast<const char *>(&data);
		out.insert(out.end(), ptr, ptr + sizeof(T));
	}

	template <typename T>
	static std::enable_if_t<std::is_enum_v<T>, void>
	SerializeImp(const T &data, std::vector<char> &out)
	{
		SerializeImp(static_cast<std::underlying_type_t<T>>(data), out);
	}

	template <typename Container>
	static std::enable_if_t<IsContainerV<Container>, void>
	SerializeImp(const Container &container, std::vector<char> &out)
	{
		uint64_t size = static_cast<uint64_t>(container.size());
		SerializeImp(size, out);
		for (const auto &item : container)
		{
			SerializeImp(item, out);
		}
	}

	template <typename T1, typename T2>
	static void SerializeImp(const std::pair<T1, T2> &pair, std::vector<char> &out)
	{
		SerializeImp(pair.first, out);
		SerializeImp(pair.second, out);
	}

	template <typename Cls>
	static std::enable_if_t<HasSerializeV<Cls>, void>
	SerializeImp(const Cls &obj, std::vector<char> &out)
	{
		auto tmp = obj.Serialize();
		out.insert(out.end(), tmp.begin(), tmp.end());
	}

	template <typename Cls>
	static std::enable_if_t<!HasSerializeV<Cls> && HasTieV<Cls>, void>
	SerializeImp(const Cls &obj, std::vector<char> &out)
	{
		std::apply([&](auto &...members)
				   { (void)std::initializer_list<int>{([&]
													   { SerializeImp(members, out); }(), 0)...}; }, obj.Tie());
	}

	// DeSerialize
	template <typename T>
	static std::enable_if_t<std::is_arithmetic_v<T>, void>
	DeSerializeImp(T &data, const std::vector<char> &buffer, size_t &offset)
	{
		if (offset + sizeof(T) > buffer.size())
		{
			throw std::runtime_error("Buffer too small to deserialize arithmetic type.");
		}
		std::memcpy(&data, buffer.data() + offset, sizeof(T));
		offset += sizeof(T);
	}

	template <typename T>
	static std::enable_if_t<std::is_enum_v<T>, void>
	DeSerializeImp(T &data, const std::vector<char> &buffer, size_t &offset)
	{
		std::underlying_type_t<T> underlying;
		DeSerializeImp(underlying, buffer, offset);
		data = static_cast<T>(underlying);
	}

	template <typename Container>
	static std::enable_if_t<IsContainerV<Container>, void>
	DeSerializeImp(Container &data, const std::vector<char> &buffer, size_t &offset)
	{
		using ValueType = typename Container::value_type;
		uint64_t size64;
		DeSerializeImp(size64, buffer, offset);
		if (size64 > SIZE_MAX)
		{
			throw std::overflow_error("Size too large to fit into size_t");
		}
		size_t size = static_cast<size_t>(size64);
		data.clear();
		TryReserve(data, size);
		if constexpr (IsMapContainerV<Container>)
		{
			for (size_t i = 0; i < size; ++i)
			{
				typename Container::key_type key;
				typename Container::mapped_type val;
				DeSerializeImp(key, buffer, offset);
				DeSerializeImp(val, buffer, offset);
				data.insert(std::make_pair(std::move(key), std::move(val)));
			}
		}
		else
		{
			for (size_t i = 0; i < size; ++i)
			{
				ValueType element;
				DeSerializeImp(element, buffer, offset);
				InsertElement(data, std::move(element));
			}
		}
	}

	template <typename T1, typename T2>
	static void DeSerializeImp(std::pair<T1, T2> &pair, const std::vector<char> &buffer, size_t &offset)
	{
		DeSerializeImp(pair.first, buffer, offset);
		DeSerializeImp(pair.second, buffer, offset);
	}

	template <typename Cls>
	static std::enable_if_t<HasDeSerializeV<Cls>, void>
	DeSerializeImp(Cls &obj, const std::vector<char> &buffer, size_t &offset)
	{
		obj.DeSerialize(buffer, offset);
	}

	template <typename Cls>
	static std::enable_if_t<!HasDeSerializeV<Cls> && HasTieV<Cls>, void>
	DeSerializeImp(Cls &obj, const std::vector<char> &buffer, size_t &offset)
	{
		std::apply([&](auto &...members)
				   { (void)std::initializer_list<int>{([&]()
													   { DeSerializeImp(members, buffer, offset); }(), 0)...}; }, obj.Tie());
	}

	template <typename Container, typename T>
	static auto InsertElement(Container &c, T &&val) -> decltype(c.push_back(std::forward<T>(val)), void())
	{
		c.push_back(std::forward<T>(val));
	}

	template <typename Container, typename T>
	static auto InsertElement(Container &c, T &&val) -> decltype(c.insert(std::forward<T>(val)), void())
	{
		c.insert(std::forward<T>(val));
	}
};
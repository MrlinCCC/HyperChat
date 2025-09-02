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

// Variant
template <typename T>
inline std::enable_if_t<std::is_unsigned_v<T>, void> SerializeVariant(T data, std::vector<char> &out)
{
	while (data >= 0x80) // 128
	{
		out.push_back(static_cast<char>((data & 0x7F) | 0x80));
		data >>= 7;
	}
	out.push_back(static_cast<char>(data));
}

template <typename T>
inline std::enable_if_t<std::is_unsigned_v<T>, void> DeserializeVariant(T &data, const std::vector<char> &buffer, size_t &offset)
{
	data = 0;
	int shift = 0;
	while (buffer.size() >= offset + 1)
	{
		uint8_t byte = buffer[offset++];
		data |= (byte & 0x7F) << shift;
		if (!(byte & 0x80))
			return;
		shift += 7;
	}
	throw std::runtime_error("Unexpected end of buffer while decoding Varint");
}

template <typename T>
std::make_unsigned_t<T> EncodeZigZag(T n)
{
	using U = std::make_unsigned_t<T>;
	return static_cast<U>((n << 1) ^ (n >> (sizeof(T) * 8 - 1)));
}

template <typename U>
auto DecodeZigZag(U n) -> std::make_signed_t<U>
{
	using S = std::make_signed_t<U>;
	return static_cast<S>((n >> 1) ^ -static_cast<S>(n & 1));
}

template <typename T>
struct IsInteger : std::bool_constant<
					   std::is_integral_v<T> &&
					   !std::is_same_v<T, bool> &&
					   !std::is_same_v<T, char> &&
					   !std::is_same_v<T, signed char> &&
					   !std::is_same_v<T, unsigned char>>
{
};

template <typename T>
inline constexpr bool IsIntegerV = IsInteger<T>::value;

template <typename T>
struct CanUseVariant : std::bool_constant<
						   IsIntegerV<T> && std::is_unsigned_v<T>>
{
};

template <typename T>
inline constexpr bool CanUseVariantV = CanUseVariant<T>::value;

template <typename T>
struct CanUseZigZag : std::bool_constant<
						  IsIntegerV<T> && std::is_signed_v<T>>
{
};

template <typename T>
inline constexpr bool CanUseZigZagV = CanUseZigZag<T>::value;

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
		if constexpr (CanUseVariantV<T>)
		{
			SerializeVariant(data, out);
		}
		else if constexpr (CanUseZigZagV<T>)
		{
			SerializeVariant(EncodeZigZag(data), out);
		}
		else
		{
			const char *ptr = reinterpret_cast<const char *>(&data);
			out.insert(out.end(), ptr, ptr + sizeof(T));
		}
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
		SerializeVariant(container.size(), out);
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
		if constexpr (CanUseVariantV<T>)
		{
			DeserializeVariant(data, buffer, offset);
		}
		else if constexpr (CanUseZigZagV<T>)
		{
			std::make_unsigned_t<T> udata;
			DeserializeVariant(udata, buffer, offset);
			data = DecodeZigZag(udata);
		}
		else
		{
			if (offset + sizeof(T) > buffer.size())
			{
				throw std::runtime_error("Buffer too small to deserialize arithmetic type.");
			}
			std::memcpy(&data, buffer.data() + offset, sizeof(T));
			offset += sizeof(T);
		}
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
		size_t size;
		DeserializeVariant(size, buffer, offset);
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
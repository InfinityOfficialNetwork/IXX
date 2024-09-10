#pragma once

#include <concepts>
#include <tuple>

struct TypePack
{
	template <typename... T>
	struct Pack
	{

	};

	template <typename Pack>
	struct Unpack {};

	template <typename...T>
	struct Unpack<Pack<T...>>
	{
		template <template <typename...> typename Template>
		using Apply = Template<T...>;
	};
};

template <typename T>
concept TypePackType = requires
{
	typename TypePack::Unpack<T>;  // Ensures the type can be unpacked, i.e., it's a Pack
};


template <typename T>
struct DataPack
{
	template <T...Data>
	struct Pack
	{
		static constexpr uint64_t CountOf () { return sizeof...(Data); }
		static constexpr std::array<T, CountOf ()> ArrayOf () { return { Data... }; };
	};

	template <typename Pack>
	struct Unpack {};

	template <T...Data>
	struct Unpack<Pack<Data...>>
	{
		template <template <T...> typename Template>
		using Apply = Template<Data...>;
	};
};

template <typename D, typename T>
concept DataPackType = requires
{
	typename DataPack<D>::template Unpack<T>;
};
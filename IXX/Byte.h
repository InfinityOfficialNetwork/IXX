#pragma once

#include <cstdint>
#include <concepts>

template <uint64_t...>
struct Byte { static_assert(false); };

template <>
struct alignas(1) Byte<> {};

template <uint64_t Size>
	requires ([] (uint64_t _Size) consteval
			  {
				  int count = 0;
				  while (_Size)
				  {
					  _Size &= (_Size - 1);
					  ++count;
				  }
				  return count == 1;
			  }(Size))
	struct alignas(Size) Byte<Size> final : Byte<>  {};

template <uint64_t Size, uint64_t Alignment>
	requires ([] (uint64_t _Size) consteval
			  {
				  int count = 0;
				  while (_Size)
				  {
					  _Size &= (_Size - 1);
					  ++count;
				  }
				  return count == 1;
			  }(Alignment) && (Size% Alignment == 0))
	struct alignas(Alignment) Byte<Size, Alignment> final : Byte<>  { char Memory[Size]; };

template <typename T>
concept ByteType = std::derived_from<T, Byte<>>;

namespace Assertions
{
	template <uint64_t...>
	struct Assert
	{

	};

	template <uint64_t Size, uint64_t Align>
		requires (Align < 8192 && Size < 1024 * 8)
	struct Assert<Size, Align>
	{
		static_assert(sizeof (Byte<Size, Align>) == Size, "SIZE FAILURE");
		static_assert(alignof (Byte<Size, Align>) == Align, "ALIGNMENT FAILURE");

		Assert<Size * 2, Align> Test1;
		Assert<Size * 3, Align> Test2;
		Assert<Size * 2, Align * 2> Test3;
		Assert<Size * 12, Align * 4> Test5;
	};

	inline Assert<1, 1> a;
}

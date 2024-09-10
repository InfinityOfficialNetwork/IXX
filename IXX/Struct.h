#pragma once

#include <string>
#include <array>

#include "TypePack.h"
#include "Byte.h"

template <DataPackType<char> DP, TypePackType...>
struct Struct
{
	static_assert(false);
};

template <DataPackType<char> DP>
struct Struct<DP>
{

};

template<typename T, DataPackType<char> DP>
	requires(std::derived_from<T, Byte<>> || std::derived_from<T, Struct<DP>>)
struct Member
{
	using NameType = DP;

	template <char... C>
	struct NameOfHelper
	{
		static constexpr const std::array<char, sizeof...(C) + 1> Invoke ()
		{
			return { C..., '\0' }; // Create a constexpr array with the characters and a null terminator
		}

		static constexpr const uint64_t SizeOf ()
		{
			return sizeof...(C);
		}
	};

	static constexpr const std::array<char, DP::CountOf () + 1> NameOf ()
	{
		return DataPack<char>::Unpack<DP>::template Apply<NameOfHelper>::Invoke ();
	}

	static constexpr const std::string Name ()
	{
		auto var = NameOf ();
		std::string Ret = std::string (var.data ());
		return Ret;
	}

	T Data;
};

template <DataPackType<char> DP, TypePackType TP>
struct Struct<DP, TP> : Struct<DP>
{
private:
	template <typename...T>
	struct AlignOfHelper
	{
		static constexpr uint64_t Invoke ()
		{
			// Helper lambda to compute maximum alignment
			auto max_helper = [] (uint64_t a, uint64_t b) constexpr
				{
					return (a > b) ? a : b;
				};

			// Initial alignment to be compared
			uint64_t alignments[] = { alignof(T)... };

			// Compute max alignment
			uint64_t max_align = alignments[0];
			for (uint64_t i = 1; i < sizeof (alignments) / sizeof (alignments[0]); ++i)
			{
				max_align = max_helper (max_align, alignments[i]);
			}
			return max_align;
		}
	};

	template <typename...T>
	struct SizeOfHelper
	{
		static constexpr uint64_t Invoke ()
		{
			auto align_up = [] (uint64_t offset, uint64_t alignment) constexpr
				{
					return (offset + alignment - 1) & ~(alignment - 1);
				};

			uint64_t total_size = 0;
			uint64_t current_offset = 0;
			uint64_t max_align = AlignOfHelper<T...> ::Invoke ();

			// Lambda to process each type
			auto process_type = [&] (auto type) constexpr
				{
					uint64_t type_size = sizeof (type);
					uint64_t type_alignment = alignof(decltype(type));
					// Align the current offset to the type's alignment
					current_offset = align_up (current_offset, type_alignment);
					// Update total size with the new type's size
					total_size = current_offset + type_size;
					// Update the offset to the end of the newly added type
					current_offset = total_size;
				};

			// Apply the lambda to each type in the parameter pack
			(process_type (T {}), ...);

			if (total_size % max_align != 0)
				total_size += max_align - (total_size % max_align);

			return total_size;
		}
	};

	template <typename...T>
	struct OffsetOfHelper
	{
		template <uint64_t Idx>
		static constexpr uint64_t Invoke ()
		{
			static_assert(Idx < (sizeof...(T) - 1), "INDEX OUT OF BOUNDS");
			// Helper lambda to align offset to the required alignment
			auto align_up = [] (uint64_t offset, uint64_t alignment) constexpr
				{
					return (offset + alignment - 1) & ~(alignment - 1);
				};

			uint64_t current_offset = 0;
			uint64_t idx = 0;

			// Helper lambda to process types up to the specified index
			auto process_type = [&] (auto type) constexpr
				{
					if (idx < Idx)
					{
						uint64_t type_size = sizeof (type);
						uint64_t type_alignment = alignof(decltype(type));
						// Align the current offset to the type's alignment
						current_offset = align_up (current_offset, type_alignment);
						// Update offset for the next type
						current_offset += type_size;
						idx++;
					}
					else return;
				};

			// Apply the lambda to each type in the parameter pack
			(process_type (T {}), ...);

			// Return the offset of the type at the specified index
			uint64_t type_size = sizeof (std::tuple_element_t<Idx, std::tuple<T...>>);
			uint64_t type_alignment = alignof(std::tuple_element_t<Idx, std::tuple<T...>>);
			return align_up (current_offset, type_alignment);
		}
	};

	template <TypePackType...DPs>
	struct IndexOfHelper
	{
		template <DataPackType<char> Name>
		static constexpr uint64_t Invoke ()
		{
			// Get the array representation of the `Name` DataPack
			constexpr auto nameArray = Name::ArrayOf ();
			uint64_t Idx = 0;
			bool Found = false;

			// Helper lambda to compare the `Name` array with each `DP` array
			auto compare_packs = [&] (auto dp) constexpr
				{
					// Get the array representation of the current `DP`
					// Compare the two arrays

					if (nameArray == dp)
						Found = true;

					if (!Found) Idx++;

					return nameArray == dp;
				};

			// Use a fold expression to check if any of the `DPs` matches `Name`
			((compare_packs (DPs::NameType::ArrayOf ()) || ...));

			if (Found)
				return Idx;
			else
				return std::numeric_limits<uint64_t>::max();
		}

	};

public:
	static constexpr uint64_t SizeOf ()
	{
		return TypePack::Unpack<TP>::template Apply<SizeOfHelper>::Invoke ();
	}

	static constexpr uint64_t AlignOf ()
	{
		return TypePack::Unpack<TP>::template Apply<AlignOfHelper>::Invoke ();
	}

	template <uint64_t Idx>
	static constexpr uint64_t OffsetOf ()
	{
		return TypePack::Unpack<TP>::template Apply<OffsetOfHelper>::template Invoke<Idx> ();
	}

	template <DataPackType<char> DP2>
	static constexpr uint64_t IndexOf ()
	{
		return TypePack::Unpack<TP>::template Apply<IndexOfHelper>::template Invoke<DP2> ();

		struct Assert
		{
			static_assert(TypePack::Unpack<TP>::template Apply<IndexOfHelper>::template Invoke<DP2> () < std::numeric_limits<uint64_t>::max (), "MEMBER NAME NOT FOUND");
		};
	}

private:
	alignas(AlignOf ()) char Memory[SizeOf ()];
};

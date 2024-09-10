// IXX.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <typeinfo>
#include <vector>
#include <type_traits>
#include <concepts>

#include "Struct.h"

#pragma warning(push)
#pragma warning(disable : 5040)

#ifdef NO_DEF

#define sc static_cast
#define dc dynamic_cast
#define rc reinterpret_cast
#define cc const_cast
#define c const
#define v volatile
#define cv const volatile
#define cexpr constexpr
#define ceval consteval
#define inl inline
#define tmpl template
#define req requires
#define con concept
#define tnm typename
#define var auto
#define st struct
#define cl class
#define us using

cexpr int countOnes (uint64_t n)
{
	int count = 0;
	while (n)
	{
		n &= (n - 1); // Drop the lowest set bit
		++count;
	}
	return count;
}
tmpl <uint64_t... Bytes>
st Byte { static_assert(false); };

tmpl <>
st alignas(1) Byte<> {};

tmpl <uint64_t Bytes>
req (countOnes (Bytes) == 1)
st alignas(Bytes) Byte<Bytes> final : Byte<>
{};

tmpl <typename T>
con ByteType = std::derived_from<T, Byte<>>;

tmpl <tnm Tuple>
struct TupleToParameterPack;

tmpl <typename... Args>
struct TupleToParameterPack<std::tuple<Args...>>
{
	tmpl <tmpl <typename...> class Classtmpl>
		using Apply = Classtmpl<Args...>;
};


tmpl <typename... T>
ceval auto Max (T... t)
{
	// Helper function to compute maximum
	auto max_helper = [] (auto a, auto b) constexpr
		{
			return (a > b) ? a : b;
		};

	// Fold expression to compute max of all arguments
	return (std::max ({ t... }));
}

tmpl <typename... T>
ceval uint64_t AlignOf ()
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

tmpl <typename... T>
ceval uint64_t SizeOf ()
{
	// Helper lambda to align offset to the required alignment
	auto align_up = [] (uint64_t offset, uint64_t alignment) constexpr
		{
			return (offset + alignment - 1) & ~(alignment - 1);
		};

	uint64_t total_size = 0;
	uint64_t current_offset = 0;
	uint64_t max_align = AlignOf<T...> ();

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

tmpl <uint64_t Index, typename... T>
	requires (Index <= sizeof...(T))
cexpr uint64_t OffsetOf ()
{
	static_assert(Index < (sizeof...(T) - 1), "INDEX OUT OF BOUNDS");
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
			if (idx < Index)
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
	uint64_t type_size = sizeof (std::tuple_element_t<Index, std::tuple<T...>>);
	uint64_t type_alignment = alignof(std::tuple_element_t<Index, std::tuple<T...>>);
	return align_up (current_offset, type_alignment);
}


tmpl <uint64_t Index, typename T, typename... Ts>
struct TypeAtIndex
{
	using Type = typename TypeAtIndex<Index - 1, Ts...>::Type;
};

tmpl <typename T, typename... Ts>
struct TypeAtIndex<0, T, Ts...>
{
	using Type = T;
};



struct VirtualTable
{
	void** Entries;
};

struct DataTable
{
	void* Data;
};

struct Class
{
	DataTable const* DataPtr;
	VirtualTable const* VTablePtr;
};

tmpl <uint64_t _Size>
struct SizeDefinedType : Class
{
	static const uint64_t Size = _Size;
};

tmpl <typename...Members>
struct MemberDefinedType : SizeDefinedType <SizeOf<Members...> ()>
{
	using Types = std::tuple<Members...>;
	static const uint64_t Count = sizeof... (Members);
	static const uint64_t Alignment = AlignOf<Members...> ();

	tmpl <std::uint64_t Index>
		cexpr inl typename TypeAtIndex<Index, Members...>::Type& GetDataMember ()
	{
		return *cc< TypeAtIndex<Index, Members...>::Type* > (
			rc<const typename TypeAtIndex<Index, Members...>::Type*> (
				&(
					(rc<const char*> (Class::DataPtr))[OffsetOf<Index, Members...> ()]
					)
			)
		);
	}
};

struct FunctionDefiner
{
	tmpl <typename _Return>
		struct Return
	{
		tmpl <typename... _Args>
			struct Args
		{
			tmpl <typename..._Throws>
				struct Throws
			{
				virtual _Return Invoke (Args...) = 0;
				using ReturnType = _Return;
				using ArgTypes = std::tuple<_Args...>;
				using ThrowTypes = std::tuple<_Throws...>;
			};
		};
	};
};

tmpl <typename Return, typename ArgsTuple, typename ThrowsTuple>
struct FunctionPacker
{
	using ReturnType = typename FunctionDefiner::Return<Return>;
	using ArgsType = typename TupleToParameterPack<ArgsTuple>::tmpl Apply<ReturnType::tmpl Args>;
	using ThrowsType = typename TupleToParameterPack<ArgsTuple>::tmpl Apply<ArgsType::tmpl Throws>;

	using FunctionType = ThrowsType;
};

tmpl <typename Return, typename ArgsTuple, typename ThrowsTuple>
using Function = FunctionPacker<Return, ArgsTuple, ThrowsTuple>::FunctionType;

struct MemberFunctionDefiner
{
	tmpl <typename _Return>
		struct Return
	{
		tmpl <typename... _Args>
			struct Args
		{
			tmpl <typename..._Throws>
				struct Throws : FunctionDefiner::Return<_Return>::Args<Class&, _Args...>::Throws<_Throws...>
			{};
		};
	};
};

tmpl <typename Return, typename ArgsTuple, typename ThrowsTuple>
struct MemberFunctionPacker
{
	using ReturnType = typename MemberFunctionDefiner::Return<Return>;
	using ArgsType = typename TupleToParameterPack<ArgsTuple>::tmpl Apply<ReturnType::tmpl Args>;
	using ThrowsType = typename TupleToParameterPack<ArgsTuple>::tmpl Apply<ArgsType::tmpl Throws>;

	using FunctionType = ThrowsType;
};

tmpl <typename Return, typename ArgsTuple, typename ThrowsTuple>
using MemberFunction = MemberFunctionPacker<Return, ArgsTuple, ThrowsTuple>::FunctionType;



class TestFunc : MemberFunction<int, std::tuple<int, bool>, std::tuple<Class>>
{

};


//tmpl <tmpl<typename...> typename Members, typename...T>
//struct FunctionDefinedClass : MemberDefinedType<Members>
//{
//
//};



//tmpl <typename... Args>
//struct MemberFunctionDefiner
//{
//
//	virtual Return Invoke (Class& This, Args<>::Pack...) override = 0;
//};

//tmpl <tmpl <typename...> typename Members, tmpl <typename Return, typename...Args> typename Methods>
//struct MethodDefinedType : MemberDefinedType<Members>
//{
//
//};

tmpl <const uint64_t Size>
struct Stack
{
	char* StackPointer;
	char Data[Size];

	Stack ()
		: Data (), StackPointer (Data + Size)
	{}
};

static Stack<1024> MainStack;

struct State
{

};

struct While
{
	virtual bool InvokeCondition (State&) = 0;
	virtual void InvokeBody (State&) = 0;
	void Invoke (State& s)
	{
		while (InvokeCondition (s))
			InvokeBody (s);
	}
};

struct DoWhile
{
	virtual bool InvokeCondition (State&) = 0;
	virtual void InvokeBody (State&) = 0;
	void Invoke (State& s)
	{
		do
			InvokeBody (s);
		while (InvokeCondition (s));
	}
};

struct For
{
	virtual bool InvokeCondition (State&) = 0;
	virtual void InvokeInitialization (State&) = 0;
	virtual void InvokeUpdate (State&) = 0;
	virtual void InvokeBody (State&) = 0;
	void Invoke (State& s)
	{
		for (InvokeInitialization (s); InvokeCondition (s); InvokeUpdate (s))
			InvokeBody (s);
	}
};

struct Exception : Class
{

};
struct Statement;
struct Expression;

struct ExceptionHandlerResult
{
	bool Continue;
};




//struct ExceptionHandler : FunctionDefiner<ExceptionHandlerResult, const Class&, State&>
//{
//	virtual ExceptionHandlerResult Invoke (const Class& e, State& s) override = 0;
//	virtual bool IsCatchable (const Class& e) = 0;
//};

struct Try
{
	void InvokeBody (State&);
	void InvokeExceptionHandler (State&);

};

#endif

using TestStruct = Struct
<
	DataPack<char>::Pack<'t', 'e', 's', 't'>,
	TypePack::Pack
	<
	Member<Byte<4>, DataPack<char>::Pack<'a', 'b'>>,
	Member<Byte<4>, DataPack<char>::Pack<'d', 'e'>>,
	Member<Byte<4>, DataPack<char>::Pack<'f', 'g'>>
	>
>;


int main ()
{
	//MemberDefinedType<int, int, double, double, void*, bool> TestType;

	//TestType.DataPtr = (DataTable*)alloca (TestType.Size);


	//TestType.GetDataMember<0> () = 123;

	TestStruct Str {};

	auto a = TestStruct::template IndexOf < DataPack<char>::Pack<'d', 'e'> >();
	auto b = TestStruct::template OffsetOf< TestStruct::template IndexOf < DataPack<char>::Pack<'d', 'e'> > ()> ();



	return 0;
}
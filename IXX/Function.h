#pragma once

#include "TypePack.h"
#include "Struct.h"

template <typename Return, TypePack Args, TypePack Throws>
class Function
{
	struct InvokeHelper
	{
		template <typename _Return>
		struct Return
		{
			template <typename..._Args>
			struct Args
			{
				template <typename..._Throws>
				struct Throws
				{
					stat
						ic _Return Invoke (_Args...args) noexcept(!sizeof...(_Throws))
					{

					}
				};
			};
		};
	};
};
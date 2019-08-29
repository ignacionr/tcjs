#include <emscripten/val.h>
#include <vector>
#include <initializer_list>
#include "explicit_cast.h"
#include "range.h"
#include "range_defines.h"
#include "js_bootstrap.h"

using tc::js::js_string;
using tc::js::globals::Array;
using tc::js::globals::ReadonlyArray;
using tc::js::globals::console;

enum class MyIntEnum { Foo = 10 };

namespace tc::js {
template<> struct IsJsIntegralEnum<MyIntEnum> : std::true_type {};
} // namespace tc::js

int main() {
   {
        auto arr = tc::explicit_cast<ReadonlyArray<double>>(std::initializer_list<double>{1, 2, 3});
        static_assert(!tc::is_explicit_castable<ReadonlyArray<double>, double>::value);
        console()->log(arr);
        _ASSERTEQUAL(arr->length(), 3);
        _ASSERTEQUAL(arr[0], 1);
        _ASSERTEQUAL(arr[1], 2);
        _ASSERTEQUAL(arr[2], 3);
    }

    {
        Array<js_string> arr(std::initializer_list<char const*>{"Hello", "Hi!"});
        _ASSERTEQUAL(arr->length(), 2);
        _ASSERTEQUAL(arr[0].length(), 5);
        _ASSERTEQUAL(arr[1].length(), 3);
    }

    auto arr = tc::explicit_cast<Array<double>>(std::initializer_list<double>{1, 2, 3});
    static_assert(!tc::is_explicit_castable<Array<double>, double>::value);
    console()->log(arr);
    _ASSERTEQUAL(arr->length(), 3);
    _ASSERTEQUAL(arr[1], 2);
    arr->_setIndex(1, 15);
    _ASSERTEQUAL(arr[1], 15);

    {
        std::vector<double> result;
        tc::for_each(arr, [&](double item) { result.push_back(item); });
        _ASSERTEQUAL(result, (std::vector{1.0, 15.0, 3.0}));
    }

    {
        std::vector<double> result;
        tc::for_each(
            tc::filter(
                tc::transform(arr,
                    [](double x) { return x * x; }
                ),
                [](double x) { return x >= 2; }
            ),
            [&](double item) { result.push_back(item); }
        );
        _ASSERTEQUAL(result, (std::vector{225.0, 9.0}));
    }

    {
        emscripten::val emval(MyIntEnum::Foo);
        _ASSERT(emval.isNumber());
        _ASSERTEQUAL(emval.template as<double>(), 10.0);
        _ASSERTEQUAL(emval.template as<MyIntEnum>(), MyIntEnum::Foo);
    }
    return 0;
}

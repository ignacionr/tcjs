#pragma once

#include <emscripten/val.h>
#include <utility>
#include <type_traits>
#include "break_or_continue.h"
#include "explicit_cast.h"
#include "type_traits.h"
#include "range.h"
#include "js_types.h"
#include "js_callback.h"
#include "tc_move.h"

namespace tc::js {

namespace no_adl {
template<typename> struct _js_Array;
template<typename> struct _js_ReadonlyArray;
struct _js_console;

template<typename T> using Array = jst::js_ref<_js_Array<T>>;
template<typename T> using ReadonlyArray = jst::js_ref<_js_ReadonlyArray<T>>;
using console = jst::js_ref<_js_console>;

template<typename T>
struct _js_Array : virtual jst::IObject {
	static_assert(jst::IsJsInteropable<T>::value);

	struct _tcjs_definitions {
		using value_type = T;
	};

	auto length() noexcept { return tc::explicit_cast<int>(_getProperty<double>("length")); }

	auto push(T const& item) noexcept { return _call<void>("push", item); }

	auto operator[](int i) && noexcept { return _getProperty<T>(tc::as_dec(i)); }

	void _setIndex(int i, T value) noexcept { _setProperty(tc::as_dec(i), tc_move(value)); }

	// Generator range. This adds operator() to array interface (which did not exist before), but it's ok.
	template<typename Fn>
	auto operator()(Fn fn) noexcept {
		if (_call<bool>("some", jst::js_lambda_wrap([&](T value, jst::js_unknown, jst::js_unknown) noexcept {
			return tc::break_ == tc::continue_if_not_break(fn, tc_move(value));
		})))
			return tc::break_;
		else
			return tc::continue_;
	}

	static Array<T> _tcjs_construct() noexcept {
		return Array<T>(emscripten::val::array());
	}

	template<typename Rng, typename = std::enable_if_t<tc::is_explicit_castable<T, tc::range_value_t<Rng>&&>::value>>
	static Array<T> _tcjs_construct(Rng&& rng) noexcept {
		Array<T> result(emscripten::val::array());
		tc::for_each(rng, [&](auto&& value) noexcept {
			result->push(tc::explicit_cast<T>(std::forward<decltype(value)>(value)));
		});
		return result;
	}
};

template<typename T>
struct _js_ReadonlyArray : virtual jst::IObject {
	static_assert(jst::IsJsInteropable<T>::value);

	struct _tcjs_definitions {
		using value_type = T;
	};

	auto length() noexcept { return tc::explicit_cast<int>(_getProperty<double>("length")); }

	auto operator[](int i) noexcept { return _getProperty<T>(tc::as_dec(i)); }

	// Generator range. This adds operator() to array interface (which did not exist before), but it's ok.
	template<typename Fn>
	auto operator()(Fn fn) noexcept {
		if (_call<bool>("some", jst::js_lambda_wrap([&](T value, jst::js_unknown, jst::js_unknown) noexcept {
			return tc::break_ == tc::continue_if_not_break(fn, tc_move(value));
		})))
			return tc::break_;
		else
			return tc::continue_;
	}

	static ReadonlyArray<T> _tcjs_construct() noexcept {
		return ReadonlyArray<T>(emscripten::val::array());
	}

	template<typename Rng, typename = std::enable_if_t<tc::is_explicit_castable<T, tc::range_value_t<Rng>&&>::value>>
	static ReadonlyArray<T> _tcjs_construct(Rng&& rng) noexcept {
		return ReadonlyArray<T>(
			Array<T>(jst::create_js_object, std::forward<Rng>(rng)).getEmval()
		);
	}
};

struct _js_console : virtual jst::IObject {
	struct _tcjs_definitions {
		template<typename... Args>
		static void log(Args&&... args) noexcept {
			static_assert((jst::IsJsInteropable<tc::remove_cvref_t<Args>>::value && ...));
			emscripten::val::global("console")["log"](std::forward<Args>(args)...);
		}
	};
};
} // namespace no_adl

using no_adl::Array;
using no_adl::ReadonlyArray;
using no_adl::console;

} // namespace tc::js

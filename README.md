# tcjs

The goal is to write web front-end completely in C++ instead of JavaScript.
The mean is to create type-safe JavaScript bindings for C++/Emscripten from TypeScript declaration files.

Close analogues are Rust's [stdweb](https://github.com/koute/stdweb) and [wasm-bindgen](https://github.com/rustwasm/wasm-bindgen).

# Dependencies

* [Emscripten](https://emscripten.org/) (Node.js and NPM are included)
* [think-cell public library](https://github.com/think-cell/range)
* Boost 1.68.0 or newer

# Misc thoughts
* C++ callbacks passed to JS are always `noexpect` because exceptions cannot be passed between JS and C++ at the moment.
* Member functions converted to callbacks are always called with a non-const-this.
* Methods for `js_*` types do not have consistent const-qualifiers at the moment, so do not use const qualifiers on them.
* `js_ref` may be inherited from, assuming you're ok with slicing (e.g. `CUniqueDetachableJsFunction`).
  Allowed slicing is argument in favor of `std::is_convertible`/`std::is_constructible`, not
  `tc::is_safely_convertible`, `tc::is_safely_constructible`.
* It may make sense to replace `tc::remove_cvref_t` with `tc::decay_t` in some places so it works with e.g. `vector<bool>::reference`.
* It may make sense to decay subtypes of `js_ref` to `js_ref`.
* `js_union` currently does not force any specific order of arguments, probably it should to some extent
  (even though most instances will be autogenerated).
* Standard `std::variant` construction is somewhat more restrictive than `js_union`:
  the former requires successful overload resolution, the latter requires that the
  argument is convertible to at least one option.

# Naming conventions
* Global variables start with `g_`.
* Member fields start with `m_`.
* All variables are prefixed with a type tag: `<type-tag>Name`.
    * `Name` is camel-case.
    * There should be no duplicates between type tag and name, e.g. `vecstrArguments`, not `vecstrArgumentsVector`.
* Functions' and variables' names are `CamelCase`.
* `int`'s type tag is `n`, e.g. `nArguments`.
* `std::string`'s type tag is `str`.
* `std::vector<T>`'s type tag is `vec<type-tag-of-T>`, name is singular, e.g. `vecstrArguments`.
* `js_ref<T>`'s type tag is `j<type-tag-of-T>`
    * `ts::Symbol`'s type tag is `sym`.
    * `js_optional<T>`'s type tag is `o<type-tag-of-T>`.
    * E.g. `js_ref<js_optional<ts::Symbol>> josymDeclaration;`
* For (almost) singleton objects from TypeScript Compiler API: `jts<exact-type-name-from-typescript>`.
    * "Almost singleton" ~ "does not require any name in local scope, type is enough".
    * The decision is made on a per-class basis, e.g. `Symbol` is never "almost singleton" to avoid confusion.
    * E.g. `jtsTypeChecker`.
* Optional singleton objects are `jots<exact-type-name-from-typescript>`.

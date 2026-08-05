#pragma once

#include <type_traits>

namespace utils {

	template <typename E>
	constexpr E next_enum(E current, E max) {
		using U = std::underlying_type_t<E>;
		auto next = (static_cast<U>(current) + 1) % static_cast<U>(max);
		return static_cast<E>(next);
	}

	template <typename E>
	constexpr E prev_enum(E current, E max) {
		using U = std::underlying_type_t<E>;
		auto cur_val = static_cast<U>(current);
		auto max_val = static_cast<U>(max);
		auto prev = (cur_val == 0) ? (max_val - 1) : (cur_val - 1);
		return static_cast<E>(prev);
	}

	template <typename T>
	concept CountableEnum = std::is_enum_v<T> && requires { T::Count; };

} // namespace utils

template <utils::CountableEnum E>
constexpr E& operator++(E& e) {
	e = utils::next_enum(e, E::Count);
	return e;
}

template <utils::CountableEnum E>
constexpr E operator++(E& e, int) {
	E temp = e;
	++e;
	return temp;
}

template <utils::CountableEnum E>
constexpr E& operator--(E& e) {
	e = utils::prev_enum(e, E::Count);
	return e;
}

template <utils::CountableEnum E>
constexpr E operator--(E& e, int) {
	E temp = e;
	--e;
	return temp;
}

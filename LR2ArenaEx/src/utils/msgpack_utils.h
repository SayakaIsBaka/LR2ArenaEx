#pragma once

#include <msgpack.hpp>
#include <vector>

namespace msgpack_utils {
	template <typename T>
	inline T unpack(std::vector<char> data) {
		auto oh = msgpack::unpack(data.data(), data.size());
		auto obj = oh.get();

		T res;
		obj.convert(res);
		return res;
	}

	template <typename T>
	inline std::vector<char> pack(T msg) {
		msgpack::sbuffer msgPack;
		msgpack::pack(msgPack, msg);
		std::vector<char> data(msgPack.data(), msgPack.data() + msgPack.size());
		return data;
	}
}
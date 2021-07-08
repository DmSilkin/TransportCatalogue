#pragma once
#include <string>
#include <vector>
#include <deque>

#include "geo.h"

namespace detail {
	struct Stop {
		std::string name;
		geo::Coordinates coordinates = { 0.0, 0.0 };
	};

	using StopPtr = const Stop*;

	struct Bus {
		size_t unique_stops = 0;
		size_t stops_number = 0;
		std::string number;
		std::vector<std::string> raw_route;
		std::deque<StopPtr> route;
		bool is_round_trip = false;
		uint64_t distance = 0;		
	};

	using BusPtr = const Bus*;

	class StopHasher {
	public:
		size_t operator()(std::pair<StopPtr, StopPtr> stop) const {
			return hasher_(stop.first) + hasher_(stop.second) * 1000;
		}
	private:
		std::hash<const void*> hasher_;
	};
}
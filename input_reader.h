#pragma once
#include "transport_catalogue.h"

#include <iostream>
#include <set>

enum class RequestType {
	STOP,
	BUS,
	UNCORRECT,
};

enum class RouteType {
	CIRCULAR,
	REGULAR,
	SINGLE,
};

namespace parsing {
	void ReadRequest(std::istream& is, catalogue::TransportCatalogue& transport_catalogue);

	detail::Stop GetStopFromRequest(std::string_view request);

	detail::Bus GetBusFromRequest(std::string_view request, const RouteType& route_type);

	void ParseRequestBus(detail::Bus& bus, std::string_view request, char separator);

	std::vector<std::string> ReadRequestRoute(std::istream& is);

	std::vector<std::pair<RequestType, std::string>> ReadRequestRouteOrStop(std::istream& is);

	std::pair<detail::StopPtr, std::vector<std::pair<detail::StopPtr, uint64_t>>> ReadDistancesFromRequest(std::string_view request, const catalogue::TransportCatalogue& transport_catalogue);

	RequestType GetRequestType(std::string_view request);

	RouteType GetRouteType(std::string_view request);
}







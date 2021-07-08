#pragma once
#include "transport_catalogue.h"
#include "input_reader.h"
#include <iostream>

namespace output {
	void PrintRouteInformation(catalogue::TransportCatalogue& transport_catalogue, std::ostream& out, std::string bus);

	void PrintStopsInformation(catalogue::TransportCatalogue& transport_catalogue, std::ostream& out, std::string stop);

	void PrintInformation(std::ostream& out, catalogue::TransportCatalogue& transport_catalogue, std::vector<std::pair<RequestType, std::string>> result);
}


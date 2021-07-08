#include "stat_reader.h"

using namespace std; 

namespace output {
	using namespace catalogue;

	void PrintRouteInformation(TransportCatalogue& transport_catalogue, ostream& out, string bus) {

		const auto route_info = transport_catalogue.GetRouteInfromation(bus);
		out << "Bus "s << bus << ": "s;
		if (route_info.second == nullptr) {
			out << "not found"s << endl;
			return;
		}

		out << route_info.second->stops_number << " stops on route, " << route_info.second->unique_stops << " unique stops, " <<
			route_info.first.first << " route length, " << setprecision(6) << route_info.first.first / route_info.first.second << " curvature" << endl;

	}

	void PrintStopsInformation(TransportCatalogue& transport_catalogue, ostream& out, string stop) {
		const auto stop_info = transport_catalogue.FindBusesByStop(stop);
		out << "Stop "s << stop << ": "s;
		if (stop_info.first == nullptr) {
			out << "not found"s << endl;
			return;
		}
		else if (stop_info.second.size() == 0) {
			out << "no buses"s << endl;
			return;
		}
		else {
			out << "buses "s;
			for (const auto& bus : stop_info.second) {
				out << bus << " ";
			}
			out << endl;
		}
	}

	void PrintInformation(ostream& out, TransportCatalogue& transport_catalogue, vector<pair<RequestType, string>> result) {
		for (const auto& elem : result) {
			switch (elem.first) {
			case(RequestType::STOP):
				PrintStopsInformation(transport_catalogue, out, elem.second);
				break;
			case(RequestType::BUS):
				PrintRouteInformation(transport_catalogue, out, elem.second);
				break;
			case(RequestType::UNCORRECT):
				throw "Uncorrect!"s;
				break;
			}
		}
	}
}


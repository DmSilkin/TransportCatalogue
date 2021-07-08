#include "input_reader.h"

using namespace std;

namespace parsing {
	using namespace catalogue;
	using namespace detail;

	void ReadRequest(istream& is, TransportCatalogue& transport_catalogue) {
		int request_number = 0;
		string request;
		vector<string> stops;
		vector<string> routes;

		is >> request_number;
		is.ignore();
		for (int i = 0; i < request_number; ++i) {
			getline(is, request);
			RequestType request_type = GetRequestType(move(request));
			switch (request_type) {
			case RequestType::BUS:
				request = request.substr(4);
				routes.push_back(move(request));
				break;
			case RequestType::STOP:
				request = request.substr(5);
				stops.push_back(move(request));
				break;
			default:
				throw "Incorrect request type!"s;
				break;

			}
		}

		for (auto& stop : stops) {
			transport_catalogue.AddStop(GetStopFromRequest(move(stop)));
		}

		for (auto& stop : stops) {
			const auto distances = ReadDistancesFromRequest(move(stop), transport_catalogue);
			transport_catalogue.AddDistance(distances.first, distances.second);

		}
		/*StopPtr stop_1 = transport_catalogue.FindStopByName("A"sv);
		StopPtr stop_2 = transport_catalogue.FindStopByName("B"sv);
		cout << transport_catalogue.GetDistanceBetweenStops(stop_1, stop_2) << endl;

		stop_1 = transport_catalogue.FindStopByName("B"sv);
		stop_2 = transport_catalogue.FindStopByName("A"sv);
		cout << transport_catalogue.GetDistanceBetweenStops(stop_1, stop_2) << endl;

		stop_1 = transport_catalogue.FindStopByName("C"sv);
		stop_2 = transport_catalogue.FindStopByName("B"sv);
		cout << transport_catalogue.GetDistanceBetweenStops(stop_1, stop_2) << endl;*/

		for (auto& route : routes) {
			RouteType route_type = GetRouteType(route);
			Bus bus = GetBusFromRequest(move(route), route_type);
			transport_catalogue.AddBus(bus);
		}
	}

	Stop GetStopFromRequest(string_view request) {
		Stop stop;
		size_t sep = request.find(':');
		stop.name = request.substr(0, sep);
		request.remove_prefix(sep + 1);
		sep = request.find(',');
		stop.coordinates.lat = stod(string(request.substr(0, sep)));
		request.remove_prefix(sep + 1);
		sep = request.find(',');
		if (sep != request.npos) {
			stop.coordinates.lng = stod(string(request.substr(0, sep)));
		}
		else {
			stop.coordinates.lng = stod(string(request));
		}


		return stop;
	}

	pair<StopPtr, vector<pair<StopPtr, uint64_t>>> ReadDistancesFromRequest(string_view request, const TransportCatalogue& transport_catalogue) {
		size_t sep = request.find(':');

		StopPtr this_stop = transport_catalogue.FindStopByName(request.substr(0, sep));
		vector<pair<StopPtr, uint64_t>> found_stops;

		sep = request.find(',');
		request.remove_prefix(sep + 1);
		sep = request.find(',');
		request.remove_prefix(sep + 1);

		while (1) {
			sep = request.find('m');
			uint64_t distance = stoi(string(request.substr(0, sep)));
			request.remove_prefix(sep + 5);
			sep = request.find(',');
			StopPtr found_stop = transport_catalogue.FindStopByName(request.substr(0, sep));
			found_stops.push_back({ found_stop, distance });

			if (sep == request.npos) {
				break;
			}
			else {
				request.remove_prefix(sep + 1);
			}
		}

		return { this_stop, found_stops };
	}

	Bus GetBusFromRequest(string_view request, const RouteType& route_type) {
		Bus bus;
		switch (route_type) {
		case(RouteType::CIRCULAR):
			ParseRequestBus(bus, request, '>');
			break;
		case(RouteType::REGULAR):
			ParseRequestBus(bus, request, '-');
			break;
		case(RouteType::SINGLE):
			ParseRequestBus(bus, request, '|');
			break;
		default:
			throw "INCORRECT ROUTE TYPE!"s;
			break;
		}
		return bus;
	}

	void ParseRequestBus(Bus& bus, string_view request, char separator) {
		set<string_view> uniq_stops;
		size_t sep = request.find(':');
		bus.number = request.substr(0, sep);
		request.remove_prefix(sep + 1);

		if (separator == '|') {
			bus.raw_route.push_back(move(string(request)));
			bus.stops_number = 1;
			bus.unique_stops = 1;

			return;
		}

		while (1) {
			sep = request.find(separator);
			string_view stop = request.substr(1, sep - 2);
			uniq_stops.insert(stop);
			bus.raw_route.push_back(move(string(stop)));
			if (sep == request.npos)
				break;
			else {
				request.remove_prefix(sep + 1);
			}
		}

		if (separator == '-') {
			vector<string> raw_copy;
			for (auto it = bus.raw_route.rbegin(); it != bus.raw_route.rend(); it++)
				raw_copy.push_back(*it);
			bus.raw_route.insert(bus.raw_route.end(), raw_copy.begin() + 1, raw_copy.end());
		}

		bus.stops_number = bus.raw_route.size();
		bus.unique_stops = uniq_stops.size();
	}





	vector<string> ReadRequestRoute(istream& is) {
		int buses_number = 0;
		is >> buses_number;
		is.ignore();
		vector<string> parsed_buses;
		for (int i = 0; i < buses_number; ++i) {
			string request;
			getline(is, request);
			string_view request_sv = request;
			request_sv.remove_prefix(4);
			parsed_buses.push_back(string(request_sv));
		}

		return parsed_buses;
	}

	vector<pair<RequestType, string>> ReadRequestRouteOrStop(istream& is) {
		vector<pair<RequestType, string>> result;
		int request_number;
		is >> request_number;
		is.ignore();
		for (int i = 0; i < request_number; ++i) {
			string request;
			getline(is, request);
			RequestType request_type = GetRequestType(move(request));
			switch (request_type) {
			case(RequestType::STOP):
				result.push_back({ RequestType::STOP, request.substr(5, request.npos) });
				break;
			case(RequestType::BUS):
				result.push_back({ RequestType::BUS, request.substr(4, request.npos) });
				break;
			case(RequestType::UNCORRECT):
				throw "Uncorrect!"s;
				break;
			}
		}
		return result;
	}

	RequestType GetRequestType(string_view request) {
		if (request.substr(0, 4) == "Stop") {
			return RequestType::STOP;
		}
		else if (request.substr(0, 3) == "Bus") {
			return RequestType::BUS;
		}

		return RequestType::UNCORRECT;
	}

	RouteType GetRouteType(string_view raw_route) {
		if (raw_route.find('>') != raw_route.npos) {
			return RouteType::CIRCULAR;
		}
		else if (raw_route.find('-') != raw_route.npos) {
			return RouteType::REGULAR;
		}

		return RouteType::SINGLE;
	}
}


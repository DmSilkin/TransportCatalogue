#include "transport_catalogue.h"

using namespace std;

namespace catalogue {
	using namespace detail;
	using namespace geo;

	void TransportCatalogue::AddStop(const Stop& stop) {
		auto inserted_stop = &stops_name_.emplace_back(stop);
		stop_to_buses_[inserted_stop].insert({});
		stops_[inserted_stop->name] = inserted_stop;
	}

	void TransportCatalogue::AddBus(const Bus& bus) {
		deque<StopPtr> bus_stops;
		for (size_t i = 0; i < bus.raw_route.size(); ++i) {
			const auto stop = FindStopByName(move(bus.raw_route[i]));
			//stop_to_buses_[stop].insert(bus.number);
			bus_stops.push_back(move(stop));
		}

		if (!bus.is_round_trip && bus.raw_route.size() >= 2) {
			for (int i = static_cast<int>(bus.raw_route.size()) - 2; i >= 0; --i) {
				const auto stop = FindStopByName(move(bus.raw_route[i]));
				bus_stops.push_back(move(stop));
			}
		}
		Bus buf_bus{ bus.unique_stops, bus_stops.size(), move(bus.number), move(bus.raw_route), move(bus_stops), bus.is_round_trip };
		auto inserted_bus = &buses_.emplace_back(buf_bus);
		routes_[inserted_bus->number] = inserted_bus;
		FillStopToBuses(inserted_bus);
	}

	StopPtr TransportCatalogue::FindStopByName(string_view name) const {
		if (!stops_.count(name)) {
			return nullptr;
		}
		return stops_.at(name);
	}

	BusPtr TransportCatalogue::FindBusByNumber(string_view number) const {
		if (!routes_.count(number)) {
			return nullptr;
		}
		return routes_.at(number);
	}

	pair<pair<double, double>, BusPtr> TransportCatalogue::GetRouteInfromation(const string_view& number) const {
		BusPtr bus = FindBusByNumber(number);

		if (bus == nullptr) {
			return { {0.0, 0.0}, nullptr };
		}
		return { {ComputeRouteDistance(bus), ComputeLength(bus)}, bus };
	}

	pair<StopPtr, set<string_view>> TransportCatalogue::FindBusesByStop(const string_view& name) const
	{
		StopPtr stop = FindStopByName(name);
		if (stop == nullptr) {
			return { nullptr, {} };
		}

		return { stop, stop_to_buses_.at(stop) };
	}

	void TransportCatalogue::AddDistance(StopPtr stop_from, vector<pair<StopPtr, double>> stops_to) {
		for (auto& stop : stops_to) {
			if (stop.first != nullptr) {
				distances_[{stop_from, move(stop.first)}] = stop.second;
			}
		}
	}

	void TransportCatalogue::SetRoutingSettings(int timeWait, double velocity) {
		routing_settings_ = { timeWait, velocity };
	}

	double TransportCatalogue::GetDistanceBetweenStops(StopPtr stop_from, StopPtr stop_to) const {
		if (!distances_.count({ stop_from, stop_to })) {
			return distances_.at({ stop_to, stop_from });
		}
		return distances_.at({ stop_from, stop_to });
	}

	double TransportCatalogue::ComputeLength(BusPtr& bus) const {
		double length = 0.0;
		for (size_t i = 0; i < bus->route.size() - 1; ++i) {
			const auto stop_from = FindStopByName(move(bus->route[i]->name));
			const auto stop_to = FindStopByName(move(bus->route[i + 1]->name));

			length += ComputeDistance(stop_from->coordinates, stop_to->coordinates);
		}

		return length;
	}

	double TransportCatalogue::ComputeRouteDistance(BusPtr bus) const {
		double distance = 0;
		for (size_t i = 0; i < bus->route.size() - 1; ++i) {
			distance += GetDistanceBetweenStops(bus->route.at(i), bus->route.at(i + 1));
		}
		return distance;
	}

	void TransportCatalogue::FillStopToBuses(BusPtr bus) {
		for (const auto stop : bus->route) {
			stop_to_buses_[stop].insert(bus->number);
		}
	}

	std::map<std::string_view, detail::BusPtr> TransportCatalogue::GetRoutes() const {
		std::map<std::string_view, detail::BusPtr> sorted_routes;
		for (const auto& route : routes_) {
			sorted_routes.insert(route);
		}
		return sorted_routes;
	}

	std::map<std::string_view, detail::StopPtr> TransportCatalogue::GetStops() const {
		std::map<std::string_view, detail::StopPtr> sorted_stops;
		for (const auto& stop : stops_) {
			sorted_stops.insert(stop);
		}
		return sorted_stops;
	}

	std::pair<int, double> TransportCatalogue::GetRoutingSettings() const {
		return routing_settings_;
	}

	std::unordered_map<std::pair<detail::StopPtr, detail::StopPtr>, double, detail::StopHasher> TransportCatalogue::GetDistances() const {
		return distances_;
	}


}


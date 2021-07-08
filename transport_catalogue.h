#pragma once
#include <string>
#include <string_view>
#include <deque>
#include <vector>
#include <set>
#include <unordered_map>
#include <map>
#include <iomanip>

#include "geo.h"
#include "domain.h"



namespace catalogue {
	class TransportCatalogue {
	public:
		
		void AddStop(const detail::Stop& stop);
		void AddBus(const detail::Bus& bus);
		void AddDistance(detail::StopPtr stop_from, std::vector<std::pair<detail::StopPtr, double>> stops_to);
		void SetRoutingSettings(int timeWait, double velocity);

		detail::BusPtr FindBusByNumber(std::string_view number) const;
		detail::StopPtr FindStopByName(std::string_view name) const;
		std::pair<detail::StopPtr, std::set<std::string_view>> FindBusesByStop(const std::string_view& name) const;

		std::map<std::string_view, detail::BusPtr> GetRoutes() const;
		std::map<std::string_view, detail::StopPtr> GetStops() const;
		std::pair<std::pair<double, double>, detail::BusPtr> GetRouteInfromation(const std::string_view& number) const;
		double GetDistanceBetweenStops(detail::StopPtr stop_from, detail::StopPtr stop_to) const;
		std::pair<int, double> GetRoutingSettings() const;
		std::unordered_map<std::pair<detail::StopPtr, detail::StopPtr>, double, detail::StopHasher> GetDistances() const;

	private:
		std::deque<detail::Stop> stops_name_;
		std::deque<detail::Bus> buses_;
		std::unordered_map<std::string_view, detail::StopPtr> stops_;
		std::unordered_map<std::string_view, detail::BusPtr> routes_;
		std::unordered_map <detail::StopPtr, std::set<std::string_view>> stop_to_buses_;
		std::unordered_map<std::pair<detail::StopPtr, detail::StopPtr>, double, detail::StopHasher> distances_;
		std::pair<int, double> routing_settings_;

		double ComputeLength(detail::BusPtr& bus) const;
		double ComputeRouteDistance(detail::BusPtr bus) const;
		void FillStopToBuses(detail::BusPtr bus);
	};
}




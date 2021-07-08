#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_router.h"
#include "graph.h"

#include <fstream>
#include <vector>
#include <transport_catalogue.pb.h>



namespace serialize {
	
	const int METERS_TO_KM = 1000;
	const int HOURS_TO_MINUTES = 60;


	class Serializator {
	public:
		Serializator(const catalogue::TransportCatalogue& transport_catalogue, const renderer::MapRenderer& map_renderer)
			:db_(transport_catalogue),
			 map_renderer_(map_renderer)
		{
		}

		void Serialize(const std::string& filename);


	private:
		const catalogue::TransportCatalogue& db_;
		const renderer::MapRenderer& map_renderer_;
		transport_catalogue_serialize::TransportCatalogue tc_serialize_;

		//Graph Indices
		std::unordered_map<detail::StopPtr, router::VertexInfo> stops_to_vertex_ids_;
		transport_catalogue_serialize::Router buf_router_;
		transport_catalogue_serialize::Graph buf_graph_;
		double bus_wait_time_ = 0.0;
		double bus_velocity_ = 0.0;


		void SerializeStop(transport_catalogue_serialize::TransportCatalogue& tc_serialize);
		void SerializeBus(transport_catalogue_serialize::TransportCatalogue& tc_serialize);
		void SerializeDistance(transport_catalogue_serialize::TransportCatalogue& tc_serialize);
		void SerializeRenderSettings(transport_catalogue_serialize::TransportCatalogue& tc_serialize);
		transport_catalogue_serialize::Color SerializeColor(transport_catalogue_serialize::RenderSettings buf_render_settings, const svg::Color color);
		void SerializeGraph();
		void SerializeRoutingSettings(transport_catalogue_serialize::Router& buf_router);
		void SerializeEdgeInfo(transport_catalogue_serialize::Router& buf_router, const router::EdgesInfo& edge_info);
		void SerializeStopToVertexIds(transport_catalogue_serialize::Router& buf_router, const std::pair<detail::StopPtr, router::VertexInfo>& stop);
		void SerializeGraphEdge(transport_catalogue_serialize::Graph& buf_graph, const graph::Edge<double>& edge);
		void SerializeWaitStop(const std::pair<std::string_view, detail::StopPtr>& stop, graph::VertexId& vertex_id);
		void SerializeRoute(const detail::BusPtr& route);
		void SetRouterSerialize(const transport_catalogue_serialize::Router& buf_router);
	};



	class Deserializator {
	public:
		using StopPtrPair = std::pair<detail::StopPtr, detail::StopPtr>;
		void Deserialize(catalogue::TransportCatalogue& transport_catalogue, renderer::MapRenderer& map_renderer, router::TransportRouter& router, const std::string& filename);

	private:
		detail::Stop DeserializeStop(const transport_catalogue_serialize::Stop& stop_serialize);
		detail::Bus DeserializeBus(const transport_catalogue_serialize::Bus& bus_serialize);
		std::pair<StopPtrPair, uint64_t> DeserializeDistance(const transport_catalogue_serialize::Distance& distance_serialize, const catalogue::TransportCatalogue& transport_catalogue);
		renderer::RenderSettings DeserializeRenderSettings(const transport_catalogue_serialize::RenderSettings& render_settings_serialize);
		svg::Color DeserializeColor(const transport_catalogue_serialize::Color& color_serialize);
		void DeserializeRouter(const transport_catalogue_serialize::Router& router_serialize, const catalogue::TransportCatalogue& transport_catalogue, router::TransportRouter& route);
	};
}

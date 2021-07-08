#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "svg.h"

#include <optional>
#include <set>
#include <string_view>

/*
 * ����� ����� ���� �� ���������� ��� ����������� �������� � ����, ����������� ������, ������� ��
 * �������� �� �������� �� � transport_catalogue, �� � json reader.
 *
 * � �������� ��������� ��� ���� ���������� ��������� �� ���� ������ ����������� ��������.
 * �� ������ ����������� ��������� �������� ��������, ������� ������� ���.
 *
 * ���� �� ������������� �������, ��� ����� ���� �� ��������� � ���� ����,
 * ������ �������� ��� ������.
 */

 // ����� RequestHandler ������ ���� ������, ����������� �������������� JSON reader-�
 // � ������� ������������ ����������.
 // ��. ������� �������������� �����: https://ru.wikipedia.org/wiki/�����_(������_��������������)
 
 class RequestHandler {
 public:
     using BusStat = json::Dict;
     using RoutesStat = json::Dict;
     
     // MapRenderer ����������� � ��������� ����� ��������� �������
     RequestHandler(const catalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer)
         :db_(db),
          renderer_(renderer)
     {
    
     }

     // ���������� ���������� � �������� (������ Bus)
     std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

     // ���������� ��������, ���������� �����
     const std::optional<std::set<std::string_view>> GetBusesByStop(const std::string_view& stop_name) const;

     // ���� ����� ����� ����� � ��������� ����� ��������� �������
     svg::Document RenderMap() const;

 private:
     // RequestHandler ���������� ��������� �������� "������������ ����������" � "������������ �����"
     const catalogue::TransportCatalogue& db_;
     const renderer::MapRenderer& renderer_;
 };
 
#pragma once

#include "indexer/feature_decl.hpp"

#include "std/string.hpp"
#include "std/utility.hpp"
#include "std/vector.hpp"


class FeatureType;
class Index;

namespace search
{
namespace v2
{
class HouseToStreetTable;
}

class ReverseGeocoder
{
  Index const & m_index;

  struct Object
  {
    FeatureID m_id;
    double m_distanceMeters;
    string m_name;

    Object() : m_distanceMeters(-1.0) {}
    Object(FeatureID const & id, double dist, string const & name)
      : m_id(id), m_distanceMeters(dist), m_name(name)
    {
    }

    inline bool IsValid() const { return m_id.IsValid(); }
  };

public:
  /// All "Nearby" functions work in this lookup radius.
  static double const kLookupRadiusM;

  explicit ReverseGeocoder(Index const & index);

  using Street = Object;

  struct Building : public Object
  {
    m2::PointD m_center;

    // To investigate possible errors.
    // There are no houses in (0, 0) coordinates.
    Building() : m_center(0, 0) {}

    Building(FeatureID const & id, double dist, string const & number, m2::PointD const & center)
      : Object(id, dist, number), m_center(center)
    {
    }
  };

  static size_t GetMatchedStreetIndex(string const & keyName, vector<Street> const & streets);

  struct Address
  {
    Building m_building;
    Street m_street;

    string GetHouseNumber() const { return m_building.m_name; }
    string GetStreetName() const { return m_street.m_name; }
    double GetDistance() const { return m_building.m_distanceMeters; }
  };

  /// @return Sorted by distance streets vector for the specified MwmId.
  //@{
  void GetNearbyStreets(MwmSet::MwmId const & id, m2::PointD const & center,
                        vector<Street> & streets) const;
  void GetNearbyStreets(FeatureType & ft, vector<Street> & streets) const;
  //@}

  /// @todo Leave const reference for now to support client's legacy code.
  /// It's better to use honest non-const reference when feature can be modified in any way.
  pair<vector<Street>, uint32_t> GetNearbyFeatureStreets(FeatureType const & ft) const;

  /// @return The nearest exact address where building has house number and valid street match.
  void GetNearbyAddress(m2::PointD const & center, Address & addr) const;
  /// @return The exact address for feature.
  /// @precondition ft Should have house number.
  void GetNearbyAddress(FeatureType & ft, Address & addr) const;

private:

  /// Helper class to incapsulate house 2 street table reloading.
  class HouseTable
  {
    unique_ptr<search::v2::HouseToStreetTable> m_table;
    MwmSet::MwmHandle m_mwmHandle;
  public:
    bool Get(Index const & index, FeatureID fId,
             vector<Street> const & streets, uint32_t & stIndex);
  };

  bool GetNearbyAddress(HouseTable & table, Building const & bld, Address & addr) const;

  /// @return Sorted by distance houses vector with valid house number.
  void GetNearbyBuildings(m2::PointD const & center, vector<Building> & buildings) const;

  static Building FromFeature(FeatureType & ft, double distMeters);
  static m2::RectD GetLookupRect(m2::PointD const & center, double radiusM);
};

} // namespace search

#ifndef TR_CACHE_FETCHER
#define TR_CACHE_FETCHER

#include <pqxx/pqxx> 
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <math.h>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <stdlib.h>

#include "calculation_time.hpp"
#include "stop.hpp"
#include "route.hpp"
#include "point.hpp"
#include "trip.hpp"
#include "od_trip.hpp"
#include "tuple_boost_serialize.hpp"
#include "toolbox.hpp"
#include "parameters.hpp"

namespace TrRouting
{
  
  class CacheFetcher
  {
  
  public:
    
    CacheFetcher() {}
    CacheFetcher(std::string applicationShortname) {
      
    }
    
    template<class T>
    static const T loadFromCacheFile(T& data, std::string applicationShortname, std::string cacheFileName) {
      std::ifstream iCacheFile;
      iCacheFile.open(applicationShortname + "_" + cacheFileName + ".cache", std::ios::in | std::ios::binary);
      boost::archive::binary_iarchive iarch(iCacheFile);
      iarch >> data;
      iCacheFile.close();
      return data;
    }
    
    template<class T>
    static void saveToCacheFile(std::string applicationShortname, T& data, std::string cacheFileName) {
      std::ofstream oCacheFile;
      oCacheFile.open(applicationShortname + "_" + cacheFileName + ".cache", std::ios::out | std::ios::trunc | std::ios::binary);
      boost::archive::binary_oarchive oarch(oCacheFile);
      oarch << data;
      oCacheFile.close();
    }
    
    static bool cacheFileExists(std::string applicationShortname, std::string cacheFileName) {
      std::ifstream iCacheFile;
      bool notEmpty = false;
      iCacheFile.open(applicationShortname + "_" + cacheFileName + ".cache", std::ios::in | std::ios::binary | std::ios::ate);
      notEmpty = iCacheFile.tellg() > 0;
      iCacheFile.close();
      return notEmpty;
    }
    
    const std::pair<std::vector<Stop> , std::map<unsigned long long, int>> getStops( std::string applicationShortname);
    const std::pair<std::vector<Route>, std::map<unsigned long long, int>> getRoutes(std::string applicationShortname);
    const std::pair<std::vector<Trip> , std::map<unsigned long long, int>> getTrips( std::string applicationShortname);
    const std::pair<std::vector<std::tuple<int,int,int,int,int,short,short,int>>, std::vector<std::tuple<int,int,int,int,int,short,short,int>>> getConnections(std::string applicationShortname, std::map<unsigned long long, int> stopIndexesById, std::map<unsigned long long, int> tripIndexesById);
    const std::pair<std::vector<std::tuple<int,int,int>>, std::vector<std::pair<int,int>>> getFootpaths(std::string applicationShortname, std::map<unsigned long long, int> stopIndexesById);
    const std::pair<std::vector<OdTrip>, std::map<unsigned long long, int>> getOdTrips(std::string applicationShortname, std::vector<Stop> stops, Parameters& params);
    
  private:
    
  };
    
}

#endif // TR_CACHE_FETCHER

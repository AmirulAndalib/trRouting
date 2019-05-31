#include "calculator.hpp"

namespace TrRouting
{
  
  void Calculator::reset(bool resetAccessPaths, bool resetFilters)
  {
    
    using JourneyTuple = std::tuple<int,int,int,int,int,short>;

    int benchmarkingStart = algorithmCalculationTime.getEpoch();

    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();
    
    std::fill(nodesTentativeTime.begin()       , nodesTentativeTime.end()       , MAX_INT);
    std::fill(nodesReverseTentativeTime.begin(), nodesReverseTentativeTime.end(), -1);
    std::fill(nodesAccessTravelTime.begin()    , nodesAccessTravelTime.end()    , -1);
    std::fill(nodesEgressTravelTime.begin()    , nodesEgressTravelTime.end()    , -1);
    if (resetAccessPaths)
    {
      accessFootpaths.clear();
      accessFootpaths.shrink_to_fit();
      egressFootpaths.clear();
      egressFootpaths.shrink_to_fit();
    }
    std::fill(tripsEnterConnection.begin()                  , tripsEnterConnection.end()                  , -1);
    std::fill(tripsExitConnection.begin()                   , tripsExitConnection.end()                   , -1);
    std::fill(tripsEnterConnectionTransferTravelTime.begin(), tripsEnterConnectionTransferTravelTime.end(), MAX_INT);
    std::fill(tripsExitConnectionTransferTravelTime.begin() , tripsExitConnectionTransferTravelTime.end() , MAX_INT);
    if (resetFilters)
    {
      std::fill(tripsEnabled.begin(), tripsEnabled.end(), 1);
    }
    std::fill(tripsUsable.begin()          , tripsUsable.end()          , -1);
    std::fill(forwardJourneys.begin()      , forwardJourneys.end()      , JourneyTuple(-1,-1,-1,-1,-1,-1));
    std::fill(forwardEgressJourneys.begin(), forwardEgressJourneys.end(), JourneyTuple(-1,-1,-1,-1,-1,-1));
    std::fill(reverseJourneys.begin()      , reverseJourneys.end()      , JourneyTuple(-1,-1,-1,-1,-1,-1));
    std::fill(reverseAccessJourneys.begin(), reverseAccessJourneys.end(), JourneyTuple(-1,-1,-1,-1,-1,-1));
    
    departureTimeSeconds = -1;
    arrivalTimeSeconds   = -1;
    
    if(odTrip != nullptr && params.forwardCalculation == true)
    {
      departureTimeSeconds = odTrip->departureTimeSeconds;
    }
    else if (params.departureTimeSeconds >= 0)
    {
      departureTimeSeconds = params.departureTimeSeconds;
    }
    if (params.arrivalTimeSeconds >= 0)
    {
      arrivalTimeSeconds = params.arrivalTimeSeconds;
    }

    if (params.debugDisplay)
      std::cerr << "-- reset and preparations -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";

    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

    int i {0};

    // fetch nodes footpaths accessible from origin using params or osrm fetcher if not provided:
    minAccessTravelTime = MAX_INT;
    maxEgressTravelTime = -1;
    minEgressTravelTime = MAX_INT;
    maxAccessTravelTime = -1;
    
    if (!params.returnAllNodesResult || departureTimeSeconds >= -1)
    {
      if (resetAccessPaths)
      {

        if (params.debugDisplay)
          std::cerr << "  resetting access paths " << std::endl;

        if(odTrip != nullptr)
        {
          if (params.debugDisplay)
            std::cerr << "  using odTrip with " << odTrip->originNodesIdx.size() << " accessible nodes" << std::endl;

          accessFootpaths.clear();
          for (auto & accessNodeIdx : odTrip->originNodesIdx)
          {
            accessFootpaths.push_back(std::make_pair(accessNodeIdx, odTrip->originNodesTravelTimesSeconds[i]));
          }
        }
        else if (params.accessNodesIdx.size() > 0 && params.accessNodeTravelTimesSeconds.size() == params.accessNodesIdx.size())
        {
          accessFootpaths.clear();
          for (auto & accessNodeIdx : params.accessNodesIdx)
          {
            accessFootpaths.push_back(std::make_pair(accessNodeIdx, params.accessNodeTravelTimesSeconds[i]));
          }
        }
        else
        {
          if (params.debugDisplay)
            std::cout << "  fetching nodes with osrm with mode " << params.accessMode << std::endl;

          accessFootpaths = std::move(OsrmFetcher::getAccessibleNodesFootpathsFromPoint(params.origin, nodes, params.accessMode, params));
        }
      }

      if (params.debugDisplay)
        std::cout << "  parsing access footpaths to find min/max access travel times" << std::endl;

      int footpathTravelTimeSeconds;
      for (auto & accessFootpath : accessFootpaths)
      {
        footpathTravelTimeSeconds = (int)ceil((float)accessFootpath.second / params.walkingSpeedFactor);

        nodesAccessTravelTime[accessFootpath.first] = footpathTravelTimeSeconds;
        forwardJourneys[accessFootpath.first]       = std::make_tuple(-1, -1, -1, -1, footpathTravelTimeSeconds, -1);
        nodesTentativeTime[accessFootpath.first]    = departureTimeSeconds + footpathTravelTimeSeconds + params.minWaitingTimeSeconds;
        if (footpathTravelTimeSeconds < minAccessTravelTime)
        {
          minAccessTravelTime = footpathTravelTimeSeconds;
        }
        if (footpathTravelTimeSeconds > maxAccessTravelTime)
        {
          maxAccessTravelTime = footpathTravelTimeSeconds;
        }
        //std::cerr << "origin_node: " << nodes[accessFootpath.first].get()->name << " - " << Toolbox::convertSecondsToFormattedTime(nodesTentativeTime[accessFootpath.first]) << std::endl;
        //std::cerr << std::to_string(nodes[accessFootpath.first].get()->id) + ",";
      }
    }
  
    if (!params.returnAllNodesResult || arrivalTimeSeconds >= -1)
    {
      if (resetAccessPaths)
      {
        // fetch nodes footpaths accessible to destination using params or osrm fetcher if not provided:
        if(odTrip != nullptr)
        {

          if (params.debugDisplay)
            std::cerr << "  using odTrip with " << odTrip->destinationNodesIdx.size() << " egressible nodes" << std::endl;

          egressFootpaths.clear();
          for (auto & egressNodeIdx : odTrip->destinationNodesIdx)
          {
            egressFootpaths.push_back(std::make_pair(egressNodeIdx, odTrip->destinationNodesTravelTimesSeconds[i]));
          }
        }
        else if (params.egressNodesIdx.size() > 0 && params.egressNodeTravelTimesSeconds.size() == params.egressNodesIdx.size())
        {
          egressFootpaths.clear();
          for (auto & egressNodeIdx : params.egressNodesIdx)
          {
            egressFootpaths.push_back(std::make_pair(egressNodeIdx, params.egressNodeTravelTimesSeconds[i]));
          }
        }
        else
        {
          egressFootpaths = std::move(OsrmFetcher::getAccessibleNodesFootpathsFromPoint(params.destination, nodes, params.accessMode, params));
        }
      }
      
      if (params.debugDisplay)
        std::cout << "  parsing egress footpaths to find min/max egress travel times" << std::endl;

      int footpathTravelTimeSeconds;
      for (auto & egressFootpath : egressFootpaths)
      {
        footpathTravelTimeSeconds                       = (int)ceil((float)egressFootpath.second / params.walkingSpeedFactor);
        nodesEgressTravelTime[egressFootpath.first]     = footpathTravelTimeSeconds;
        reverseJourneys[egressFootpath.first]           = std::make_tuple(-1, -1, -1, -1, footpathTravelTimeSeconds, -1);
        nodesReverseTentativeTime[egressFootpath.first] = arrivalTimeSeconds - footpathTravelTimeSeconds;
        if (footpathTravelTimeSeconds > maxEgressTravelTime)
        {
          maxEgressTravelTime = footpathTravelTimeSeconds;
        }
        if (footpathTravelTimeSeconds < minEgressTravelTime)
        {
          minEgressTravelTime = footpathTravelTimeSeconds;
        }
        //nodesD[egressFootpath.first]                = egressFootpath.second;
        //result.json += "origin_node: " + nodes[accessFootpath.first].get()->name + " - " + Toolbox::convertSecondsToFormattedTime(nodesTentativeTime[accessFootpath.first]) + "\n";
        //result.json += std::to_string((int)(ceil(egressFootpath.second))) + ",";
      }
    }
    
    //std::cerr << "-- maxEgressTravelTime = " << maxEgressTravelTime << std::endl;

    if (params.debugDisplay)
      std::cerr << "-- access and egress footpaths -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";
    
    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();


    // disable trips according to parameters:

    if (resetFilters)
    {

      if (params.debugDisplay)
        std::cout << "  resetting filters" << std::endl;

      if (params.calculateAllOdTrips)
      {
        // reset connections demand:
        for (auto & tripConnectionDemand : tripConnectionDemands)
        {
          std::generate(tripConnectionDemand.begin(), tripConnectionDemand.end(), []() { return std::make_unique<float>(0.0); });
        }
      }
      
      i = 0;
      for (auto & trip : trips)
      {
        if (tripsEnabled[i] == 1 && params.onlyServicesIdx.size() > 0)
        {
          if (std::find(params.onlyServicesIdx.begin(), params.onlyServicesIdx.end(), trip->serviceIdx) == params.onlyServicesIdx.end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && params.onlyLinesIdx.size() > 0)
        {
          if (std::find(params.onlyLinesIdx.begin(), params.onlyLinesIdx.end(), trip->lineIdx) == params.onlyLinesIdx.end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && params.onlyModesIdx.size() > 0)
        {
          if (std::find(params.onlyModesIdx.begin(), params.onlyModesIdx.end(), trip->modeIdx) == params.onlyModesIdx.end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && params.onlyNodesIdx.size() > 0)
        {
          if (std::find(params.onlyNodesIdx.begin(), params.onlyNodesIdx.end(), trip->modeIdx) == params.onlyNodesIdx.end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && params.onlyAgenciesIdx.size() > 0)
        {
          if (std::find(params.onlyAgenciesIdx.begin(), params.onlyAgenciesIdx.end(), trip->agencyIdx) == params.onlyAgenciesIdx.end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && params.exceptServicesIdx.size() > 0)
        {
          if (std::find(params.exceptServicesIdx.begin(), params.exceptServicesIdx.end(), trip->serviceIdx) != params.exceptServicesIdx.end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && params.exceptLinesIdx.size() > 0)
        {
          if (std::find(params.exceptLinesIdx.begin(), params.exceptLinesIdx.end(), trip->lineIdx) != params.exceptLinesIdx.end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && params.exceptNodesIdx.size() > 0)
        {
          if (std::find(params.exceptNodesIdx.begin(), params.exceptNodesIdx.end(), trip->modeIdx) != params.exceptNodesIdx.end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && params.exceptModesIdx.size() > 0)
        {
          if (std::find(params.exceptModesIdx.begin(), params.exceptModesIdx.end(), trip->modeIdx) != params.exceptModesIdx.end())
          {
            tripsEnabled[i] = -1;
          }
        }

        if (tripsEnabled[i] == 1 && params.exceptAgenciesIdx.size() > 0)
        {
          if (std::find(params.exceptAgenciesIdx.begin(), params.exceptAgenciesIdx.end(), trip->agencyIdx) != params.exceptAgenciesIdx.end())
          {
            tripsEnabled[i] = -1;
          }
        }
        i++;
      }
    }

    
    if (params.debugDisplay)
      benchmarking["reset"] += algorithmCalculationTime.getEpoch() - benchmarkingStart;


    if (params.debugDisplay)
      std::cerr << "-- filter trips -- " << algorithmCalculationTime.getDurationMicrosecondsNoStop() - calculationTime << " microseconds\n";

    calculationTime = algorithmCalculationTime.getDurationMicrosecondsNoStop();

  }

}

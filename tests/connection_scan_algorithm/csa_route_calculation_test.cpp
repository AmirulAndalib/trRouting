#include <errno.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "gtest/gtest.h"
#include "calculator.hpp"
#include "csa_test_base.hpp"
#include "csa_route_calculation_test.hpp"
#include "constants.hpp"

// TODO:
// Test transferable mode, it has separate code path
// Test optimization cases

/**
 * This file covers use cases of tests without alternatives or transfer,
 * using the RouteParameters parameter values
 * */

void SingleRouteCalculationFixtureTests::SetUp()
{
    BaseCsaFixtureTests::SetUp();

    scenario = calculator.scenarios[calculator.scenarioIndexesByUuid[scenarioUuid]].get();
}

// Test from south node to the opposite direction
TEST_F(SingleRouteCalculationFixtureTests, NoRoutingBecauseNoPath)
{
    TrRouting::RouteParameters testParameters = TrRouting::RouteParameters(
        std::make_unique<TrRouting::Point>(45.5269, -73.58912),
        std::make_unique<TrRouting::Point>(45.52184, -73.57817),
        *scenario,
        getTimeInSeconds(9, 50),
        DEFAULT_MIN_WAITING_TIME,
        DEFAULT_MAX_TOTAL_TIME,
        DEFAULT_MAX_ACCESS_TRAVEL_TIME,
        DEFAULT_MAX_EGRESS_TRAVEL_TIME,
        DEFAULT_MAX_TRANSFER_TRAVEL_TIME,
        DEFAULT_FIRST_WAITING_TIME,
        false,
        true
    );

    try {
        calculateOd(testParameters);
        FAIL() << "Expected TrRouting::NoRoutingFoundException, no exception thrown";
    } catch (TrRouting::NoRoutingFoundException const & e) {
        assertNoRouting(e, TrRouting::NoRoutingFoundException::NoRoutingReason::NO_ROUTING_FOUND);
    } catch(...) {
        FAIL() << "Expected TrRouting::NoRoutingFoundException, another type was thrown";
    }
}

// Test origin and destination far from network
TEST_F(SingleRouteCalculationFixtureTests, NoRoutingBecauseNoNode)
{
    TrRouting::RouteParameters testParameters = TrRouting::RouteParameters(
        std::make_unique<TrRouting::Point>(45.5349, -73.55478),
        std::make_unique<TrRouting::Point>(45.5155, -73.56797),
        *scenario,
        getTimeInSeconds(9, 50),
        DEFAULT_MIN_WAITING_TIME,
        DEFAULT_MAX_TOTAL_TIME,
        DEFAULT_MAX_ACCESS_TRAVEL_TIME,
        DEFAULT_MAX_EGRESS_TRAVEL_TIME,
        DEFAULT_MAX_TRANSFER_TRAVEL_TIME,
        DEFAULT_FIRST_WAITING_TIME,
        false,
        true
    );

    try {
        calculateOd(testParameters);
        FAIL() << "Expected TrRouting::NoRoutingFoundException, no exception thrown";
    } catch (TrRouting::NoRoutingFoundException const & e) {
        assertNoRouting(e, TrRouting::NoRoutingFoundException::NoRoutingReason::NO_ROUTING_FOUND);
    } catch(...) {
        FAIL() << "Expected TrRouting::NoRoutingFoundException, another type was thrown";
    }
}

// Test from first to second node of SN path, but before the time of the trip (6:50)
TEST_F(SingleRouteCalculationFixtureTests, NoRoutingBecauseTooEarly)
{
    TrRouting::RouteParameters testParameters = TrRouting::RouteParameters(
        std::make_unique<TrRouting::Point>(45.5269, -73.58912),
        std::make_unique<TrRouting::Point>(45.53258, -73.60196),
        *scenario,
        getTimeInSeconds(6, 50),
        DEFAULT_MIN_WAITING_TIME,
        DEFAULT_MAX_TOTAL_TIME,
        DEFAULT_MAX_ACCESS_TRAVEL_TIME,
        DEFAULT_MAX_EGRESS_TRAVEL_TIME,
        DEFAULT_MAX_TRANSFER_TRAVEL_TIME,
        DEFAULT_FIRST_WAITING_TIME,
        false,
        true
    );

    try {
        calculateOd(testParameters);
        FAIL() << "Expected TrRouting::NoRoutingFoundException, no exception thrown";
    } catch (TrRouting::NoRoutingFoundException const & e) {
        assertNoRouting(e, TrRouting::NoRoutingFoundException::NoRoutingReason::NO_ROUTING_FOUND);
    } catch(...) {
        FAIL() << "Expected TrRouting::NoRoutingFoundException, another type was thrown";
    }
    
}

// Test from first to second node of SN path
TEST_F(SingleRouteCalculationFixtureTests, NodeToNodeCalculation)
{
    int departureTime = getTimeInSeconds(9, 50);
    int travelTimeInVehicle = 210;
    int expectedTransitDepartureTime = getTimeInSeconds(10);

    TrRouting::RouteParameters testParameters = TrRouting::RouteParameters(
        std::make_unique<TrRouting::Point>(45.5269, -73.58912),
        std::make_unique<TrRouting::Point>(45.53258, -73.60196),
        *scenario,
        departureTime,
        DEFAULT_MIN_WAITING_TIME,
        DEFAULT_MAX_TOTAL_TIME,
        DEFAULT_MAX_ACCESS_TRAVEL_TIME,
        DEFAULT_MAX_EGRESS_TRAVEL_TIME,
        DEFAULT_MAX_TRANSFER_TRAVEL_TIME,
        DEFAULT_FIRST_WAITING_TIME,
        false,
        true
    );

    std::unique_ptr<TrRouting::RoutingResultNew> result = calculateOd(testParameters);
    assertSuccessResults(*result.get(),
        departureTime,
        expectedTransitDepartureTime,
        travelTimeInVehicle);
}

// Test from OD With access/egress, origin is further south of South2, in the line axis, destination slightly north-west of midpoint
// Use default parameters for non-mandatory ones
TEST_F(SingleRouteCalculationFixtureTests, SimpleODCalculationDepartureTime)
{
    int departureTime = getTimeInSeconds(9, 45);
    int travelTimeInVehicle = 420;
    // This is where mocking would be interesting. Those were taken from the first run of the test
    int accessTime = 469;
    int egressTime = 138;
    int expectedTransitDepartureTime = getTimeInSeconds(10);

    TrRouting::RouteParameters testParameters = TrRouting::RouteParameters(
        std::make_unique<TrRouting::Point>(45.5242, -73.5817),
        std::make_unique<TrRouting::Point>(45.54, -73.6146),
        *scenario,
        departureTime,
        DEFAULT_MIN_WAITING_TIME,
        DEFAULT_MAX_TOTAL_TIME,
        DEFAULT_MAX_ACCESS_TRAVEL_TIME,
        DEFAULT_MAX_EGRESS_TRAVEL_TIME,
        DEFAULT_MAX_TRANSFER_TRAVEL_TIME,
        DEFAULT_FIRST_WAITING_TIME,
        false,
        true
    );

    std::unique_ptr<TrRouting::RoutingResultNew> result = calculateOd(testParameters);
    assertSuccessResults(*result.get(),
        departureTime,
        expectedTransitDepartureTime,
        travelTimeInVehicle,
        accessTime,
        egressTime);
}

// Same as SimpleODCalculationDepartureTime, but with an arrival time
TEST_F(SingleRouteCalculationFixtureTests, SimpleODCalculationArrivalTime)
{
    int arrivalTime = getTimeInSeconds(11, 15);
    int travelTimeInVehicle = 420;
    // This is where mocking would be interesting. Those were taken from the first run of the test
    int accessTime = 469;
    int egressTime = 138;
    int expectedTransitDepartureTime = getTimeInSeconds(11);

    TrRouting::RouteParameters testParameters = TrRouting::RouteParameters(
        std::make_unique<TrRouting::Point>(45.5242, -73.5817),
        std::make_unique<TrRouting::Point>(45.54, -73.6146),
        *scenario,
        arrivalTime,
        DEFAULT_MIN_WAITING_TIME,
        DEFAULT_MAX_TOTAL_TIME,
        DEFAULT_MAX_ACCESS_TRAVEL_TIME,
        DEFAULT_MAX_EGRESS_TRAVEL_TIME,
        DEFAULT_MAX_TRANSFER_TRAVEL_TIME,
        DEFAULT_FIRST_WAITING_TIME,
        false,
        false
    );

    std::unique_ptr<TrRouting::RoutingResultNew> result = calculateOd(testParameters);
    assertSuccessResults(*result.get(),
        -1,
        expectedTransitDepartureTime,
        travelTimeInVehicle,
        accessTime,
        egressTime);
}

// Test from OD With access/egress, origin is further south of South2, in the line axis, destination slightly north-west of midpoint
// Specify each parameter with a value that allows the result
TEST_F(SingleRouteCalculationFixtureTests, SimpleODCalculationWithAllParams)
{
    int departureTime = getTimeInSeconds(9, 45);
    int minWaitingTime = 240;
    int travelTimeInVehicle = 420;
    // This is where mocking would be interesting. Those were taken from the first run of the test
    int accessTime = 469;
    int egressTime = 138;
    int expectedTransitDepartureTime = getTimeInSeconds(10);

    TrRouting::RouteParameters testParameters = TrRouting::RouteParameters(
        std::make_unique<TrRouting::Point>(45.5242, -73.5817),
        std::make_unique<TrRouting::Point>(45.54, -73.6146),
        *scenario,
        departureTime,
        minWaitingTime,
        expectedTransitDepartureTime - departureTime + egressTime + travelTimeInVehicle + 5,
        accessTime + 5,
        egressTime + 5,
        DEFAULT_MAX_TRANSFER_TRAVEL_TIME,
        expectedTransitDepartureTime - departureTime,
        false,
        true
    );

    std::unique_ptr<TrRouting::RoutingResultNew> result = calculateOd(testParameters);
    assertSuccessResults(*result.get(),
        departureTime,
        expectedTransitDepartureTime,
        travelTimeInVehicle,
        accessTime,
        egressTime,
        0,
        minWaitingTime,
        minWaitingTime);
}

// Same as SimpleODCalculation, but with max_access_travel_time lower than access time
TEST_F(SingleRouteCalculationFixtureTests, NoRoutingAccessTimeLimit)
{
    int departureTime = getTimeInSeconds(9, 45);
    // This is where mocking would be interesting. Those were taken from the first run of the test
    int accessTime = 469;

    TrRouting::RouteParameters testParameters = TrRouting::RouteParameters(
        std::make_unique<TrRouting::Point>(45.5242, -73.5817),
        std::make_unique<TrRouting::Point>(45.54, -73.6146),
        *scenario,
        departureTime,
        DEFAULT_MIN_WAITING_TIME,
        DEFAULT_MAX_TOTAL_TIME,
        accessTime - 5,
        DEFAULT_MAX_EGRESS_TRAVEL_TIME,
        DEFAULT_MAX_TRANSFER_TRAVEL_TIME,
        DEFAULT_FIRST_WAITING_TIME,
        false,
        true
    );

    try {
        calculateOd(testParameters);
        FAIL() << "Expected TrRouting::NoRoutingFoundException, no exception thrown";
    } catch (TrRouting::NoRoutingFoundException const & e) {
        assertNoRouting(e, TrRouting::NoRoutingFoundException::NoRoutingReason::NO_ROUTING_FOUND);
    } catch(...) {
        FAIL() << "Expected TrRouting::NoRoutingFoundException, another type was thrown";
    }
}

// Same as SimpleODCalculation, but with max_egress_travel_time lower than egress time
TEST_F(SingleRouteCalculationFixtureTests, NoRoutingEgressTimeLimit)
{
    int departureTime = getTimeInSeconds(9, 45);
    // This is where mocking would be interesting. Those were taken from the first run of the test
    int egressTime = 138;

    TrRouting::RouteParameters testParameters = TrRouting::RouteParameters(
        std::make_unique<TrRouting::Point>(45.5242, -73.5817),
        std::make_unique<TrRouting::Point>(45.54, -73.6146),
        *scenario,
        departureTime,
        DEFAULT_MIN_WAITING_TIME,
        DEFAULT_MAX_TOTAL_TIME,
        DEFAULT_MAX_ACCESS_TRAVEL_TIME,
        egressTime - 5,
        DEFAULT_MAX_TRANSFER_TRAVEL_TIME,
        DEFAULT_FIRST_WAITING_TIME,
        false,
        true
    );

    try {
        calculateOd(testParameters);
        FAIL() << "Expected TrRouting::NoRoutingFoundException, no exception thrown";
    } catch (TrRouting::NoRoutingFoundException const & e) {
        assertNoRouting(e, TrRouting::NoRoutingFoundException::NoRoutingReason::NO_ROUTING_FOUND);
    } catch(...) {
        FAIL() << "Expected TrRouting::NoRoutingFoundException, another type was thrown";
    }
}

// Same as SimpleODCalculation, but with max_first_waiting_time lower than should be
TEST_F(SingleRouteCalculationFixtureTests, NoRoutingMaxFirstWaitingTime)
{
    int departureTime = getTimeInSeconds(9, 45);
    int travelTimeInVehicle = 420;
    // This is where mocking would be interesting. Those were taken from the first run of the test
    int accessTime = 469;
    int egressTime = 138;
    int expectedTransitDepartureTime = getTimeInSeconds(10);

    TrRouting::RouteParameters testParameters = TrRouting::RouteParameters(
        std::make_unique<TrRouting::Point>(45.5242, -73.5817),
        std::make_unique<TrRouting::Point>(45.54, -73.6146),
        *scenario,
        departureTime,
        DEFAULT_MIN_WAITING_TIME,
        DEFAULT_MAX_TOTAL_TIME,
        DEFAULT_MAX_ACCESS_TRAVEL_TIME,
        DEFAULT_MAX_EGRESS_TRAVEL_TIME,
        DEFAULT_MAX_TRANSFER_TRAVEL_TIME,
        expectedTransitDepartureTime - accessTime - departureTime - 5,
        false,
        true
    );

    try {
        calculateOd(testParameters);
        FAIL() << "Expected TrRouting::NoRoutingFoundException, no exception thrown";
    } catch (TrRouting::NoRoutingFoundException const & e) {
        assertNoRouting(e, TrRouting::NoRoutingFoundException::NoRoutingReason::NO_ROUTING_FOUND);
    } catch(...) {
        FAIL() << "Expected TrRouting::NoRoutingFoundException, another type was thrown";
    }
}

// Same as SimpleODCalculation, but with min_waiting_time higher than available
TEST_F(SingleRouteCalculationFixtureTests, NoRoutingMinWaitingTime)
{
    int departureTime = getTimeInSeconds(9, 45);
    int travelTimeInVehicle = 420;
    // This is where mocking would be interesting. Those were taken from the first run of the test
    int accessTime = 469;
    int egressTime = 138;
    int expectedTransitDepartureTime = getTimeInSeconds(10);

    TrRouting::RouteParameters testParameters = TrRouting::RouteParameters(
        std::make_unique<TrRouting::Point>(45.5242, -73.5817),
        std::make_unique<TrRouting::Point>(45.54, -73.6146),
        *scenario,
        departureTime,
        expectedTransitDepartureTime - departureTime,
        DEFAULT_MAX_TOTAL_TIME,
        DEFAULT_MAX_ACCESS_TRAVEL_TIME,
        DEFAULT_MAX_EGRESS_TRAVEL_TIME,
        DEFAULT_MAX_TRANSFER_TRAVEL_TIME,
        DEFAULT_FIRST_WAITING_TIME,
        false,
        true
    );

    try {
        calculateOd(testParameters);
        FAIL() << "Expected TrRouting::NoRoutingFoundException, no exception thrown";
    } catch (TrRouting::NoRoutingFoundException const & e) {
        assertNoRouting(e, TrRouting::NoRoutingFoundException::NoRoutingReason::NO_ROUTING_FOUND);
    } catch(...) {
        FAIL() << "Expected TrRouting::NoRoutingFoundException, another type was thrown";
    }
}

// Same as SimpleODCalculation, but with max_travel_time lower than should be
TEST_F(SingleRouteCalculationFixtureTests, NoRoutingTravelTime)
{
    int departureTime = getTimeInSeconds(9, 45);
    int travelTimeInVehicle = 420;
    // This is where mocking would be interesting. Those were taken from the first run of the test
    int accessTime = 469;
    int egressTime = 138;
    int expectedTransitDepartureTime = getTimeInSeconds(10);

    TrRouting::RouteParameters testParameters = TrRouting::RouteParameters(
        std::make_unique<TrRouting::Point>(45.5242, -73.5817),
        std::make_unique<TrRouting::Point>(45.54, -73.6146),
        *scenario,
        departureTime,
        DEFAULT_MIN_WAITING_TIME,
        travelTimeInVehicle,
        DEFAULT_MAX_ACCESS_TRAVEL_TIME,
        DEFAULT_MAX_EGRESS_TRAVEL_TIME,
        DEFAULT_MAX_TRANSFER_TRAVEL_TIME,
        DEFAULT_FIRST_WAITING_TIME,
        false,
        true
    );

    try {
        calculateOd(testParameters);
        FAIL() << "Expected TrRouting::NoRoutingFoundException, no exception thrown";
    } catch (TrRouting::NoRoutingFoundException const & e) {
        assertNoRouting(e, TrRouting::NoRoutingFoundException::NoRoutingReason::NO_ROUTING_FOUND);
    } catch(...) {
        FAIL() << "Expected TrRouting::NoRoutingFoundException, another type was thrown";
    }
}

std::unique_ptr<TrRouting::RoutingResultNew> SingleRouteCalculationFixtureTests::calculateOd(TrRouting::RouteParameters& parameters)
{
    // TODO: This needs to be called to set some default values that are still part of the global parameters
    calculator.params.setDefaultValues();
    calculator.params.birdDistanceAccessibilityEnabled = true;

    // TODO Shouldn't need to do this, but we do for now, benchmark needs to be started
    calculator.algorithmCalculationTime.start();
    calculator.benchmarking.clear();
    return calculator.calculate(parameters);
}

#ifndef __LIB_FUEL_CELL__
#define __LIB_FUEL_CELL__

#include <map>
#include "lib_util.h"

const double BTU_PER_KWH = 3412.14163;

/**
* \class FuelCell
*
* \brief
*
*  The FuelCell class provides the underlying data and methods required to model fuel cell technology in SAM.
*/
class FuelCell
{
public:
	/// Default FuelCell constructor
	FuelCell();

	/// Construct FuelCell with arguments
	FuelCell(double unitPowerMax_kW, double unitPowerMin_kW, double startup_hours, 
		double dynamicResponseUp_kWperMin, double dynamicResponseDown_kWperMin,
		double degradation_kWperHour, double degradationRestart_kW, 
		size_t replacement_option, double replacement_percent, std::vector<size_t> replacementSchedule,
		util::matrix_t<size_t> shutdownTable,
		util::matrix_t<double> efficiencyTable,
		double lowerHeatingValue_BtuPerFt3, double higherHeatingValue_BtuPerFt3, double availableFuel_MCf,
		int shutdownOption,  double dt_hour);

	/// Default destructor
	~FuelCell();

	/// Copy Constructor
	FuelCell(const FuelCell &fuelCell);

	/// Run for single time step
	void runSingleTimeStep(double power_kW);

	/// Return true if operating
	bool isRunning();

	/// Get original max power kW
	double getMaxPowerOriginal();

	/// Get fuel cell max power kW
	double getMaxPower();

	/// Return the final power kW
	double getPower();

	/// Return the final heat kW
	double getPowerThermal();

	/// Return the fuel consumption in MCF
	double getFuelConsumption();

	/// Get efficiency percent
	double getElectricalEfficiency();

	/// Get heat recovery percent
	double getHeatRecoveryEfficiency();

	/// Return the available fuel in MCF
	double getAvailableFuel();

	/// Return the number of replacements
	int getTotalReplacements();

	/// Update replacement options (for testing)
	void setReplacementOption(size_t replacementOption);
	void setReplacementCapacity(double replacement_percent);

	/// Update restart degradation
	void setDegradationRestartkW(double);

	/// Update degradation (for testing)
	void setDegradationkWPerHour(double degradation_kWPerHour);

	/// Update restart degradation
	void setScheduledShutdowns(util::matrix_t<size_t> shutdowns);

	/// Set startup hours
	void setStartupHours(double startup_hours);

	/// Calculate fuel consumption at percent load
	void calculateEfficiencyCurve(double percent);

	/// Shutdown option enumerations
	enum FC_SHUTDOWN_OPTION { SHUTDOWN, IDLE };

	/// Fuel cell replacement options
	enum FC_REPLACEMENT_OPTION { NONE, REPLACE_AT_CAPACITY, REPLACE_ON_SCHEDULE };

protected:

	enum FC_EFFICIENCY_COLUMN { PERCENT_MAX, PRECENT_ELECTRICAL_EFFICIENCY, PERCENT_HEAT_RECOVERY };
	enum FC_SHUTDOWN_COLUMN {HOUR, DURATION};

	/// Calculate time
	void calculateTime();

	/// Apply degradation 
	void applyDegradation();

	/// Apply degradation 
	void applyEfficiency();

	/// interpolate map
	double interpolateMap(double key, std::map<double, double>);

	/// Check shutdown
	bool checkShutdown();

	/// Check Min Turndown
	void checkMinTurndown();

	/// Check Max Limit
	void checkMaxLimit();

	/// Check Available Fuel
	void checkAvailableFuel();

	/// Get fuel cell power given the requested power signal
	double getPowerResponse(double power);

	/// Return percentage based on requested power
	double getPercentLoad();

	// Initialize calculated
	void init();

	// input values
	double dt_hour;
	double m_unitPowerMax_kW;
	double m_unitPowerMin_kW;
	double m_startup_hours;

	double m_dynamicResponseUp_kWperHour;
	double m_dynamicResponseDown_kWperHour;
	
	double m_degradation_kWperHour;
	double m_degradationRestart_kW;
	util::matrix_t<size_t> m_scheduledShutdowns;

	size_t m_replacementOption;
	double m_replacement_percent;
	std::vector<size_t> m_replacementSchedule;
	util::matrix_t<double> m_efficiencyTable;
	double m_lowerHeatingValue_BtuPerFt3;
	double m_higherHeatingValue_BtuPerFt3;
	double m_availableFuel_MCf;
	int m_shutdownOption;

	// calculated
	bool m_startedUp;
	double m_hoursSinceStart;
	double m_powerMax_kW;  // Maximum power after degradation
	double m_powerThermal_kW;
	double m_power_kW;
	double m_powerPrevious_kW;
	double m_fuelConsumed_MCf;
	double m_efficiency_percent;
	double m_heatRecovery_percent;
	int m_replacementCount;
	std::map<double, double> m_fuelConsumptionMap_MCf;
	std::map<double, double> m_efficiencyMap;
	std::map<double, double> m_heatRecoveryMap;
	double m_hour;
	size_t m_year;

};

#endif __LIB_FUEL_CELL__
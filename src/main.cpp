#include "eae_firmware/CanBus.h"
#include "eae_firmware/CliOptions.h"
#include "eae_firmware/FirmwareController.h"

#include <iomanip>
#include <iostream>
#include <stdexcept>

using namespace eae::firmware;

namespace {

// First-order lumped thermal model of the coolant loop. Heat load pushes the
// temperature up; passive loss toward ambient and pump/fan effort pull it
// down. The coefficients are not from a real datasheet, they were picked so
// the default 90-step run shows a full approach to the target temperature.
double updateThermalPlant(double temperatureC,
                          double ambientTempC,
                          double heatLoadKw,
                          double pumpPercent,
                          double fanPercent,
                          double dtSeconds) {
    const double passiveCooling = 0.015 * (temperatureC - ambientTempC);
    const double activeCooling = 0.0018 * pumpPercent + 0.0012 * fanPercent;
    const double heatRise = 0.055 * heatLoadKw;
    const double temperatureRate = heatRise - passiveCooling - activeCooling;
    return temperatureC + temperatureRate * dtSeconds;
}

void publishInputs(CanBus& bus,
                   double temperatureC,
                   bool coolantOk,
                   bool ignitionOn,
                   bool pumpFault) {
    bus.receive({
        CanIds::TemperatureStatus,
        "TemperatureStatus",
        {
            {"coolant_temp_c", temperatureC},
            {"sensor_valid", 1.0},
        },
    });

    bus.receive({
        CanIds::CoolantStatus,
        "CoolantStatus",
        {
            {"coolant_level_ok", coolantOk ? 1.0 : 0.0},
        },
    });

    bus.receive({
        CanIds::DriverCommand,
        "DriverCommand",
        {
            {"ignition_on", ignitionOn ? 1.0 : 0.0},
        },
    });

    bus.receive({
        CanIds::PumpStatus,
        "PumpStatus",
        {
            {"pump_fault", pumpFault ? 1.0 : 0.0},
        },
    });
}

}  // namespace

int main(int argc, char* argv[]) {
    try {
        const CliOptions options = parseCliOptions(argc, argv);
        CanBus bus;
        FirmwareController controller({options.targetTempC});

        double temperatureC = options.initialTempC;

        std::cout << "EAE Section 7.1 firmware simulation\n";
        std::cout << "target=" << options.targetTempC << "C "
                  << "initial=" << options.initialTempC << "C "
                  << "heat_load=" << options.heatLoadKw << "kW\n\n";

        std::cout << std::setw(5) << "step"
                  << std::setw(10) << "temp_C"
                  << std::setw(10) << "state"
                  << std::setw(10) << "pump_%"
                  << std::setw(10) << "fan_%"
                  << std::setw(10) << "derate"
                  << '\n';

        for (int step = 0; step < options.steps; ++step) {
            publishInputs(bus, temperatureC, options.coolantOk, options.ignitionOn, options.pumpFault);
            const ControlOutputs outputs = controller.step(bus, options.dtSeconds);

            std::cout << std::setw(5) << step
                      << std::setw(10) << std::fixed << std::setprecision(1) << temperatureC
                      << std::setw(10) << toString(outputs.state)
                      << std::setw(10) << std::setprecision(1) << outputs.pumpPercent
                      << std::setw(10) << std::setprecision(1) << outputs.fanPercent
                      << std::setw(10) << (outputs.derateRequested ? "yes" : "no")
                      << '\n';

            temperatureC = updateThermalPlant(
                temperatureC,
                options.ambientTempC,
                options.heatLoadKw,
                outputs.pumpPercent,
                outputs.fanPercent,
                options.dtSeconds);

            bus.clearTransmitted();
        }

        return 0;
    } catch (const std::invalid_argument& error) {
        std::cerr << error.what() << '\n';
        return 2;
    } catch (const std::exception& error) {
        std::cerr << "Fatal error: " << error.what() << '\n';
        return 1;
    }
}

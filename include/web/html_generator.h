#ifndef HTML_GENERATOR_H
#define HTML_GENERATOR_H

#include <Arduino.h>
#include "thermostat_state.h"
#include "interfaces/config_interface.h"
#include "interfaces/control_interface.h"

class HtmlGenerator {
public:
    static String generatePage(
        ThermostatState* state,
        ConfigInterface* config,
        ControlInterface* control = nullptr
    );

private:
    static String generateHeader();
    static String generateNavigation();
    static String generateStatusSection(ThermostatState* state);
    static String generateControlSection(ThermostatState* state);
    static String generateConfigSection(ConfigInterface* config);
    static String generatePIDSection(ControlInterface* control);
    static String generateFooter();
    static String generateStyles();
    static String generateScripts();
};

#endif // HTML_GENERATOR_H 
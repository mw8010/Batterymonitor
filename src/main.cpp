#include <Arduino.h>
#include <memory>
#include "sensesp/signalk/signalk_output.h"
#include "sensesp/transforms/linear.h"
#include "sensesp/transforms/curveinterpolator.h"
#include "sensesp/ui/config_item.h"
#include "sensesp/ui/ui_controls.h"
#include "sensesp_app_builder.h"
#include "sensesp_onewire/onewire_temperature.h"
#include "sensesp/signalk/signalk_metadata.h"
#include "INA226.h"

using namespace sensesp;
using namespace sensesp::onewire;

// Setze die INA226-Adresse
INA226 INA0(0x40);

// Setze Wifi-Netzwerkdaten
const char* hostname = "BatteryShuntESP";

// Variable Temperatursensor
uint8_t pin = 2; // Pin, an dem der Temperatursensor angeschlossen ist

// Abfrageintervall für die Sensoren
uint read_delay = 1000; // Zeit zwischen den Messungen

// Globale Variablen für Batterietyp, Spannung und AH
String battery_type = "LiFePO4"; // Standardtyp der Batterie
String battery_type_config_path = "/config/battery_type";
float battery_voltage = 12; // Standardspannung der Batterie in V
String battery_voltage_config_path = "/config/battery_voltage";
float battery_capacity = 100; // Standardkapazität der Batterie in Ah
String battery_capacity_config_path = "/config/battery_ah";

// Konfigurationspfade für den INA226
float max_current = 500.0; // Maximalstrom in A
String max_current_config_path = "/config/max_current";
float shunt_voltage = 75.0; // Shunt-Spannung in mV
String shunt_voltage_config_path = "/config/shunt_voltage";
float shunt_resistance; // Shunt-Widerstand in Ohm (Wert wird berechnet)
String shunt_resistance_config_path = "/config/shunt_resistance";

// Definiere Variable für den Batterienamen
String battery_name = "Battery1"; // Standardname für die Batterie
String battery_name_config_path = "/config/battery_name";

// Erstelle den vollständigen SignalK Pfad durch Konkatenation
String sk_path_current = "electrical.batteries." + battery_name + ".current";
String sk_path_voltage = "electrical.batteries." + battery_name + ".voltage";
String sk_path_power = "electrical.batteries." + battery_name + ".power";
String sk_path_soc = "electrical.batteries." + battery_name + ".stateOfCharge";
String sk_path_ah = "electrical.batteries." + battery_name + ".ampHours";
String sk_path_time_to_go = "electrical.batteries." + battery_name + ".timeToGo";
String sk_path_shunt_over_voltage = "electrical.batteries." + battery_name + ".alerts.shuntOverVoltage";
String sk_path_bus_over_voltage = "electrical.batteries." + battery_name + ".alerts.busOverVoltage";
String sk_path_power_over_limit = "electrical.batteries." + battery_name + ".alerts.powerOverLimit";
String temp_one_wire_path = "/Battery Temperature/" + battery_name + "/oneWire";
String temp_linear_path = "/Battery Temperature/" + battery_name + "/linear";
String temp_sk_path = "electrical.batteries." + battery_name + ".temperature";

//Definiere die Metadaten für die SignalK-Objekte
SKMetadata* metadata_current = new SKMetadata("A", "Current", "Current of the battery", "A");
SKMetadata* metadata_voltage = new SKMetadata("V", "Voltage", "Voltage of the battery", "V");
SKMetadata* metadata_power = new SKMetadata("W", "Power", "Power of the battery", "W");
SKMetadata* metadata_soc = new SKMetadata("%", "State of Charge", "State of Charge of the battery", "%");
SKMetadata* metadata_ah = new SKMetadata("Ah", "Ampere Hours", "Ampere Hours used of the battery", "Ah");
SKMetadata* metadata_time_to_go = new SKMetadata("s", "Time to Go", "Time to go of the battery", "s");
SKMetadata* metadata_shunt_over_voltage = new SKMetadata("Alert", "Shunt Over Voltage", "Shunt Over Voltage Alert", "Alert");
SKMetadata* metadata_bus_over_voltage = new SKMetadata("Alert", "Bus Over Voltage", "Bus Over Voltage Alert", "Alert");
SKMetadata* metadata_power_over_limit = new SKMetadata("Alert", "Power Over Limit", "Power Over Limit Alert", "Alert");
SKMetadata* metadata_temp = new SKMetadata("K", "Temperature", "Temperature of the battery", "K");

// Definiere Variablen für die Alarmmeldungen
SKOutputString* alert_shunt_over_voltage;
SKOutputString* alert_bus_over_voltage;
SKOutputString* alert_power_over_limit;

// Variable für verbrauchte Amperestunden
float amp_hours_used = 0.0;

// Berechne den Shunt-Widerstand basierend auf dem maximalen Strom und der Spannung
float calculateShuntResistance(float max_current, float shunt_voltage) {
    return shunt_voltage / max_current;
}

// SOCA-Interpreter Klasse
class SoCAInterpreter : public CurveInterpolator {
    public:
        SoCAInterpreter(String battery_type, float system_voltage, String config_path = "")
            : CurveInterpolator(NULL, config_path) {

        clear_samples();
        if (system_voltage == 12 && battery_type == "LiFePO4") {
            add_sample(CurveInterpolator::Sample(10.0, 0.0)); // 0%
            add_sample(CurveInterpolator::Sample(11.4, 0.1)); // 10%
            add_sample(CurveInterpolator::Sample(11.6, 0.2)); // 20%
            add_sample(CurveInterpolator::Sample(11.8, 0.3)); // 30%
            add_sample(CurveInterpolator::Sample(12.0, 0.4)); // 40%
            add_sample(CurveInterpolator::Sample(12.4, 0.5)); // 50%
            add_sample(CurveInterpolator::Sample(12.8, 0.6)); // 60%
            add_sample(CurveInterpolator::Sample(13.2, 0.7)); // 70%
            add_sample(CurveInterpolator::Sample(13.6, 0.8)); // 80%
            add_sample(CurveInterpolator::Sample(14.0, 0.9)); // 90%
            add_sample(CurveInterpolator::Sample(14.6, 1.0)); // 100%
        } else if (system_voltage == 24 && battery_type == "LiFePO4") {
            add_sample(CurveInterpolator::Sample(20.0, 0.0)); // 0%
            add_sample(CurveInterpolator::Sample(22.8, 0.1)); // 10%
            add_sample(CurveInterpolator::Sample(23.2, 0.2)); // 20%
            add_sample(CurveInterpolator::Sample(23.6, 0.3)); // 30%
            add_sample(CurveInterpolator::Sample(24.0, 0.4)); // 40%
            add_sample(CurveInterpolator::Sample(24.8, 0.5)); // 50%
            add_sample(CurveInterpolator::Sample(25.6, 0.6)); // 60%
            add_sample(CurveInterpolator::Sample(26.4, 0.7)); // 70%
            add_sample(CurveInterpolator::Sample(27.2, 0.8)); // 80%
            add_sample(CurveInterpolator::Sample(28.0, 0.9)); // 90%
            add_sample(CurveInterpolator::Sample(29.2, 1.0)); // 100%
        } else if (system_voltage == 48 && battery_type == "LiFePO4") {
            add_sample(CurveInterpolator::Sample(48.0, 0.0));  // 0% (3.00V/Zelle)
            add_sample(CurveInterpolator::Sample(48.8, 0.1));  // 10%
            add_sample(CurveInterpolator::Sample(49.6, 0.2));  // 20%
            add_sample(CurveInterpolator::Sample(50.4, 0.3));  // 30%
            add_sample(CurveInterpolator::Sample(51.2, 0.4));  // 40%
            add_sample(CurveInterpolator::Sample(52.0, 0.5));  // 50%
            add_sample(CurveInterpolator::Sample(52.8, 0.6));  // 60%
            add_sample(CurveInterpolator::Sample(53.6, 0.7));  // 70%
            add_sample(CurveInterpolator::Sample(54.0, 0.8));  // 80%
            add_sample(CurveInterpolator::Sample(54.2, 0.9));  // 90%
            add_sample(CurveInterpolator::Sample(54.4, 1.0));  // 100% (3.40V/Zelle)
        } else if (system_voltage == 12 && battery_type == "AGM") {
            add_sample(CurveInterpolator::Sample(10.5, 0.0)); // 0%
            add_sample(CurveInterpolator::Sample(11.0, 0.1)); // 10%
            add_sample(CurveInterpolator::Sample(11.2, 0.2)); // 20%
            add_sample(CurveInterpolator::Sample(11.4, 0.3)); // 30%
            add_sample(CurveInterpolator::Sample(11.6, 0.4)); // 40%
            add_sample(CurveInterpolator::Sample(11.8, 0.5)); // 50%
            add_sample(CurveInterpolator::Sample(12.0, 0.6)); // 60%
            add_sample(CurveInterpolator::Sample(12.2, 0.7)); // 70%
            add_sample(CurveInterpolator::Sample(12.4, 0.8)); // 80%
            add_sample(CurveInterpolator::Sample(12.6, 0.9)); // 90%
            add_sample(CurveInterpolator::Sample(12.8, 1.0)); // 100%
        } else if (system_voltage == 24 && battery_type == "AGM") {
            add_sample(CurveInterpolator::Sample(21.0, 0.0)); // 0%
            add_sample(CurveInterpolator::Sample(22.0, 0.1)); // 10%
            add_sample(CurveInterpolator::Sample(22.4, 0.2)); // 20%
            add_sample(CurveInterpolator::Sample(22.8, 0.3)); // 30%
            add_sample(CurveInterpolator::Sample(23.2, 0.4)); // 40%
            add_sample(CurveInterpolator::Sample(23.6, 0.5)); // 50%
            add_sample(CurveInterpolator::Sample(24.0, 0.6)); // 60%
            add_sample(CurveInterpolator::Sample(24.4, 0.7)); // 70%
            add_sample(CurveInterpolator::Sample(24.8, 0.8)); // 80%
            add_sample(CurveInterpolator::Sample(25.2, 0.9)); // 90%
            add_sample(CurveInterpolator::Sample(25.6, 1.0)); // 100%
        } else if (system_voltage == 12 && battery_type == "Gel") {
            add_sample(CurveInterpolator::Sample(10.5, 0.0)); // 0%
            add_sample(CurveInterpolator::Sample(11.4, 0.1)); // 10%
            add_sample(CurveInterpolator::Sample(11.6, 0.2)); // 20%
            add_sample(CurveInterpolator::Sample(11.75, 0.3));// 30%
            add_sample(CurveInterpolator::Sample(11.9, 0.4)); // 40%
            add_sample(CurveInterpolator::Sample(12.0, 0.5)); // 50%
            add_sample(CurveInterpolator::Sample(12.2, 0.6)); // 60%
            add_sample(CurveInterpolator::Sample(12.35, 0.7));// 70%
            add_sample(CurveInterpolator::Sample(12.5, 0.8)); // 80%
            add_sample(CurveInterpolator::Sample(12.7, 0.9)); // 90%
            add_sample(CurveInterpolator::Sample(12.9, 1.0)); // 100%   
        } else if (system_voltage == 24 && battery_type == "Gel") {
            add_sample(CurveInterpolator::Sample(21.0, 0.0)); // 0%
            add_sample(CurveInterpolator::Sample(22.8, 0.1)); // 10%
            add_sample(CurveInterpolator::Sample(23.2, 0.2)); // 20%
            add_sample(CurveInterpolator::Sample(23.5, 0.3)); // 30%
            add_sample(CurveInterpolator::Sample(23.8, 0.4)); // 40%
            add_sample(CurveInterpolator::Sample(24.0, 0.5)); // 50%
            add_sample(CurveInterpolator::Sample(24.4, 0.6)); // 60%
            add_sample(CurveInterpolator::Sample(24.7, 0.7)); // 70%
            add_sample(CurveInterpolator::Sample(25.0, 0.8)); // 80%
            add_sample(CurveInterpolator::Sample(25.4, 0.9)); // 90%
            add_sample(CurveInterpolator::Sample(25.8, 1.0)); // 100%
        } else if (system_voltage == 12 && battery_type == "Lead Acid"){
            add_sample(CurveInterpolator::Sample(10.5, 0.0)); // 0%
            add_sample(CurveInterpolator::Sample(11.2, 0.1)); // 10%
            add_sample(CurveInterpolator::Sample(11.4, 0.2)); // 20%
            add_sample(CurveInterpolator::Sample(11.6, 0.3)); // 30%
            add_sample(CurveInterpolator::Sample(11.7, 0.4)); // 40%
            add_sample(CurveInterpolator::Sample(11.9, 0.5)); // 50%
            add_sample(CurveInterpolator::Sample(12.0, 0.6)); // 60%
            add_sample(CurveInterpolator::Sample(12.2, 0.7)); // 70%
            add_sample(CurveInterpolator::Sample(12.3, 0.8)); // 80%
            add_sample(CurveInterpolator::Sample(12.5, 0.9)); // 90%
            add_sample(CurveInterpolator::Sample(12.7, 1.0)); // 100%
        } else if (system_voltage == 24 && battery_type == "Lead Acid"){
            add_sample(CurveInterpolator::Sample(21.0, 0.0)); // 0%
            add_sample(CurveInterpolator::Sample(22.4, 0.1)); // 10%
            add_sample(CurveInterpolator::Sample(22.8, 0.2)); // 20%
            add_sample(CurveInterpolator::Sample(23.2, 0.3)); // 30%
            add_sample(CurveInterpolator::Sample(23.4, 0.4)); // 40%
            add_sample(CurveInterpolator::Sample(23.8, 0.5)); // 50%
            add_sample(CurveInterpolator::Sample(24.0, 0.6)); // 60%
            add_sample(CurveInterpolator::Sample(24.4, 0.7)); // 70%
            add_sample(CurveInterpolator::Sample(24.6, 0.8)); // 80%
            add_sample(CurveInterpolator::Sample(25.0, 0.9)); // 90%
            add_sample(CurveInterpolator::Sample(25.4, 1.0)); // 100%
        }
    }
};

//Globale Variable für den SoC-Interpreter
std::unique_ptr<SoCAInterpreter> soc_interpolator; // SoC-Interpreter für die Batterie

void setupBattery() {
    // Konfiguriere, SesEsp WebUi für die Batteriewahl
    auto battery_type_config = std::make_shared<StringConfig>(battery_type, battery_type_config_path);
    auto battery_voltage_config = std::make_shared<NumberConfig>(battery_voltage, battery_voltage_config_path);
    auto battery_capacity_config = std::make_shared<NumberConfig>(battery_capacity, battery_capacity_config_path);
    auto max_current_config = std::make_shared<NumberConfig>(max_current, max_current_config_path);
    auto shunt_voltage_config = std::make_shared<NumberConfig>(shunt_voltage, shunt_voltage_config_path);
    auto battery_name_config = std::make_shared<StringConfig>(battery_name, battery_name_config_path);
    

    ConfigItem(battery_type_config)
        ->set_title("Battery Type")
        ->set_description("LiFePO4, AGM, Gel, Lead Acid")
        ->set_sort_order(100);
    ConfigItem(battery_voltage_config)
        ->set_title("Battery Voltage")
        ->set_description("12V, 24V, 48V (only LiFePO4 supported)")
        ->set_sort_order(200);
    ConfigItem(battery_capacity_config)
        ->set_title("Battery Capacity (Ah)")
        ->set_sort_order(300);
    ConfigItem(max_current_config)
        ->set_title("Shunt Current (A)")
        ->set_sort_order(400); 
    ConfigItem(shunt_voltage_config)
        ->set_title("Shunt Voltage (mV)")
        ->set_sort_order(500);
    ConfigItem(battery_name_config)
        ->set_title("Battery Name")
        ->set_description("Name of the Battery for Signal K Path (eg. House, Starter, etc.)")
        ->set_sort_order(600); 

    // Lade die Konfigurationswerte der Batterie
    battery_type = battery_type_config->get_value();
    battery_voltage = battery_voltage_config->get_value();
    battery_capacity = battery_capacity_config->get_value();
    max_current = max_current_config->get_value();
    shunt_voltage = shunt_voltage_config->get_value() / 1000.0;
    battery_name = battery_name_config->get_value();

    // Aktualisiere die SignalK-Pfade basierend auf den geladenen Konfigurationswerten
    sk_path_current = "electrical.batteries." + battery_name + ".current";
    sk_path_voltage = "electrical.batteries." + battery_name + ".voltage";
    sk_path_power = "electrical.batteries." + battery_name + ".power";
    sk_path_soc = "electrical.batteries." + battery_name + ".stateOfCharge";
    sk_path_ah = "electrical.batteries." + battery_name + ".ampHours";
    sk_path_time_to_go = "electrical.batteries." + battery_name + ".timeToGo";
    sk_path_shunt_over_voltage = "electrical.batteries." + battery_name + ".alerts.shuntOverVoltage";
    sk_path_bus_over_voltage = "electrical.batteries." + battery_name + ".alerts.busOverVoltage";
    sk_path_power_over_limit = "electrical.batteries." + battery_name + ".alerts.powerOverLimit";
    temp_one_wire_path = "/Battery Temperature/" + battery_name + "/oneWire";
    temp_linear_path = "/Battery Temperature/" + battery_name + "/linear";
    temp_sk_path = "electrical.batteries." + battery_name + ".temperature";
    

    // Berechne den Shunt-Widerstand basierend auf dem maximalen Strom und der Spannung
    shunt_resistance = calculateShuntResistance(max_current, shunt_voltage);

    // Erstelle den SoC-Interpreter mit den geladenen Konfigurationswerten
    soc_interpolator.reset(new SoCAInterpreter(battery_type, battery_voltage));    
}

// Hilfsfunktion: 100%-Spannung aus CurveInterpolator
float getFullVoltage() {
    auto& samples = soc_interpolator->get_samples();
    if (!samples.empty()) {
        return std::prev(samples.end())->input_; // Letzter Eintrag ist 100%
    }
    return 0.0; // Rückgabe 0, wenn keine Samples vorhanden sind
}

float shunt_over_voltage_limit = max_current * shunt_resistance * 1000;
float bus_over_voltage_limit = battery_voltage * 1000;
float power_over_limit = battery_voltage * max_current;

// Initialisiere den INA226
void initINA226() {
    INA0.begin();
    INA0.setAverage(INA226_64_SAMPLES);
    INA0.setBusVoltageConversionTime(INA226_1100_us);
    INA0.setShuntVoltageConversionTime(INA226_1100_us);
    INA0.setMode(7);
    INA0.setMaxCurrentShunt(max_current, shunt_resistance); // Kalibriere den INA226 Sensor (Maximalstrom und Shunt-Widerstand)

    // Konfiguriere INA226 für den Alarm
    INA0.setAlertRegister(INA226_SHUNT_OVER_VOLTAGE | INA226_BUS_OVER_VOLTAGE | INA226_POWER_OVER_LIMIT);

    //Setze die Limits für die Alarme basierend auf den Konfigurationswerten
    INA0.setAlertLimit(bus_over_voltage_limit); // Setze den Bus-Überspannungsalarm
    INA0.setAlertLimit(shunt_over_voltage_limit); // Setze den Shunt-Überspannungsalarm
    INA0.setAlertLimit(power_over_limit); // Setze den Leistungsüberschreitungsalarm
}

float read_INA0_current_callback(){
    return INA0.getCurrent();
}
float read_INA0_voltage_callback(){
    return INA0.getBusVoltage();
}
float read_INA0_power_callback(){
    return INA0.getPower();
}

float calculateSoC(float amp_hours_used, float battery_capacity) {
    return ((battery_capacity - amp_hours_used) / battery_capacity) * 100;
}

float calculateAmpHours(float current, float time_seconds) {
    return current * (time_seconds / 3600);
}
float total_amp_hours = 0;
unsigned long last_time = 0;

void updateAmpHours() {
    unsigned long current_time = millis();
    float time_diff = (current_time - last_time) / 1000.0;
    float current = INA0.getCurrent();
    float voltage = INA0.getBusVoltage();

    // Automatischer Reset, wenn Batteriespannung >= 100% (mit Toleranz)
    float full_voltage = getFullVoltage();
    if (full_voltage > 0.0 && voltage >= (full_voltage - 0.05)) { // 0.05V Toleranz
        total_amp_hours = 0; // Setze die verbrauchten Ampere-Stunden zurück
        amp_hours_used = 0; // Setze die verbrauchten Ampere-Stunden zurück
    }
    total_amp_hours += calculateAmpHours(current, time_diff);
    amp_hours_used = total_amp_hours; // Aktualisiere die verbrauchten Ampere-Stunden
    last_time = current_time; // Aktualisiere die letzte Zeit
}

float calculateTimeToGo(float current, float amp_hours_used) {
    if (current == 0) {
        return INFINITY; // Wenn der Strom 0 ist, wird unendlich zurückgegeben
    }
    return (battery_capacity - amp_hours_used) / current;
}

void IRAM_ATTR handle_Alert() {
    uint16_t alertflag = INA0.getAlertFlag();
    if (alertflag & INA226_SHUNT_OVER_VOLTAGE) {
        alert_shunt_over_voltage->set("Shunt Over Voltage");
    }
    if (alertflag & INA226_BUS_OVER_VOLTAGE) {
        alert_bus_over_voltage->set("Bus Over Voltage");
    }
    if (alertflag & INA226_POWER_OVER_LIMIT) {
        alert_power_over_limit->set("Power Over Limit");
    }
}

void setup() {
    Serial.begin(115200);
    Wire.begin(22, 21); // I2C Pins für INA226 (SDA, SCL)
    pinMode(25, INPUT_PULLUP); // Alarm-Pin
    attachInterrupt(digitalPinToInterrupt(25), handle_Alert, FALLING); // Interrupt für Alarm-Pin

    SetupLogging();

    SensESPAppBuilder builder;
    sensesp_app = (&builder)
                  ->set_hostname(hostname)
                  ->get_app();

    if (sensesp_app == nullptr) {
        printf("Unable to create SensESPApp\n");
        return;
    }

    setupBattery(); // Konfiguriere die Batterie
    initINA226(); // Initialisiere den INA226

    // Initialisiere den Temperatursensor
    DallasTemperatureSensors* dts = new DallasTemperatureSensors(pin);

    // INA0
    auto* ina0_current = new RepeatSensor<float>(read_delay, read_INA0_current_callback);
    auto* ina0_voltage = new RepeatSensor<float>(read_delay, read_INA0_voltage_callback);
    auto* ina0_power = new RepeatSensor<float>(read_delay, read_INA0_power_callback);

    // Hinzufügen der INA0-Werte (Current, Voltage & Power) zu Signal K
    ina0_current->connect_to(new SKOutputFloat(sk_path_current.c_str(), metadata_current));
    ina0_voltage->connect_to(new SKOutputFloat(sk_path_voltage.c_str(), metadata_voltage));
    ina0_power->connect_to(new SKOutputFloat(sk_path_power.c_str(), metadata_power));

    // Hinzufügen der Ampere-Stunden Used zu Signal K
    auto* ina0_amp_hours = new RepeatSensor<float>(read_delay, []() {
        updateAmpHours();
        return total_amp_hours;
    });
    ina0_amp_hours->connect_to(new SKOutputFloat(sk_path_ah.c_str(), metadata_ah));

    // Hinzufügen der Time-to-go zu Signal K
    auto* ina0_time_to_go = new RepeatSensor<float>(read_delay, []() {
        float current = INA0.getCurrent();
        return calculateTimeToGo(current, amp_hours_used);
    });
    ina0_time_to_go->connect_to(new SKOutputFloat(sk_path_time_to_go.c_str(), metadata_time_to_go));

    // Hinzufügen des State of Charge zu Signal K
    auto* ina0_soc = new RepeatSensor<float>(read_delay, []() {
        return calculateSoC(total_amp_hours, battery_capacity);
    });
    ina0_soc->connect_to(new SKOutputFloat(sk_path_soc.c_str(), metadata_soc));

    // Spannungsbasierten SoC zusätzlich an Signal K senden
    auto* soc_voltage_sensor = new RepeatSensor<float>(read_delay, read_INA0_voltage_callback);
    soc_voltage_sensor  ->connect_to(static_cast<CurveInterpolator*>(soc_interpolator.get()))
                        ->connect_to(new SKOutputFloat((sk_path_soc + "_voltageSoC").c_str(), metadata_soc));

    // Hinzufügen des Temperatursensors zu Signal K
    auto* battery_temp = new OneWireTemperature(dts, read_delay, temp_one_wire_path.c_str());
    battery_temp->connect_to(new Linear(1.0, 0.0, temp_linear_path.c_str()))
                ->connect_to(new SKOutputFloat(temp_sk_path.c_str(), metadata_temp));

    // Hinzufügen der Alarmmeldungen zu Signal K
    alert_shunt_over_voltage = new SKOutputString(sk_path_shunt_over_voltage.c_str(), metadata_shunt_over_voltage);
    alert_bus_over_voltage = new SKOutputString(sk_path_bus_over_voltage.c_str(), metadata_bus_over_voltage);
    alert_power_over_limit = new SKOutputString(sk_path_power_over_limit.c_str(), metadata_power_over_limit);

    last_time = millis();
}

void loop() {
    event_loop()->tick();
}


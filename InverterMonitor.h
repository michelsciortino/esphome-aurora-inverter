#include "esphome.h"
#include <ABBAurora.h>

using namespace esphome;
using namespace text_sensor;
using namespace sensor;

#if defined(USE_ESP32)
#define LED 2               //GPIO02, the ESP32 internal led
#define RX 14               //GPIO14
#define TX 27               //GPIO27
#define TX_CONTROL_GPIO 26  //GPIO26
#define INVERTER_MONITOR_SERIAL Serial2

#elif defined(USE_ESP8266)
#define LED LED_BUILTIN
// On esp8266 UART0 *must* use GPIO (1, 3) OR GPIO (15, 13)
// Alternate pins GPIO (15, 13) are not connected to USB-UART
#define RX 13              // GPI13, D7, UART0 alt RX
#define TX 15              // GPI15, D8, UART0 alt TX
#define TX_CONTROL_GPIO 12 // GPIO12, D6 on nodeMCU and d1 mini
#define INVERTER_MONITOR_SERIAL Serial

#else
#error Unsupported device

#endif

#define INVERTER_ADDRESS 2  //Default address; this can be modified on the inverter settings menu


#ifdef USE_ESP8266
// Check UART pins are compatible
#if !((TX == 1 && RX == 3) || (TX == 15 && RX == 13))
#error Unsupported UART pin configuration for esp8266
#endif
#endif


#define CONNECTED "CONNECTED"
#define DISCONNECTED "DISCONNECTED"

#define TAG "INVERTER_MONITOR"

class InverterMonitor : public PollingComponent
{

protected:
  InverterMonitor() : PollingComponent(5000) {}
  static InverterMonitor *instance_;

private:
  ABBAurora *inverter;
  uint8_t connection = 0;

public:
  InverterMonitor(InverterMonitor &other) = delete;
  void operator=(const InverterMonitor &) = delete;
  static InverterMonitor *get_instance();

  TextSensor *connection_status = new TextSensor();
  Sensor *v_in_1 = new Sensor();
  Sensor *v_in_2 = new Sensor();
  Sensor *i_in_1 = new Sensor();
  Sensor *i_in_2 = new Sensor();
  Sensor *power_in_1 = new Sensor();
  Sensor *power_in_2 = new Sensor();
  Sensor *power_in_total = new Sensor();
  Sensor *power_peak_today = new Sensor();
  Sensor *power_peak_max = new Sensor();
  Sensor *temperature_inverter = new Sensor();
  Sensor *temperature_booster = new Sensor();
  Sensor *cumulated_energy_today = new Sensor();
  Sensor *cumulated_energy_week = new Sensor();
  Sensor *cumulated_energy_month = new Sensor();
  Sensor *cumulated_energy_year = new Sensor();
  Sensor *cumulated_energy_total = new Sensor();
  Sensor *grid_voltage = new Sensor();
  Sensor *grid_current = new Sensor();
  Sensor *grid_power = new Sensor();
  Sensor *frequency = new Sensor();
  Sensor *v_bulk = new Sensor();
  Sensor *i_leak_dc_dc = new Sensor();
  Sensor *i_leak_inverter = new Sensor();
  Sensor *dc_dc_grid_voltage = new Sensor();
  Sensor *dc_dc_grid_frequency = new Sensor();
  Sensor *isolation_resistance = new Sensor();
  Sensor *dc_dc_v_bulk = new Sensor();
  Sensor *average_grid_voltage = new Sensor();
  Sensor *v_bulk_mid = new Sensor();
  Sensor *grid_voltage_neutral = new Sensor();
  Sensor *wind_generator_frequency = new Sensor();
  Sensor *grid_voltage_neutral_phase = new Sensor();
  Sensor *grid_current_phase_r = new Sensor();
  Sensor *grid_current_phase_s = new Sensor();
  Sensor *grid_current_phase_t = new Sensor();
  Sensor *frequency_phase_r = new Sensor();
  Sensor *frequency_phase_s = new Sensor();
  Sensor *frequency_phase_t = new Sensor();
  Sensor *v_bulk_positive = new Sensor();
  Sensor *v_bulk_negative = new Sensor();
  Sensor *temperature_supervisor = new Sensor();
  Sensor *temperature_alim = new Sensor();
  Sensor *temperature_heat_sink = new Sensor();
  Sensor *temperature_1 = new Sensor();
  Sensor *temperature_2 = new Sensor();
  Sensor *temperature_3 = new Sensor();
  Sensor *fan_speed_1 = new Sensor();
  Sensor *fan_speed_2 = new Sensor();
  Sensor *fan_speed_3 = new Sensor();
  Sensor *fan_speed_4 = new Sensor();
  Sensor *fan_speed_5 = new Sensor();
  Sensor *power_saturation_limit = new Sensor();
  Sensor *v_panel_micro = new Sensor();
  Sensor *grid_voltage_phase_r = new Sensor();
  Sensor *grid_voltage_phase_s = new Sensor();
  Sensor *grid_voltage_phase_t = new Sensor();

  void setup() override
  {
    ESP_LOGD(TAG, "Setting up");
    pinMode(LED, OUTPUT);
    ABBAurora::setup(INVERTER_MONITOR_SERIAL, RX, TX, TX_CONTROL_GPIO);
    inverter = new ABBAurora(INVERTER_ADDRESS);
    connection_status->publish_state(DISCONNECTED);
  }

  void publish_dsp_value(DSP_VALUE_TYPE type, Sensor *sensor)
  {
    if (inverter->ReadDSPValue(type, MODULE_MEASUREMENT)) {
      ESP_LOGD(TAG, "ReadDSPValue %d %f", type, inverter->DSP.Value);
      sensor->publish_state(inverter->DSP.Value);
    }
    yield();
  }

  void publish_temperature(DSP_VALUE_TYPE type, Sensor *sensor)
  {
    if (inverter->ReadDSPValue(type, MODULE_MEASUREMENT)) {
      float temp = inverter->DSP.Value / 10;
      ESP_LOGD(TAG, "ReadDSPValue %d %f", type, temp);
      sensor->publish_state(temp);
    }
    yield();
  }

  void publish_cumulated_energy(CUMULATED_ENERGY_TYPE type, Sensor *sensor)
  {
    if (inverter->ReadCumulatedEnergy(type)) {
    ESP_LOGD(TAG, "ReadCumulatedEnergy %d %f.0", type, inverter->CumulatedEnergy.Energy);
      sensor->publish_state(inverter->CumulatedEnergy.Energy);
    }
    yield();
  }

  void update() override
  {
    //If inverter is connected
    if (inverter->ReadState())
    {
      if (!connection)
      {
        connection = 1;
        connection_status->publish_state(CONNECTED);
      }
      turn_led_on();

      publish_dsp_value(V_IN_1, v_in_1);
      publish_dsp_value(V_IN_2, v_in_2);
      publish_dsp_value(I_IN_1, i_in_1);
      publish_dsp_value(I_IN_2, i_in_2);
      publish_dsp_value(POWER_IN_1, power_in_1);
      publish_dsp_value(POWER_IN_2, power_in_2);
      power_in_total->publish_state(power_in_1->get_state() + power_in_2->get_state());
      publish_dsp_value(POWER_PEAK_TODAY, power_peak_today);
      publish_dsp_value(POWER_PEAK, power_peak_max);
      publish_dsp_value(TEMPERATURE_INVERTER, temperature_inverter);
      publish_dsp_value(TEMPERATURE_BOOSTER, temperature_booster);
      publish_cumulated_energy(CURRENT_DAY, cumulated_energy_today);
      publish_cumulated_energy(CURRENT_WEEK, cumulated_energy_week);
      publish_cumulated_energy(CURRENT_MONTH, cumulated_energy_month);
      publish_cumulated_energy(CURRENT_YEAR, cumulated_energy_year);
      publish_cumulated_energy(TOTAL, cumulated_energy_total);
      publish_dsp_value(GRID_VOLTAGE, grid_voltage);
      publish_dsp_value(GRID_CURRENT, grid_current);
      publish_dsp_value(GRID_POWER, grid_power);
      publish_dsp_value(FREQUENCY, frequency);
      publish_dsp_value(V_BULK, v_bulk);
      publish_dsp_value(I_LEAK_DC_DC, i_leak_dc_dc);
      publish_dsp_value(I_LEAK_INVERTER, i_leak_inverter);
      publish_dsp_value(DC_DC_GRID_VOLTAGE, dc_dc_grid_voltage);
      publish_dsp_value(DC_DC_GRID_FREQUENCY, dc_dc_grid_frequency);
      publish_dsp_value(ISOLATION_RESISTANCE, isolation_resistance);
      publish_dsp_value(DC_DC_V_BULK, dc_dc_v_bulk);
      publish_dsp_value(AVERAGE_GRID_VOLTAGE, average_grid_voltage);
      publish_dsp_value(V_BULK_MID, v_bulk_mid);
      publish_dsp_value(GRID_VOLTAGE_NEUTRAL, grid_voltage_neutral);
      publish_dsp_value(WIND_GENERATOR_FREQENCY, wind_generator_frequency);
      publish_dsp_value(GRID_VOLTAGE_NEUTRAL_PHASE, grid_voltage_neutral_phase);
      publish_dsp_value(GRID_CURRENT_PHASE_R, grid_current_phase_r);
      publish_dsp_value(GRID_CURRENT_PHASE_S, grid_current_phase_s);
      publish_dsp_value(GRID_CURRENT_PHASE_T, grid_current_phase_t);
      publish_dsp_value(FREQUENCY_PHASE_R, frequency_phase_r);
      publish_dsp_value(FREQUENCY_PHASE_S, frequency_phase_s);
      publish_dsp_value(FREQUENCY_PHASE_T, frequency_phase_t);
      publish_dsp_value(V_BULK_POSITIVE, v_bulk_positive);
      publish_dsp_value(V_BULK_NEGATIVE, v_bulk_negative);
      publish_temperature(TEMPERATURE_SUPERVISOR, temperature_supervisor);
      publish_temperature(TEMPERATURE_ALIM, temperature_alim);
      publish_temperature(TEMPERATURE_HEAT_SINK, temperature_heat_sink);
      publish_temperature(TEMPERATURE_1, temperature_1);
      publish_temperature(TEMPERATURE_2, temperature_2);
      publish_temperature(TEMPERATURE_3, temperature_3);
      publish_dsp_value(FAN_SPEED_1, fan_speed_1);
      publish_dsp_value(FAN_SPEED_2, fan_speed_2);
      publish_dsp_value(FAN_SPEED_3, fan_speed_3);
      publish_dsp_value(FAN_SPEED_4, fan_speed_4);
      publish_dsp_value(FAN_SPEED_5, fan_speed_5);
      publish_dsp_value(POWER_SATURATION_LIMIT, power_saturation_limit);
      publish_dsp_value(V_PANEL_MICRO, v_panel_micro);
      publish_dsp_value(GRID_VOLTAGE_PHASE_R, grid_voltage_phase_r);
      publish_dsp_value(GRID_VOLTAGE_PHASE_S, grid_voltage_phase_s);
      publish_dsp_value(GRID_VOLTAGE_PHASE_T, grid_voltage_phase_t);

      turn_led_off();
    }
    else
    {
      if (connection)
      {
        connection = 0;
        connection_status->publish_state(DISCONNECTED);
      }
      ESP_LOGD(TAG, "Inverter not connected");
    }
  }

  void turn_led_on()
  {
    digitalWrite(LED, HIGH);
  }

  void turn_led_off()
  {
    digitalWrite(LED, LOW);
  }
};

InverterMonitor *InverterMonitor::instance_ = nullptr;

InverterMonitor *InverterMonitor::get_instance()
{
  if (instance_ == nullptr)
    instance_ = new InverterMonitor();
  return instance_;
}

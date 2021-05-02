#include "esphome.h"
#include <ABBAurora.h>

using namespace esphome;
using namespace text_sensor;
using namespace sensor;

#define LED 2               //GPIO02, the ESP32 internal led

#define RX 14               //GPIO14
#define TX 27               //GPIO27
#define INVERTER_ADDRESS 2  //Default address; this can be modified on the inverter settings menu
#define TX_CONTROL_GPIO 26  //GPIO26

#define CONNECTED "CONNECTED"
#define DISCONNECTED "DISCONNECTED"

#define TAG "INVERTER_MONITOR"

class InverterMonitor : public PollingComponent
{

protected:
  InverterMonitor() : PollingComponent(15000) {}
  static InverterMonitor *instance_;

private:
  ABBAurora *inverter;
  bool led_state;
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

  void setup() override
  {
    ESP_LOGD(TAG, "Setupping");
    pinMode(LED, OUTPUT);
    led_state = LOW;
    ABBAurora::setup(Serial2, RX, TX, TX_CONTROL_GPIO);
    inverter = new ABBAurora(INVERTER_ADDRESS);
    connection_status->publish_state(DISCONNECTED);
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

      ESP_LOGD(TAG, "ReadDSPValue V_IN_1");
      if (inverter->ReadDSPValue(V_IN_1, MODULE_MESSUREMENT))
        v_in_1->publish_state(inverter->DSP.Value);
      yield();

      ESP_LOGD(TAG, "ReadDSPValue V_IN_2");
      if (inverter->ReadDSPValue(V_IN_2, MODULE_MESSUREMENT))
        v_in_2->publish_state(inverter->DSP.Value);
      yield();

      ESP_LOGD(TAG, "ReadDSPValue I_IN_1");
      if (inverter->ReadDSPValue(I_IN_1, MODULE_MESSUREMENT))
        i_in_1->publish_state(inverter->DSP.Value);
      yield();

      ESP_LOGD(TAG, "ReadDSPValue I_IN_2");
      if (inverter->ReadDSPValue(I_IN_2, MODULE_MESSUREMENT))
        i_in_2->publish_state(inverter->DSP.Value);
      yield();

      ESP_LOGD(TAG, "ReadDSPValue POWER_IN_1");
      if (inverter->ReadDSPValue(POWER_IN_1, MODULE_MESSUREMENT))
        power_in_1->publish_state(inverter->DSP.Value);
      yield();

      ESP_LOGD(TAG, "ReadDSPValue POWER_IN_2");
      if (inverter->ReadDSPValue(POWER_IN_2, MODULE_MESSUREMENT))
        power_in_2->publish_state(inverter->DSP.Value);
      yield();

      power_in_total->publish_state(power_in_1->get_state() + power_in_2->get_state());

      ESP_LOGD(TAG, "ReadDSPValue POWER_PEAK_TODAY");
      if (inverter->ReadDSPValue(POWER_PEAK_TODAY, MODULE_MESSUREMENT))
        power_peak_today->publish_state(inverter->DSP.Value);
      yield();

      ESP_LOGD(TAG, "ReadDSPValue POWER_PEAK");
      if (inverter->ReadDSPValue(POWER_PEAK, MODULE_MESSUREMENT))
        power_peak_max->publish_state(inverter->DSP.Value);
      yield();

      ESP_LOGD(TAG, "ReadDSPValue TEMPERATURE_INVERTER");
      if (inverter->ReadDSPValue(TEMPERATURE_INVERTER, MODULE_MESSUREMENT))
        temperature_inverter->publish_state(inverter->DSP.Value);
      yield();

      ESP_LOGD(TAG, "ReadDSPValue TEMPERATURE_BOOSTER");
      if (inverter->ReadDSPValue(TEMPERATURE_BOOSTER, MODULE_MESSUREMENT))
        temperature_booster->publish_state(inverter->DSP.Value);
      yield();

      ESP_LOGD(TAG, "ReadCumulatedEnergy CUMULATED_ENERGY_CURRENT_DAY");
      if (inverter->ReadCumulatedEnergy(CURRENT_DAY))
        cumulated_energy_today->publish_state(inverter->CumulatedEnergy.Energy);
      yield();

      ESP_LOGD(TAG, "ReadCumulatedEnergy CUMULATED_ENERGY_CURRENT_WEEK");
      if (inverter->ReadCumulatedEnergy(CURRENT_WEEK))
        cumulated_energy_week->publish_state(inverter->CumulatedEnergy.Energy);
      yield();

      ESP_LOGD(TAG, "ReadCumulatedEnergy CUMULATED_ENERGY_CURRENT_MONTH");
      if (inverter->ReadCumulatedEnergy(CURRENT_MONTH))
        cumulated_energy_month->publish_state(inverter->CumulatedEnergy.Energy);
      yield();

      ESP_LOGD(TAG, "ReadCumulatedEnergy CUMULATED_ENERGY_CURRENT_YEAR");
      if (inverter->ReadCumulatedEnergy(CURRENT_YEAR))
        cumulated_energy_year->publish_state(inverter->CumulatedEnergy.Energy);
      yield();

      ESP_LOGD(TAG, "ReadCumulatedEnergy CUMULATED_ENERGY_TOTAL");
      if (inverter->ReadCumulatedEnergy(TOTAL))
        cumulated_energy_total->publish_state(inverter->CumulatedEnergy.Energy);
      yield();

      turn_led_off();
    }
    else
    {
      if (connection)
      {
        connection = 0;
        connection_status->publish_state(DISCONNECTED);
      }
      ESP_LOGD(TAG, "Inverter not conntected");
    }
  }

  void turn_led_on()
  {
    if (led_state)
      return;
    led_state = HIGH;
    digitalWrite(LED, led_state);
  }

  void turn_led_off()
  {
    if (!led_state)
      return;
    led_state = LOW;
    digitalWrite(LED, led_state);
  }
};

InverterMonitor *InverterMonitor::instance_ = nullptr;

InverterMonitor *InverterMonitor::get_instance()
{
  if (instance_ == nullptr)
    instance_ = new InverterMonitor();
  return instance_;
}
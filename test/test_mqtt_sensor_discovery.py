#!/usr/bin/env python3
"""
Test MQTT Discovery with a simple sensor
Sensors are simpler than climate entities - if this doesn't work, discovery is disabled
"""

import paho.mqtt.client as mqtt
import json
import time

MQTT_BROKER = "192.168.178.32"
MQTT_PORT = 1883
HA_DISCOVERY_PREFIX = "homeassistant"

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print(f"✓ Connected to MQTT broker")
    else:
        print(f"✗ Connection failed with code {rc}")

def test_sensor_discovery():
    print("=" * 70)
    print("MQTT Sensor Discovery Test (simplest possible)")
    print("=" * 70)

    client = mqtt.Client(client_id=f"test_sensor_{int(time.time())}")
    client.on_connect = on_connect

    try:
        print(f"\nConnecting to {MQTT_BROKER}:{MQTT_PORT}...")
        client.connect(MQTT_BROKER, MQTT_PORT, 60)
        client.loop_start()
        time.sleep(1)

        # Publish state first
        state_topic = "test/sensor/temperature"
        print(f"\n1. Publishing state to: {state_topic}")
        client.publish(state_topic, "23.5", qos=0, retain=True)
        print(f"   Value: 23.5")

        time.sleep(0.5)

        # Publish discovery config
        config_topic = f"{HA_DISCOVERY_PREFIX}/sensor/test_sensor_simple/temperature/config"

        config = {
            "name": "Test Temperature Sensor",
            "uniq_id": "test_sensor_temp_001",
            "stat_t": state_topic,
            "unit_of_meas": "°C",
            "dev_cla": "temperature"
        }

        config_json = json.dumps(config, indent=2)

        print(f"\n2. Publishing discovery config to: {config_topic}")
        print(f"   Payload size: {len(config_json)} bytes")
        print(f"\n{config_json}\n")

        result = client.publish(config_topic, config_json, qos=0, retain=True)

        if result.rc == 0:
            print("✓ Discovery config published successfully")
        else:
            print(f"✗ Publish failed with code {result.rc}")

        print("\n" + "=" * 70)
        print("VERIFICATION:")
        print("=" * 70)
        print("1. Check MQTT Explorer - should see both topics:")
        print(f"   - {state_topic}")
        print(f"   - {config_topic}")
        print("\n2. Check Home Assistant:")
        print("   - Settings → Devices & Services → MQTT")
        print("   - Should see 'Test Temperature Sensor'")
        print("\n3. If MQTT Explorer shows topics but HA doesn't create entity:")
        print("   → MQTT discovery is DISABLED in your HA configuration")
        print("   → Check your configuration.yaml for:")
        print("      mqtt:")
        print("        discovery: true")
        print("        discovery_prefix: homeassistant")
        print("=" * 70)

        print("\nKeeping connection alive for 5 seconds...")
        time.sleep(5)

        client.loop_stop()
        client.disconnect()

        print("\n✓ Test completed")

    except Exception as e:
        print(f"\n✗ Error: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    test_sensor_discovery()

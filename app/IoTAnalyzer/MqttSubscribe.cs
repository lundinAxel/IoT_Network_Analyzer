using System.Text;
using System.Text.Json;
using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Protocol;

class MqttSubscribe
{
    public async Task StartAsync()
    {
        string broker = "broker.hivemq.com";
        int port = 1883;
        string clientId = Guid.NewGuid().ToString();
        string topic = "esp32testing/sensors";

        // Create a MQTT client factory
        var factory = new MqttFactory();

        // Create a MQTT client instance
        var mqttClient = factory.CreateMqttClient();

        // Create MQTT client options
        var options = new MqttClientOptionsBuilder()
            .WithTcpServer(broker, port) // MQTT broker address and port
            .WithClientId(clientId)
            .WithCleanSession()
            .Build();

        var connectResult = await mqttClient.ConnectAsync(options);

        if (connectResult.ResultCode == MqttClientConnectResultCode.Success)
        {
            Console.WriteLine("Connected to MQTT broker successfully.");

            // Subscribe to a topic
            await mqttClient.SubscribeAsync(topic);

            // Callback function when a message is received
            mqttClient.ApplicationMessageReceivedAsync += e =>
            {
                var json = Encoding.UTF8.GetString(e.ApplicationMessage.PayloadSegment);
                var message = JsonSerializer.Deserialize<SensorMessage>(json);

                Console.WriteLine($"deviceId: {message.DeviceId} sensor: {message.Sensor} value: {message.Raw}");
                return Task.CompletedTask;
            };
        }
        Console.WriteLine("Waiting for messages. Press Enter to exit.");
        Console.ReadLine();
    }
}
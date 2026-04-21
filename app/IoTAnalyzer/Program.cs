class Program
{
    static async Task Main(string[] args)
    {
        var MqttSubscribe = new MqttSubscribe();
        await MqttSubscribe.StartAsync();
    }
}
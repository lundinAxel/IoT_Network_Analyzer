

public class Device
{
    public required string IPAddress {get; set;}
    public required string Name {get; set;}
    public bool IsOnline {get; set;}

    public void DisplayInfo()
    {
        Console.WriteLine($"[{IPAddress}] {Name} - Status: {(IsOnline ? "Online" : "Offline")}");
    }

}
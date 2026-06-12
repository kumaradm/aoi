using System.Text;
using Microsoft.Extensions.Logging;

namespace AoiSystem.Orchestrator.Services;

public class McuManagerService : IMcuManagerService, IDisposable
{
    public event Action OnPcbDetected;
    private readonly SerialPort _serialPort;


    public McuManagerService(string portName, int baudRate)
    {
        _serialPort = new SerialPort(portName);
        _serialPort.BaudRate = baudRate;
        _serialPort.Parity = Parity.None;
        _serialPort.StopBits = StopBits.One;
        _serialPort.DataBits = 8;
        _serialPort.Handshake = Handshake.None;
        _serialPort.RtsEnable = true;
        _serialPort.DataReceived += new SerialDataReceivedEventHandler(DataReceivedHandler);
    }

    public void Init()
    {
        if (!_serialPort.isOpen) _serialPort.Open();
    }

    public async Task WaitForDataAsync(TimeSpan timeout, CancellationToken ct)
    {
        using var cts = CancellationTokenSource.CreateLinkedTokenSource(ct);
        cts.CancelAfter(timeout);

        using var reader = new StreamReader(_serialPort.BaseStream, Encoding.ASCII);

        try
        {
            _logger.LogInformation("Waiting for MCU trigger signal (Timeout: {timeout}s...)", timeout.TotalSeconds);

            while (!cts.Token.IsCancellationRequested)
            {
                string? line = await reader.ReadLineAsync(cts.Token);

                if (line != null)
                {
                    var cleanLine = line.Trim();

                    if (cleanLine.Contains("TRIGGER"))
                    {
                        
                    }
                }
            }
        }
    }

    public void SendCommand(string command)
    {
        Console.WriteLine(command);
    }
}
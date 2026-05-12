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

    public void Start()
    {
        if (!_serialPort.isOpen) _serialPort.Open();
    }

    public void DataReceivedHandler(object sender, SerialDataReceivedEventArgs e)
    {
        SerialPort sp = (SerialPort)sender;
        string data = sp.ReadExisting();

        if (data == "TRIGGER_ON")
        {
            OnPcbDetected?.Invoke();
        }
    }
}
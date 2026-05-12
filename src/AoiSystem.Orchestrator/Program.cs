using AoiSystem.Orchestrator.Interop;

class Program
{
    static void Main(string[] args)
    {
        Console.WriteLine("--- Industrial PCB Inspection System Starting ---");

        // 1. Initialize the Bridge (Loads the 1650 Ti GPU with your Engine)
        using var aoi = new ImageProcessorInterop(
            "/home/kumaradm/portfolio/aoi-on-jetson/private/tests/basler_emulator_test/images/cat_resized.png", 
            "/home/kumaradm/portfolio/aoi-on-jetson/src/AoiSystem.ImageProcessor/models/pcb_inspection_model.engine");

        int pcbLength = 4096;  // Total length of the PCB in pixels
        int sliceHeight = 512; // Height of each camera grab
        int strideY = 480;     // 112px overlap for vertical unity

        // 2. The Conveyor Loop (Vertical Scanning)
        for (int currentY = 0; currentY <= pcbLength - sliceHeight; currentY += strideY)
        {
            Console.Write($"Scanning Y-Pos: {currentY:D4} | ");

            // C++ handles the Horizontal tiling (4096 width) internally!
            int result = aoi.InspectSlice(currentY);

            if (result == -1)
            {
                Console.ForegroundColor = ConsoleColor.Green;
                Console.WriteLine("Status: CLEAR");
            }
            else
            {
                // COCO Class 15 is 'Cat'. In our simulation, a Cat = Defect.
                string defectName = (result == 15) ? "CRITICAL DEFECT (CAT)" : $"UNKNOWN OBJ (ID:{result})";
                
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine($"Status: {defectName} DETECTED");
            }
            Console.ResetColor();
        }

        Console.WriteLine("--- Inspection Complete ---");
    }
}
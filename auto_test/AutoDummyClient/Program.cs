using System.Text.Json;

namespace AutoTestClient
{
    public class Program
    {
        private static void Main(string[] args)
        {
            var configs = LoadConfig(args[0]);
            if (configs is null)
            {
                Console.WriteLine($"Failed load a config file");
                return;
            }

            var errorCode = configs.Verify();
            if (errorCode != ErrorCode.None)
            {
                Console.WriteLine($"Failed verify config with error : {errorCode}");
                return;
            }

            var runner = new ScenarioRunner();
            runner.Init(configs);
            runner.Run();
        }

        private static ScenarioRunnerConfig LoadConfig(string fileName)
        {
            var path = Directory.GetCurrentDirectory() + $"\\ScenarioConfigFile\\{fileName}.json";

            try
            {
                using (StreamReader file = File.OpenText(path))
                {
                    var nakedData = file.ReadToEnd();
                    ScenarioRunnerConfig configs = JsonSerializer.Deserialize<ScenarioRunnerConfig>(nakedData);

                    return configs;
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
                return null;
            }
        }
    }
}
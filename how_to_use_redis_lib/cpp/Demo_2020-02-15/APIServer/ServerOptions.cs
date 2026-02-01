using CommandLine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace APIServer
{
    public class ServerOption
    {
        [Option("redisName", Required = true, HelpText = "Redis Server Name")]
        public string RedisName { get; set; }

        [Option("redisAddress", Required = true, HelpText = "Redis Server Address")]
        public string RedisAddress { get; set; }


        [Option("gameServerIP", Required = true, HelpText = "GameServer IP")]
        public string GameServerIP { get; set; }

        [Option("gameServerPort", Required = true, HelpText = "GameServer Port")]
        public int GameServerPort { get; set; }
    }
}

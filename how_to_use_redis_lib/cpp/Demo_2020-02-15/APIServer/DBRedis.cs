using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using CloudStructures;
using CloudStructures.Structures;


namespace APIServer
{
    public class DBRedis {

        public static RedisConnection Connection { get; set; }
        
        static public void Init(string name, string address)
         {
            var config = new RedisConfig(name, address);
            Connection = new RedisConnection(config);
        }


        
        static public async Task<RedisResult<TReturn>> GetValue<TReturn>( string key )  {
            var defaultExpiry = TimeSpan.FromSeconds(60);
            var redis = new RedisString<TReturn>(DBRedis.Connection, key, defaultExpiry);
            var cachedObject = await redis.GetAsync();
            return cachedObject;
        }


        static public async Task<bool> SetValue<T>( string key, T value ) where T : class  {
            var defaultExpiry = TimeSpan.FromSeconds(60);
            var redis = new RedisString<T>(DBRedis.Connection, key, defaultExpiry);

            var result = await redis.SetAsync(value);
            return result;
        }      
    }
}

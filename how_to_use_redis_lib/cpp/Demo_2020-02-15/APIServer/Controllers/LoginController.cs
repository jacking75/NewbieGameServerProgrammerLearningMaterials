using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;

namespace APIServer.Controllers
{
    [Produces("application/json")]
    [Route("api/Login")]
    [ApiController]
    public class LoginController : Controller
    {
        [HttpPost]
        public async Task<LoginRes> Process([FromBody] LoginReq request)
        {
            var response = new LoginRes() 
            { 
                result = 1, 
                authToken = "fake",
                gameServerIP = LoginServer.ServerOpt.GameServerIP,
                gameServerPort = (UInt16)LoginServer.ServerOpt.GameServerPort,
            };

            var authToken = CreateAuthToken();

            var result = await DBRedis.SetValue(request.userID, authToken);
            
            response.authToken = authToken;
            return response;                        
        }

        string CreateAuthToken()
        {
            string token = Convert.ToBase64String(Guid.NewGuid().ToByteArray());
            return token;
        }

    }

    public class LoginReq
    {
        public string userID { get; set; }
        public string userPW { get; set; }
    }

    public class LoginRes
    {
        public int result { get; set; }
        public string authToken { get; set; }

        public string gameServerIP { get; set; }
        public UInt16 gameServerPort { get; set; }
    }
}
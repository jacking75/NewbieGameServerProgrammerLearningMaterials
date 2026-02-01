using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;
using System.Diagnostics;

namespace APIServer.Controllers
{
    [Produces("application/json")]
    [Route("api/Account/Create")]
    public class AccountController : Controller
    {
        [HttpPost]
        public async Task<CreateuserRes> Process([FromBody] CreateuserReq request)
        {
            Debug.WriteLine("Process Function Called");
            return new CreateuserRes() { Result = (short)0 };
        }
                

        public class CreateuserReq
        {
            public string UserID;
            public string UserPW;       
        }
         
        public class CreateuserRes 
        {
             public short Result; 
        }
    }
}

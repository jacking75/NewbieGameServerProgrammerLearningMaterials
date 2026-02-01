// 클라이언트가 플랫폼 서버에서 인증을 받았는지 확인해 준다
// 클라이언트는 플랫폼 서버에서 받은 인증토큰과 자신의 인증ID(계정 ID 혹은 플랫폼에서 만들어준 ID)로 보낸다.
// 사용할 수 있는 인증ID와 인증토큰은 이미 정해져 있다.
// 게임 서버는 인증이 성공하면
// - 이 유저의 default 게임데이터가 없다면 생성해줘야 한다.

using System.Collections.Generic;
using System.ComponentModel.DataAnnotations;

using Microsoft.AspNetCore.Mvc;


namespace APIServer.Controllers;

[ApiController]
[Route("[controller]")]
public class AuthCheck : ControllerBase
{
    Dictionary<string, string> authUserData = new();

    public AuthCheck()
    {
        Init();
    }

    [HttpGet]
    public PkAuthCheckResponse Get(PkAuthCheckRequest request)
    {
        PkAuthCheckResponse response = new();

        response.Result = Authenticate(request.AuthID, request.AuthToken);

        return response;
    }

    [HttpPost]
    public PkAuthCheckResponse Post(PkAuthCheckRequest request)
    {
        PkAuthCheckResponse response = new();

        response.Result = Authenticate(request.AuthID, request.AuthToken);

        return response;
    }

    void Init()
    {
        authUserData.Add("test01", "DUWPQCFN5DQF4P");
        authUserData.Add("test02", "DYG5R07M7RUV07");
        authUserData.Add("test03", "5GZF7OFY05P4TT");
        authUserData.Add("test04", "94ILRSD4LRXE6N");
        authUserData.Add("test05", "GPKJ442KR1BK0U");
        authUserData.Add("test06", "P2H95LNF6NT8UC");
        authUserData.Add("test07", "JXOU845OYZJUXG");
        authUserData.Add("test08", "N21SK6AXKQWS5B");
        authUserData.Add("test09", "X7S4WCTKMY6YVK");
        authUserData.Add("test10", "HIB0KU1A6FGVT1");
        authUserData.Add("test11", "0HM20Q8A4GFCBX");
        authUserData.Add("test12", "9IPHAAF6P88BMP");
        authUserData.Add("test13", "D58RFSAAAP1RWG");
        authUserData.Add("test14", "MYQOR56M574OIG");
        authUserData.Add("test15", "M0A7BOS0CVVN5L");
        authUserData.Add("test16", "0KJLTAMCVQBRLX");
        authUserData.Add("test17", "1E4XH0PL1XRGI8");
        authUserData.Add("test18", "FK4K9SYSB63L7R");
    }

    ErrorCode Authenticate(string authID, string authToken)
    {
        if (authUserData.TryGetValue(authID, out var authToken_ori))
        {
            if (authToken_ori == authToken)
            {
                return ErrorCode.None;
            }
        }
        return ErrorCode.AythCheckFail;
    }
}

public class PkAuthCheckRequest
{
    public string AuthID { get; set; }
    public string AuthToken { get; set; }
}

public class PkAuthCheckResponse
{
    [Required] public ErrorCode Result { get; set; } = ErrorCode.None;
}

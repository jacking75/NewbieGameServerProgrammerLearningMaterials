using System.Collections.Generic;

using Microsoft.AspNetCore.Mvc;

namespace APIServer.Controllers;

[ApiController]
[Route("[controller]")]
public class InAppCheck : Controller
{
    List<string> _receiptList = new();

    public InAppCheck()
    {
        Init();
    }

    [HttpPost]
    public PkReceiptCheckRes Post(PkReceiptCheckReq request)
    {
        PkReceiptCheckRes response = new();

        if (ReceiptCheck(request.Receipt) == false)
        {
            response.Result = ErrorCode.ReceiptCheckFail;
        }

        return response;
    }


    void Init()
    {
        _receiptList.Add("QKsM2oPCeUiLWlwpTqXN5fIr4w0I7gMEB73573W8hGIn0WU9bQjNnxA7nQxhcvMP");
        _receiptList.Add("WkuOATWDQ909OET9cBjVEXEgI3KqTTbThNFe206bywlkSBiUD1hgrCltj3g1a84d");
        _receiptList.Add("1OjwHKctOp29VE1KLK75BbXYxKpTHufOOAwcCWo32xFAvthQdyX2UaOECIxIl802");
        _receiptList.Add("NuIDV687iXasG5wFuTQbtpiAHBVbAjwfpI5dMUHfw76PBipS7cheFc0SksK6R2Gn");
        _receiptList.Add("FSrhNe0gbFAKjjY7ZN6FPK1ImWHsQrsyMnyWqR14JntyLofloticXJ7oFVUpSLBd");
        _receiptList.Add("KEUqnainhuHKThpCxXnT7vGr0tMQ6IEL3pu764kkGUNTtdhAxcT2umID0LmVNi7K");
        _receiptList.Add("Cmu2JCJsVndrsCpFMg520SAY9nrwVSSFIQor4FXJpaRk4VXpNtsNwa2Yc9gIbLuH");
        _receiptList.Add("7IJi2nTCMVZ5HbE3KF8cUJ73Uw7f84aQhMMatjf21ZxajeBMffQePfN89uROSB5n");
        _receiptList.Add("gXIoGpNMkWcO74FQPlSNvUhUqmGp7kXvh5bZHCJCDr7ePXfkyyJTrwFBGYO5qr8b");
        _receiptList.Add("wglqUxLmKZjr3L6xUwrOT0ijNKqFY9PwDA4f143vUKB6rP4Sd1iScPakuRP0LMF2");
        _receiptList.Add("AFwoseSvylxeyhvssAtLKRUBrq71wXgW5Lrp62IOPYon8U1sNJGC5iMuLpt8yWPZ");
        _receiptList.Add("XbRd1nN5ct0IKWNHiULeuxhnLOfhuqHs5vOvRCKVPh0cSQrZk9Dy8q6atVkY3Bg6");
        _receiptList.Add("lOkJrV8drexk1aAsLrphZijejTYruYGnjXe0G6r27iJiKhJxFTi1Hc3RYhQ5m7hq");
        _receiptList.Add("dk8IbTxH1Kgn9GFqA8VV0EqEDFUI2P5aXAEmf2nhMJsmNO86IgLSwVgNEGpXHxWg");
        _receiptList.Add("YOMGwAkbASNEgMibM63w4PTVKa43cHmv6rfdvHGaZjdU19f5iIE9y1EiXUyALO0R");
        _receiptList.Add("aepIhSInxFk68yvdk66cwfskjti6sBKTqPBHo6vdI5J664EpOVBYN4lwqk89n1YJ");
    }

    bool ReceiptCheck(string receipt)
    {
        return _receiptList.Contains(receipt);
    }
}

public class PkReceiptCheckReq
{
    public string Receipt { get; set; }
}


public class PkReceiptCheckRes
{
    public ErrorCode Result { get; set; } = ErrorCode.None;
}

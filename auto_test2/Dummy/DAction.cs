using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AutoTestClient.Dummy;

public class DAction
{
    public Func<Task<bool>> Connection;
    public Func<Task<bool>> Disconnection;
    public Func<Task<bool>, byte[]> Send;
}

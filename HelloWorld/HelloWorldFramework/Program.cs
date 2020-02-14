using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace HelloWorld
{
    class Program
    {
        static void Main(string[] args)
        {
            MyInjectionTarget();

            Console.WriteLine("Hello World!");
        }

        private static void MyInjectionTarget()
        {
            int x = 1;
            int y = 2;
            int z = x + y;
            Console.WriteLine(z);
        }
    }
}

using System;

namespace HelloWorld
{
    class Program
    {
        static void Main(string[] args)
        {
            MyInjectionTarget();

            Console.WriteLine("Hello World!");
            Console.WriteLine("Press Enter.");
            Console.ReadLine();
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

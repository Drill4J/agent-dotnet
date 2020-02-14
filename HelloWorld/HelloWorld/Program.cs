using System;

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
            int z;
            try
            {
                z = x + y;
                Console.WriteLine("Greetings from MyInjectionTarget");
            }
            catch (Exception)
            {
                z = 0;
            }
            finally
            {
                string s = "Greetings from the Profiler";
                // For Debug builds, will inject
                // Console.WriteLine(s);
                // Such injection is not possible with Release builds,
                // because the C# compiler optimizes s variable away.
            }

            if (z % 2 == 1)
            {
                Console.WriteLine(z);
                // Will inject here
                // if (x > 0)
                // {
                //     Console.WriteLine(42);
                // }
                // and 128 NOP operations
            }
        }
    }
}
